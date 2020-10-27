CC = gcc
CFLAGS = -Wall
OBJS = http_get.o myrequest.o
TARGET = http_get

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

clean:
	$(RM) $(OBJS) $(TARGET)