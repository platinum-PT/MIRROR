obj-m := mirror.o

ALL:
	make -C /usr/src/linux SUBDIRS=$(PWD) modules

clean:
	rm -rf *.o *.ko .tmp* .*cmd *.mod.c *.symvers

install:
	mkdir -p /lib/modules/`uname -r`/kernel/drivers/PT
	cp mirror.ko /lib/modules/`uname -r`/kernel/drivers/PT/
	depmod -a
