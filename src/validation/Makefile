
INC_PATHS=-I../../include
LIB_PATHS=-L. -L../..
CXX=g++ 
CXXFLAGS=-fPIC -std=c++11 $(INC_PATHS) $(LIB_PATHS)

all: ../../validate

../../validate: validate.cpp
	$(CXX) $(CXXFLAGS) -L. -lIREX10 validate.cpp -o $@

clean:
	rm -f ../../validate

