include Make_linux.inc
#include Make_msys2.inc
#include Make_osx.inc

CXXFLAGS = -std=c++11 -fPIC
ifdef DEBUG
CXXFLAGS += -g -O0 -Wall -fbounds-check -pedantic -D_GLIBCXX_DEBUG
else
CXXFLAGS += -O3 -march=native -Wall
endif

ALL=matvec_ParalleleRow.exe Mandelbrot_Parallele.exe matvec_ParalleleCol.exe

default:	help

all: $(ALL)

clean:
	@rm -fr *.o *.exe *~ *.tga

%.exe: %.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIB)


help:
	@echo "Available targets : "
	@echo "    all            : compile all executables"
	@echo "    matvec_ParalleleRow.exe     : compile matrix vector product executable"
	@echo "    Mandelbrot_Parallele.exe : compile Mandelbrot set computation executable"
	@echo "Add DEBUG=yes to compile in debug"
	@echo "Configuration :"
	@echo "    CXX      :    $(CXX)"
	@echo "    CXXFLAGS :    $(CXXFLAGS)"
