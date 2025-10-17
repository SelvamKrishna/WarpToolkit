# Compiler settings
CXX := g++
CXXFLAGS := -std=c++20 -Wall -Wextra -O3 -I./warp_log -I./warp_timer

# Platform detection
ifeq ($(OS),Windows_NT)
    SHARED_EXT := dll
    IMPORT_LIB_LOG := libwarp_log.dll.a
    IMPORT_LIB_TIMER := libwarp_timer.dll.a
    IMPORT_LIB_TOOLKIT := libwarp_toolkit.dll.a
    SHARED_FLAGS := -shared
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
LOG_SRC := $(wildcard warp_log/*.cpp)
TIMER_SRC := $(wildcard warp_timer/*.cpp)
LOG_OBJ := $(LOG_SRC:.cpp=.o)
TIMER_OBJ := $(TIMER_SRC:.cpp=.o)
COMBINED_OBJ := $(LOG_OBJ) $(TIMER_OBJ)

# Targets
.PHONY: all help warp_log_static warp_log_shared warp_timer_static warp_timer_shared warp_toolkit_static warp_toolkit_shared

all: warp_log_static warp_log_shared warp_timer_static warp_timer_shared warp_toolkit_static warp_toolkit_shared

# Static libraries
warp_log_static: $(LOG_OBJ)
	$(AR) $(ARFLAGS) libwarp_log.a $^
	@echo "Built static library: libwarp_log.a"

warp_timer_static: $(TIMER_OBJ)
	$(AR) $(ARFLAGS) libwarp_timer.a $^
	@echo "Built static library: libwarp_timer.a"

warp_toolkit_static: $(COMBINED_OBJ)
	$(AR) $(ARFLAGS) libwarp_toolkit.a $^
	@echo "Built static library: libwarp_toolkit.a"

# Shared libraries
warp_log_shared: $(LOG_OBJ)
	$(CXX) $(CXXFLAGS) $(SHARED_FLAGS) -o libwarp_log.$(SHARED_EXT) $^
	@echo "Built shared library: libwarp_log.$(SHARED_EXT)"

warp_timer_shared: $(TIMER_OBJ)
	$(CXX) $(CXXFLAGS) $(SHARED_FLAGS) -o libwarp_timer.$(SHARED_EXT) $^
	@echo "Built shared library: libwarp_timer.$(SHARED_EXT)"

warp_toolkit_shared: $(COMBINED_OBJ)
	$(CXX) $(CXXFLAGS) $(SHARED_FLAGS) -o libwarp_toolkit.$(SHARED_EXT) $^
	@echo "Built shared library: libwarp_toolkit.$(SHARED_EXT)"

# Compile .cpp -> .o
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Help
help:
	@echo "make all                    Build all static and shared libraries"
	@echo "make warp_log_static        Build Warp Log static library only"
	@echo "make warp_log_shared        Build Warp Log shared library only"
	@echo "make warp_timer_static      Build Warp Timer static library only"
	@echo "make warp_timer_shared      Build Warp Timer shared library only"
	@echo "make warp_toolkit_static    Build combined Warp Toolkit static library"
	@echo "make warp_toolkit_shared    Build combined Warp Toolkit shared library"