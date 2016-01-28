#!/usr/bin/python

import os

print "read_time"
os.system("insmod task1_1.ko fifo_size=4")
os.system("./fifo_perm.sh")
os.system("echo 'data' > /dev/deeds_fifo")
os.system("cat /dev/deeds_fifo")

