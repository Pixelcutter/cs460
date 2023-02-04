# compiles all programs with command 'make all'
all: cat sed uniq

# compiles my-cat program with command 'make cat'
cat: my-cat.o
	gcc -o my-cat my-cat.c -Wall -Werror
my-cat.o: my-cat.c
	gcc -c my-cat.c

# compiles my-sed programs with command 'make sed'
sed: my-sed.o
	gcc -o my-sed my-sed.c -Wall -Werror
my-sed.o: my-sed.c
	gcc -c my-sed.c

# compiles my-uniq programs with command 'make uniq'
uniq: my-uniq.o
	gcc -o my-uniq my-uniq.c -Wall -Werror
my-uniq.o: my-uniq.c
	gcc -c my-uniq.c

# cleans directory of all compiled programs and their object files
# leaves .c files
clean:
	rm my-cat my-sed my-uniq my-cat.o my-sed.o my-uniq.o