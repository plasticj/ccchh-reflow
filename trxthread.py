
import threading 
import serial
import time
 
class TRXThread(threading.Thread): 
	last = ""
	lastlock = threading.Lock()
	stopped = False

	def __init__(self, ttyname, baud): 
		threading.Thread.__init__(self) 
		self.ttyname = ttyname
		self.serCon = None
		try:
			print(ttyname, baud)
			self.serCon = serial.Serial(ttyname, baud)
		except Exception as e:
			print(e)

	def stop(self):
		self.stopped = True
		if self.serCon:
			self.serCon.close()
 
	def run(self): 
		buf = ''
		while not self.stopped:
			if self.serCon:
				buf = buf + self.serCon.read(self.serCon.inWaiting())
			if '\n' in buf:
				lines = buf.split('\n') # Guaranteed to have at least 2 entries
				self.lastlock.acquire()
				self.last = lines[-2]
				self.lastlock.release()
				#If the Arduino sends lots of empty lines, you'll lose the
				#last filled line, so you could make the above statement conditional
				#like so: if lines[-2]: last_received = lines[-2]
				buf = lines[-1]
			time.sleep(0.1)
