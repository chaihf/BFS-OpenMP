defule:
	g++ -fopenmp -msse2 -msse3 -lm -g -O2 -o bfs main.cpp bfs.cpp graph.cpp 
test:
	./bfs graphs/tiny.graph
clean:
	rm -rf bfs *.o *~ *.*~
