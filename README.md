# MouseKV
oh, something cool. just for fun

base from [OneValue](https://github.com/onexsoft/OneValue)

项目简介：
1. 多线程`Reactor`模型，主线程监听连接请求，8子线程处理收发包和业务逻辑，底层配置多个`LevelDb`构成`LevelDbCluster`
2. 支持主从同步，`Binlog`实现机制
3. `Redis`客户端协议，可复用`redis-cli`


## 图解

##### SVR UML简介

![Image text](https://github.com/panzg123/MouseKV/blob/master/images/MouseKv_UML.png)

关键类：
- `Mouse`：继承于Server，服务的主体类，启动函数`runMouseSvr`，读取配置文件、初始化`CmdTable`、`LeveldbCluster`、`EventLoopThreadPool`、`SyncThread`
- `CmdTable`： 管理端CMD到Handle的映射，如`GET/SET/DEL/SYNC/COPY`等
- `LeveldbCluster`：管理端多个`LevelDb`，封装`GET/SET/DEL`，同步写`BinLog`
- `EventLoopThreadPool`：线程池，`Mouse`监听到客户端连接后，将fd注册到某个thread的`event_loop`中，创建`Context`，监听client读写事件
- `Context`：`redis-cli`的上下文信息，包含`server`指针、`io buf`、`event_loop`指针
- `SyncThread`：从节点用于从主节点同步数据的线程

#### BinLog机制

关键类：
- `LogItem` 一条日志信息
- `BinLog` 单个日志文件
- `BinlogFileList` 日志文件列表类
- `BinlogSyncStream` 主从同步时的一条数据流
- `BinlogBufferReader` 主节点读取binLog记录类
- `SyncThread` 为Slave节点的同步线程，向Master节点发送`__SYNC`命令，然后接收数据写存储
- `DbCopyThread` 当A节点收到B节点`__COPY`命令时，在后台启动Copy线程，伪装客户端建立同节点B的链接，然后遍历LevelDB，将数据同步到节点B中

单条操作binLog日志信息，`LogItem`示意图如下，存储于`binlog`目录下多个文件，binlog文件的索引存在于`BINLOG_INDEX`；Slave节点的
同步偏移信息存储于`MASTER_INFO`文件中。

![Image text](https://github.com/panzg123/MouseKV/blob/master/images/MouseKv_LogItem.png)




## TODO
- [x] Log
- [x] levelDb
- [x] binlog
- [x] master-slave
- [ ] restful api
- [x] multi work process
- [ ] support cas
- [ ] support set,list,hash


## ChangeLog
20180714 Tag1.0 单线程done，默认配置8个leveldb

20200329 Tag2.0 多线程+Binlog主从同步 done


## BenchMark

    ubuntu@VM-0-3-ubuntu:~/tool/redis-4.0.0/src$ ./redis-benchmark -h localhost -p 10086 -t set,get -c 10 -n 10000 -q
    SET: 14306.15 requests per second
    GET: 18315.02 requests per second

作为对比，redis4.0 benchmark信息

    ubuntu@VM-0-8-ubuntu:~/github/redis-4.0.10/src$ ./redis-benchmark -h localhost -t set,get -c 10 -n 10000 -q
    SET: 52356.02 requests per second
    GET: 51282.05 requests per second

### Environment

	腾讯云 1核 1GB 2Mbps
