#!/usr/bin/env python3

from time import *
import math
from random import random

maxt = 300
startt = 20
noise = 2

def main():
	start = time()
	while True:
		sleep(0.1)
		x = time()-start
		t = startt + maxt*(1-math.exp(-x/50))+(random()-0.5)*noise
		t = abs(t)
		print(t)



main()
