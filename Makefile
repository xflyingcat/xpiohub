OS = WINDOWS64
ifeq ($(OS), WINDOWS64)
CC = gcc
INC = -ISDK/CHeaders/XPLM
CFLAGS = -Wall -O2 -D$(OS) $(INC)
CFLAGS += -MD -MP -MT $(*F).o -MF .dep/$(@F).d
LDFLAGS =
LDDYNFLAGS = -shared
LIBS = SDK/Libraries/Win/XPLM_64.lib
TARGET = xpiohub.xpl
endif

SOURCE= logger.c ini.c config.c serial.c misc_utils.c devside.c simside.c
#SOURCE= simside.c
        
OBJS = $(SOURCE:.c=.o)

all: xpio_win.xpl copy

xpio_win.xpl: $(OBJS)
	$(CC) $(LDFLAGS) $(LDDYNFLAGS) $(OBJS) -o xpio_win.xpl $(LIBS)
	cp xpio_win.xpl $(TARGET)

copy:
ifeq ($(OS), WINDOWS64)
	cp $(TARGET)  d:/X-Plane1030/Resources/plugins
endif

%.o: %.c Makefile
	$(CC) $(CFLAGS) -c $<

clean:
	-rm -f $(OBJS)
	-rm -f *.xpl
	-rm -f -r .dep
	-mkdir .dep

# Include the dependency files.
-include $(wildcard .dep/*)
