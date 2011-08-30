#!/usr/bin/env python

from Tkinter import *

class Graph(Canvas):
	def __init__(self, parent= None, **kw):
		Canvas.__init__(self,parent, kw)
#		self.create_rectangle(10,10,30,30,fill="green")
		self.rangex=(0,10)
		self.rangey=(0,10)
		self.xtics=10
		self.ytics=10
		self.xlabel="x"
		self.ylabel="y"

		self.startx = 30
		self.endx   = self.winfo_reqwidth()-20
		self.starty = self.winfo_reqheight()-20
		self.endy   = 20

		self.points = []
		self.lastpoint = ()

	def reset(self):
		self.points = []
		self.lastpoint = ()
		self.refresh()

	def draw_background(self):
		self.create_rectangle(0,0,self.winfo_reqwidth(),self.winfo_reqheight(),fill="white")

	def refresh(self):
		self.draw_background()
		self.drawaxis()
		self.drawlabels()
		for p in self.points:
			self.mkpoint(p)
		self.connect_points()

	def drawlabels(self):
		startx = self.startx 
		endx   = self.endx
		starty = self.starty
		endy   = self.endy
		self.create_text(endx-10,starty+10, text=self.xlabel, font=("courier","10","normal"))
		self.create_text(startx-10,endy+10, text=self.ylabel, font=("courier","10","normal"))
	
	def x_pix_per_value(self):
		xpix = self.endx - self.startx
		xr = self.rangex[1]-self.rangex[0]
		return xpix / xr

	def y_pix_per_value(self):
		ypix = self.starty - self.endy
		yr = self.rangey[1]-self.rangey[0]
		return ypix / yr


	def x_value_to_pix(self, xval):
		return int(self.startx+xval*self.x_pix_per_value())

	def y_value_to_pix(self, yval):
		return int(self.starty - yval*self.y_pix_per_value())

	def drawaxis(self):
		startx = self.startx 
		endx   = self.endx
		starty = self.starty
		endy   = self.endy


		#y axis
		self.create_line(startx, starty, startx, endy, width=1)
		#x axis
		self.create_line(startx, starty, endx, starty, width=1)

		xtics_list = [t-self.rangex[0] for t in range(self.rangex[0],self.rangex[1],self.xtics)]
		ytics_list = [t-self.rangey[0] for t in range(int(self.rangey[0]),int(self.rangey[1]),self.ytics)]
	
		for t in xtics_list:
			x = self.x_value_to_pix(t)
			self.create_line(x, starty+2, x,starty-2 )
			self.create_text(x, starty+7, text=str(t), font=("courier","10","normal"))



		for t in ytics_list:
			y = self.y_value_to_pix(t)
			self.create_line(startx-2, y, startx+2, y)
			self.create_text(startx-4, y, text=str(t), anchor="e", font=("courier","10","normal"))


	def add_point(self,xval,yval):
		p = (xval,yval)
		self.points.append(p)
		self.mkpoint(p)
		self.connect_to_point(p)
		self.lastpoint = p

	def mkpoint(self, p):
		self.create_rectangle(
                                self.x_value_to_pix(p[0])-2,
                                self.y_value_to_pix(p[1])-2,
                                self.x_value_to_pix(p[0])+2,
                                self.y_value_to_pix(p[1])+2,
                                fill="red")

	def connect_to_point(self, p):
		if self.lastpoint:
			self.create_line(
				self.x_value_to_pix(self.lastpoint[0]),self.y_value_to_pix(self.lastpoint[1]),
				self.x_value_to_pix(p[0]),self.y_value_to_pix(p[1]),
				fill="blue",
				width=2)

	def connect_points(self):
		if len(self.points) > 1:
			tmp = self.lastpoint
			self.lastpoint = self.points[0]
			for p in self.points[1:]:
				self.connect_to_point(p)
				self.lastpoint = p
			self.lastpoint = tmp
