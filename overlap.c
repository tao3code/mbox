#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define FTMP	"./rise-tmp.txt"

static char tmplt[] = ">: --+-+-+-+-+----";

static size_t can_play(const char *arg)
{
	size_t len = 0;

	len = strlen(arg);

	if (len < 3)
		return 0;

	if (strncmp(arg, tmplt, 3))
		return 0;

	return len;
}

static int line_count = 0;

static int rise_lbuf(const char *in1, char *in2)
{
	int i;

	size_t len1, len2;

	len1 = can_play(in1);
	len2 = can_play(in2);

	if (!(len1 || len2))
		return len2;

	if (!(len1 && len2)) {
		fprintf(stderr, "error: line %d attribute, (%s)->(%s)\n",
			line_count, in1, in2);
		return -1;
	}

	for (i = 0; i < sizeof(tmplt); i++) {
		if (in1[i] == 'X' || in1[i] == 'V') {
			if (in2[i] == 'X' || in2[i] == 'V') {
				fprintf(stderr,
					"Can't overlap, has same key!!!\n");
				return -1;
			}

			if (in2[i] != tmplt[i]) {
				fprintf(stderr,
					"The line can't play, !!!\n");
				return -1;
			}
			in2[i] = in1[i];
		}
	}

	return len2;
}

int main(int argc, char *argv[])
{
	FILE *fd_in1 = NULL;
	FILE *fd_in2 = NULL;
	FILE *fd_out = NULL;
	char lbuf1[128];
	char lbuf2[128];
	int ret = 0;

	if (argc != 3)
		return 0;

	fd_in1 = fopen(argv[1], "r");
	if (!fd_in1) {
		perror(argv[1]);
		goto open_input1;
	}

	fd_in2 = fopen(argv[2], "r");
	if (!fd_in2) {
		perror(argv[2]);
		goto open_input2;
	}

	fd_out = fopen(FTMP, "w+");
	if (!fd_out) {
		perror(FTMP);
		goto open_output;
	}

	while (!(feof(fd_in1) || feof(fd_in2))) {
		memset(lbuf1, 0, sizeof(lbuf1));
		memset(lbuf2, 0, sizeof(lbuf2));

		fgets(lbuf1, sizeof(lbuf1), fd_in1);
		fgets(lbuf2, sizeof(lbuf2), fd_in2);
		line_count++;

		ret = rise_lbuf(lbuf1, lbuf2);
		if (ret < 0) {
			fprintf(stderr, "error: %s, %s at line %d\n",
				lbuf1, lbuf2, line_count);
			break;
		}

		fprintf(fd_out, "%s", lbuf2);
	}

	fclose(fd_out);
 open_output:
	fclose(fd_in2);
 open_input2:
	fclose(fd_in1);
 open_input1:

	if (!ret)
		rename(FTMP, argv[2]);
	else
		unlink(FTMP);
	return 0;
}
