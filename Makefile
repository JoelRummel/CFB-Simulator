compile: main.cpp
	g++ -std=c++17 -g main.cpp -o main

test: testmain.cpp
	g++ -std=c++17 -g testmain.cpp -o testmain

clean:
	rm -f main
	rm -f testmain