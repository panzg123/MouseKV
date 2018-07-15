//
// Created by zgpan on 2018/7/8.
//

#include "leveldb.h"
#include <sstream>
#include <functional>
#include <cassert>

LevelDb::LevelDb()
{

}

LevelDb::~LevelDb()
{
    delete m_db;
}

void LevelDb::InitDb(const string& db_name)
{
    m_db = NULL;
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
}

bool LevelDbCluster::InitCluster(int db_num,string db_dir)
{
    m_level_db_size = db_num;
    m_work_dir = db_dir;  //TODO 需要检查data是否存在
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
    return NULL;
}

bool LevelDbCluster::setValue(const string &key, const string &value)
{
    LevelDb *db = getLevelDbByKey(key);
    if(db == NULL)
        return false;
    return db->setValue(key,value);
}

bool LevelDbCluster::getValue(const string &key, string &value)
{
    LevelDb *db = getLevelDbByKey(key);
    if(db == NULL)
        return false;
    return db->getValue(key,value);
}

bool LevelDbCluster::delKey(const string &key)
{
    LevelDb *db = getLevelDbByKey(key);
    if(db == NULL)
        return false;
    return db->delKey(key);
}

LevelDbCluster* LevelDbCluster::instance(void)
{
    static LevelDbCluster cluster;
    return &cluster;
}