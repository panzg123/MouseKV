//
// Created by zgpan on 2018/7/8.
//

#include "leveldb.h"
#include <sstream>
#include <functional>
#include <cassert>
#include <utility>

LevelDb::LevelDb()
{

}

LevelDb::~LevelDb()
{
    delete m_db;
}

void LevelDb::InitDb(const string& db_name)
{
    m_db = nullptr;
    m_options.create_if_missing = true;
    m_db_name = db_name;
}

bool LevelDb::openDb()
{
    leveldb::Status status = leveldb::DB::Open(m_options, m_db_name, &m_db);
    if(status.ok())
    {
        return true;
    }
    return false;
}

bool LevelDb::setValue(const string &key, const string &value)
{
    leveldb::Slice _key(key.data(), key.size());
    leveldb::Slice _value(value.data(), value.size());
    leveldb::WriteOptions options;
    leveldb::Status status = m_db->Put(options, _key, _value);
    return status.ok();
}

bool LevelDb::getValue(const string &key, string &value)
{
    leveldb::Slice _key(key.data(), key.size());
    leveldb::ReadOptions options;
    leveldb::Status status = m_db->Get(options, _key, &value);
    return status.ok();
}

bool LevelDb::delKey(const string& key)
{
    leveldb::Slice _key(key.data(), key.size());
    leveldb::WriteOptions options;
    leveldb::Status status = m_db->Delete(options, _key);
    return status.ok();
}


//-------------------------------------------------leveldbCluster-------------------------------------------------------
LevelDbCluster::LevelDbCluster()
{
}
LevelDbCluster::~LevelDbCluster()
{
    for (unsigned int i = 0; i < m_vec_dbs.size(); ++i)
    {
        LevelDb* db = m_vec_dbs[i];
        delete db;
    }
    m_curBinlog.close();
}

bool LevelDbCluster::InitCluster(int db_num,string db_dir)
{
    m_level_db_size = db_num;
    m_work_dir = std::move(db_dir);  //TODO 需要检查data是否存在
    for (int i = 0; i < m_level_db_size; ++i)
    {
        LevelDb *db = new LevelDb;
        ostringstream oss;
        oss << m_work_dir << "db" << i;
        db->InitDb(oss.str());
        if(!db->openDb())
        {
            COMM_LOG(Logger::ERROR,"InitCluster error, open failed\n");
            return false;
        }
        m_vec_dbs.push_back(db);
    }

    //初始化BinLogFileList
    m_binlogFileList.loadBinlogListFromIndex();

    //需要处理binlog目录不存在的问题--从binlogfile里面打开最后一个文件
    string last_binlog_file;
    if(!m_binlogFileList.isEmpty())
    {
        last_binlog_file = m_binlogFileList.fileName(m_binlogFileList.fileCount() - 1);
    }
    else
    {
        //创建一个全新的BinLog日志文件
        last_binlog_file = buildRandomBinlogFileBaseName();
        m_binlogFileList.appendNewBinlogFile(last_binlog_file);
        m_binlogFileList.saveBinlogListToIndex();
    }
    m_curBinlog.open(getBinlogFullPath(last_binlog_file));
    COMM_LOG(Logger::DEBUG, "cur binlog file[%s]", last_binlog_file.c_str());
    return true;
}

LevelDb* LevelDbCluster::getLevelDbByKey(const string &key)
{
    //先根据key来计算db索引值
    //todo 暂时先用std::hash，考虑到后期hash性能，可以用mururhash
    size_t key_hash = std::hash<std::string>{}(key);
    size_t db_idx = key_hash%m_level_db_size;
    if(m_vec_dbs.size()>db_idx)
    {
        return m_vec_dbs[db_idx];
    }
    return nullptr;
}

bool LevelDbCluster::setValue(const string &key, const string &value)
{
    LevelDb *db = getLevelDbByKey(key);
    if(db == nullptr)
        return false;
    bool _ret =  db->setValue(key,value);
    if(_ret)
    {
        m_curBinlog.appendSetRecord(key,value);
        adjustCurrentBinlogFile();
    }
    return _ret;
}

bool LevelDbCluster::getValue(const string &key, string &value)
{
    LevelDb *db = getLevelDbByKey(key);
    if(db == nullptr)
        return false;
    return db->getValue(key,value);
}

bool LevelDbCluster::delKey(const string &key)
{
    LevelDb *db = getLevelDbByKey(key);
    if(db == nullptr)
        return false;
    bool _ret =  db->delKey(key);
    if(_ret)
    {
        m_curBinlog.appendDelRecord(key);
        adjustCurrentBinlogFile();
    }
    return _ret;
}

void LevelDbCluster::adjustCurrentBinlogFile()
{
    COMM_LOG(Logger::DEBUG,"adjustCurrentBinlogFile begin, m_writenSize[%d]", m_curBinlog.writtenSize());
    if (m_curBinlog.writtenSize() >= 64*1024*1024) //Binlog日志最大64M
    {
        m_curBinlog.close();
        std::string newFileBaseName = buildRandomBinlogFileBaseName();
        std::string fullPath =   getBinlogFullPath(newFileBaseName);
        if (m_curBinlog.open(fullPath)) {
            m_binlogFileList.appendNewBinlogFile(newFileBaseName);
            m_binlogFileList.saveBinlogListToIndex();
        }
        COMM_LOG(Logger::DEBUG, "adjust binlog file, newFileBaseName[%s]", newFileBaseName.c_str());
    }
}

std::string LevelDbCluster::buildRandomBinlogFileBaseName() const
{
    time_t t = time(nullptr);
    tm* lt = localtime(&t);

    char buff[1024];
    sprintf(buff, "%d%02d%02d_%02d%02d%02d-%d-bin",
            lt->tm_year + 1900, lt->tm_mon + 1, lt->tm_mday,
            lt->tm_hour, lt->tm_min, lt->tm_sec, rand() % 100000);
    return buff;
}

LevelDbCluster* LevelDbCluster::instance()
{
    static LevelDbCluster cluster;
    return &cluster;
}