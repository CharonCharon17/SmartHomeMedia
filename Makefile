CC = arm-linux-gcc
CFLAGS = -Wall -O2 -lm
TARGET = shm
SRC = src/main.c src/menu.c src/music.c src/photo.c src/video.c src/bmp.c src/touch.c src/game.c
OBJ = $(SRC:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJ) $(TARGET)

install:
	cp $(TARGET) /mnt/udisk/shixun/shm/