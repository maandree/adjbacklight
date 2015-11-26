/**
 * adjbacklight – Convenient method for adjusting the backlight on your portable computer
 * 
 * Copyright © 2012, 2013, 2014, 2015  Mattias Andrée (maandree@member.fsf.org)
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <stdlib.h>
#include <stdio.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <limits.h>
#include <alloca.h>
#include <grp.h>
#include <pwd.h>



#ifndef PATH_MAX
#warning PATH_MAX is not defined
#define PATH_MAX 4096
#endif



/**
 * The years this software has been developed
 */
#define _YEARS_  "2012, 2013, 2014"


/**
 * Forking method
 */
#ifndef FORK
#define FORK  fork
#endif


/**
 * The directory where the backlight devices are found and configured
 */
#ifndef BACKLIGHT_DIR
#define BACKLIGHT_DIR  "/sys/class/backlight"
#endif


/**
 * Print a line to stdout
 * 
 * @param  S:const char*  The string to print
 */
#define P(S)  printf("%s", S "\n")


/**
 * Cut a string of at the first line break
 * A help `int` named `i` must be available
 * 
 * @param   data:char*  The string, it will be modified
 * @return  :char*      The input string, it has been modified
 */
#define unnl(data)			\
  ({					\
    for (i = 0; *(data + i); i++)	\
      if (*(data + i) == '\n')		\
	*(data + i) = 0;		\
    data;				\
  })



/**
 * Test whether a string numerical and with at most two trailing '%'
 * 
 * @param   str  The string
 * @return       Whether the test passed
 */
static int isnumerical(const char* str);

/**
 * Interactively adjust the backlight on a device
 * 
 * @param  cols    The number of columns on the terminal
 * @param  device  The device on which to adjust backlight
 */
static void adjust(int cols, const char* device);

/**
 * Gets the current backlight setting on a device
 * 
 * @param   device  The device from which to get backlight
 * @return          The brightness as a [0; 1] float, negative on error
 */
static float getbrightness(const char* device);

/**
 * Sets the current backlight setting on a device
 * 
 * @param  device      The device from which to get backlight
 * @param  adjustment  The adjustment to make
 */
static void setbrightness(const char* device, const char* adjustment);

/**
 * Read a file
 * 
 * @param   output  Buffer to store the file's content in
 * @param   file    The file to read
 * @return          Zero on success, non-zero on failure
 */
static int readfile(char* output, const char* file);

/**
 * Write an integer to a file
 * 
 * @param   buffer   Buffer for temporary data
 * @param   integer  The integer to write
 * @param   file     The file to which to write
 * @return           Zero on success, non-zero on failure
 */
static int writefile(char* buffer, int integer, const char* file);

/**
 * Print the status
 * 
 * @param  min   The minimum possible brightness
 * @param  max   The maximum possible brightness
 * @param  init  The brightness used when the program started
 * @param  cur   The current brightness
 * @param  cols  The number of columns on the terminal
 */
static void bars(int min, int max, int init, int cur, int cols);



/**
 * A series of spaces used to print the bar
 */
static char* space;

/**
 * A series of vertical lines used to print the bar
 */
static char* line;



/**
 * This is the mane entry point of the program
 * 
 * @param   argc  The number of elements in `argv`
 * @parma   argv  Command line arguments, including the command
 * @return        Exit value, zero on success
 */
int main(int argc, char** argv)
{
  struct winsize win;
  struct termios saved_stty;
  struct termios stty;
  size_t i, ndevices = 0;
  pid_t pid = 0;
  char* set = NULL;
  int j, get = 0, help = 0, all = 0, cols = 80;
  char** devices = alloca((size_t)argc * sizeof(char*));
  
  
  if (argc > 1)
    {
      char* arg;
      for (i = 1; i < (size_t)argc; i++)
	{
	  #define T(S)  (!strcmp(arg, S))
	  arg = *(argv + i);
	  if (T("-h") || T("--help"))
	    help = 1;
	  else if (T("-a") || T("--all"))
	    all = 1;
	  else if (T("-c") || T("--copyright") || T("--copying"))
	    {
	      P("\n");
	      P("adjbacklight – Convenient method for adjusting the backlight on your portable computer");
	      P("");
	      P("Copyright © " _YEARS_ "  Mattias Andrée (maandree@member.fsf.org)");
	      P("");
	      P("This program is free software: you can redistribute it and/or modify");
	      P("it under the terms of the GNU General Public License as published by");
	      P("the Free Software Foundation, either version 3 of the License, or");
	      P("(at your option) any later version.");
	      P("");
	      P("This program is distributed in the hope that it will be useful,");
	      P("but WITHOUT ANY WARRANTY; without even the implied warranty of");
	      P("MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the");
	      P("GNU General Public License for more details.");
	      P("");
	      P("You should have received a copy of the GNU General Public License");
	      P("along with this program.  If not, see <http://www.gnu.org/licenses/>.");
	      P("\n");
	      return 0;
	    }
	  else if (T("-w") || T("--warranty"))
	    {
	      P("\n");
	      P("This program is distributed in the hope that it will be useful,");
	      P("but WITHOUT ANY WARRANTY; without even the implied warranty of");
	      P("MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the");
	      P("GNU General Public License for more details.");
	      P("\n");
	      return 0;
	    }
	  else if (T("-s") || T("--set"))
	    {
	      char* tmp;
	      if (i + 1 == (size_t)argc)
		fprintf(stderr, "%s: argument for option %s is missing, ignoring option\n", *argv, arg);
	      else
		if (!isnumerical(tmp = *(argv + ++i)))
		  fprintf(stderr, "%s: argument for option %s is malformated, ignoring option\n", *argv, arg);
		else
		  {
		    set = tmp;
		    get = 0;
		  }
	    }
	  else if (T("-g") || T("--get"))
	    {
	      get = 1;
	      set = NULL;
	    }
	  else if (((*arg == '-') || (*arg == '+') || (*arg == '=')) && isnumerical(arg + 1))
	    {
	      set = arg;
	      get = 0;
	    }
	  else
	    {
	      if (*arg && (*arg != '-'))
		*(devices + ndevices++) = arg;
	      else
		fprintf(stderr, "%s: ignoring unrecognised argument: %s\n", *argv, arg);
	    }
	  #undef T
	}
      fflush(stderr);
      if (help || (((size_t)all + ndevices + (size_t)get == 0) && !set))
	{
	  P("\n");
	  P("adjbacklight - Convenient method for adjusting the backlight on your portable computer");
	  P("");
	  P("USAGE: adjbacklight (-c | -w | [-g | -s LEVEL | LEVEL] [-a | DEVICE...])");
	  P("");
	  P("Run with options to adjust the backlight on your monitors.");
	  P("");
	  P("");
	  P("OPTIONS:");
	  P("");
	  P("-c");
	  P("--copyright");
	  P("--copying       Display copyright information");
	  P("");
	  P("-w");
	  P("--warranty      Display warranty disclaimer");
	  P("");
	  P("-a");
	  P("--all           Run for all devices, including ACPI devices");
	  P("");
	  P("-g");
	  P("--get           Get average brightness on devices");
	  P("");
	  P("-s");
	  P("--set LEVEL[%]  Set brightness on devices");
	  P("");
	  P("+LEVEL          Increase brightness on devices by actual value");
	  P("-LEVEL          Decrease brightness on devices by actual value");
	  P("=LEVEL          Set brightness on devices by actual value");
	  P("");
	  P("+LEVEL%         Increase brightness on devices by percentage-points");
	  P("-LEVEL%         Decrease brightness on devices by percentage-points");
	  P("=LEVEL%         Set brightness on devices by percentage-points");
	  P("");
	  P("+LEVEL%%        Increase brightness on devices by percentage");
	  P("-LEVEL%%        Decrease brightness on devices by percentage");
	  P("=LEVEL%%        Set brightness on devices by percentage");
	  P("");
	  P("");
	  P("KEYBOARD:");
	  P("");
	  P("←");
	  P("↓               Darken the screen");
	  P("");
	  P("→");
	  P("↑               Brighten the screen");
	  P("");
	  P("q");
	  P("enter");
	  P("C-d             Continue to next controller, or if at last, quit");
	  P("");
	  P("");
	  P("");
	  P("Copyright © " _YEARS_ "  Mattias Andrée (maandree@member.fsf.org)");
	  P("");
	  P("This program is free software: you can redistribute it and/or modify");
	  P("it under the terms of the GNU General Public License as published by");
	  P("the Free Software Foundation, either version 3 of the License, or");
	  P("(at your option) any later version.");
	  P("");
	  return 0;
	}
    }
  
  
  /* Check permissions */
  if (getuid()) /* Always accept root */
  {
    struct group* video_grp = getgrnam("video");
    if (video_grp) /* Accept everypony if the group ‘video’ does not exist */
      {
	struct passwd* realuser = getpwuid(getuid());
	char** mems;
	char* realname;
	int ok;
	if (!realuser)
	  {
	    P("You do not exist, go away!");
	    return 1;
	  }
	mems = video_grp->gr_mem;
	realname = realuser->pw_name;
	ok = 0;
	for (; *mems; mems++)
	  if (!strcmp(realname, *mems))
	    {
	      ok = 1;
	      break;
	    }
	endgrent();
	if (!ok)
	  {
	    P("Sorry, you need to be in the group 'video'.");
	    return 1;
	  }
      }
    endpwent();
  }
  
  
  if (!get && !set)
    {
      P("\n");
      P("If the program is abnormally aborted the may be some residual");
      P("effects on the terminal. the following commands should reset it:");
      P("");
      P("    stty icanon echo");
      P("    echo -en '\\e[?25h'");
      P("");
      P("\n\n\n");
    }
  
  
  if (!get && !set)
    {
      /* Get the size of the terminal */
      if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &win) == -1)
	perror(*argv);
      else
	cols = win.ws_col;
      
      /* Hide cursor */
      printf("%s", "\033[?25l");
      fflush(stdout);
      
      /* stty -icanon -echo */
      if (tcgetattr(STDIN_FILENO, &stty))
	{
	  perror(*argv);
	  return 1;
	}
      saved_stty = stty;
      stty.c_lflag &= (tcflag_t)~(ICANON | ECHO);
      if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &stty))
	{
	  perror(*argv);
	  return 1;
	}
    }
  
  
  /* Fork to diminish risk of unclean exit */
  if (!get && !set)
    {
      pid = FORK();
      if (pid == (pid_t)-1)
	{
	  perror(*argv);
	  pid = 0;
	}
    }
  
  if (pid)
    waitpid(pid, NULL, 0);
  else
    {
      float brightness = 0;
      int nbrightness = 0;
      
      if (!get && !set)
	{
	  line = malloc((size_t)cols * 3 * sizeof(char));
	  space = malloc((size_t)cols * sizeof(char));
	  for (i = 0; i < (size_t)cols; i++)
	    {
	      *(line + i * 3 + 0) = (char)(0xE2);
	      *(line + i * 3 + 1) = (char)(0x94);
	      *(line + i * 3 + 2) = (char)(0x80);
	      *(space + i) = ' ';
	    }
	  *(space + cols - 1) = 0;
	  *(line + (cols - 2) * 3) = 0;
	}
      
      if (ndevices)
	{
	  char* device;
	  for (i = 0; i < ndevices; i++)
	    {
	      device = *(devices + i);
	      for (j = 0; *(device + j); j++)
		if (*(device + j) == '/')
		  {
		    device += j + 1;
		    j = -1;
		  }
	      if (get)
		{
		  float value = getbrightness(device);
		  if (value >= 0.f)
		    {
		      brightness += value;
		      nbrightness++;
		    }
		}
	      else if (set)
		setbrightness(device, set);
	      else
		adjust(cols, device);
	    }
	}
      else
	{
	  struct dirent* ent;
	  DIR* dir = opendir(BACKLIGHT_DIR);
	  if (dir)
	    {
	      char* device;
	      while ((ent = readdir(dir)))
		{
		  device = ent->d_name;
		  if (all || (strstr(device, "acpi_video") != device))
		    if (*device && (*device != '.'))
		      {
			if (get)
			  {
			    float value = getbrightness(device);
			    if (value >= 0.f)
			      {
				brightness += value;
				nbrightness++;
			      }
			  }
			else if (set)
			  setbrightness(device, set);
			else
			  adjust(cols, device);
		      }
		}
	      closedir(dir);
	    }
	}
      
      if (!get && !set)
	{
	  free(line);
	  free(space);
	}
      else if (get)
	{
	  if (nbrightness)
	    {
	      brightness *= 100.f;
	      brightness /= (float)nbrightness;
	      printf("%.2f%%\n", (double)brightness);
	      fflush(stdout);
	    }
	  else
	    {
	      printf("%s\n", "100.00%");
	      fflush(stdout);
	    }
	}
    }
  
  
  if (!get && !set)
    {
      /* `stty icanon echo` */
      if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &saved_stty))
	{
	  perror(*argv);
	  return 1;
	}
      
      /* Show cursor */
      printf("%s", "\033[?25h");
      fflush(stdout);
    }
  
  return 0;
}


/**
 * Test whether a string numerical and with at most two trailing '%'
 * 
 * @param   str  The string
 * @return       Whether the test passed
 */
static int isnumerical(const char* str)
{
  int p = 0, d = 0;
  if ((*str == 0) || (*str == '%'))
    return 0;
  for (; *str; str++)
    {
      switch (*str)
	{
	case '.':
	  if (d++)
	    return 0;
	  /* fall through */
	case '0' ... '9':
	  if (p)
	    return 0;
	  break;
	  
	case '%':
	  p++;
	  break;
	  
	default:
	  return 0;
	}
    }
  return p <= 2;
}


/**
 * Interactively adjust the backlight on a device
 * 
 * @param  cols    The number of columns on the terminal
 * @param  device  The device on which to adjust backlight
 */
static void adjust(int cols, const char* device)
{
  int min, max, cur, step, init, i, d;
  size_t lendir;
  char* dir = alloca(PATH_MAX * sizeof(char));
  char* buf = alloca(256 * sizeof(char));
  
  *dir = 0;
  dir = strcat(dir, BACKLIGHT_DIR "/");
  dir = strcat(dir, device);
  dir = strcat(dir, "/");
  lendir = strlen(dir);
  
  /* Get brightness parameters */
  min = 0;
  if (readfile(buf, strcat(dir, "max_brightness")))
    return;
  max = atoi(unnl(buf));
  *(dir + lendir) = 0;
  if (readfile(buf, strcat(dir, "brightness")))
    return;
  cur = atoi(unnl(buf));
  
  if (max <= min)
    return; /* what the buck */
  
  step = (max - min) / 200 ?: 1;
  init = cur;
  
  P("\n\n\n\n\n");
  bars(min, max, init, cur, cols);
  
  while ((d = getchar()) != -1)
    switch (d)
      {
      case 'q':
      case '\n':
      case 4:
	P("");
	return;
	
      case 'A':
      case 'C':
	cur += step << 1;
	/* fall through */
      case 'B':
      case 'D':
	cur -= step;
	if (cur < min)  cur = min;
	if (cur > max)  cur = max;
	if (writefile(buf, cur, dir))
	  return;
	bars(min, max, init, cur, cols);
      }
}



/**
 * Gets the current backlight setting on a device
 * 
 * @param   device  The device from which to get backlight
 * @return          The brightness as a [0; 1] float, negative on error
 */
static float getbrightness(const char* device)
{
  int min, max, cur, i;
  size_t lendir;
  char* dir = alloca(PATH_MAX * sizeof(char));
  char* buf = alloca(256);
  
  *dir = 0;
  dir = strcat(dir, BACKLIGHT_DIR "/");
  dir = strcat(dir, device);
  dir = strcat(dir, "/");
  lendir = strlen(dir);
  
  /* Get brightness parameters */
  min = 0;
  if (readfile(buf, strcat(dir, "max_brightness")))
    return -1.f;
  max = atoi(unnl(buf));
  *(dir + lendir) = 0;
  if (readfile(buf, strcat(dir, "brightness")))
    return -1.f;
  cur = atoi(unnl(buf));
  
  if (max <= min)
    return -1.f; /* what the buck */
  
  return (float)cur / (float)(max - min);
}



/**
 * Sets the current backlight setting on a device
 * 
 * @param  device      The device from which to get backlight
 * @param  adjustment  The adjustment to make
 */
static void setbrightness(const char* device, const char* adjustment)
{
  int min, max, cur, i, adj;
  size_t lendir;
  char* dir = alloca(PATH_MAX * sizeof(char));
  char* buf = alloca(256);
  int act = 0, integer = 0, decimal = 0, p = 0, d = 0;
  
  *dir = 0;
  dir = strcat(dir, BACKLIGHT_DIR "/");
  dir = strcat(dir, device);
  dir = strcat(dir, "/");
  lendir = strlen(dir);
  
  /* Get brightness parameters */
  min = 0;
  if (readfile(buf, strcat(dir, "max_brightness")))
    return;
  max = atoi(unnl(buf));
  *(dir + lendir) = 0;
  if (readfile(buf, strcat(dir, "brightness")))
    return;
  cur = atoi(unnl(buf));
  
  if (max <= min)
    return; /* what the buck */
  
  /* Read -/+/= head */
  if (*adjustment == '-')
    act = -1;
  else if (*adjustment == '+')
    act = 1;
  else if (*adjustment != '=')
    adjustment--;
  adjustment++;
  
  /* Parse numerical part */
  for (; *adjustment && (*adjustment != '%'); adjustment++)
    if (*adjustment == '.')
      d = 1;
    else if (d)
      {
	if ((long long)d * 10 > (long long)INT_MAX)
	  continue;
	if ((long long)decimal * 10 + 9 > (long long)INT_MAX)
	  continue;
	d *= 10;
	decimal *= 10;
	decimal += (*adjustment) - '0';
      }
    else
      {
	integer *= 10;
	integer += (*adjustment) - '0';
      }
  if (!d)
    d = 1;
  
  /* Count number of p:s */
  while (*adjustment++)
    p++;
  
  /* Calculate value to send */
  if (p == 0)
    adj = integer + (int)((double)decimal / (double)d + 0.5d);
  else if (p == 1)
    adj = (int)(((double)integer + (double)decimal / (double)d) * (double)(max - min) / 100.d);
  else
    adj = (int)(((double)integer + (double)decimal / (double)d) * (double)cur / 100.d);
  adj = (act & 1) * cur + (act | 1) * adj;
  if (adj < min)  adj = min;
  if (adj > max)  adj = max;
  
  /* Send value */
  writefile(buf, adj, dir);
}



/**
 * Read a file
 * 
 * @param   output  Buffer to store the file's content in
 * @param   file    The file to read
 * @return          Zero on success, non-zero on failure
 */
static int readfile(char* output, const char* file)
{
  #define BLOCK_SIZE  256  /* We will not even encounter that much */
  FILE* f;
  size_t got, have = 0;
  int ended = 0;
  
  if ((f = fopen(file, "r")) == NULL)
    {
      perror(file);
      return -1;
    }
  
  while (!ended)
    {
      got = fread(output + have, 1, BLOCK_SIZE, f);
      if (got != BLOCK_SIZE)
	{
	  ended = feof(f);
	  have += got;
	  clearerr(f);
	  fclose(f);
	  if (!ended)
	    {
	      perror(file);
	      return -1;
	    }
	}
      else
	have += got;
    }
  
  *(output + have) = 0;
  return 0;
}



/**
 * Write an integer to a file
 * 
 * @param   buffer   Buffer for temporary data
 * @param   integer  The integer to write
 * @param   file     The file to which to write
 * @return           Zero on success, non-zero on failure
 */
static int writefile(char* buffer, int integer, const char* file)
{
  FILE* f;
  size_t n = 0;
  
  buffer += 32;
  do
    {
      *(buffer - n++) = (char)((integer % 10) + '0');
      integer /= 10;
    }
      while (integer);
  buffer -= n - 1;
  *(buffer + n++) = '\n';
  
  if ((f = fopen(file, "w")) == NULL)
    {
      perror(file);
      return -1;
    }
  
  if (fwrite(buffer, 1, n, f) != n)
    {
      perror(file);
      fclose(f);
      return -1;
    }
  
  fflush(f);
  fclose(f);
  return 0;
}



/**
 * Print the status
 * 
 * @param  min   The minimum possible brightness
 * @param  max   The maximum possible brightness
 * @param  init  The brightness used when the program started
 * @param  cur   The current brightness
 * @param  cols  The number of columns on the terminal
 */
static void bars(int min, int max, int init, int cur, int cols)
{
  int mid = (int)((float)(cur - min) * (float)(cols - 2) / (float)(max - min) + 0.5f);
  
  printf("\033[1000D\033[6A");
  printf("\033[2K┌%s┐\n", line);
  *(space + mid) = 0;
  printf("\033[2K│\033[47m%s\033[49m%s│\n", space, space + mid + 1);
  *(space + mid) = ' ';
  printf("\033[2K└%s┘\n", line);
  printf("\033[2KMaximum brightness: %i\n", max);
  printf("\033[2KInitial brightness: %i\n", init);
  printf("\033[2KCurrent brightness: %i\n", cur);
  
  fflush(stdout);
}

