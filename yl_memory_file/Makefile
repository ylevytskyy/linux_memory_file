TARGET_MODULE:=simple-module

CONFIG_MODULE_SIG=n
CONFIG_MODULE_SIG_ALL=n

# If we are running by kernel building system
ifneq ($(KERNELRELEASE),)
	$(TARGET_MODULE)-objs := main.o device_file.o yl_memory_file.o
	obj-m := $(TARGET_MODULE).o

# If we running without kernel build system
else
	BUILDSYSTEM_DIR:=/lib/modules/$(shell uname -r)/build
	PWD:=$(shell pwd)

#echo $(PWD)
#echo $(M)
all : 
# run kernel build system to make module
#@echo
#@echo $(PWD)
#@echo $(BUILDSYSTEM_DIR)
#@echo
	
	$(MAKE) -C $(BUILDSYSTEM_DIR) M=$(PWD) modules

clean:
# run kernel build system to cleanup in current directory
	$(MAKE) -C $(BUILDSYSTEM_DIR) M=$(PWD) clean

load:
	insmod ./$(TARGET_MODULE).ko

unload:
	rmmod ./$(TARGET_MODULE).ko

endif
