# Compiler and flags
CC = gcc
CFLAGS = -Wall -g  # Show all warnings and include debug symbols

# Object files required for compilation
OBJS = buffer_mgr.o buffer_mgr_stat.o dberror.o storage_mgr.o

# Default target builds both test cases
all: test_assign2_1 test_assign2_2

# Build test_assign2_1
test_assign2_1: test_assign2_1.o $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# Build test_assign2_2
test_assign2_2: test_assign2_2.o $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# Compile storage_mgr.c if it exists
storage_mgr.o: storage_mgr.c storage_mgr.h
	$(CC) $(CFLAGS) -c storage_mgr.c -o storage_mgr.o

# Generic rule for compiling .c files into .o object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up object files and executables
clean:
	rm -f *.o test_assign2_1 test_assign2_2
