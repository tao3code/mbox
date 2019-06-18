#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define FTMP	"./rise-tmp.txt"

static char tmplt[] = ">: --+-+-+-+-+----";

static int rise_lbuf(const char *in, char *out, size_t len)
{
	int i;

	memcpy(out, in, len);

	for (i = 1; i < len; i++) {
		if (in[i] == 'X') {
			if (i <= 3) {
				fprintf(stderr,
					"Can't desend, has lowest key!!!\n");
				return -1;
			}
			out[i] = tmplt[i];
			out[i - 1] = 'X';
		}
	}

	return 0;
}

int main(int argc, char *argv[])
{
	FILE *fd_in = NULL;
	FILE *fd_out = NULL;
	char lbuf[128];
	char buf[128];
	unsigned lcount = 0;
	int ret = 0;
	size_t len;

	if (argc != 2)
		return 0;

	fd_in = fopen(argv[1], "r");
	if (!fd_in) {
		perror(argv[1]);
		goto open_input;
	}

	fd_out = fopen(FTMP, "w+");
	if (!fd_out) {
		perror(FTMP);
		goto open_output;
	}

	while (!feof(fd_in)) {
		memset(lbuf, 0, sizeof(lbuf));
		memset(buf, 0, sizeof(buf));

		fgets(lbuf, sizeof(lbuf), fd_in);
		lcount++;

		len = strlen(lbuf);

		ret = rise_lbuf(lbuf, buf, len);
		if (ret) {
			fprintf(stderr, "error: %s at line %d\n", lbuf, lcount);
			break;
		}

		fwrite(buf, len, 1, fd_out);
	}

	fclose(fd_out);
 open_output:
	fclose(fd_in);
 open_input:

	if (!ret)
		rename(FTMP, argv[1]);
	else
		unlink(FTMP);
	return 0;
}
