obj-m += tracker.o

INCDIRS  = $(TOP_DIR)/logger
INCFLAGS= $(addprefix -I,$(INCDIRS))
EXTRA_CFLAGS := $(INCFLAGS)

tracker-objs := bitdriver.o ioctlhandler.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
