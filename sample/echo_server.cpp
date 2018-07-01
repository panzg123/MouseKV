//
// Created by zgpan on 2018/6/30.
//

#include "../src/server.h"


class EchoServer : public Server
{
public:
    EchoServer(void);
    ~EchoServer(void);

    //重写
    virtual void readRequestFinished(Context *c);
    virtual void writeReplyFinished(Context *c);
};

void EchoServer::readRequestFinished(Context *c)
{
    //这里做测试，把先请求包打印一下
    fprintf(stderr,"req:%s\n",c->recvBuff.data());
    //测试做个echo
    c->sendBuff = c->recvBuff;
    c->sendBytes = 0;
    writeReply(c);
}

void EchoServer::writeReplyFinished(Context *c)
{
    //回包完毕之后，要清除sendBuf和recvBuf
    c->recvBuff.clear();
    c->sendBuff.clear();
}


int main()
{
    EchoServer _server;
    _server.run(10086);
}