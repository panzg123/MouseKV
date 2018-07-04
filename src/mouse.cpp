//
// Created by zgpan on 2018/7/1.
//


#include "mouse.h"
#include <string>
#include <vector>
using namespace std;

Context* Mouse::createContextObject()
{
    return Server::createContextObject();
}

void Mouse::destroyContextObject(Context *c)
{
    return Server::destroyContextObject(c);
}

void Mouse::closeConnection(Context *c)
{
    return Server::closeConnection(c);
}

void Mouse::clientConnected(Context *c)
{
    //todo 统计逻辑
}

//解析协议，判断收发包是否完整
Mouse::ReadStatus Mouse::readingRequest(Context *c)
{
	fprintf(stderr,"readingRequest begin\n");
	fprintf(stderr,"req:%s\n",c->recvBuff.data());
	fprintf(stderr,"req len[%d]\n",c->recvBuff.size());
    //解析Redis协议
    int ret = 0;
    int parar_num = 0; //请求参数个数
    int cur_pos = 0;  //当前解析的位置
    int cur_param_idx = 0; //当前解析的参数索引
    int str_len = 0;
    vector<string> vec_parars; //解析后的参数
    char *buf = c->recvBuff.data();
    int len = c->recvBuff.size();
    if(len < 1)
        return Mouse::ReadIncomplete;
    if(buf[0] != '*')
        return Mouse::ReadError;
    cur_pos += 1;
    //先解析出参数个数
    ret = readNumber(buf + cur_pos, len-cur_pos, parar_num);
	fprintf(stderr,"parar_num[%d] ret[%d]\n",parar_num,ret);
    cur_pos += ret;
    if(ret < 0)
    {
        return Mouse::ReadError;
    }
    //循环解析出每个参数
    while(cur_pos < len && cur_param_idx != parar_num)
    {
	    fprintf(stderr,"cur_pos[%d] cur_param_idx[%d]\n",cur_pos,cur_param_idx);	
        if(buf[cur_pos] != '$')
            return Mouse::ReadError;
        cur_pos += 1;
        ret = readNumber(buf+cur_pos, len-cur_pos, str_len);
		fprintf(stderr,"ret[%d] str_len[%d]\n",ret,str_len);
        if(ret<0)
            return Mouse::ReadError;
        cur_pos += ret;
        string cur_param;
        cur_param.assign(buf+cur_pos, str_len);
        c->vec_req_params.push_back(cur_param);
		fprintf(stderr,"cur_param[%s] cur_pos[%d]\n", cur_param.c_str(), cur_pos);
		cur_pos += str_len;
        if(cur_pos + 2 <= len && buf[cur_pos] == '\r' && buf[cur_pos+1] == '\n')
            cur_pos += 2;
        else
            return Mouse::ReadError;
        cur_param_idx += 1;
    }

    if(cur_param_idx != parar_num)
        return Mouse::ReadError;
    return Mouse::ReadFinished;
}


//收到完整包后的业务逻辑，回调命令对应的处理函数
void Mouse::readRequestFinished(Context *c)
{
	fprintf(stderr,"readRequestFinished begin\n");
    //TODO cmd 处理逻辑
    //先拿到cmd
    if(c->vec_req_params.size() < 2)
	{
	   fprintf(stderr,"readRequestFinished not find cmd\n");
		c->setFinishedState(Context::ProtoNotSupport);
	    return;
    }
	string cmd = c->vec_req_params[0];
	CmdHandler handler; 
	bool find_ret = cmd_table->GetCmdHandler(cmd,handler);
	fprintf(stderr,"find_ret[%d], cmd[%s]\n",find_ret,cmd.c_str());
	if(!find_ret)
	{	
		fprintf(stderr,"readRequestFinished not find cmd\n");
		c->setFinishedState(Context::ProtoNotSupport);
	    return;
	}
	handler(c);
}


void Mouse::writeReply(Context *c)
{
	fprintf(stderr,"writeReply begin\n");
    return Server::writeReply(c);
}

void Mouse::writeReplyFinished(Context *c)
{
    c->sendBuff.clear();
    c->sendBytes = 0;
    c->recvBuff.clear();
    c->recvBytes = 0;
	c->vec_req_params.clear();
	waitRequest(c);
}

int Mouse::readNumber(char *buf, int len, int& num)
{
    int pos = 0;
    num = 0;
    while(pos < len && buf[pos] != '\r')
    {
        if(buf[pos] >= '0' && buf[pos] <= '9')
            num = 10*num + (buf[pos] - '0');
        else
            return err_parse_num_from_buf;
        ++pos;
    }
    if(len > (pos + 1) && buf[pos] == '\r' && buf[pos+1] == '\n') // \r\n
        return pos + 2;
    else
        return err_parse_num_from_buf;
}

bool Mouse::run(const HostAddress &addr)
{
	if (isRunning()) 
	{
        return false;
    }
	//初始化命令表
	cmd_table = CmdTable::instance();

	//启动server
	return Server::run(addr);
}