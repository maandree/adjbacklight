/**
 * adjbacklight – Convient method for adjusting the backlight on your portable computer
 * 
 * Copyright © 2012  Mattias Andrée (maandree@kth.se)
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
 * @author  Mattias Andrée, <a href="mailto:maandree@kth.se">maandree@kth.se</a>
 */
public class Adjbacklight
{
    public static void main(final String... args) throws IOException
    {
	String dir = "/sys/class/backlight/intel_backlight/";
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
        int step = (max - min) / 100;
	int init = cur;
	
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

