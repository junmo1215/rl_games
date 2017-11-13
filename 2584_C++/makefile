# all:
# 	g++ -std=c++0x -O3 -g -Wall -fmessage-length=0 -o 2584 2584.cpp 
# clean:
# 	rm 2584

all:
	g++ -std=c++0x -O3 -g -Wall -Wl,-stack_size,0x100000000 -fmessage-length=0 -o 2584sol 2584sol.cpp 
clean:
	rm 2584sol