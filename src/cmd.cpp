#include "cmd.h"
#include <sstream>
#include <string>
using namespace std;


//下面定义GET/SET的处理函数
void MouseGetCmdHandler(Context* c)
{
	if(c->vec_req_params.size() != 2)
	{
	    c->setFinishedState(Context::WrongNumberOfArguments);
		return;
	}
	string send_str = "echo_" + c->vec_req_params[1];
	c->sendBuff.appendFormatString("$%d\r\n", send_str.size());
	c->sendBuff.append(send_str.data(),send_str.size());
	c->sendBuff.append("\r\n");
	c->setFinishedState(Context::RequestFinished);
}


void MouseSetCmdHandler(Context *c)
{
	if(c->vec_req_params.size() != 3)
	{
	    c->setFinishedState(Context::WrongNumberOfArguments);
		return;
	}
	//先简单打印一下kv，然后回包ok
	fprintf(stderr,"key[%s],value[%s]\n",c->vec_req_params[1].c_str(), c->vec_req_params[2].c_str());
	c->sendBuff.append("+OK\r\n");
	c->setFinishedState(Context::RequestFinished);
}



//下面是CmdTable的实现
bool CmdTable::AddCmd(string cmd, CmdHandler handler)
{
    fprintf(stderr,"AddCmd, cmd[%s]\n",cmd.c_str());
	m_map_cmd_handler[cmd] = handler;
}


bool CmdTable::GetCmdHandler(const string & cmd, CmdHandler& handler)
{
	fprintf(stderr,"GetCmdHandler, cmd[%s]\n",cmd.c_str());
    if(m_map_cmd_handler.count(cmd))
    {
    	handler = m_map_cmd_handler[cmd];
		return true;
    }
	return false;
}


CmdTable* CmdTable::instance(void)
{
	//不是线程安全的，MouseKV单线程
	static CmdTable* table = NULL;
	if (!table) 
	{
	  table = new CmdTable;
	  table->AddCmd("SET", MouseSetCmdHandler);
	  table->AddCmd("GET", MouseGetCmdHandler);
	}
	return table;

}



