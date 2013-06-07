debug:=-DDEBUG
#debug:=
mobile-bench:worker.o mobile-bench.o call-back.o
	g++ worker.o mobile-bench.o call-back.o $(debug) -o mobile-bench

call-back.o:call-back.c
	g++ -c call-back.c $(debug) -o call-back.o

worker.o:worker.c 
	g++ -c worker.c $(debug) -o worker.o

mobile-bench.o:mobile-bench.c
	g++ -c mobile-bench.c $(debug) -o mobile-bench.o

all:
	rm *.o mobile-bench
