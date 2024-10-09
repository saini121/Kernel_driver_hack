obj-y += memory.o
obj-y += process.o
obj-y += entry.o
# Target for building the kernel module
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
