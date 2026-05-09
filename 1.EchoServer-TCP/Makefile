.PHONY:all
all:tcp_client tcp_server

tcp_client:TcpClient.cc
	g++ -o $@ $^ -std=c++17
tcp_server:TcpServer.cc
	g++ -o $@ $^ -std=c++17
.PHONY:clean
clean:
	rm -f tcp_client tcp_server