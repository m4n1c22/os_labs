#!/usr/bin/python

import shutil
import os

def create_kernel_producers():
	shutil.copy2('producer.c' , 'producer_1.c')
	shutil.copy2('producer.c' , 'producer_2.c')


def create_kernel_consumers():
	shutil.copy2('consumer.c' , 'consumer_1.c')


def create_make_file():
	fo = open('Makefile', 'w+')
	fo.write('#MACROS'
	'obj-m += fifo.o\n'
	'obj-m += producer_1.o\n'
	'obj-m += producer_2.o\n'
	'obj-m += consumer_1.o\n'
	'PWD := $(shell pwd)\n'
	'KVER := $(shell uname -r)\n'
	'#Target option for Compiling all the tasks\n'
	'default:\n'
	'	make -C /lib/modules/$(KVER)/build SUBDIRS=$(PWD) modules\n'
	'clean:\n'
	'	make -C /lib/modules/$(KVER)/build SUBDIRS=$(PWD) clean\n'
	)
	fo.close()


def insert_kernel_modules():
	os.system("sudo insmod fifo.ko fifo_size=43")
	os.system("./fifo_perm.sh")
	os.system("gcc -o task task2_2.c")
	os.system("sudo insmod producer_1.ko rate=2 msg='funnymsgs' instance='producer_1'")
	os.system("sudo insmod producer_2.ko rate=2 msg='funnymsgs' instance='producer_2'")
	os.system("./task -p 3 p1_hello p1")
	os.system("./task -p 3 p2_hello p2")
	os.system("./task -p 5 p3_hello p3")
	os.system("sudo insmod consumer_1.ko rate=6 instance='consumer_1'")
	os.system("./task -c 2 c1")
	os.system("./task -c 4 c2")

create_kernel_producers()
create_kernel_consumers()
create_make_file()
os.system("make clean")
os.system("make")
insert_kernel_modules()
