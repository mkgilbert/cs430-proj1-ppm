PROG="ppmrw"
INPUT="ppmrw.c"

all:
	gcc -O0 $(INPUT) -o $(PROG)

clean:
	rm -rf $(PROG)
