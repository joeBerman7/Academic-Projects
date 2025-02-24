OBJS    = encoder.o
SOURCE  = encoder.c
OUT     = encoder
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
	



