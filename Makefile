TARGET = cssl
LIBS = -lm
CC = gcc
CFLAGS = -g -mavx -Wall -O3 -std=gnu99

default: $(TARGET)
all: default

OBJECTS = $(patsubst %.c, %.o, $(wildcard *.c))
HEADERS = $(wildcard *.h)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

.PRECIOUS: $(TARGET) $(OBJECTS)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -Wall $(LIBS) -o $@
	
ycsb:
	$(CC) $(CFLAGS) $(LIBS) skiplist.h skiplist.c ycsb.c -o ycsb

clean:
	-rm -f *.o
	-rm -f $(TARGET)
	-rm -f ycsb
