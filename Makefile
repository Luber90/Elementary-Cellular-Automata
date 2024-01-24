all:
	g++ main.cpp -lSDL2 -Wall -o main

prof:
	g++ main.cpp -lSDL2 -Wall -g -pg -o main

prof-img:
	gprof main gmon.out | gprof2dot -w | dot -Gdpi=200 -Tpng -o output.png

clean:
	rm main
	rm gmon.out