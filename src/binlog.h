//
// Created by zgpan on 2019/4/14.
//

#ifndef MOUSEKV_BINLOG_H
#define MOUSEKV_BINLOG_H

#include <string>
#include <cstdio>
#include <vector>
#include <fstream>
#include "comm_def.h"

using namespace std;


class TextConfigFile
{
public:
    static bool read(const std::string& fname, std::vector<std::string>& values)
    {
        ifstream ifs(fname.c_str());
        if(!ifs)
        {
            return false;
        }
        string str;
        while(getline(ifs, str))
        {
            values.push_back(str);
        }
        ifs.close();
    }
    static bool write(const std::string& fname, const std::vector<std::string>& values)
    {
        ofstream ofs;
        ofs.open(fname.c_str(), ios::out | ios::trunc);
        if(!ofs)
        {
            return false;
        }
        for (auto &line : values) {
            ofs << line << endl;
        }
        ofs.close();
    }
};

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
    //从日志索引文件里面加载日志文件列表
    void loadBinlogListFromIndex();
    //保存一个日志文件信息到索引文件中
    void saveBinlogListToIndex();

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
    BinlogBufferReader(){}
    BinlogBufferReader(char* buf, int size)
    {
        m_buf = buf;
        m_len = size;
    }
    ~BinlogBufferReader(){}

    BinLog::LogItem* firstItem(void) const {
        if (m_len == 0) {
            return nullptr;
        }
        return (BinLog::LogItem*)(m_buf);
    }
    BinLog::LogItem* nextItem(BinLog::LogItem* item) const {
        if ((char*)item + item->item_size - m_buf >= m_len) {
            return nullptr;
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
    BinlogParser();
    ~BinlogParser();

    bool open(const std::string& fname);
    BinlogBufferReader reader() const;
    void close();

private:
    char* m_base;
    int m_size;
    int m_fd;

    BinlogParser(const BinlogParser&);
    BinlogParser& operator=(const BinlogParser&);
};

#endif //MOUSEKV_BINLOG_H
