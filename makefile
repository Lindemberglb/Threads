CC = gcc
PROGRAMA = mandelbrot

all: $(PROGRAMA)

$(PROGRAMA): mandelbrot.c
	$(CC) mandelbrot.c -o $(PROGRAMA) -fopenmp

clean:
	rm -f $(PROGRAMA)
