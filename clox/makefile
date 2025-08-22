CC = gcc
CFLAGS = -Wall -Wextra -O2

# Project settings
TARGET = main
SRC    = $(wildcard *.c)
OBJ    = $(SRC:.c=.o)

# Default rule
all: $(TARGET)

# Link objects into final binary
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

# Compile .c to .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean build files
clean:
	rm -f $(OBJ) $(TARGET)