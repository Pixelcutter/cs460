FLAGS = -Wall -Werror
CC = gcc
OBJS = main.o utilFuncs.o io.o parser.o printStats.o rr.o sjf.o fcfs.o pr.o
OUT = main

main : $(OBJS)
	$(CC) $(OBJS) -o $(OUT)

main.o : main.c h_files/*.h
	$(CC) main.c -c $(FLAGS)

parser.o : ./c_files/parser.c ./h_files/global.h ./h_files/utilFuncs.h
	$(CC) ./c_files/parser.c -c $(FLAGS)

io.o : ./c_files/io.c ./h_files/global.h ./h_files/utilFuncs.h
	$(CC) ./c_files/io.c -c $(FLAGS)

utilFuncs.o : ./c_files/utilFuncs.c ./h_files/global.h
	$(CC) ./c_files/utilFuncs.c -c $(FLAGS)

printStats.o : ./c_files/printStats.c ./h_files/global.h
	$(CC) ./c_files/printStats.c -c $(FLAGS)

sjf.o : ./c_files/sjf.c ./h_files/global.h ./h_files/utilFuncs.h
	$(CC) ./c_files/sjf.c -c $(FLAGS)

fcfs.o : ./c_files/fcfs.c ./h_files/global.h ./h_files/utilFuncs.h
	$(CC) ./c_files/fcfs.c -c $(FLAGS)

pr.o : ./c_files/pr.c ./h_files/global.h ./h_files/utilFuncs.h
	$(CC) ./c_files/pr.c -c $(FLAGS)

rr.o : ./c_files/rr.c ./h_files/global.h ./h_files/utilFuncs.h
	$(CC) ./c_files/rr.c -c $(FLAGS)

clean :
	rm $(OBJS) $(OUT)

run :
	./main