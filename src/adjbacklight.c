/**
 * adjbacklight – Convient method for adjusting the backlight on your portable computer
 * 
 * Copyright © 2012, 2013, 2014  Mattias Andrée (maandree@member.fsf.org)
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
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <limits.h>



/**
 * The years this software has been developed
 */
#define _YEARS_  "2012, 2013, 2014"


/**
 * Forking method
 */
#define FORK  fork


/**
 * Print a line to stdout
 * 
 * @param  S:char*  The string to print
 */
#define P(S)  printf("%s", S "\n")



/**
 * Interactively adjust the backlight on a device
 * 
 * @param  cols    The number of columns on the terminal
 * @param  device  The device on which to adjust backlight
 */
static void adjust(int cols, const char* device);

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
  pid_t pid;
  int i, j;
  int all = 0, cols = 80, ndevices = 0;
  char** devices = alloca(argc * sizeof(char*));
  
  
  if (argc > 1)
    {
      char* arg;
      for (i = 1; i < argc; i++)
	{
	  #define T(S)  (!strcmp(arg, S))
	  arg = *(argv + i);
	  if (T("-a") || T("--all"))
	    all = 1;
	  else if (T("-c") || T("--copyright") || T("--copying"))
	    {
	      P("\n");
	      P("adjbacklight – Convient method for adjusting the backlight on your portable computer");
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
	  else
	    {
	      if (*arg && (*arg != '-'))
		*(devices + ndevices++) = arg;
	    }
	  #undef T
	}
      if (all + ndevices == 0)
	{
	  P("\n");
	  P("adjbacklight - Convient method for adjusting the backlight on your portable computer");
	  P("");
	  P("USAGE: adjbacklight [ -c | -w | -a | DEVICE...]");
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
	  P("--all           Run for all devices, including ACPI devices ");
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
  
  
  P("\n");
  P("If the program is abnormally aborted the may be some residual");
  P("effects on the terminal. the following commands should reset it:");
  P("");
  P("    stty icanon echo");
  P("    echo -en '\\e[?25h'");
  P("");
  P("\n\n\n");
  
  
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
  stty.c_lflag &= ~(ICANON | ECHO);
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &stty))
    {
      perror(*argv);
      return 1;
    }
  
  
  /* Fork to diminish risk of unclean exit */
  pid = FORK();
  if (pid == (pid_t)-1)
    {
      perror(*argv);
      pid = 0;
    }
  
  if (pid)
    waitpid(pid, NULL, 0);
  else
    {
      line = malloc(cols * 3 * sizeof(char));
      space = malloc(cols * sizeof(char));
      for (i = 0; i < cols; i++)
	{
	  *(line + i * 3 + 0) = 0xE2;
	  *(line + i * 3 + 1) = 0x94;
	  *(line + i * 3 + 2) = 0x80;
	  *(space + i) = ' ';
	}
      *(space + cols - 1) = 0;
      *(line + (cols - 2) * 3) = 0;
      
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
	      adjust(cols, device);
	    }
	}
      else
	{
	  struct dirent* ent;
	  DIR* dir = opendir("/sys/class/backlight");
	  if (dir)
	    {
	      char* device;
	      char* forbidden = "acpi_video";
	      while ((ent = readdir(dir)))
		{
		  device = ent->d_name;
		  if (all || (strstr(device, forbidden) != forbidden))
		    if (*device && (*device != '.'))
		      adjust(cols, device);
		}
	      closedir(dir);
	    }
	}
      
      free(line);
      free(space);
    }
  
  
  /* `stty icanon echo` */
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &saved_stty))
    {
      perror(*argv);
      return 1;
    }
  
  /* Show cursor */
  printf("%s", "\033[?25h");
  fflush(stdout);
  
  return 0;
}



/**
 * Interactively adjust the backlight on a device
 * 
 * @param  cols    The number of columns on the terminal
 * @param  device  The device on which to adjust backlight
 */
static void adjust(int cols, const char* device)
{
  #define unnl(data)			\
    ({					\
      for (i = 0; *(data + i); i++)	\
	if (*(data + i) == '\n')	\
	  *(data + i) = 0;		\
      data;				\
    })
  
  int min, max, cur, step, init, i, d;
  size_t lendir;
  char* dir = alloca(PATH_MAX * sizeof(char));
  char* buf = alloca(256);
  
  *dir = 0;
  dir = strcat(dir, "/sys/class/backlight/");
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
      *(buffer - n++) = (integer % 10) + '0';
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
  int mid = (int)((cur - min) * (float)(cols - 2) / (float)(max - min) + 0.5f);
  
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

