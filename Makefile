CXX      = g++
CXXFLAGS = -O2 -std=c++17

BOTS_SRC := $(wildcard bots/*.cpp)
BOTS_BIN := $(patsubst bots/%.cpp, bin/%, $(BOTS_SRC))

.PHONY: all clean

all: bin/make-move $(BOTS_BIN) sim

sim: simulator/sim.py
	@printf '#!/bin/sh\nexec python3 "%s/simulator/sim.py" "$$@"\n' "$(CURDIR)" > sim
	@chmod +x sim

bin/make-move: simulator/make-move.cpp | bin tmp
	$(CXX) $(CXXFLAGS) $< -o $@

bin/%: bots/%.cpp | bin
	$(CXX) $(CXXFLAGS) $< -o $@

bin tmp:
	mkdir -p $@

clean:
	rm -f bin/make-move $(BOTS_BIN) sim
	rm -f tmp/*.txt tmp/*.in
