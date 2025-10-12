# Compiler settings
CXX := g++
CXXFLAGS := -std=c++20 -Wall -Wextra -O3 -I./warp_log

# Platform detection
ifeq ($(OS),Windows_NT)
    SHARED_EXT := dll
    IMPORT_LIB := libwarp_log.dll.a
    SHARED_FLAGS := -shared -Wl,--out-implib,$(IMPORT_LIB)
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Linux)
        SHARED_EXT := so
        SHARED_FLAGS := -fPIC -shared
    endif
endif

AR := ar
ARFLAGS := rcs

# Source files
SRC := $(wildcard warp_log/*.cpp)
OBJ := $(SRC:.cpp=.o)

# Targets
.PHONY: all help warp_log_static warp_log_shared

all: warp_log_static warp_log_shared

# Static library
warp_log_static: $(OBJ)
	$(AR) $(ARFLAGS) libwarp_log.a $(OBJ)
	@echo "Built static library: libwarp_log.a"

# Shared library
warp_log_shared: $(OBJ)
	$(CXX) $(CXXFLAGS) $(SHARED_FLAGS) -o libwarp_log.$(SHARED_EXT) $(OBJ)
	@echo "Built shared library: libwarp_log.$(SHARED_EXT)"
ifeq ($(OS),Windows_NT)
	@echo "Import library: $(IMPORT_LIB)"
endif

# Compile .cpp -> .o
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Help
help:
	@echo "make all                 Build both static and shared libraries"
	@echo "make warp_log_static     Build static library only"
	@echo "make warp_log_shared     Build shared library only"