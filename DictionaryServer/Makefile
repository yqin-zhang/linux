.PHONY:all
all:dict_client dict_server

dict_client:DictClient.cc
	g++ -o $@ $^ -std=c++17
dict_server:DictServer.cc
	g++ -o $@ $^ -std=c++17
.PHONY:clean
clean:
	rm -f dict_client dict_server