#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define SAMPLE_RATE	44100	
#define BUF_SIZE	4096
#define READ_SPAN	(SAMPLE_RATE >>2)
#define RELOAD_SPAN	(SAMPLE_RATE >>1)

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
	unsigned busy;
	unsigned timer;
	char trigger;
	float amplif;
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

static unsigned tick = 0;

static char sample(void)
{
	float a, t, amplif, hz;
	int i;

	t = (float)tick / (float)SAMPLE_RATE;
	a = 0.0f;

	for (i = 0; i < NKEYS; i++) {
		amplif = keys[i].amplif;
		if (amplif < 0.1f) {
			continue;
		}

		hz = keys[i].hz;

		a += amplif + amplif * sin(hz * t * 2 * 3.1415926);
	}
	a = a * 8.0f;
	return (unsigned char)a;
}

static int line_count = 0;

static int trigger_key(int key)
{
	if (!keys[key].trigger)
		return 0;

	if (keys[key].timer) {
		keys[key].timer--;
		return 0;
	}

	if (keys[key].busy) {
		printf("Too tight! Key busy at line:%d\n", line_count);
		return -1;
	}

	keys[key].trigger = 0;
	keys[key].amplif = 1.0f;
	keys[key].busy = RELOAD_SPAN - 1;

	return 0;
}

static int play_keys(void)
{
	int i, err;

	for (i = 0; i < NKEYS; i++) {
		err = trigger_key(i);
		if (err) {
			printf("Can't play key %d\n", i);
			return -1;
		}
	}

	return 0;
}

static void damp_with_tick(void)
{
	int i;

	for (i = 0; i < NKEYS; i++) {
		if (keys[i].busy)
			keys[i].busy--;

		if (keys[i].amplif < 0.1f) {
			keys[i].amplif = 0.0f;
			continue;
		}
		keys[i].amplif = keys[i].amplif * 
			pow(0.5f, 4.0f / (float)SAMPLE_RATE);
	}
}

static int read_list(FILE * fp)
{
	char lbuf[128];
	int len = 0, find = 0, i;

	while (!find) {
		if (feof(fp))
			return -1;

		memset(lbuf, 0, sizeof(lbuf));
		fgets(lbuf, sizeof(lbuf), fp);

		len = strlen(lbuf);
		line_count++;

		if (len < 3)
			continue;

		if (strncmp(lbuf, ">: ", 3))
			continue;

		find = 1;
	}

	if (!find)
		return -1;

	for (i = 0; i < NKEYS; i++) {
		switch (lbuf[i + 3]) {
		case 'X':
			keys[i].trigger = 1;
			break;
		case 'V':
			keys[i].trigger = 1;
			keys[i].timer = READ_SPAN >> 1;
			break;
		}
	}
	return 0;
}

int main(int argc, char *argv[])
{
	int err, i, fd;
	int len = 0, find = 0, data_size = 0;
	char *buf;
	FILE *fd_in = NULL;
	char path[128] = { };
	unsigned next_read = 0;

	if (argc != 2)
		return 0;

	buf = malloc(BUF_SIZE);
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

	while (1) {
		if (tick >= next_read) {
			err = read_list(fd_in);
			next_read += READ_SPAN;
		}

		if (err)
			break;

		err = play_keys();
		if (err)
			break;

		buf[len] = sample();
		damp_with_tick();

		len++;
		if (len >= BUF_SIZE - 1) {
			data_size += write(fd, buf, len);
			len = 0;
		}

		tick++;
	}

	if (len)
		data_size += write(fd, buf, len);

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
