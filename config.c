#include "Utils.c"

typedef struct config_data {
	char **hostnames;
	int *ports;
	int vhosts_cnt;
} config_data;


int MAX_THREAD_CONN = 1000;
int MAX_CONN_QUEUE = 1000;
int THREADS_NUM = 4;
int MAX_EPOLL_EVENTS_IN_ITERATION = 100;
char *CONF_DIR;
char *DATA_DIR;
const char *VHOSTS_FILENAME = "vhosts.txt";
config_data CONF_DATA;


int validate_hostname(const char *hostname, int len) {
	// hostname should consist of letters, digits and dots
	for (int i = 0; i < len; ++i) {
		if (!isalnum(hostname[i]) && hostname[i] != '.') {
			return 0;
		}
	}
	return 1;
}

void parse_vhosts_data(const char *vhosts_data, int vhosts_data_len) {
	int cnt_strs = get_cnt_strs(vhosts_data, vhosts_data_len);

	CONF_DATA.hostnames = malloc(cnt_strs, sizeof(char*));
	CONF_DATA.ports = malloc(cnt_strs, sizeof(char*));

	int i = 0, vhost_num = 0;
	while (i < vhosts_data_len) {
		int j = i;
		int colon_pos = -1;
		while (j < vhosts_data_len && vhosts_data[j] != '\n') {
			if (vhosts_data[j] == ':') {
				if (colon_pos == -1) {
					colon_pos = j;
				}
				else {
					colon_pos = -1;
				}
			}
			++j;
		}

		if (i == j) {
			// empty line
			i = j + 1;
			continue;
		}

		if (colon_pos == -1) {
			CFAIL("Wrong vhosts.txt format: invalid number of colons in line");
		}
		char *hostname_str = substr(vhosts_data, i, colon_pos - i);
		char *port_str = substr(vhosts_data, colon_pos + 1, j - (colon_pos + 1));
		if (!is_natural(port_str, j - (colon_pos + 1))) {
			CFAIL("Wrong vhosts.txt format: bad port number");
		}
		CONF_DATA.ports[vhost_num] = strtol(port_str, NULL, 10); // no nullbytes already
		if (!validate_hostname(hostname_str, colon_pos - i)) {
			CFAIL("Wrong vhosts.txt format: bad hostname");
		}
		CONF_DATA.hostnames[vhost_num] = hostname_str; // no null bytes already

		i = j + 1;
		++vhost_num;
	}
	CONF_DATA.vhosts_cnt = vhost_num;
}


void init_conf(const char *conf_dir, const char *data_dir) {
	CONF_DIR = conf_dir;
	DATA_DIR = data_dir;
	
	char *vhosts_path = concat(CONF_DIR, VHOSTS_FILENAME);
	FILE *vhosts_file = fopen(vhosts_path, "r");
	if (vhosts_file == NULL) {
		FAIL("Opening vhosts.txt");
	}
	free(vhosts_path);

	int vhosts_data_len;
	char *vhosts_data = read_file(vhosts_file, &vhosts_data_len);
	parse_vhosts_data(vhosts_data, vhosts_data_len);
	free(vhosts_data);
}