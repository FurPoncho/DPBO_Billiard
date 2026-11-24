# Build DPBO_Billiard
# Pemakaian:  make          -> bangun game utama (./billiard)
#             make run      -> bangun lalu jalankan
#             make menu     -> prototipe menu (MAIIN.cpp)
#             make clean
# Kalau SFML 3 terpasang di folder sendiri (bukan /usr):  make SFML_PREFIX=$$HOME/sfml-3

CXX      ?= g++
CXXFLAGS ?= -std=c++17 -Wall -Wno-reorder -O2
LIBS      = -lsfml-graphics -lsfml-window -lsfml-system
INC       =
LIBDIR    =
ifdef SFML_PREFIX
INC       = -I$(SFML_PREFIX)/include
LIBDIR    = -L$(SFML_PREFIX)/lib -Wl,-rpath,$(SFML_PREFIX)/lib
endif
HDR       = $(wildcard *.hpp) game_logic.H

# Setiap .cpp di bawah punya main() sendiri, jadi TIDAK boleh dikompilasi bersamaan.
all: billiard

billiard: Main_Gameplay.cpp game_logic.cpp $(HDR)
	$(CXX) $(CXXFLAGS) $(INC) Main_Gameplay.cpp game_logic.cpp -o $@ $(LIBDIR) $(LIBS)

menu: MAIIN.cpp
	$(CXX) $(CXXFLAGS) $(INC) $< -o $@ $(LIBDIR) $(LIBS)

run: billiard
	./billiard

clean:
	rm -f billiard menu game

.PHONY: all run clean