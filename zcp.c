#include <stdio.h>
#include <string.h>
#include <unistd.h>

static char tmplt[] = ">: --+-+-+-+-+----";

static int rise_lbuf(const char *in, char *out, size_t len)
{
	int i;

	memcpy(out, in, len);

	for (i = 0; i < len; i++) {
		if (in[i] == 'X' || in[i] == 'V') {
			out[i] = tmplt[i];
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

	if (argc != 3)
		return 0;

	fd_in = fopen(argv[1], "r");
	if (!fd_in) {
		perror(argv[1]);
		goto open_input;
	}

	fd_out = fopen(argv[2], "w+");
	if (!fd_out) {
		perror(argv[2]);
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

	if (ret)
		unlink(argv[2]);

	return 0;
}
