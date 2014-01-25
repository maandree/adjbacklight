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
import java.io.*;
import java.util.*;


/**
 * Interactive backlight adjustment
 * 
 * @author  Mattias Andrée, <a href="mailto:maandree@member.fsf.org">maandree@member.fsf.org</a>
 */
public class Adjbacklight
{
    /**
     * The is the main entry point of the program
     * 
     * @param   args         Expects the three values: terminal height, terminal width, backlight device
     * @throws  IOException  On error
     */
    public static void main(final String... args) throws IOException
    {
	String dir = "/sys/class/backlight/" + args[2] + "/";
	int width = Integer.parseInt(args[1]);
	int max = 0, min = 0, cur = 0;
	
	{   InputStream is = null;
	    try
	    {	is = new FileInputStream(dir + "brightness");
		Scanner sc = new Scanner(is);
		cur = Integer.parseInt(sc.nextLine());
	    }
	    finally
	    {   if (is != null)
		    is.close();
	    }
	}
	{   InputStream is = null;
	    try
	    {	is = new FileInputStream(dir + "max_brightness");
		Scanner sc = new Scanner(is);
		max = Integer.parseInt(sc.nextLine());
	    }
	    finally
	    {   if (is != null)
		    is.close();
	    }
	}
	
	if (max <= min)
	    System.exit(127);
        int step = (max - min) / 200;
	int init = cur;
	if (step == 0)
	    step = 1;
	
	System.out.print("\n\n\n\n\n\n");
	print(min, max, init, cur, width);
	
	for (int d; (d = System.in.read()) != -1;)
	    switch (d)
	    {
		case 'q':
		case '\n':
		case 4:
		    System.out.println();
		    return;
		
		case 'A':
		case 'C':  cur += step << 1;
		case 'B':
		case 'D':  cur -= step;
		    
		    if (cur < min)  cur = min;
		    if (cur > max)  cur = max;
		    
		    {   OutputStream os = null;
			try
			{   os = new FileOutputStream(dir + "brightness");
			    os.write((cur + "\n").getBytes("UTF-8"));
			    os.flush();
			    print(min, max, init, cur, width);
			}
			finally
			{   if (os != null)
				os.close();
			}
		    }
		    break;
	    }
    }
    
    /**
     * Print the status
     * 
     * @param  min    The minimum possible brightness
     * @param  max    The maximum possible brightness
     * @param  init   The brightness used when the program started
     * @param  cur    The current brightness
     * @param  width  The width of the terminal
     */
    private static void print(int min, int max, int init, int cur, int width)
    {
	String line = "──────────────────────────────────";
	while (line.length() < width - 2)
	    line += line;
	line = line.substring(0, width - 2);
	
	String space = "                                  ";
	while (space.length() < width - 2)
	    space += space;
	space = space.substring(0, width - 2);
	
	int mid = (int)((cur - min) * (width - 2.) / (max - min) + 0.5);
	
	System.out.print("\033[1000D\033[6A");
	System.out.println("\033[2K┌" + line + "┐");
	System.out.println("\033[2K│\033[47m" + space.substring(0, mid) + "\033[49m" + space.substring(mid) + "│");
	System.out.println("\033[2K└" + line + "┘");
	System.out.println("\033[2KMaximum brightness: " + max);
	System.out.println("\033[2KInitial brightness: " + init);
	System.out.println("\033[2KCurrent brightness: " + cur);
    }
    
}

