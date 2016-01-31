#MACROS
obj-m := task1_1.o
#obj-m += task1_1.o
#obj-m += producer.o
#obj-m += consumer.o
PWD := $(shell pwd)
KVER := $(shell uname -r)

#Target option for Compiling all the tasks
default:
	make -C /lib/modules/$(KVER)/build SUBDIRS=$(PWD) modules
clean:
	make -C /lib/modules/$(KVER)/build SUBDIRS=$(PWD) clean

