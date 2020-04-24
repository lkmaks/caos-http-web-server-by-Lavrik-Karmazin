#include <fcntl.h>
#include <string.h>

void modify_nonblock(int fd) {
	int flags = fcntl(fd, F_GETFL);
	flags |= O_NONBLOCK;
	fcntl(fd, F_SETFL, flags);
}


char *concat(const char *s, const char *t) {
	int n = strlen(s), m = strlen(t);
	char *res = malloc(n + m + 1);
	strcpy(res, s);
	strcat(res, t);
	return res;
}

void FAIL(const char *cause) {
	fprintf(stderr, "Failure detected. Printing cause and explanation.\n");
	perror(cause);
	exit(1);
}

void CFAIL(const char *cause) {
	// custom fail
	fprintf(stderr, "%s\n", cause);
	exit(1);
}

int filesize(FILE *file) {
	fseek(file, 0, SEEK_END); 
	int size = ftell(file);
	fseek(file, 0, SEEK_SET); 
	return size;
}

char *read_file(FILE *file, int *len) {
	*len = filesize(file);
	char *res = malloc(len + 1);
	fread(res, 1, *len, file);
	return res;
}

char *substr(char *str, int pos, int len) {
	char *res = malloc(len + 1);
	for (int i = 0; i < len; ++i) {
		res[i] = str[pos + i];
	}
	res[len] = 0;
}

int get_cnt_strs(const char *data, int len) {
	int res = 1;
	for (int i = 0; i < len; ++i) {
		if (data[i] == '\n') {
			++res;
		}
	}
	return res;
}