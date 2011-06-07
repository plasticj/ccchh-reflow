#!/usr/bin/env python3

from tkinter import *

class Graph(Canvas):
	def __init__(self, parent= None, **kw):
		Canvas.__init__(self,parent, kw)
		self.create_rectangle(10,10,30,30,fill="green")
		self.rangex=(0,1)
		self.rangey=(0,1)

	def drawaxis(self):
		starty = self.winfo_reqheight()-20
		startx = 20
		endy   = 20
		endx   = self.winfo_reqwidth()-20

		#y axis
		self.create_line(startx, starty, startx, endy)
		#x axis
		self.create_line(startx, starty, endx, starty)

		xpix = self.winfo_reqwidth() -40
		xr = self.rangex[1]-self.rangex[0]
		xtnum = int(xr / self.ticksx)
		w = xpix / xtnum
		for i in range(xtnum):
			x = startx + i*w
			self.create_line(x, starty+2, x,starty-2 )


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
		self.graph.rangex = (10,100)
		self.graph.rangey = (10,100)
		self.graph.ticksy = 20
		self.graph.ticksx = 20
		self.graph.drawaxis()

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

	def draw(self, t, temp):
		self.mkpoint(t,temp)
		self.graph.create_line(
				self.t_to_x(self.lastpoint[0]),
				self.temp_to_y(self.lastpoint[1]),
				self.t_to_x(t),
				self.temp_to_y(temp),

				fill="blue",
				width=2)

		self.lastpoint=(t,temp)


	
	def say_hi(self):
		print("hi there, everyone!")

root = Tk()

app = App(root)


f = open("test.dat")
i = 1
for line in f.readlines():
	x = i
	y = round(float(line))
	app.draw(i, y)
	i = i + 1

root.mainloop()


