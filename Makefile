	CC = gcc
CFLAGS = -Wall -Wextra -O2

TARGET = bytevault

SRCS = main.c stego.c
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS) hidden.bmp extracted.txt secret.txt