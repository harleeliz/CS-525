CC = gcc
CFLAGS = -g -Iinclude

SRC = \
    src/record_mgr.c \
    src/tables.c \
    src/rm_serializer.c \
    src/dberror.c \
    src/buffer_mgr.c \
    src/buffer_mgr_stat.c \
    src/expr.c \
    src/storage_mgr.c \
    src/test_assign3_1.c

OBJ = $(SRC:.c=.o)

all: record_manager_test

record_manager_test: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	del /F /Q src\*.o record_manager_test 2>nul || true
