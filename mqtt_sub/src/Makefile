BIN:=mqtt_sub
CFLAGS:=-lmosquitto -luci

.PHONY: all clean

all: $(BIN)

$(BIN): mqtt_sub.c
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	rm -f $(BIN)
