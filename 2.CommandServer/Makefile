.PHONY:all
all:command_client command_server

command_client:CommandClient.cc
	g++ -o $@ $^ -std=c++17
command_server:Main.cc
	g++ -o $@ $^ -std=c++17
.PHONY:clean
clean:
	rm -f command_client command_server