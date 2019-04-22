//
// Created by zgpan on 2019/4/14.
//

#ifndef MOUSEKV_BINLOG_H
#define MOUSEKV_BINLOG_H

#include <string>
#include <cstdio>
#include <comm_def.h>
#include <vector>

using namespace std;

//单个binLog文件类
class BinLog
{
public:
    struct Header
    {
        int handshake = HandShake; //这是一个魔法数
        bool IsHeaderValid()
        {
            return handshake == HandShake;
        }
    };

    struct LogItem
    {
        //binlog操作类型--两种
        enum {
            SET,
            DEL
        };
        int item_size;  //这条LogItem大小
        int type;       //操作类型
        int key_size;   //key长度
        int value_size; //value长度

        char* keyBuffer(void) { return (char*)(this+1); }
        char* valueBuffer(void) { return keyBuffer()+key_size; }
    };

    BinLog();
    ~BinLog();
    bool open(const std::string& file_name);
    std::string fileName() const { return m_fileName; }
    size_t writtenSize() const { return m_writtenSize; }
    bool appendSetRecord(const string& key, const string& value);
    bool appendDelRecord(const string& key);
    void sync();
    void close();

private:
    std::string m_fileName;   //当前binlog文件名
    FILE* m_fp;               //文件读写file*
    size_t m_writtenSize;     //已经写入得长度
    BinLog(const BinLog&);
    BinLog& operator=(const BinLog&);
};

//binLog日志列表
class BinlogFileList
{
public:
    BinlogFileList(){}
    ~BinlogFileList(){}

    void appendNewBinlogFile(const string& file_name)
    {
        m_binlogFileList.push_back(file_name);
    }
    bool isEmpty()
    {
        return m_binlogFileList.empty();
    }
    int fileCount()
    {
        return m_binlogFileList.size();
    }
    string fileName(int index) const
    {
        return m_binlogFileList[index];
    }
    int indexOfFileName(const string& fileName) const;

    static void loadBinlogListFromIndex();
    static void saveBinlogListToIndex();

    //todo 还需要个生成binlog新文件的函数

    //todo 也可以把调整log文件等功能在此实现，不需要再levelCluster中判断

private:
    vector<string> m_binlogFileList;   //所有binlog文件名集合
    const string m_default_index_file = "./binlog/BINLOG_INDEX"; //默认索引文件
};

//同步的二进制数据流--用于slave解析二进制数据
struct BinlogSyncStream {
    enum Error {
        NoError = 0,
        InvalidFileName
    };

    enum {
        MaxStreamSize = 32*1024*1024  //max stream size. 32M
    };

    char ch;                //+
    int streamSize;         //stream size
    int error;              //error
    char errorMsg[128];     //error message
    char srcFileName[128];  //last binlog file name
    int lastUpdatePos;      //last update in the binlog
    int logItemCount;       //item count

    //获取二进制流中的第一个LogItem
    BinLog::LogItem* firstLogItem(void) {
        return (BinLog::LogItem*)(this+1);
    }

    BinLog::LogItem* nextLogItem(BinLog::LogItem* cur) {
        return (BinLog::LogItem*)(((char*)cur)+cur->item_size);
    }
};

//解析器 -- 主要用于master解析binlog文件
class BinlogBufferReader
{
public:
    BinlogFileList(){}
    BinlogFileList(char* buf, int size)
    {
        m_buf = buf;
        m_len = size;
    }
    ~BinlogBufferReader(){}

    BinLog::LogItem* firstItem(void) const {
        if (m_len == 0) {
            return NULL;
        }
        return (BinLog::LogItem*)(m_buff);
    }
    BinLog::LogItem* nextItem(BinLog::LogItem* item) const {
        if ((char*)item + item->item_size - m_buff >= m_len) {
            return NULL;
        }
        return (BinLog::LogItem*)(((char*)item)+item->item_size);
    }

private:
    char* m_buf = nullptr;
    int m_len = 0;
};

//真正的解析器--读取BinLog
//buf数据 -- header + LogItem
class BinlogParser
{
public:
    BinlogParser(void);
    ~BinlogParser(void);

    bool open(const std::string& fname);
    BinlogBufferReader reader(void) const;
    void close(void);

private:
    char* m_base;
    int m_size;
    int m_fd;

    BinlogParser(const BinlogParser&);
    BinlogParser& operator=(const BinlogParser&);
};

#endif //MOUSEKV_BINLOG_H
