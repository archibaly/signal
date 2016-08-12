#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>

#define VERSION	"1.0"
#define RELEASE	"2016/08/12"

static pid_t child_pid;

static void sighup_handler(int signo)
{
	if (signo == SIGHUP)
		kill(child_pid, SIGTERM);
}

static void sigterm_handler(int signo)
{
	if (signo == SIGTERM)
		_exit(0);
}

int handle_reload(void)
{
	FILE *fp;
	pid_t pid;

	if (!(fp = fopen("pid", "r"))) {
		fprintf(stderr, "process is not running\n");
		return -1;
	}

	if (fread(&pid, sizeof(pid), 1, fp) < 0) {
		perror("fread");
		fclose(fp);
		return -1;
	}

	fclose(fp);

	if (kill(pid, SIGHUP) < 0) {
		fprintf(stderr, "process is not running\n");
		return -1;
	}
}

static void usage(void)
{
    fprintf(stdout, "Usage: a.out [options]\n");
    fprintf(stdout, "\n");
    fprintf(stdout, "options:\n");
    fprintf(stdout, "  -c [filename] Use this config file\n");
    fprintf(stdout, "  -s [reload]   Reload config file\n");
    fprintf(stdout, "  -h            Print usage\n");
    fprintf(stdout, "  -v            Print version information\n");
    fprintf(stdout, "  -r            Print release date\n");
    fprintf(stdout, "\n");
}

static void handle_cmd_line(int argc, char **argv)
{
	int oc;

	while ((oc = getopt(argc, argv, ":c:s:vr")) != -1) {
		switch (oc) {
		case 'c':

			break;
		case 's':
			if (strcmp("reload", optarg) == 0) {
				exit(handle_reload());
			}
			exit(0);
		case 'h':
			usage();
			exit(0);
		case 'v':
			fprintf(stdout, "%s\n", VERSION);
			exit(0);
		case 'r':
			fprintf(stdout, "%s\n", RELEASE);
			exit(0);
		case '?':
		case ':':
			usage();
			exit(-1);
		default:
			break;
		}
	}
}

int main(int argc, char **argv)
{
	handle_cmd_line(argc, argv);

	FILE *fp;

	if (!(fp = fopen("pid", "w")))
		exit(-1);

	pid_t pid = getpid();
	if (fwrite(&pid, sizeof(pid_t), 1, fp) < 0)
		exit(-1);

	fclose(fp);

	for (;;) {
		pid = fork();
		if (pid < 0) {
			exit(-1);
		} else if (pid == 0) {
			signal(SIGTERM, sigterm_handler);
			/* read_config(); */
			for (;;) {
				printf("child\n");
				sleep(3);
			}
		} else {
			child_pid = pid;
			signal(SIGHUP, sighup_handler);
			wait(NULL);
			printf("wait returned\n");
		}
	}

	return 0;
}
