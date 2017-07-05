MODULE = sys_status_module

obj-m += sys_status_module.o

CC	= gcc
CFLAGS-y	= -g
RM	= rm -f
KDIR = /lib/modules/4.10.0-22-generic/build

all: readinfo
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
readinfo: readinfo.c
		$(CC) $(CFLAGS-y) -o readinfo readinfo.c

install:
		sudo insmod $(MODULE).ko
remove:
		sudo rmmod sys_status_module

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	$(RM) readinfo
	sudo rmmod sys_status_module
