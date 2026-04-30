CC 	   := gcc
CFLAGS := -std=c11 -Wall -Wextra -Werror -pedantic -fsanitize=address
LDLIBS := -lasan

.PHONY: clean

test: test.c
	$(CC) $(CFLAGS) $< -o $@
	./test

clean:
	rm -f test