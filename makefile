all:client reactor relay_server

client:client.cpp
	g++ -g -std=c++11 -o client client.cpp
reactor:reactor.cpp InetAddress.cpp
	g++ -g -std=c++11 -o reactor reactor.cpp InetAddress.cpp Socket.cpp Epoll.cpp Channel.cpp EventLoop.cpp TcpServer.cpp TcpClient.cpp Acceptor.cpp Connection.cpp Buffer.cpp GatewayServer.cpp ThreadPool.cpp Timestamp.cpp -lpthread

relay_server:relay_main.cpp InetAddress.cpp Socket.cpp Epoll.cpp Channel.cpp EventLoop.cpp TcpServer.cpp Acceptor.cpp Connection.cpp Buffer.cpp ThreadPool.cpp Timestamp.cpp RelayServer.cpp DatabaseHelper.cpp
	g++ -g -o relay_server relay_main.cpp InetAddress.cpp Socket.cpp Epoll.cpp Channel.cpp EventLoop.cpp TcpServer.cpp Acceptor.cpp Connection.cpp Buffer.cpp ThreadPool.cpp Timestamp.cpp RelayServer.cpp DatabaseHelper.cpp -lpthread -lmysqlclient

clean:
	rm -f client reactor relay_server
