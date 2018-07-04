#include "server.h"

void onReadClientHandler(int, short, void* arg)
{
    Context* c = (Context*)arg;
    IOBuffer* buf = &c->recvBuff;
    IOBuffer::DirectCopy cp = buf->beginCopy();
    int ret = c->clientSocket.nonblocking_recv(cp.address, cp.maxsize);
    switch (ret)
    {
    case Socket::IOAgain:
        c->server->waitRequest(c);
        break;
    case Socket::IOError:
        c->server->closeConnection(c);
        break;
    default:
        buf->endCopy(ret);
        c->recvBytes += ret;
        switch (c->server->readingRequest(c))
        {
        case Server::ReadFinished:
            c->server->readRequestFinished(c);
            break;
        case Server::ReadIncomplete:             //请求包不完整，继续读取
            c->server->waitRequest(c);
            break;
        case Server::ReadError:
            c->server->closeConnection(c);
            break;
        }
        break;
    }
}


void onWriteClientHandler(int, short, void* arg)
{
    fprintf(stderr,"onWriteClientHandler begin\n");
    Context* c = (Context*)arg;
	char* data = c->sendBuff.data() + c->sendBytes;
	int size = c->sendBuff.size() - c->sendBytes;
	int ret = c->clientSocket.nonblocking_send(data,size);

	switch (ret)
    {
		case Socket::IOAgain:
			c->_event.set(c->eventLoop, c->clientSocket.getSocket(), EV_WRITE, onWriteClientHandler, c);
			c->_event.active();
			break;
		case Socket::IOError:
        {
            fprintf(stderr,"onWriteClientHandler, IOError, ret=%d\n",ret);
            c->server->closeConnection(c);
            break;
        }
		default:
			c->sendBytes += ret;
			if(c->sendBytes != c->sendBuff.size())
				onWriteClientHandler(0,0,c);
			else
				c->server->writeReplyFinished(c);
			break;
	}
}

//监听套接字回调函数
void Server::onAcceptHandler(evutil_socket_t sock, short, void * arg)
{
    fprintf(stderr,"onAcceptHandler begin\n");
    sockaddr_in clientAddr;
	socklen_t len = sizeof(sockaddr_in);
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
            c->eventLoop = server->getEventLoop();
        }
        server->clientConnected(c);
        server->waitRequest(c);
    } else {
        _sock.close();
    }
    fprintf(stderr,"onAcceptHandler end\n");
}


void Context::setFinishedState(Context::State state)
{
    switch (state) {
    case Context::Unknown:
        sendBuff.append("-Unknown state\r\n");
        break;
    case Context::ProtoError:
        sendBuff.append("-Proto error\r\n");
        break;
    case Context::ProtoNotSupport:
        sendBuff.append("-Proto not support\r\n");
        break;
    case Context::WrongNumberOfArguments:
        sendBuff.append("-Wrong number of arguments\r\n");
        break;
    case Context::RequestError:
        sendBuff.append("-Request error\r\n");
        break;
    case Context::RequestFinished:
        break;
    default:
        break;
    }
    server->writeReply(this);
}



Server::Server() {}

Server::~Server()
{
    stop();
}

bool Server::run(const HostAddress &addr)
{
    //1.socket
    if(isRunning())
    {
        fprintf(stderr,"Server is already run!\n");
        return false;
    }
    Socket sock = Socket::CreateSocket();
    if(sock.isNull())
    {
        fprintf(stderr,"Server Run, create sock error\n");
        return false;
    }
    //2.bind
    sock.setReuseaddr();
    sock.setNoDelay();
    sock.setNonBlocking();
    if(!sock.bind(addr))
    {
        fprintf(stderr,"Server Run, bind sock error\n");
        sock.close();
        return false;
    }
    //3.listen
    if(!sock.listen(128))
    {
        fprintf(stderr,"Server Run, sock listen error\n");
        sock.close();
        return false;
    }
    //4.注册事件
    m_listener.set(&m_loop,sock.getSocket(),EV_READ | EV_PERSIST, onAcceptHandler, this);
    m_listener.active();
    m_socket = sock;
    m_addr = addr;
    //5.启动循环
    m_loop.exec();
    fprintf(stderr,"Server Run, success\n");
    return true;
}

bool Server::isRunning() const
{
    return !m_socket.isNull();
}

void Server::stop()
{
    //1. 停止事件循环
    //2. 关闭套接字
    if(isRunning())
    {
        m_listener.remove();
        m_socket.close();
        m_loop.exit();
        fprintf(stderr,"Server stop success\n");
    }
}

Context* Server::createContextObject()
{
    return new Context;
}

void Server::destroyContextObject(Context *c)
{
    delete c;
}

void Server::closeConnection(Context *c)
{
    //1.关闭套接字
    //2.移除事件(不用移除，不是EV_PERSIST)
    //3.删除context对象，释放内存
    c->clientSocket.close();
    destroyContextObject(c);
}

void Server::clientConnected(Context *c)
{

}

void Server::waitRequest(Context *c)
{
    c->_event.set(c->eventLoop,c->clientSocket.getSocket(),EV_READ,onReadClientHandler,c);
    c->_event.active();
}

//解析请求包，由派生类重写，不同的协议，不同的逻辑
Server::ReadStatus Server::readingRequest(Context *c)
{
    return Server::ReadFinished;
}

//读取请求包结束，执行一些业务逻辑，由派生类重写
void Server::readRequestFinished(Context *c)
{

}

//直接把context中的数据写出去
void Server::writeReply(Context *c)
{
    onWriteClientHandler(0,0,c);
}

//回包结束，可以做一些统计或者激活下一次可读事件，由派生类做具体逻辑
void Server::writeReplyFinished(Context *c)
{

}
