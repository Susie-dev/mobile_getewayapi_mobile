all:client reactor

client:client.cpp
	g++ -g -std=c++11 -o client client.cpp
reactor:reactor.cpp InetAddress.cpp
	g++ -g -std=c++11 -o reactor reactor.cpp InetAddress.cpp Socket.cpp Epoll.cpp Channel.cpp EventLoop.cpp TcpServer.cpp TcpClient.cpp Acceptor.cpp Connection.cpp Buffer.cpp GatewayServer.cpp ThreadPool.cpp Timestamp.cpp -lpthread
clean:
	rm -f client reactor
