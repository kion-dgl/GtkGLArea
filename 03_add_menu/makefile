all:
	gcc `pkg-config --cflags gtk+-3.0` main.c -lGL -lGLEW `pkg-config --libs gtk+-3.0`

clean:
	rm ./example

run:
	./example
