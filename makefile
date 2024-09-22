ifeq ($(OS),Windows_NT)
    output := win_bot.exe
    libs := -lws2_32
else
    output := linux_bot
    libs := -lpthread
endif

all:
	g++ -std=c++20 *.cpp -Wall $(libs) -o $(output) -Iinclude

header:
	python gen_header.py