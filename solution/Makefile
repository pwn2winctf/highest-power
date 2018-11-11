all: solve makeflag

solve: solve.c
	gcc -fopenmp -Wall -Ofast $< -o $@

makeflag: makeflag.c
	gcc -Wall -Ofast $< -o $@

clean:
	rm -f solve makeflag
