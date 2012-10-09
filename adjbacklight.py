#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
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

#Popen(compressCommand(ext).split(' '), stdout=fileout, stdin=filein).communicate()
#(out, err) = Popen(['env', 'python', '--version'], stdout=PIPE, stderr=PIPE).communicate()


'''
Start the program from Adjbacklight.__init__ if this is the executed file
'''
if __name__ == '__main__':
    Adjbacklight()
