SRCS = Buf.c Disk.c TestCase.c FileSystem.c
OBJ = Buf.o Disk.o TestCase.o FileSystem.o
TARGET = FileSystem

$(TARGET) : $(OBJ)
	gcc -o $@ $^ -lm
	mkdir OBJ
	mv *.o OBJ

$(OBJ) : $(SRCS)
	gcc -c $^

clear	:
	rm -rf OBJ

clean	:
	rm -rf OBJ
