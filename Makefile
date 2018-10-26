# Linux (default)
EXE = Chip8
	LDFLAGS = -lX11 -lXxf86vm -lXrandr -lpthread -lXi -std=c++11
	CXXFLAGS= -g -std=c++14 -Wall -pedantic
# Windows (cygwin)
ifeq "$(OS)" "Windows_NT"
EXE = bin/Chip8.exe
	LDFLAGS = -lmingw32 -lSDL2main -lSDL2 
	CXXFLAGS= -g --std=c++14 -Wall -pedantic
	#Specify the location of SDL2 in the following field
	LIB_SDL2 = ./deps/SDL2/lib
	INC_SDL2 = ./deps/SDL2/include
endif

# OS X
ifeq "$(OSTYPE)" "darwin"
	LDFLAGS = -framework Carbon -framework OpenGL -framework GLUT
	CXXFLAGS= -g -std=c++14 -Wall -pedantic
endif

OBJ_DIR = ./obj

# find cpp files in subdirectories
#SOURCES := $(shell find . -name '*.cpp') 
SOURCES := ./chip-8.cpp

OBJECTS := $(SOURCES:.cpp=.o)
OBJECTS := $(subst ./,obj/, $(OBJECTS))


# find headers
HEADERS := $(shell find . -name '*.h')

all: $(OBJ_DIR)/chip-8.o $(EXE)

$(OBJ_DIR)/chip-8.o:
	-@mkdir -p ./obj 
	$(CXX) -L$(LIB_SDL2) -I$(INC_SDL2) --std=c++11 -c ./chip-8.cpp -o $@

	
$(EXE) : 
	-@mkdir -p ./bin 
	$(CXX) -L$(LIB_SDL2) -I$(INC_SDL2) -o $@  $(OBJECTS) $(CXXFLAGS) $(LDFLAGS)