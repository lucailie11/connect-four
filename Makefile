CXX      = g++
CXXFLAGS = -O2 -std=c++17

BOTS_SRC := $(wildcard bots/*.cpp)
BOTS_BIN := $(patsubst bots/%.cpp, bin/%, $(BOTS_SRC))

.PHONY: all clean

all: bin/make-move $(BOTS_BIN) sim

sim: simulator/sim.py
	@ printf '#!/bin/sh\nexec python3 "%s/simulator/sim.py" "$$@"\n' "$(CURDIR)" > sim
	@ chmod +x sim

bin/make-move: simulator/make-move.cpp | bin
	@ $(CXX) $(CXXFLAGS) $< -o $@
	@ echo '$@ compiled succesfully!!'

bin/%: bots/%.cpp | bin
	@ $(CXX) $(CXXFLAGS) $< -o $@
	@ echo '$@ compiled succesfully!!'

bin:
	@ mkdir -p $@

clean:
	@ rm -rf bin tmp sim
	@ echo 'Everything cleaned up!!'
