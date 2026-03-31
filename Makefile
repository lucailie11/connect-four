CXX      = g++
CXXFLAGS = -O2 -std=c++17

BOTS_SRC := $(wildcard bots/*.cpp)
BOTS_BIN := $(patsubst bots/%.cpp, bin/%, $(BOTS_SRC))

.PHONY: all clean run

all: bin/make-move $(BOTS_BIN)

bin/make-move: simulator/make-move.cpp | bin tmp
	$(CXX) $(CXXFLAGS) $< -o $@

bin/%: bots/%.cpp | bin
	$(CXX) $(CXXFLAGS) $< -o $@

bin tmp:
	mkdir -p $@

run: all
	python3 simulator/sim.py

clean:
	rm -f bin/make-move $(BOTS_BIN)
	rm -f tmp/*.txt tmp/*.in
