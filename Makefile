# use bash as shell backend
SHELL=bash

# compiler
CC = gcc
CXX = g++


# flags
CFLAGS = -O3 -std=gnu99
CXXFLAGS = -O3 -std=c++11
LDFLAGS = -I./include/

# sources
CSOURCES = sha256.c
CXXSOURCES = 


# directory
SRC_DIR = src
OBJ_DIR = obj
INCLUDE_DIR = include

# modules
MODULES = btc json-parser tcp
MODULE_DIR = $(addprefix $(SRC_DIR)/,$(MODULES))


SRC_FILES := $(shell find $(SRC_DIR) -maxdepth 2 -name '*.cpp')\
			 $(shell find $(SRC_DIR) -maxdepth 2 -name '*.c')

HEADER_FILES := $(shell find $(SRC_DIR) -name '*.hpp')\
				$(shell find $(SRC_DIR) -name '*.h')


OBJ_FILES := $(SRC_FILES:%.c=%.o)
OBJ_FILES := $(OBJ_FILES:%.cpp=%.o)
OBJ_FILES := $(addprefix $(OBJ_DIR)/, $(notdir $(OBJ_FILES)))

# target & executable
EXECUTABLE = main
CXXTARGETS = example001 example002
HEADER_TARGETS := $(foreach hdr,$(HEADER_FILES),$(patsubst $(SRC_DIR)/%,$(INCLUDE_DIR)/%,$(hdr)))

SRC_PATH = $(SRC_DIR):$(MODULE_DIR)

vpath %.c $(SRC_PATH)
vpath %.cpp $(SRC_PATH)

#============== RULE ==============

#====== target: all ======
.PHONY: all
all: $(HEADER_TARGETS) $(EXECUTABLE) $(CXXTARGETS)
	@echo "[info] Makefile complete"

.PHONY: debug
debug: LDFLAGS+=-D_DEBUG_
debug: all	

#===== Create include directory ====

$(INCLUDE_DIR)/%.h: $(SRC_DIR)/%.h
	@mkdir -p $$(dirname $@) && cp -v $< $@ ;

$(INCLUDE_DIR)/%.hpp: $(SRC_DIR)/%.hpp
	@mkdir -p $$(dirname $@) && cp -v $< $@ ; 

#====== Build Executable File =====

$(EXECUTABLE): $(EXECUTABLE:=.cpp) $(OBJ_FILES)
	@echo "[info] Building executable file"
	$(CXX) $(CXXFLAGS) -o $@ $< $(addprefix $(OBJ_DIR)/,$(notdir $(OBJ_FILES))) $(LDFLAGS) $(CSOURCES) $(CXXSOURCES)


#====== Build Target File =====

$(CXXTARGETS): %: %.cpp
	@echo "[info] Building target $<"
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS) $(CSOURCES) $(CXXSOURCES)

#====== Build Object File =====

$(OBJ_DIR)/%.o: %.cpp $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@ $(LDFLAGS)

$(OBJ_DIR)/%.o: %.c $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@ $(LDFLAGS)

$(OBJ_DIR):
	@mkdir -p $(OBJ_DIR)


#======= Clean =========

.PHONY: clean
clean:
	rm -rf $(EXECUTABLE)
	rm -rf $(CXXTARGETS) $(CXXTARGETS:=.o) 
	rm -rf $(OBJ_DIR)
	rm -rf $(CSOURCES:.c=.o) $(CXXSOURCES:.cpp=.o)


