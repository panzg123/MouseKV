#include "server.h"

void onReadClientHandler(socket_t, short, void* arg)
{
    Context* c = (Context*)arg;
    IOBuffer* buf = &c->recvBuff;
    IOBuffer::DirectCopy cp = buf->beginCopy();
    int ret = c->clientSocket.nonblocking_recv(cp.address, cp.maxsize);
    switch (ret) {
    case Socket::IOAgain:
        c->server->waitRequest(c);
        break;
    case Socket::IOError:
        c->server->closeConnection(c);
        break;
    default:
        buf->endCopy(ret);
        c->recvBytes += ret;
        switch (c->server->readingRequest(c)) {
        case Server::ReadFinished:
            c->server->readRequestFinished(c);
            break;
        case Server::ReadIncomplete:
            c->server->waitRequest(c);
            break;
        case Server::ReadError:
            c->server->closeConnection(c);
            break;
        }
        break;
    }
}


void onWriteClientHandler(socket_t, short, void* arg)
{
    Context* c = (Context*)arg;
	char* data = c->sendBuff.data() + c->sendBytes;
	int size = c->sendBuff.size() - c->sendBytes;
	int ret = c->clientSocket.nonblocking_send(data,size);

	switch (ret){
		case Socket::IOAgain:
			c->_event.set(c->eventLoop, c->clientSocket, EV_WRITE, onWriteClientHandler, c);
			c->_event.active();
			break;
		case Socket::IOError:
			c->server.closeConnection(c);
			break;
		default:
			c->sendBytes += ret;
			if(c->sendBytes != c->sendBuff.size())
				onWriteClientHandler(0,0,c);
			else
				c->server->writeReplyFinished(c);
			break;
	}
}

void Server::onAcceptHandler(evutil_socket_t sock, short, void * arg)
{
    socketaddr_in clientAddr;
	socketlen_t len = sizeof(socketaddr_t);
	int client_sock = accept(sock,(sockaddr*)&clientAddr, &len);
	Socket _sock(client_sock);
	if(_sock.isNull())
	{
	    return;
	}
	_sock.setKeepAlive();
	_sock.setNonBlocking();
	_sock.setNoDelay();

	Server* server = (Server*)arg;
	Context* c = server->createContextObject();
    if (c != NULL) {
        c->clientSocket = _sock;
        c->clientAddress = HostAddress(clientAddr);
        c->server = server;
        if (c->eventLoop == NULL) {
            c->eventLoop = srv->eventLoop();
        }
        server->clientConnected(c);
        server->waitRequest(c);
    } else {
        _sock.close();
    }
}