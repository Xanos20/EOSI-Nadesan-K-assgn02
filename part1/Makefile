
#KDIR:=/usr/src/linux-headers-4.15.0-76-generic

#CC = gcc

#CROSS_COMPILE =

#PWD:= $(shell pwd)

#ARCH = x86

#SROOT=/opt/iot-devkit/1.7.2/sysroots/i586-poky-linux/

#EXTRA_CFLAGS += -g -Wall -O0

#APP=main

#obj-m= Mydriver.o

#all:
#	make ARCH=x86 CROSS_COMPILE=$(CROSS_COMPILE) -C $(KDIR) M=$(PWD) modules
#	$(CC) -Wall -o $(APP) main.c -lpthread

#clean:
#	make -C $(KDIR) M=$(PWD) clean










FLAGS=-std=c99
EXTRA_CFLAGS += -g -Wall -O0 
CC = i586-poky-linux-gcc
ARCH = x86
CROSS_COMPILE = i586-poky-linux-
SDKTARGETSYSROOT=/opt/iot-devkit/1.7.2/sysroots/i586-poky-linux
export PATH:=/opt/iot-devkit/1.7.2/sysroots/x86_64-pokysdk-linux/usr/bin:/opt/iot-devkit/1.7.2/sysroots/x86_64-pokysdk-linux/usr/bin/i586-poky-linux:$(PATH)

APP = main

obj-m:= Mydriver.o

all:
	make ARCH=x86 FLAGS=-std=gnu99 CROSS_COMPILE=i586-poky-linux- -C $(SDKTARGETSYSROOT)/usr/src/kernel M=$(PWD) modules
	$(CC) -std=c99 -o $(APP) main.c -lpthread --sysroot=$(SDKTARGETSYSROOT)

clean:
	rm -f *.ko
	rm -f *.o
	rm -f Module.symvers
	rm -f modules.order
	rm -f *.mod.c
	rm -rf .tmp_versions
	rm -f *.mod.c
	rm -f *.mod.o
	rm -f \.*.cmd
	rm -f Module.markers
	rm -f $(APP) 







# To run "make", you can either set up environment variables via
#		source /opt/iot-devkit/1.7.2/environment-setup-i586-poky-linux
# or set up the following make variables
#




#EXTRA_CFLAGS += -g -Wall -O0 
#CC = i586-poky-linux-gcc
#ARCH = x86
#CROSS_COMPILE = i586-poky-linux-
#SDKTARGETSYSROOT=/opt/iot-devkit/1.7.2/sysroots/i586-poky-linux
#export PATH:=/opt/iot-devkit/1.7.2/sysroots/x86_64-pokysdk-linux/usr/bin:/opt/iot-devkit/1.7.2/sysroots/x86_64-pokysdk-linux/usr/bin/i586-poky-linux:$(PATH)

#APP = main

#obj-m:= Mydriver.o

#all:
#	make ARCH=x86 CROSS_COMPILE=i586-poky-linux- -C $(SDKTARGETSYSROOT)/usr/src/kernel M=$(PWD) modules
#	$(CC) -std=c99 -o $(APP) main.c -lpthread --sysroot=$(SDKTARGETSYSROOT)

#clean:
#	rm -f *.ko
#	rm -f *.o
#	rm -f Module.symvers
#	rm -f modules.order
#	rm -f *.mod.c
#	rm -rf .tmp_versions
#	rm -f *.mod.c
#	rm -f *.mod.o
#	rm -f \.*.cmd
#	rm -f Module.markers
#	rm -f $(APP) 










































#EXTRA_CFLAGS += -g -Wall -O0 
#CC = i586-poky-linux-gcc
#ARCH = x86
#CROSS_COMPILE = i586-poky-linux-
#SDKTARGETSYSROOT=/opt/iot-devkit/1.7.2/sysroots/i586-poky-linux
#export PATH:=/opt/iot-devkit/1.7.2/sysroots/x86_64-pokysdk-linux/usr/bin:/opt/iot-devkit/1.7.2/sysroots/x86_64-pokysdk-linux/usr/bin/i586-poky-linux:$(PATH)

#APP = kbuf_tester

#obj-m:= Mydriver.o

#all:
#	make ARCH=x86 CROSS_COMPILE=i586-poky-linux- -C $(SDKTARGETSYSROOT)/usr/src/kernel M=$(PWD) modules
#	$(CC) -o $(APP) main.c -lpthread --sysroot=$(SDKTARGETSYSROOT)

#clean:
#	rm -f *.ko
#	rm -f *.o
#	rm -f Module.symvers
#	rm -f modules.order
#	rm -f *.mod.c
#	rm -rf .tmp_versions
#	rm -f *.mod.c
#	rm -f *.mod.o
#	rm -f \.*.cmd
#	rm -f Module.markers
#	rm -f $(APP) 

