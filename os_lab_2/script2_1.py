#!/usr/bin/python

import threading
import shutil
import sys
import os

def change_bufsize():
	print "change buffer size"
	os.system('sudo echo 4 > /proc/fifo_config')

def read_buf():
	print "Reading the buffer"
	os.system('cat /dev/fifo1')

def write_buf():
	print "Writing to buffer"
	os.system('sudo echo -n "123" > /dev/fifo0')

count = 5
while(count > 1):
	count = count - 1
	os.system('cat /proc/fifo_config')
	ch_buf = threading.Thread(target=change_bufsize)
	wr_buf = threading.Thread(target=write_buf)
	re_buf = threading.Thread(target=read_buf)
	ch_buf.start()
	wr_buf.start()
	re_buf.start()

