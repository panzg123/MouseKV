#include "cmd.h"
#include "mouse.h"
#include "log.h"
#include <sstream>
#include <string>
#include "DbCopyThread.h"

using namespace std;


class Mouse;

//装换为大写
void strToUpper(string& str)
{
    for (size_t i = 0; i < str.size(); ++i)
    {
        str[i] = static_cast<char>(toupper(str[i]));
    }
}


//下面定义GET/SET的处理函数
void MouseGetCmdHandler(Context* c)
{
	if(c->vec_req_params.size() != 2)
	{
	    c->setFinishedState(Context::WrongNumberOfArguments);
		return;
	}
    //处理逻辑
    Mouse *_mouse_server = (Mouse*)c->server;
	string value;
	bool get_ret = _mouse_server->getDbCluster()->getValue(c->vec_req_params[1],value);
	if(get_ret)
    {
        c->sendBuff.appendFormatString("$%d\r\n", value.size());
        c->sendBuff.append(value.data(),value.size());
        c->sendBuff.append("\r\n");
        c->setFinishedState(Context::RequestFinished);
    } else
        c->setFinishedState(Context::RequestError);
}


void MouseSetCmdHandler(Context *c)
{
	if(c->vec_req_params.size() != 3)
	{
	    c->setFinishedState(Context::WrongNumberOfArguments);
		return;
	}
	//先简单打印一下kv，然后回包ok
	COMM_LOG(Logger::DEBUG,"key[%s],value[%s]\n",c->vec_req_params[1].c_str(), c->vec_req_params[2].c_str());
	//处理逻辑
    Mouse *_mouse_server = (Mouse*)c->server;
    bool set_ret = _mouse_server->getDbCluster()->setValue(c->vec_req_params[1],c->vec_req_params[2]);
	if(set_ret)
    {
        c->sendBuff.append("+OK\r\n");
        c->setFinishedState(Context::RequestFinished);
    } else
        c->setFinishedState(Context::RequestError);
}


//删除操作
void MouseDelCmdHandler(Context* c)
{
    if(c->vec_req_params.size() != 2)
    {
        c->setFinishedState(Context::WrongNumberOfArguments);
        return;
    }
    //处理逻辑
    COMM_LOG(Logger::DEBUG,"MouseDelCmdHandler, para size[%uz],  para1[%s], para2[%s]",
        c->vec_req_params[0].c_str(),c->vec_req_params[1].c_str());

    Mouse *_mouse_server = (Mouse*)c->server;
    bool del_ret = _mouse_server->getDbCluster()->delKey(c->vec_req_params[1]);
    if(del_ret)
    {
        c->sendBuff.append("+OK\r\n");
        c->setFinishedState(Context::RequestFinished);
    } else
        c->setFinishedState(Context::RequestError);
}

//Binlog同步命令__SYNC
void MouseSyncToSlaveCmdHandle(Context* c)
{
    if(c->vec_req_params.size() != 3)
    {
        c->setFinishedState(Context::WrongNumberOfArguments);
        COMM_LOG(Logger::ERROR, "param invalid, vec_req_params.size[%zu]", c->vec_req_params.size());
        return;
    }
    string last_update_file = c->vec_req_params[1]; //可能为空，则表示从第一个BinLog文件开始拷贝
    int last_update_pos = std::stoi(c->vec_req_params[2]); //可能为-1.则表示从文件头开始
    COMM_LOG(Logger::DEBUG, "sync to slave begin, last_update_file[%s] last_update_pos[%d]",
            last_update_file.c_str(), last_update_pos);
    IOBuffer &reply = c->sendBuff;
    reply.reserve(BinlogSyncStream::MaxStreamSize); //每次同步最大32M

    Mouse *mouse_svr = (Mouse*)c->server;
    LevelDbCluster *dbCluster = mouse_svr->getDbCluster();
    BinlogFileList *binlog_list = dbCluster->binlogFileList();

    BinlogSyncStream stream;
    reply.append((char*)&stream, sizeof(BinlogSyncStream)); //拷贝首部信息

    int err = BinlogSyncStream::NoError;
    std::string errMsg;
    int logitem_count = 0;
    int read_file_pos = last_update_pos;  //记录每个Binlog文件整体的偏移量，先初始化为上次读取的信息
    int binlog_now_pos = -1; //每个Binlog文件的真正添加到reply中偏移量

    if(!binlog_list->isEmpty()) //binlog filelist非空
    {
        if(last_update_file == " ")
        {
            last_update_pos = -1;
            last_update_file = binlog_list->fileName(0); //从第一个binlog文件开始同步
        }
        //找到file_name的位置，开始同步
        int binlog_index = binlog_list->indexOfFileName(last_update_file);
        if(binlog_index == -1)
        {
            err = BinlogSyncStream::InvalidFileName;
            errMsg = "invalid binlog file name";
        }
        else
        {
            COMM_LOG(Logger::DEBUG, "before for loop, binlog_index[%d] binlog_count[%d]", binlog_index, binlog_list->fileCount());
            for (int i = binlog_index; i < binlog_list->fileCount(); ++i)
            {
                string binlog_file_name = binlog_list->fileName(i);
                BinlogParser stParse;
                bool ret = stParse.open(dbCluster->getBinlogFullPath(binlog_file_name));
                if(!ret)
                {
                    err = BinlogSyncStream::InvalidFileName;
                    errMsg = "open binlog file failed";
                    break;
                }
                else
                {
                    binlog_now_pos = -1;
                    bool need_break = false; //标记是否break，stream超大
                    BinlogBufferReader stReader = stParse.reader();
                    BinLog::LogItem *log_item = stReader.firstItem();
                    COMM_LOG(Logger::DEBUG,"binlog_filename[%s] binlog_now_pos[%d] read_file_pos[%d]",
                            binlog_file_name.c_str(), binlog_now_pos, read_file_pos);
                    for(; log_item != nullptr; log_item = stReader.nextItem(log_item))
                    {
                        COMM_LOG(Logger::DEBUG,"find log_item, binlog_now_pos[%d] read_file_pos[%d], key[%s]",
                                binlog_now_pos, read_file_pos, log_item->keyBuffer());
                        if(binlog_now_pos >= read_file_pos)
                        {
                            reply.append((char*)log_item, log_item->item_size);
                            ++logitem_count;
                        }
                        ++binlog_now_pos;

                        //如果超出最大同步buf大小，则break
                        if(reply.size() >= BinlogSyncStream::MaxStreamSize)
                        {
                            COMM_LOG(Logger::DEBUG,"reply much too big, break, rely.size[%zu]", reply.size());
                            need_break = true;
                            break;
                        }
                    }

                    //读完一个文件了
                    last_update_pos = binlog_now_pos;
                    last_update_file = binlog_file_name;
                    read_file_pos = -1; //因为要开始读取下一个文件了，重置偏移量
                    if(need_break)
                    {
                        break;
                    }
                }
            }
        }
    }
    else //空-不报错了
    {
        errMsg = "master binlog file list empty";;
    }
    //读取结束
    COMM_LOG(Logger::DEBUG,"sync end, error[%d] errMs[%s] last_file[%s] last_pos[%d] logItemCnt[%d]",
            err, errMsg.c_str(), last_update_file.c_str(), last_update_pos, logitem_count);
    reply.append("\r\n", 2); //TODO ？ 这里/r/n不必须把
    BinlogSyncStream *pStream = (BinlogSyncStream*)reply.data();
    pStream->ch = '+';
    pStream->streamSize = reply.size();
    pStream->error = err;
    strcpy(pStream->errorMsg, errMsg.c_str());
    strcpy(pStream->srcFileName, last_update_file.c_str());
    pStream->lastUpdatePos = last_update_pos;
    pStream->logItemCount = logitem_count;
    c->setFinishedState(Context::RequestFinished); //写到buf中了，准备sendto
}

//SYNCFROM ip port,  A-->B 该出是B节点的handle
void MouseSyncFromCmdHandle(Context* c)
{
    if(c->vec_req_params.size() != 3)
    {
        c->setFinishedState(Context::WrongNumberOfArguments);
        return;
    }
    string src_ip = c->vec_req_params[1];
    int src_port = std::stoi(c->vec_req_params[2]);
    COMM_LOG(Logger::DEBUG, "SyncFrom begin, src_ip[%s] src_port[%d]", src_ip.c_str(), src_port);

    //先创建一条与src svr的tcp链接
    Socket sock = Socket::CreateSocket();
    if (!sock.connect(HostAddress(src_ip.c_str(), src_port)))
    {
        sock.close();
        c->sendBuff.appendFormatString("-ERR Can't connect to host '%s:%d'\r\n", src_ip.c_str(), src_port);
        c->setFinishedState(Context::RequestFinished);
        return;
    }

    Mouse *mouse_svr = (Mouse*)c->server;
    char portbuf[32];
    char sendbuf[128];
    int n = sprintf(portbuf, "%d", mouse_svr->getAddress().port());
    sprintf(sendbuf, "*2\r\n$6\r\n__copy\r\n$%d\r\n%s\r\n", n, portbuf);
    if (sock.send(sendbuf, strlen(sendbuf)) <= 0) {
        sock.close();
        c->sendBuff.append("-ERR Request the host failed\r\n");
        c->setFinishedState(Context::RequestFinished);
        return;
    }

    //接收src节点的消息回复
    char ch;
    sock.recv(&ch, 1);
    if (ch == '1') {
        c->sendBuff.append("+OK\r\n");
    } else {
        c->sendBuff.append("-ERR Copy failed\r\n");
    }
    sock.close();
    c->setFinishedState(Context::RequestFinished);
}

//__COPY port,  A-->B 该处是A节点的handle
void MouseCopyCmdHandle(Context* c)
{
    if(c->vec_req_params.size() != 2)
    {
        c->setFinishedState(Context::WrongNumberOfArguments);
        return;
    }
    int dst_port = std::stoi(c->vec_req_params[1]); //目标端口
    Mouse* mouse_svr = (Mouse*)c->server;
    string dst_ip = c->clientAddress.ip();
    //TODO 单独启动一个DBCOPY线程
    DbCopyThread *copyThread = new DbCopyThread(mouse_svr, dst_ip, dst_port);//TODO 这里是否有内存泄漏
    int ret = copyThread->start();
    if(!ret)
    {
        c->sendBuff.append("1");
    }
    else
    {
        c->sendBuff.append("0");
    }
    c->setFinishedState(Context::RequestFinished);
}


//下面是CmdTable的实现
bool CmdTable::AddCmd(string cmd, CmdHandler handler)
{
    COMM_LOG(Logger::ERROR,"AddCmd, cmd[%s]",cmd.c_str());
	m_map_cmd_handler[cmd] = handler;
	return true;
}


bool CmdTable::GetCmdHandler(const string & cmd, CmdHandler& handler)
{
	COMM_LOG(Logger::DEBUG,"GetCmdHandler, cmd[%s]",cmd.c_str());
	//这里统一转换为大写命令字
    string str_cmd = cmd;
    strToUpper(str_cmd);
    if(m_map_cmd_handler.count(str_cmd))
    {
    	handler = m_map_cmd_handler[str_cmd];
		return true;
    }
	return false;
}


CmdTable* CmdTable::instance(void)
{
	static CmdTable* table = nullptr;
	if (!table) 
	{
	  table = new CmdTable;
	  table->AddCmd("SET", MouseSetCmdHandler);
	  table->AddCmd("GET", MouseGetCmdHandler);
	  table->AddCmd("DEL", MouseDelCmdHandler);
	  table->AddCmd("__SYNC", MouseSyncToSlaveCmdHandle); //BinLog同步命令，主节点处理__SYNC命令
	  table->AddCmd("__COPY", MouseCopyCmdHandle);        //A-->B节点复制时， A节点收到的来自B节点的cmd
	  table->AddCmd("SYNCFROM", MouseSyncFromCmdHandle);  //A-->B节点复制时，B节点收到的来自client的命令
	}
	return table;
}



