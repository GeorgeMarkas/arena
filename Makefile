TEST_DIR := tests

TARGET := $(TEST_DIR)/test

SRC := $(TEST_DIR)/tests.c

CC 	   := gcc
CFLAGS := -std=c11 -Wall -Wextra -Werror -Wno-unused-parameter -pedantic -fsanitize=address
LDLIBS := -lasan

.PHONY: clean

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $< -o $@
	$(TARGET)

clean:
	rm -f $(TEST_DIR)/test