TARGET = vm_riskxvii

CC = gcc

CFLAGS     = -c  -Os -fno-math-errno
# -Wall -Wvla
# -Wall -Wvla -Werror -O0 -g -std=c11
# ASAN_FLAGS = -fsanitize=address
SRC        = vm_riskxvii.c
OBJ        = $(SRC:.c=.o)

all:$(TARGET)

$(TARGET):$(OBJ)
	$(CC) $(ASAN_FLAGS) -o $@ $(OBJ)

.SUFFIXES: .c .o

.c.o:
	 $(CC) $(CFLAGS) $(ASAN_FLAGS) $<
	strip --strip-debug --strip-unneeded $(TARGET).o 
	

run:
	./$(TARGET)

test:
	echo what are we testing?!

clean:
	rm -f *.o *.obj $(TARGET)
