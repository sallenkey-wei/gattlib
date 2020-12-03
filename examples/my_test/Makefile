CC 		:= gcc
CFLAGS 		:= 
LDFLAGS 	:= -lgattlib -L ~/workspace/gattlib/build/dbus -Wl,-rpath ~/workspace/gattlib/build/dbus \
		   -lglib-2.0	\
		   -lpthread

INCDIRS 	:= 	. \
			~/workspace/gattlib/include \
			/usr/include/glib-2.0	\
			/usr/lib/arm-linux-gnueabihf/glib-2.0/include

SRCDIRS 	:= 	.

CFLAGS 		+= $(patsubst %, -I %, $(INCDIRS))
SRCS 		:= $(foreach dir, $(SRCDIRS), $(wildcard $(dir)/*.c))
OBJS 		:= $(patsubst %.c, %.o, $(SRCS))

TARGET 		:= login


build : $(TARGET)

$(TARGET) : $(OBJS)
	$(CC) -o $@ $^  $(LDFLAGS)

%.o : %.c
	$(CC) -o $@ -c $< $(CFLAGS)

.PHONY += clean

clean:
	rm -rf $(OBJS) $(TARGET)

print:
	@echo SRCS = $(SRCS)
	@echo OBJS = $(OBJS)
