CONTIKI=../../../../../../../

TARGET = mikro-e
CONTIKI_WITH_IPV6 = 1
CONTIKI_WITH_RPL = 0
USE_CA8210 = 1
VERSION? = $(shell git describe --abbrev=4 --dirty --always --tags)

APPS += letmecreateiot mqtt

CONTIKI_SOURCEFILES += mcp3004.c ../fsm.c

CFLAGS += -DDEBUG_IP=fe80:0000:0000:0000:0019:f5ff:fe89:1e32
CFLAGS += -DVERSION=$(VERSION)
CFLAGS += -Wall -Wno-pointer-sign
CFLAGS += -I $(CONTIKI)/platform/$(TARGET)
CFLAGS += -I ..
CFLAGS += -fno-short-double
LDFLAGS += -Wl,--defsym,_min_heap_size=32000

SMALL=0

all: main
	xc32-bin2hex main.$(TARGET)

include $(CONTIKI)/Makefile.include
