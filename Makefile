PROG="ppmrw"
INPUT="ppmrw.c"

all:
	gcc $(INPUT) -o $(PROG)

clean:
	rm -rf $(PROG)
