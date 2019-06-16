#include "log.h"
#include <cassert>
#include <sstream>


Logger* Logger::instance()
{
    static Logger log;
	return &log;
}

bool Logger::InitLogger(Logger::Level level)
{
	m_level = level;
	m_cur_file_size = 0;
    //打开文件，如果文件不存在，则创建
    m_file_fd = ::open(m_default_log.c_str(), O_CREAT | O_WRONLY | O_APPEND, 0644);
	if(m_file_fd == -1){
		fprintf(stderr, "open log file failed, errno[%d]\n", errno);
		return false;
	}

	struct stat st;
	int ret = fstat(m_file_fd, &st);
	m_cur_file_size = st.st_size;  //记录下当前日志的大小     
	return true;
}

void Logger::LogRotation()
{
	::close(this->m_file_fd);
	char newpath[100];
	time_t time;
	struct timeval tv;
	struct tm *tm, tm_tmp;
	gettimeofday(&tv, NULL);
	time = tv.tv_sec;
	tm = localtime_r(&time, &tm_tmp);
	sprintf(newpath, "%s.%04d%02d%02d-%02d%02d%02d",
		this->m_default_log.c_str(),
		tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
		tm->tm_hour, tm->tm_min, tm->tm_sec);
	fprintf(stderr,"rename. from[%s], to[%s]\n", m_default_log.c_str(), newpath);
	int ret = rename(this->m_default_log.c_str(), newpath);
	if(ret == -1)
	{
		fprintf(stderr,"rename failed, ret=%d\n",ret);
		return;
	}
	this->m_file_fd= ::open(m_default_log.c_str(), O_CREAT | O_WRONLY | O_APPEND, 0644);
	if(this->m_file_fd == -1){
		fprintf(stderr, "open log file %s error - %s\n", m_default_log.c_str(), strerror(errno));
		return;
	}
	m_cur_file_size = 0;
}



int Logger::outputLog(int level, const char *format, ...)
{
    if(level < m_level)
		return 0;
	char buf[m_max_line_size];
	char* cur_pos = buf;
	//获取当前日期
	time_t cur_time;
	struct tm *cur_tm;
	time(&cur_time);
	cur_tm=localtime(&cur_time); /*取得当地时间*/
	int len = sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d ",
		cur_tm->tm_year + 1900, cur_tm->tm_mon + 1, cur_tm->tm_mday,
		cur_tm->tm_hour, cur_tm->tm_min, cur_tm->tm_sec);
	if(len < 0)
		return -1;
	string str_now_time(buf);
	cur_pos += len;
	//把日志级别加上
	switch (level)
	{
	case Logger::DEBUG:
		{
			memcpy(cur_pos,"[DEBUG] ",8);
			cur_pos+=8;
			break;
		}
	case Logger::WARN:
		{
		    memcpy(cur_pos,"[WARN] ",7);
			cur_pos+=7;
			break;
		}
	case Logger::ERROR:
		{
		    memcpy(cur_pos,"[ERROR] ",8);
			cur_pos+=8;
			break;
		}
	default:
		return -2;
	}	
	
	//然后把日志内容写入
    va_list marker;
    va_start(marker, format);
    len = vsprintf(cur_pos, format, marker);
    va_end(marker);
	if(len < 0)
		return -2;
	cur_pos += len;
	*cur_pos = '\n';
	++cur_pos;
	*cur_pos = '\0';

	//写入文件，暂时不加锁
	write(m_file_fd, buf, cur_pos - buf);
	//fflush(m_file_fd);

	//如果文件超大，则滚动
	m_cur_file_size += (cur_pos - buf);
	if(m_cur_file_size > m_max_file_size)
	{
	   LogRotation();
	}
	return cur_pos-buf;
	
}
