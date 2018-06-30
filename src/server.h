#ifndef SERVER_H
#define SERVER_H

#include "iobuffer.h"
#include "eventloop.h"
#include "socket.h"

class Server;
class Context
{
public:
    Context(void) {
        server = NULL;
        sendBytes = 0;
        recvBytes = 0;
        eventLoop = NULL;
    }

    virtual ~Context(void) {}

    Socket clientSocket;     //Client socket
    HostAddress clientAddress;  //Client address
    Server* server;          //The Connected server
    IOBuffer sendBuff;          //Send buffer
    IOBuffer recvBuff;          //Recv buffer
    int sendBytes;              //Current send bytes
    int recvBytes;              //Current recv bytes
    EventLoop* eventLoop;       //Use the event loop
    Event _event;               //Read/Write event
};


class Server
{
public:
    enum ReadStatus {
        ReadFinished = 0,
        ReadIncomplete = 1,
        ReadError = 2
    };

    Server(void);
    virtual Server(void);
    EventLoop* getEventLoop(void) { return &m_loop; }
    const HostAddress& address(void) const { return m_addr; }
    bool run(const HostAddress& addr);
    bool isRunning(void) const;
    void stop(void);

    virtual Context* createContextObject(void);
    virtual void destroyContextObject(Context* c);
    virtual void closeConnection(Context* c);
    virtual void clientConnected(Context* c);
    virtual void waitRequest(Context* c);
    virtual ReadStatus readingRequest(Context* c);
    virtual void readRequestFinished(Context* c);
    virtual void writeReply(Context* c);
    virtual void writeReplyFinished(Context* c);

protected:
    static void onAcceptHandler(evutil_socket_t sock, short, void* arg);

private:
    HostAddress m_addr;
    Event m_listener;
    EventLoop m_loop;
    Socket m_socket;
    Server(const Server&);
    Server& operator=(const Server&);
};


#endif