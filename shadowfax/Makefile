# the compiler: gcc for C program, define as g++ for C++
CC = gcc
CFLAGS  = -g
INCLUDEDIR= -I$(TOP_DIR)/diskroller

# the build target executable:
TARGET = ioctls

all: $(TARGET)

$(TARGET): $(TARGET).c
	$(CC) $(CFLAGS) $(INCLUDEDIR) -o $(TARGET) $(TARGET).c
clean:
	$(RM) $(TARGET)
