PROG="ppmrw"
INPUT="ppmrw.c"

all:
	gcc -O0 -g $(INPUT) -o $(PROG)

clean:
	rm -rf $(PROG)
