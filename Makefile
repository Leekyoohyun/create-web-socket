CC = gcc
CFLAGS = -Wall -Wextra -Werror
TARGET = myserver
SRC = server.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)
