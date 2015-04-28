SRC = $(wildcard *.c)
OBJ = $(patsubst %.c, %.o, $(SRC))
OPT = -lfuse -D_FILE_OFFSET_BITS=64
TARGET = tinyfs

$(TARGET) : $(OBJ)
	$(CC) -o $(TARGET) $(OBJ) $(OPT)
$(OBJ) : $(SRC)
	$(CC) -c $(SRC) $(OPT)

clean :
	$(RM) -rf $(OBJ) $(TARGET)
