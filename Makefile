obj-m := cipherdev.o
cipherdev-objs := cipherdev_main.o


KERNELDIR ?= /lib/modules/$(shell uname -r)/build
PWD       := $(shell pwd)

default: all

all: build cipherctl

build:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

cipherctl: 
	gcc -g -Wall cipherctl.c -o cipherctl

test:
	gcc -g -Wall test.c -o test.o

clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions Module.* modules.* cipherctl
