BIN:=mqtt_sub
CFLAGS:=-lmosquitto -luci -lsqlite3
SOURCE=mqtt_sub.c mqtt_config.c
OBJECTS=$(SOURCE:.c=.o)

.PHONY: all clean

all: $(BIN)

$(BIN): $(OBJECTS)
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	rm -fr *o $(BIN)
