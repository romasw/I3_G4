CC = gcc
TARGET = i1i2i3_phone
OBJS = src/main.o src/call_thread.o src/wait_thread.o src/setup.o src/talk_session.o

$(TARGET): $(OBJS)
	$(CC) -I./include -o bin/$@ $(OBJS)

src/main.o: src/main.c
	$(CC) -I./include -c -o src/main.o src/main.c

src/call_thread.o: src/call_thread.c
	$(CC) -I./include -c -o src/call_thread.o src/call_thread.c

src/wait_thread.o: src/wait_thread.c
	$(CC) -I./include -c -o src/wait_thread.o src/wait_thread.c

src/setup.o: src/setup.c
	$(CC) -I./include -c -o src/setup.o src/setup.c

src/talk_session.o: src/talk_session.c band_shift.o 
	$(CC) -I./include -c -o src/talk_session.o src/talk_session.c

src/band_shift.o: src/band_shift.c 
	$(CC) -I./include -c -o src/band_shift.o src/band_shift.c

.PHONY: clean
clean:
	rm src/*.o bin/*
