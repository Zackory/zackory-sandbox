
# g++ -O3 -shared -fPIC -std=c++11 -I /usr/local/include/python2.7 `python-config --cflags --ldflags` pybindings.cpp -o pybindings.so

CXX = g++

CXXFLAGS = -O3 -shared -fPIC -std=c++11 -I/usr/local/include/python2.7 `python-config --cflags`
LDFLAGS = `python-config --ldflags`

all: pybindings

pybindings: pybindings.cpp
	$(CXX) $(CXXFLAGS) $(LDFLAGS) pybindings.cpp -o pybindings.so

