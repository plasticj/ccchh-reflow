#!/usr/bin/env python2.6

from Tkinter import *
import os


class App:
	def __init__(self, master):
		self.master = master
		frame = Frame(master)
		frame.pack()
		self.button = Button(frame, text="QUIT", fg="red", command=frame.quit)
		self.button.pack(side=LEFT)

		self.hi_there = Button(frame, text="Read", command=self.say_hi)
		self.hi_there.pack(side=LEFT)

		self.gw = 800
		self.gh = 600

		self.lastline = "."


	def say_hi(self):
		print("trying to read ...")
		try:
			print("1")
			f = os.open('test.fifo', os.O_RDONLY | os.O_NONBLOCK)
			fd = os.fdopen(f)
			print("2")
			self.master.createfilehandler(fd, tkinter.READABLE, self.handler)
			print("handler created")
		except Exception as e:
			print("except: "+  e)

	def __del__(self):
		try:
			self.master.deletefilehandler(f)
		except:
			pass


	def handler(self, f, mask):
		try:
			l = f.readline()
			if len(l) > 0:
				self.lastline = l
		except:
			pass



root = Tk()
app = App(root)

def printlastline():
	print(app.lastline)
	root.after(1000,printlastline)

	
root.after(0,printlastline)

root.mainloop()
