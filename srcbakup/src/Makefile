mobile-bench:worker.o mobile-bench.o call-back.o
	g++ worker.o mobile-bench.o call-back.o -DDEBUG -o mobile-bench

call-back.o:call-back.c
	g++ -c call-back.c -DDEBUG -o call-back.o

worker.o:worker.c 
	g++ -c worker.c -DDEBUG -o worker.o

mobile-bench.o:mobile-bench.c
	g++ -c mobile-bench.c -DDEBUG -o mobile-bench.o

