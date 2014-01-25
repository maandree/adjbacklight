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


/**
 * The years this software has been developed
 */
#define _YEARS_  "2012, 2013, 2014"


/**
 * Forking method
 */
#define FORK  vfork


/**
 * Print a line to stdout
 * 
 * @param  S:char*  The string to print
 */
#define P(S)  printf("%s", S "\n")



#define  adjust(X,Y,Z)



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
  int rows, cols;
  struct termios saved_stty;
  struct termios stty;
  pid_t pid;
  
  int i, j;
  int all = 0;
  char** devices = alloca(argc * sizeof(char*));
  int ndevices = 0;
  
  
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
  
  
  /* rows cols = $(stty size) */
  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &win) == -1)
    {
      perror(*argv);
      rows = 24;
      cols = 80;
    }
  else
    {
      rows = win.ws_row;
      cols = win.ws_col;
    }
  
  /* Hide cursor */
  printf("%s", "\033[?25l");
  fflush(stdout);
  
  /* stty -icanon -echo */
  if (tcgetattr(STDIN_FILENO, &saved_stty))
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
  
  
  /* Fork to diminish risk for unclean exit  */
  pid = FORK();
  if (pid == (pid_t)-1)
    {
      perror(*argv);
      pid = 0;
    }
  
  if (pid)
    waitpid(pid, NULL, 0);
  else
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
	    adjust(rows, cols, device);
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
		  adjust(rows, cols, device);
	      }
	    closedir(dir);
	  }
      }
  
  
  /* `stty icanon echo` */
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &saved_stty);
    {
      perror(*argv);
      return 1;
    }
  
  /* Show cursor */
  printf("%s", "\033[?25h");
  fflush(stdout);
  
  return 0;
}

