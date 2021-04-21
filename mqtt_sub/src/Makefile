BIN:=mqtt_sub
LDFLAGS:=-lmosquitto -luci -lsqlite3 -ljson-c -lcurl
CFLAGS:=-g -Wall
SOURCE:=$(wildcard *.c)
OBJECTS=$(SOURCE:.c=.o)
.PHONY: all clean

all: $(BIN)

$(BIN): $(OBJECTS)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

clean:
	rm -fr $(OBJECTS) $(BIN)
