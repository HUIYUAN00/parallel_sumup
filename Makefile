CC = gcc
CFLAGS = -Wall -Wextra -O2
SRCDIR = src
TARGET = matrix_sum

SRCS = $(SRCDIR)/matrix_sum.c $(SRCDIR)/main.c
OBJS = $(SRCS:.c=.o)

.PHONY: all clean run test

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(SRCDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS)

run: $(TARGET)
	./$(TARGET)

test: $(TARGET)
	./$(TARGET) 512 512