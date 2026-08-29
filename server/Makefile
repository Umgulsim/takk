CC ?= $(CROSS_COMPILE)gcc
CFLAGS ?= -Wall -Werror
TARGET ?= aesdsocket

all: $(TARGET)

$(TARGET): aesdsocket.o
	$(CC) $(CFLAGS) -o $(TARGET) aesdsocket.o

aesdsocket.o: aesdsocket.c
	$(CC) $(CFLAGS) -c aesdsocket.c

clean:
	rm -f $(TARGET) *.o

install: $(TARGET)
	install -d $(TARGET_DIR)/usr/bin
	install -m 0755 $(TARGET) $(TARGET_DIR)/usr/bin/