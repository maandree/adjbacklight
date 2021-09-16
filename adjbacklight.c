/* See LICENSE file for copyright and license details. */
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <alloca.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <grp.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#include "arg.h"


#ifndef BACKLIGHT_DIR
# define BACKLIGHT_DIR "/sys/class/backlight"
#endif



char *argv0;
static double brightness = 0;
static int nbrightness = 0;
static int cols = 80;
static char *space;
static char *line;



static void
usage(void)
{
	fprintf(stderr, "usage: %s [-g | -s [+|-]level[%%|%%%%]] [-a | device ...]\n", argv0);
	exit(1);
}


static long int
readfile(const char *path)
{
	int fd;
	char buf[64];
	ssize_t i, n;
	long int value = 0;

	fd = open(path, O_RDONLY);
	if (fd < 0)
		return -1;

	while ((n = read(fd, buf, sizeof(buf))) > 0) {
		for (i = 0; i < n; i++) {
			if (isdigit(buf[i]))
				value = value * 10 + (buf[i] - '0');
			else if (buf[i] != '\n')
				goto out;
		}
	}

out:
	close(fd);
	return n ? -1 : value;
}


static double
getbrightness(const char *device, long int *out_cur, long int *out_max)
{
	size_t n = sizeof("//max_brightness") - 1;
	char *path;
	long int max, cur;

	n += strlen(BACKLIGHT_DIR);
	n += strlen(device);
	path = alloca(n + 1);

	if (sprintf(path, "%s/%s/max_brightness", BACKLIGHT_DIR, device) != (int)n)
		abort();
	max = readfile(path);
	if (max < 1)
		return -1;
	if (out_max)
		*out_max = max;

	if (sprintf(path, "%s/%s/brightness", BACKLIGHT_DIR, device) != (int)n - 4)
		abort();
	cur = readfile(path);
	if (cur < 0)
		return -1;
	if (out_cur)
		*out_cur = cur;

	return (double)cur / (double)max;
}


static int
setbrightness(const char *device, long int value)
{
	size_t p = 0, n = sizeof("//brightness") - 1;
	char *path, buf[128];
	ssize_t r = 0;
	int fd;

	n += strlen(BACKLIGHT_DIR);
	n += strlen(device);
	path = alloca(n + 1);

	if (sprintf(path, "%s/%s/brightness", BACKLIGHT_DIR, device) != (int)n)
		abort();
	if (sprintf(buf, "%li\n", value) < 0)
		abort();

	fd = open(path, O_WRONLY);
	if (fd < 0)
		return -1;

	for (n = strlen(buf); p < n; p += (size_t)r)
		if ((r = write(fd, &buf[p], n - p)) < 0)
			break;

	close(fd);
	return r < 0 ? -1 : 0;
}


static int
adjbrightness(const char *device, double pcur, long int cur, long int max, double adj, int inc, const char *suf)
{
	switch (strlen(suf)) {
	case 2:
		pcur = adj * pcur / 100 + pcur * inc;
		cur = (long int)(pcur * (double)max + (double)0.5f);
		break;
	case 1:
		pcur = adj / 100 + pcur * inc;
		cur = (long int)(pcur * (double)max + (double)0.5f);
		break;
	case 0:
		cur = (long int)adj + cur * inc;
		break;
	default:
		abort();
	}

	cur = cur < 0 ? 0 : cur < max ? cur : max;
	return setbrightness(device, cur);
}


static void
bars(long int max, long int init, long int cur)
{
	long int mid = (long int)((double)cur * (double)(cols - 2) / (double)max + (double)0.5f);

	printf("\033[%iD\033[6A", cols);
	printf("\033[2K┌%s┐\n", line);
	space[mid] = '\0';
	printf("\033[2K│\033[47m%s\033[49m%s│\n", space, &space[mid + 1]);
	space[mid] = ' ';
	printf("\033[2K└%s┘\n", line);
	printf("\033[2KMaximum brightness: %li\n", max);
	printf("\033[2KInitial brightness: %li\n", init);
	printf("\033[2KCurrent brightness: %li\n", cur);

	fflush(stdout);
}


static void
interactive(const char *device, long int cur, long int max)
{
	long int step, init;
	int c;

	step = max / 200;
	step = step ? step : 1;
	init = cur;

	printf("\n\n\n\n\n\n");
	bars(max, init, cur);
	while ((c = getchar()) != -1) {
		switch (c) {
		case 'q':
		case '\n':
		case 4:
			printf("\n");
			return;
		case 'A':
		case 'C':
			cur += step << 1;
			/* fall through */
		case 'B':
		case 'D':
			cur -= step;
			cur = cur < 0 ? 0 : cur < max ? cur : max;
			adjbrightness(device, 0, 0, max, (double)cur, 0, "");
			bars(max, init, cur);
		}
	}
}


static void
handle_device(const char *device, int get, int set, double adj, int inc, const char *suf)
{
	double value;
	long int cur, max;

	value = getbrightness(device, &cur, &max);

	if (get) {
		if (value >= 0) {
			brightness += value;
			nbrightness++;
		}
	} else if (set) {
		adjbrightness(device, value, cur, max, adj, inc, suf);
	} else {
		interactive(device, cur, max);
	}
}


static int
parse_set_argument(const char *str, char *set_prefix, double *set_value, const char **set_suffix)
{
	*set_prefix = '=';

	if (*str == '+' || *str == '-') {
		*set_prefix = *str;
		str++;
	}
	if (!isdigit(*str) && *str != '.')
		return -1;
	errno = 0;
	*set_value = strtod(str, (void *)set_suffix);
	if (errno || *set_value < 0) {
		fprintf(stderr, "%s: strtod %s: %s\n", argv0, str, strerror(errno));
		exit(1);
	}
	if (**set_suffix && strcmp(*set_suffix, "%") && strcmp(*set_suffix, "%%"))
		usage();
	if (*set_prefix == '-') {
		*set_value = -*set_value;
		*set_prefix = '+';
	}
	return 0;
}


static int
init_terminal(pid_t *pid, struct termios *saved_stty)
{
	struct termios stty;
	struct winsize win;
	int i;

	printf("\n\n");
	printf("If the program is abnormally aborted the may be some residual\n");
	printf("effects on the terminal. the following commands should reset it:\n");
	printf("\n");
	printf("    stty sane\n");
	printf("    printf '\\ec'\n");
	printf("\n");
	printf("\n\n\n\n");

	/* Get the size of the terminal */
	if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &win) == -1)
		fprintf(stderr, "%s: ioctl <stdout> TIOCGWINSZ: %s\n", argv0, strerror(errno));
	else
		cols = win.ws_col;

	line = malloc((size_t)cols * 3);
	space = malloc((size_t)cols);
	if (!line || !space) {
		fprintf(stderr, "%s: malloc: %s\n", argv0, strerror(ENOMEM));
		exit(1);
	}
	for (i = 0; i < cols; i++) {
		line[i * 3 + 0] = (char)(0xE2);
		line[i * 3 + 1] = (char)(0x94);
		line[i * 3 + 2] = (char)(0x80);
	}
	memset(space, ' ', (size_t)cols);
	space[cols - 1] = '\0';
	line[(cols - 2) * 3] = '\0';

	/* stty -icanon -echo */
	if (tcgetattr(STDIN_FILENO, &stty)) {
		fprintf(stderr, "%s: tcgetattr <stdin>: %s\n", argv0, strerror(errno));
		exit(1);
	}
	*saved_stty = stty;
	stty.c_lflag &= (tcflag_t)~(ICANON | ECHO);
	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &stty)) {
		fprintf(stderr, "%s: tcsetattr <stdin>: %s\n", argv0, strerror(errno));
		exit(1);
	}

	/* Hide cursor */
	printf("%s", "\033[?25l");
	fflush(stdout);

	/* Fork to diminish risk of unclean exit */
	*pid = fork();
	if (*pid == (pid_t)-1) {
		fprintf(stderr, "%s: fork: %s\n", argv0, strerror(errno));
	} else if (*pid) {
		waitpid(*pid, NULL, 0);
		return -1;
	}

	return 0;
}


static int
update(int argc, char *argv[], int all, int get, char *set, int set_prefix, double set_value, const char *set_suffix)
{
	DIR *dir;
	struct dirent *ent;
	int any = argc;

	if (argc) {
		for (; *argv; argv++) {
			if (strchr(*argv, '/'))
				*argv = strrchr(*argv, '/');
			handle_device(*argv, get, !!set, set_value, set_prefix == '+', set_suffix);
		}
	} else {
		if ((dir = opendir(BACKLIGHT_DIR))) {
			while ((ent = readdir(dir))) {
				if (*ent->d_name && *ent->d_name != '.') {
					handle_device(ent->d_name, get, !!set, set_value, set_prefix == '+', set_suffix);
					any = 1;
					if (!all)
						break;
				}
			}
			closedir(dir);
		}
		if (!any)
			fprintf(stderr, "%s: cannot find any backlight devices\n", argv0);
	}

	return any;
}


int
main(int argc, char *argv[])
{
	int all = 0;
	int get = 0;
	char *set = NULL;
	char set_prefix = '=';
	double set_value = 0;
	const char *set_suffix = NULL;
	pid_t pid = -1;
	struct termios saved_stty;
	int isinteractive = 0;
	size_t size = 0;
	ssize_t len;

	ARGBEGIN {
	case 'a':
		all = 1;
		break;
	case 'g':
		get = 1;
		break;
	case 's':
		set = EARGF(usage());
		break;
	default:
		usage();
	} ARGEND;

	if ((get && set) || (argc && all))
		usage();

	/* Parse -s argument */
	if (set && parse_set_argument(set, &set_prefix, &set_value, &set_suffix))
		usage();

	if (!get && !set) {
		isinteractive = isatty(STDIN_FILENO);
		if (isinteractive && init_terminal(&pid, &saved_stty))
			goto done;
	}

	if (get || set || isinteractive) {
		update(argc, argv, all, get, set, set_prefix, set_value, set_suffix);
	} else {
		while ((len = getline(&set, &size, stdin)) > 0) {
			if (len && set[len - 1] == '\n')
				set[--len] = '\0';
			if (!len)
				continue;
			if (parse_set_argument(set, &set_prefix, &set_value, &set_suffix)) {
				fprintf(stderr, "%s: invalid input: %s\n", argv0, set);
				return 0;
			}
			update(argc, argv, all, get, set, set_prefix, set_value, set_suffix);
		}
		if (ferror(stdin)) {
			fprintf(stderr, "%s: getline <stdin>: %s\n", argv0, strerror(errno));
			return 1;
		}
		free(set);
	}

	if (get) {
		if (nbrightness) {
			brightness *= 100;
			brightness /= (double)nbrightness;
			printf("%.2lf%%\n", brightness);
			fflush(stdout);
		} else {
			printf("%s\n", "100.00%");
			fflush(stdout);
		}
	}

done:
      if (isinteractive && pid) {
	      /* Show cursor */
	      printf("%s", "\033[?25h");
	      fflush(stdout);

	      /* `stty icanon echo` */
	      if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &saved_stty)) {
		      fprintf(stderr, "%s: tcsetattr <stdin>: %s\n", argv0, strerror(errno));
		      return 1;
	      }
      }

      return 0;
}
