# CS430 Project 1 - Converting ppm files #

This project is an introduction to image file types--specifically the intermediary
file type `*.ppm`.

The resulting binary from running `make` converts between P3 (ascii) and P6 (rawbits)
variants of the ppm file format.

## Compilation ##
Run `make` from the root directory to build the binary. This will generate a binary called `ppmrw`.

To rebuild, run `make clean` to remove the binary and any other temporary output files, then run `make` again.

## Usage ##
```
./ppmrw TYPE <input.ppm> <output.ppm>
```

The arguments are positional and must be presented in the order specified above.

**TYPE**
    - specifies the ppm format you want the *output* file to be
    - There are only two options: 3 or 6.

## Example ##
To convert the file `image.ppm` to the P6 format and store the result in another file called `image_p6.ppm`:

`./ppmrw 6 image.ppm image_p6.ppm`


