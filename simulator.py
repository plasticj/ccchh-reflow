#!/usr/bin/env python3

from time import *
import math,os,sys,fcntl
from random import random

maxt = 300
startt = 20
noise = 2

def main():
	start = time()
	fd = sys.stdout
	fcntl.fcntl(fd, fcntl.F_SETFL, os.O_NONBLOCK) 
	while True:
		sleep(0.1)
		x = time()-start
		t = startt + maxt*(1-math.exp(-x/50))+(random()-0.5)*noise
		t = abs(t)
		try:
			fd.write(str(t)+'\n')
			fd.flush()
		except:
			pass



main()
