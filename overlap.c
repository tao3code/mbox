#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define FTMP	"./rise-tmp.txt"

static char tmplt[] = ">: --+-+-+-+-+----";

static int rise_lbuf(const char *in1, const char *in2, char *out, size_t len)
{
	int i;

	memcpy(out, in2, len);

	for (i = 0; i < len; i++) {
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
			out[i] = in1[i];
		}
	}

	return 0;
}

int main(int argc, char *argv[])
{
	FILE *fd_in1 = NULL;
	FILE *fd_in2 = NULL;
	FILE *fd_out = NULL;
	char lbuf1[128];
	char lbuf2[128];
	char buf[128];
	unsigned lcount = 0;
	int ret = 0;
	size_t len1, len2;

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
		memset(buf, 0, sizeof(buf));

		fgets(lbuf1, sizeof(lbuf1), fd_in1);
		fgets(lbuf2, sizeof(lbuf2), fd_in2);
		lcount++;

		len1 = strlen(lbuf1);
		len2 = strlen(lbuf2);

		if (len1 != len2) {
			fprintf(stderr, "diff: %s, %s at line %d\n", lbuf1, lbuf2, lcount);
			ret = -1;
			break;
		}

		ret = rise_lbuf(lbuf1, lbuf2, buf, len1);
		if (ret) {
			fprintf(stderr, "error: %s, %s at line %d\n", lbuf1, lbuf2, lcount);
			break;
		}

		fwrite(buf, len1, 1, fd_out);
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
