//
// Created by zgpan on 2018/7/8.
// desc: 简单的日志库，支持日志级别和按文件大小滚动
// thanks for ssdb
//

#ifndef MOUSEKV_LOG_H
#define MOUSEKV_LOG_H

#include <string>
#include <time.h>
#include <sys/time.h>
#include <cassert>
#include <errno.h>
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdarg.h>


using namespace std;

class Logger
{
	public:
    enum Level
    {
    	DEBUG = 0,
		WARN = 1,
		ERROR = 2,
    };

	public:
		bool InitLogger(Logger::Level level);//走配置
		int outputLog(int level, const char *format, ...);//写日志
		static Logger* instance();
		void LogRotation(); //日志文件滚动
		
		
	private:
		int m_file_fd;                              //文件描述符
		int m_level;                                //配置的日志级别
		const int m_max_file_size = 20*1024*1024;   //单个日志文件最大20M
		const int m_max_line_size = 4*1024;         //单行最大4k
		int m_cur_file_size;
		const string m_default_log = "./log/server.log"; //默认日志文件
};


//宏定义
#define COMM_LOG(level,fmt, args...)	\
		Logger::instance()->outputLog(level,"%s(%d): " fmt, __FILE__, __LINE__, ##args)

#endif
