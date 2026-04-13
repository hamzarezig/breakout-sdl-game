CC = gcc

all: main.c
	$(CC) main.c -o main -l SDL3 -l SDL3_ttf 
run: main.c
	$(CC) main.c -o main -l SDL3 -l SDL3_ttf
	./main
clean: main
	rm main 
