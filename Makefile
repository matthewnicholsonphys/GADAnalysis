GCC=g++

CXXFLAGS=-std=c++17 -lstdc++
#CFLAGS= -g -O3 -Wpedantic  -Wall -Wconversion -Werror -Inc
#CFLAGS = -Werror -Wall -Wextra -Wpedantic -Wcast-align -Wcast-qual -Wctor-dtor-privacy -Wdisabled-optimization -Wformat=2 -Winit-self -Wlogical-op -Wmissing-include-dirs -Wnoexcept -Wold-style-cast -Woverloaded-virtual -Wredundant-decls -Wshadow -Wsign-promo -Wstrict-null-sentinel -Wstrict-overflow=5 -Wundef -Wno-unused -Wno-variadic-macros -Wno-parentheses -fdiagnostics-show-option

CFLAGS= -g -O3 -Wall -Wpedantic -Wconversion  -Wold-style-cast -Wshadow -Wuninitialized -Winit-self -Werror

ROOT_FLAGS=`root-config --cflags --libs | sed 's/-I/-isystem/g'`


COMP_IN= $(CFLAGS)  $< -o $@ $(ROOT_FLAGS) $(CXXFLAGS)	

all: main lookin set_plotter resolution run_data_analysis emptytube

main: main.cpp gad_utils.cpp
	$(GCC) $(CFLAGS) $^ -o $@ $(ROOT_FLAGS) $(CXXFLAGS)

run_data_analysis: run_data_analysis.cpp gad_utils.cpp
	$(GCC) $(CFLAGS) $^ -o $@ $(ROOT_FLAGS) $(CXXFLAGS)

emptytube: emptytube.cpp gad_utils.cpp
	$(GCC) $(CFLAGS) $^ -o $@ $(ROOT_FLAGS) $(CXXFLAGS)	

resolution: resolution.cpp
	$(GCC) $(CFLAGS) $^ -o $@ $(ROOT_FLAGS) $(CXXFLAGS)

lookin: lookin.cpp
	$(GCC) $(CFLAGS) $^ -o $@ $(ROOT_FLAGS) $(CXXFLAGS)	

set_plotter: set_plotter.cpp
	$(GCC) $(CFLAGS) $^ -o $@ $(ROOT_FLAGS) $(CXXFLAGS)	


clean:
	rm main lookin resolution set_plotter run_data_analysis
