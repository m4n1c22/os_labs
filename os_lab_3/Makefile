#MACROS
obj-m += fifo.o
obj-m += producer_1.o
obj-m += producer_2.o
obj-m += consumer_1.o
PWD := $(shell pwd)
KVER := $(shell uname -r)
#Target option for Compiling all the tasks
default:
	make -C /lib/modules/$(KVER)/build SUBDIRS=$(PWD) modules
clean:
	make -C /lib/modules/$(KVER)/build SUBDIRS=$(PWD) clean
