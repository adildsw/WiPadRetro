# Compiler
CC=gcc

# Include directory for header files
INCLUDES=-I./include/

# Libraries
LIBS=`sdl-config --cflags --libs` -lSDL_image -lSDL_ttf -lpthread

# Compiler flags
CFLAGS=-mmacosx-version-min=14.0

# Source files
SOURCES=src/app.c src/ip_util.c src/gamepad_util.c src/tcp_server.c

# Object files
OBJECTS=$(SOURCES:.c=.o)

# Executable name
EXECUTABLE=app

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(OBJECTS) -o $@ $(LIBS) $(CFLAGS)

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f src/*.o $(EXECUTABLE)
