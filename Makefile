TARGET = vm_riskxvii

CC = gcc

CFLAGS     = -c -Wall -Wvla -Oz -s -fno-unwind-tables -fno-asynchronous-unwind-tables -fmerge-all-constants -fno-ident 
# -mno-riscv-attribute
# -m32
# -fno-math-errno -fno-unroll-loops -fno-exceptions 
# -nostartfiles -nodefaultlibs
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
	strip --strip-debug --strip-unneeded --remove-section=.comment --remove-section=.note $(TARGET).o 
# --strip-all

run:
	./$(TARGET)

run_test:
	bash run_tests.sh

clean:
	rm -f *.o *.obj $(TARGET)
