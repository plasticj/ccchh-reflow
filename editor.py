#!/usr/bin/env python

from Tkinter import *
from graph import *

import re
import serial
import os,sys

class App:
	def __init__(self, master):
		self.master = master

		frame = Frame(master)
		frame.pack()

		self.button = Button(frame, text="QUIT", fg="red", command=frame.quit)
		self.button.pack(side=LEFT)

		self.hi_there = Button(frame, text="Hello", command=self.say_hi)
		self.hi_there.pack(side=LEFT)

		self.gw = 800
		self.gh = 600

		self.graph = Graph(master, width=self.gw, height=self.gh, background="white")
		self.graph.rangex = (0,11)
		self.graph.rangey = (10,150)
		self.graph.xtics = 10
		self.graph.ytics = 50
		self.graph.xlabel="t"
		self.graph.ylabel="T"
		self.graph.refresh()

#		self.graph.create_line(10,10,400,400, fill="red")
		self.graph.pack()
		self.lastpoint=(0,20)
		self.lastline=""
		self.t = 0

	def add_value(self, t, temp):
		self.graph.add_point(t,temp)

		self.lastpoint=(t,temp)


	
	def say_hi(self):
		print("hi there, everyone!")

	def refresh(self):
		print("lastline is: %s" % self.lastline)
		if len(self.lastline.strip()) > 1:
			newT = float(self.lastline.strip())
		else:
			newT = None

		if (newT):
			self.add_value(self.t,newT)
		self.t = self.t + 1
		if (self.t >= self.graph.rangex[1]):
			self.graph.rangex = (self.graph.rangex[0],self.t+5)
			self.graph.refresh()

		if (newT > self.graph.rangey[1]):
			self.graph.rangey = (self.graph.rangey[0],newT+50)
			self.graph.refresh()

		self.master.after(1000,self.refresh)

	def handle_input(self, f, mask):
		try:
			l = f.readline()
			if l:
				self.lastline = l
		except:
			pass






#
#	Temp = None
#	try:
#		ser = serial.Serial('/dev/ttyUSB0',1000000,timeout=2,parity=serial.PARITY_NONE)
#		line = ser.readline()
#		ser.close()
#
#
#		print(line)
#		if len(line) > 1:
#			Temp = float(re.search('T: (.*?) ', str(line)).group(1))
#
#		print(Temp)
#	except:
#		pass


#	if (Temp):
#		app.add_value(t,Temp)
#	t = t+1
#	if (t >= app.graph.rangex[1]):
#		app.graph.rangex = (app.graph.rangex[0],t+5)
#		app.graph.draw_background()
#
#	root.after(1,handle_serial)
#






#f = open("test.dat")
#i = 1
#for line in f.readlines():
#	x = i
#	y = round(float(line))
#	app.add_value(i, y)
#	i = i + 1
#

root = Tk()
app = App(root)

try:
	filename = sys.argv[1]
	f = os.open(filename, os.O_RDONLY | os.O_NONBLOCK)
	fd = os.fdopen(f)
	root.createfilehandler(fd, tkinter.READABLE, app.handle_input)
except Exception as e:
	print("exception '%s' occured" % e)
	sys.exit(1)

root.after(0,app.refresh)
root.mainloop()

try:
	root.deletefilehandler(fd)
except:
	pass


