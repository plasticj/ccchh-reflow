#!/usr/bin/env python3

from tkinter import *
from graph import *

import re
import serial

class App:
	def __init__(self, master):

		frame = Frame(master)
		frame.pack()

		self.button = Button(frame, text="QUIT", fg="red", command=frame.quit)
		self.button.pack(side=LEFT)

		self.hi_there = Button(frame, text="Hello", command=self.say_hi)
		self.hi_there.pack(side=LEFT)

		self.gw = 800
		self.gh = 600

		self.graph = Graph(master, width=self.gw, height=self.gh, background="white")
		self.graph.rangex = (0,10)
		self.graph.rangey = (10,150)
		self.graph.xtics = 10
		self.graph.ytics = 50
		self.graph.xlabel="t"
		self.graph.ylabel="T"
		self.graph.draw()

#		self.graph.create_line(10,10,400,400, fill="red")
		self.graph.pack()
		self.lastpoint=(0,20)

	def t_to_x(self,t):
		scx = 8
		return 20+t*scx

	def temp_to_y(self,temp):
		scy = 7
		refy = 11
		return self.gh-((temp-refy)*scy)

	def mkpoint(self, t, temp):
		self.graph.create_rectangle(
				self.t_to_x(t)-2,
				self.temp_to_y(temp)-2,
				self.t_to_x(t)+2,
				self.temp_to_y(temp)+2,
				fill="red")

	def add_value(self, t, temp):
		self.graph.add_point(t,temp)
#		self.graph.create_line(
#				self.t_to_x(self.lastpoint[0]),
#				self.temp_to_y(self.lastpoint[1]),
#				self.t_to_x(t),
#				self.temp_to_y(temp),
#
#				fill="blue",
#				width=2)
#
		self.graph.draw()
		self.lastpoint=(t,temp)


	
	def say_hi(self):
		print("hi there, everyone!")



root = Tk()
app = App(root)
t = 0
def handle_serial():
	global t
	Temp = None
	try:
		ser = serial.Serial('/dev/ttyUSB0',1000000,timeout=2,parity=serial.PARITY_NONE)
		line = ser.readline()
		ser.close()


		print(line)
		if len(line) > 1:
			Temp = float(re.search('T: (.*?) ', str(line)).group(1))

		print(Temp)
	except:
		pass


	if (Temp):
		app.add_value(t,Temp)
	t = t+1
	if (t >= app.graph.rangex[1]):
		app.graph.rangex = (app.graph.rangex[0],t+5)
		app.graph.draw_background()
		app.graph.draw()

	root.after(1,handle_serial)







#f = open("test.dat")
#i = 1
#for line in f.readlines():
#	x = i
#	y = round(float(line))
#	app.add_value(i, y)
#	i = i + 1
#
root.after(0,handle_serial)
root.mainloop()


