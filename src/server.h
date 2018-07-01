#ifndef SERVER_H
#define SERVER_H

#include "iobuffer.h"
#include "eventloop.h"
#include "socket.h"
#include <string>
#include <vector>
using namespace std;

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
    IOBuffer sendBuff;          //发送buf
    IOBuffer recvBuff;          //Recv buffer
    int sendBytes;              //目前已发送的偏移
    int recvBytes;              //Current recv bytes
    EventLoop* eventLoop;       //Use the event loop
    Event _event;               //Read/Write event

    vector<string> vec_req_params; //存下解析后的参数
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
    virtual ~Server(void);
    EventLoop* getEventLoop(void) { return &m_loop; }
    const HostAddress& getAddress(void) const { return m_addr; }
    bool run(const HostAddress& addr);                                 //启动运行
    bool isRunning(void) const;                                        //是否在运行
    void stop(void);                                                   //停止运行

    virtual Context* createContextObject(void);                        //创建一个新的连接对象
    virtual void destroyContextObject(Context* c);                     //销毁一个连接
    virtual void closeConnection(Context* c);                          //关闭一个连接
    virtual void clientConnected(Context* c);                          //连接建立后的一些操作，如统计
    virtual void waitRequest(Context* c);                              //等待请求
    virtual ReadStatus readingRequest(Context* c);                     //读取请求数据
    virtual void readRequestFinished(Context* c);                      //读取请求数据完毕，接下来可以处理请求了
    virtual void writeReply(Context* c);                               //写响应包
    virtual void writeReplyFinished(Context* c);                       //写响应包结束，做一些操作，如统计

protected:
    static void onAcceptHandler(evutil_socket_t sock, short, void* arg);  //监听套接字回调函数

private:
    HostAddress m_addr;
    Event m_listener;
    EventLoop m_loop;
    Socket m_socket;
    Server(const Server&);
    Server& operator=(const Server&);
};


#endif