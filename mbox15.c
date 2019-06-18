#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define SAMPLE_RATE	11025

static unsigned data_size = 0;

struct wave_struct {
	char id_ref[4];
	unsigned size;
	char id_wave[4];

	char id_fmt[4];
	unsigned fmt_size;
	unsigned short audio_fmt;
	unsigned short channel;
	unsigned sample_rate;
	unsigned byte_rate;
	unsigned short fmt_data_size;
	unsigned short bit_width;

	char id_data[4];
	unsigned data_size;
	char data[0];
} head = {
	.id_ref = {
	'R', 'I', 'F', 'F'},.id_wave = {
	'W', 'A', 'V', 'E'},.id_fmt = {
	'f', 'm', 't', ' '},.fmt_size = 0x10,.audio_fmt = 1,.channel =
	    1,.sample_rate = SAMPLE_RATE,.byte_rate =
	    SAMPLE_RATE,.fmt_data_size = 1,.bit_width = 8,.id_data = {
'd', 'a', 't', 'a'},};

struct key_struct {
	char *name;
	float hz;
} keys[] = {
	{
	.name = "c1",.hz = 261.626f}, {
	.name = "d1",.hz = 293.665f}, {
	.name = "e1",.hz = 329.628f}, {
	.name = "f1",.hz = 349.228f}, {
	.name = "g1",.hz = 391.995f}, {
	.name = "a2",.hz = 440.000f}, {
	.name = "b2",.hz = 493.883f}, {
	.name = "c2",.hz = 532.251f}, {
	.name = "d2",.hz = 587.330f}, {
	.name = "e2",.hz = 659.255f}, {
	.name = "f2",.hz = 698.456f}, {
	.name = "g2",.hz = 783.991f}, {
	.name = "a3",.hz = 880.000f}, {
	.name = "b3",.hz = 987.767f}, {
.name = "c3",.hz = 1046.502f},};

#define NKEYS	(sizeof(keys)/sizeof(struct key_struct))

float amplif[NKEYS];
unsigned live[NKEYS];
char hold[NKEYS];
int line_count = 0;

static int update_key_amplif(char *arg)
{
	int i;
	int find = 0;

	if (strlen(arg) < NKEYS) {
		printf("error: %s\n", arg);
		return -1;
	}

	for (i = 0; i < NKEYS; i++)
		if (arg[i] == 'X') {
			if (hold[i]) {
				hold[i]--;
				printf("waring: ingnore %d at %s:%d\n",
					i, arg, line_count);
				continue;
			}
			amplif[i] = 1.0f;
			live[i] = 0;
			hold[i] = 1;
			printf("%s ", keys[i].name);
			find++;
		} else {
			if (hold[i])
				hold[i]--;
		}

	line_count++;

	if (find)
		printf(" :%d\n", line_count);

	return 0;
}

int main(int argc, char *argv[])
{
	int i, j, k, err;
	char *buf;
	int fd;
	float A, a, t;
	FILE *fd_in = NULL;
	char lbuf[128];
	char path[128] = {};
	int len, find = 0;

	if (argc != 2)
		return 0;

	buf = malloc(SAMPLE_RATE >> 2);
	if (!buf) {
		perror("malloc:");
		goto alloc_buf;
	}

	fd_in = fopen(argv[1], "r");
	if (!fd_in) {
		perror(argv[1]);
		goto open_input;
	}

	strcpy(path, argv[1]);
	for (i = 0; i < sizeof(path); i++) {
		if (!strcmp(&path[i], ".txt")) {
			memcpy(&path[i], ".wav", 5);
			find = 1;
			break;
		}
	}

	if (!find) {
		printf("find no *.txt\n");
		goto open_input;
	}

	fd = open(path, O_CREAT | O_RDWR | O_TRUNC, 0666);
	if (fd < 0) {
		perror("open:");
		goto open_fd;
	}
	lseek(fd, sizeof(struct wave_struct), SEEK_SET);

	while (!feof(fd_in)) {
		memset(lbuf, 0, sizeof(lbuf));
		fgets(lbuf, sizeof(lbuf), fd_in);

		len = strlen(lbuf);

		if (len < 3)
			continue;

		if (strncmp(lbuf, ">: ", 3))
			continue;

		err = update_key_amplif(&lbuf[3]);
		if (err)
			break;

		for (j = 0; j < (SAMPLE_RATE >> 2); j++) {
			t = (float)(j + data_size) / (float)SAMPLE_RATE;
			a = 0.0f;
			for (k = 0; k < NKEYS; k++) {
				if (amplif[k] < 0.1f) {
					amplif[k] = 0.0f;
					live[k] = 0;
					continue;
				}

				a += amplif[k] + amplif[k] *
				    sin(keys[k].hz * t * 2 * 3.1415926);
				amplif[k] = pow(0.5f, 
					(float)live[k]*4.0f/ SAMPLE_RATE);
				live[k]++;
			}
			A = a * 36.0f;
			buf[j] = (unsigned char)A;
		}
		data_size += write(fd, buf, SAMPLE_RATE >> 2);
	}

	head.size = data_size + sizeof(struct wave_struct) - 8;
	head.data_size = data_size;
	lseek(fd, 0, SEEK_SET);
	write(fd, &head, sizeof(struct wave_struct));
	close(fd);
 open_fd:
	fclose(fd_in);
 open_input:
	free(buf);
 alloc_buf:

	return 0;
}
