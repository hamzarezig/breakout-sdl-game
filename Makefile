CC = gcc

all: main.c
	$(CC) main.c -o main -lSDL3
run: main
	$(CC) main.c -o main -lSDL3
	./main
clean:
	rm main 
