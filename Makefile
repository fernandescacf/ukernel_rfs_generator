CXX = g++-7 -m32
CXXFLAGS = $(MODE) -pedantic-errors -Wall -Wextra -std=c++17
LIBS = -lpthread -lstdc++fs

export CXX
export CXXFLAGS
export EXEC

all: build

build:
	$(CXX) $(CXXFLAGS) main.cpp scanner.cpp -I. -o $(EXEC) $(LIBS)