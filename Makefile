CXX = g++
CXXFLAGS = -std=c++17 -pthread

DEBUG_FLAGS = -g -O0
OPT_FLAGS = -O3 -DNDEBUG

DEBUG_EXECUTABLES = wifi4_debug wifi5_debug wifi6_debug
OPT_EXECUTABLES = wifi4_opt wifi5_opt wifi6_opt

SOURCES = wifi4.cpp wifi5.cpp wifi6.cpp

all: debug opt

debug: $(DEBUG_EXECUTABLES)

wifi4_debug: wifi4.cpp
	$(CXX) $(CXXFLAGS) $(DEBUG_FLAGS) -o wifi4_debug wifi4.cpp

wifi5_debug: wifi5.cpp
	$(CXX) $(CXXFLAGS) $(DEBUG_FLAGS) -o wifi5_debug wifi5.cpp

wifi6_debug: wifi6.cpp
	$(CXX) $(CXXFLAGS) $(DEBUG_FLAGS) -o wifi6_debug wifi6.cpp

opt: $(OPT_EXECUTABLES)

wifi4_opt: wifi4.cpp
	$(CXX) $(CXXFLAGS) $(OPT_FLAGS) -o wifi4_opt wifi4.cpp

wifi5_opt: wifi5.cpp
	$(CXX) $(CXXFLAGS) $(OPT_FLAGS) -o wifi5_opt wifi5.cpp

wifi6_opt: wifi6.cpp
	$(CXX) $(CXXFLAGS) $(OPT_FLAGS) -o wifi6_opt wifi6.cpp


clean:
	rm -f $(DEBUG_EXECUTABLES) $(OPT_EXECUTABLES)

.PHONY: all debug opt clean
