CC = g++
CFLAGS = -std=c++23 -Wall -Wextra -fsanitize=address,undefined -g
TARGET = huffman
SRC = main.cpp huffman.cpp

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

test: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)
