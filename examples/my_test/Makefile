CC 		:= gcc
CFLAGS 		:= -I ~/workspace/gattlib/include/
LDFLAGS 	:= -lgattlib -L ~/workspace/gattlib/build/bluez -Wl,-rpath ~/workspace/gattlib/build/bluez

INCDIRS 	:= ./
SRCDIRS 	:= ./

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
