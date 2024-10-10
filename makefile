ifeq ($(OS),Windows_NT)
    output := win_bot.exe
    libs := -lws2_32
    nproc := 6 # set to 6 threads for windows
    delete := del
else
    output := linux_bot
    libs := 
    nproc := $(shell nproc)  # set to number of cpu threads for linux
    delete := rm -f
endif

MAKEFLAGS += -j$(nproc)

# Object files
objects := $(patsubst %.cpp,%.o,$(wildcard *.cpp))

# Parallelized build
all: $(output)

$(output): $(objects)
	g++ -std=c++20 $(objects) -Wall $(libs) -o $(output) -O2 -Iinclude -static
	$(delete) $(objects)

%.o: %.cpp
	g++ -std=c++20 -c $< -o $@ -Iinclude -O2 -static

clean: $(ouput)
	$(delete) $(objects)

.PHONY: all clean