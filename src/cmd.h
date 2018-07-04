#ifndef CMD_H
#define CMD_H

#include "server.h"
#include <map>
using namespace std;

//定义cmd处理函数原型
typedef void (*CmdHandler)(Context*);

class CmdTable
{
public:
	bool AddCmd(string cmd, CmdHandler handler);
	bool GetCmdHandler(const string& cmd, CmdHandler &handler);
	static CmdTable* instance(void);

private:
	map<string,CmdHandler> m_map_cmd_handler;
};


#endif
