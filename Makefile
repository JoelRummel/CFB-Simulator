compile: main.cpp
	g++ -std=c++17 -g main.cpp -o main

run :
	[ ! -f main ] && make compile; \
	./main

playground: testmain.cpp
	g++ -std=c++17 -g testmain.cpp -o testmain

test: 
	g++ -I googletest/googletest/include -std=c++17 -o test tests/testMain.cpp googletest/build/lib/libgtest.a -lpthread
	./test

.PHONY: all test clean

clean:
	rm -f main
	rm -f testmain
	rm -f test