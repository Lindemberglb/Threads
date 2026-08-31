CC = gcc
PROGRAMA = mandelbrot

all: $(PROGRAMA)

$(PROGRAMA): mandelbrot.c
	$(CC) mandelbrot.c -o $(PROGRAMA) -fopenmp -pthread

clean:
	rm -f $(PROGRAMA)
