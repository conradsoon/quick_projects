# Variables
CC = g++
CFLAGS = -std=c++20 -Wall -Wextra
SRC = quick_hashmap.cpp
OBJ = $(SRC:.cpp=.o)
OUT = quick_hashmap

# Rules
all: $(OUT)

$(OUT): $(OBJ)
	$(CC) $(CFLAGS) -o $(OUT) $(OBJ)

.cpp.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(OUT)
