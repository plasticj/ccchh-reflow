#!/usr/bin/env python

from Tkinter import *
from tkFileDialog import *
from graph import *

import re
import serial
import os,sys

class App:
	def __init__(self, master):
		self.master = master

		frame = Frame(master)
		frame.pack()

		self.T = Label(frame, text="Aktuelle Temperatur: X")
		self.T.pack(side=LEFT)

		self.button = Button(frame, text="QUIT", fg="red", command=frame.quit)
		self.button.pack(side=LEFT)

		self.openSource= Button(frame, text="Open", command=self.openDia)
		self.openSource.pack(side=LEFT)



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

		self.openSource= Button(frame, text="Reset", command=self.startNew)
		self.openSource.pack(side=LEFT)

#		self.graph.create_line(10,10,400,400, fill="red")
		self.graph.pack()
		self.lastpoint=(0,20)
		self.source = ""
		self.lastline=""
		self.t = 0
	
	def startNew(self):
		self.t = 0
		self.graph.reset()
		self.lastpoint=(0,20)
		self.lastline=""

	def setSource(self, s):
		olds = self.source
		try:
			self.source = s
			f = os.open(self.source, os.O_RDONLY | os.O_NONBLOCK)
			fd = os.fdopen(f)
			self.master.createfilehandler(fd, tkinter.READABLE, self.handle_input)
		except Exception as e:
			showwarning("Error", e)
			self.source = olds


	def openDia(self):
		self.setSource(askopenfilename())
		self.startNew()

	def add_value(self, t, temp):
		self.graph.add_point(t,temp)

		self.lastpoint=(t,temp)

	def refresh(self):
		#print("lastline is: %s" % self.lastline)
		if len(self.lastline.strip()) > 1:
			newT = float(self.lastline.strip())
		else:
			newT = None

		if (newT):
			self.T.config(text="Last T: %.1f" % newT)
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

app.setSource(sys.argv[1])

root.after(0,app.refresh)
root.mainloop()

try:
	root.deletefilehandler(fd)
except:
	pass


