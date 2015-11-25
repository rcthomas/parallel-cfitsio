
TARGET=parallel-cfitsio-read

# C++11.

CXX=CC
CXXFLAGS+=-std=c++11

# CFITSIO.

INCLUDE+=-I$(CFITSIO_DIR)/include
LDFLAGS+=-L$(CFITSIO_DIR)/lib
LIBS+=-lcfitsio

all: $(TARGET)

$(TARGET): $(TARGET).o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(OBJ) $@.o -o $@ $(LIBS)

.cc.o :
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

clean:
	rm -rf *.o

realclean: clean
	rm -rf $(TARGET)

