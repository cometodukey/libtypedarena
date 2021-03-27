CFLAGS += -I. -Wall -Wextra -Wpedantic -Werror -std=gnu11
BIN := example

SRCS = examples/example.c

$(BIN):
	$(CC) $(CFLAGS) -o $@ $(SRCS)

.PHONY: clean
clean:
	rm -f $(BIN)
