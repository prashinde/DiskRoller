obj-m +=intercept.o

INC1  = $(TOP_DIR)/diskroller/ 
INC2  = $(TOP_DIR)/logger
INCFLAGS = $(addprefix -I,$(INC1))
INCFLAGS += $(addprefix -I,$(INC2))

KBUILD_EXTRA_SYMBOLS = $(TOP_DIR)/bitmapdriver/Module.symvers
EXTRA_CFLAGS := $(INCFLAGS)

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
