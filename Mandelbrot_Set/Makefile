CC = gcc
CXX = g++
LDLIBS = -lpng -ldl
CFLAGS = -lm -O3
pra_pthread: CFLAGS += -pthread
pra_hybrid: CC = mpicc
pra_hybrid: CXX = mpicxx
pra_hybrid: CFLAGS += -fopenmp
CXXFLAGS = $(CFLAGS)
TARGETS = seq pra_pthread pra_hybrid

.PHONY: all
all: $(TARGETS)

.PHONY: clean
clean:
	rm -f $(TARGETS) $(TARGETS:=.o)
