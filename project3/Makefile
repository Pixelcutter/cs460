OPTIONS = -Wall -Werror
COMPILER = gcc
OUT = BackItUp 

$(OUT) : backitup.o
	$(COMPILER) backitup.o -o $(OUT)

backitup.o : backitup.c
	$(COMPILER) backitup.c -c $(OPTIONS) -pthread

clean :
	rm *.o $(OUT)

run :
	./$(OUT)