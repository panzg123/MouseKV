//
// Created by zgpan on 2018/7/1.
//

#ifndef MOUSEKV_MOUSE_H
#define MOUSEKV_MOUSE_H

#include "server.h"
#include "cmd.h"
#include "leveldb.h"
#include "log.h"

enum ErrCode
{
    err_parse_num_from_buf = -10000,
};



class ClientPacket : public Context
{

};


class Mouse: public Server
{
public:

    //重写虚函数
    Context* createContextObject(void);
    void destroyContextObject(Context* c);
    void closeConnection(Context* c);
    void clientConnected(Context* c);
    ReadStatus readingRequest(Context* c);
    void readRequestFinished(Context* c);
    void writeReply(Context* c);
    void writeReplyFinished(Context* c);
	bool run(const HostAddress &addr);
	LevelDbCluster* getDbCluster();

private:
    //默认端口
    unsigned int m_default_port = 10010;

    //从buf中读取整数
    int readNumber(char *buf, int len, int& num);
	//命令表
	CmdTable *cmd_table;
	//levelDb表
    LevelDbCluster *db_cluster;
    //线程池
    EventLoopThreadPool *thread_pool;
};

#endif //MOUSEKV_MOUSE_H
