# Define variables
OBJS    = menu_map.o
SOURCE  = menu_map.c
OUT     = menu
FLAGS   = -g -Wall
CC      = gcc

# Default target
all: $(OUT)

# Rule to create the final executable
$(OUT): $(OBJS)
	$(CC) -g -m32 $(OBJS) -o $(OUT)

# Rule to compile each source file into object files
%.o: %.c
	$(CC) $(FLAGS) -m32 -c $< -o $@

# Clean the build directory
clean:
	rm -f $(OBJS) $(OUT)

# Run the program
run: $(OUT)
	./$(OUT)
