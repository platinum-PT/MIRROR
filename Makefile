obj-m := mirror.o

ifndef KERNEL_DIR
	KERNEL_DIR=/lib/modules/$(shell uname -r)/build
endif

ALL:
	make -C $(KERNEL_DIR) SUBDIRS=$(PWD) modules

clean:
	rm -rf *.o *.ko .tmp* .*cmd *.mod.c *.symvers

install:
	mkdir -p /lib/modules/`uname -r`/kernel/drivers/PT
	cp mirror.ko /lib/modules/`uname -r`/kernel/drivers/PT/
	depmod -a
