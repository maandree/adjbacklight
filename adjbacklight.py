#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import sys
from subprocess import Popen, PIPE



'''
Hack to enforce UTF-8 in output (in the future, if you see anypony not using utf-8 in programs by default, report them to Princess Celestia so she can banish them to the moon)
'''
def print(text = '', end = '\n'):
    sys.stdout.buffer.write((str(text) + end).encode('utf-8'))



'''
This is the mane class of the program
'''
class Adjbacklight():
    def __init__(self):
        Popen('stty -icanon'.split(' ')).wait()
        try:
            dir = '/sys/class/backlight/intel_backlight/'
            (size, dummy) = Popen('stty size'.split(' '), stdout=PIPE, stderr=PIPE).communicate()
            width = int(size.decode('UTF-8').replace('\n', '').split(' ')[1])
            with open(dir + 'brightness', 'r') as file:
                current = int(file.readline().replace('\n', ''))
            with open(dir + 'max_brightness', 'r') as file:
                max = int(file.readline().replace('\n', ''))
            
        finally:
            Popen('stty icanon'.split(' ')).wait()


'''
Start the program from Adjbacklight.__init__ if this is the executed file
'''
if __name__ == '__main__':
    Adjbacklight()
