CXX = g++
LDFLAGS = -lm
CXXFLAGS = -O3 -std=c++11 -I./lib/json/include
TARGETS = converter

.PHONY: all
all: $(TARGETS)

%: %.cpp
	$(CXX) $(CXXFLAGS) $< -o $@ $(LDFLAGS)

.PHONY: clean
clean:
	rm -f $(TARGETS) $(TARGETS:=.o)
