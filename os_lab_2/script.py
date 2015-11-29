#!/usr/bin/python

import threading
import shutil
import sys
import itertools

def read_time():
	print "read_time"
	with open("/proc/deeds_clock", "r") as f:
		shutil.copyfileobj(f, sys.stdout)

def change_format(val):
	print "change_format"
	with open("/proc/deeds_clock_config", "w") as fwrite:
		fwrite.write("" + str(val))
	with open("/proc/deeds_clock_config", "r") as fread:
		shutil.copyfileobj(fread,sys.stdout)

count = 100
while(count > 1):
	val = count % 2
	count = count - 1
	rt = threading.Thread(target=read_time)
	ct = threading.Thread(target=change_format, args=(val,))
	rt.start()
	ct.start()

