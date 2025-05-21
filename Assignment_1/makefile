# Compiler to be used
CC = gcc

# Compiler flags: -Wall enables warnings, -g enables debugging
CFLAGS = -Wall -g

# Object files needed for the executable
OBJ = dberror.o storage_mgr.o test_assign1_1.o

# The default target (entry point for `make` command)
all: test_assign1

# Rule to create the executable test_assign1
# $@ represents the target name (test_assign1)
# $^ represents all the dependencies (object files)
test_assign1: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

# Rule to compile C source files into object files (.o)
# $< represents the first prerequisite (a .c file)
%.o: %.c
	$(CC) $(CFLAGS) -c $<

# Clean up generated files
clean:
	# Remove compiled binary and object files
	rm -f test_assign1 *.o