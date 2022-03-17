#ifndef  _AFX_LOG_FILE_H_
#define _AFX_LOG_FILE_H_

#include "stdio.h"
#include "stdlib.h"
#include <vector>
#include <pthread.h>
#include <unistd.h>
#include <string>
#include <string.h>
#include <linux/string.h>
#include <sys/stat.h> 
#include  <cstdarg>
#include <limits.h>
#include <sys/types.h>

#define  MAX_STR_LEN 2048
#define MAX_PATH  256

#define DEBUG_VERSION   // 定义此变量则表示调试软件，未定义则为发布版 所有文件路径创建在bin文件夹下

using namespace std;

class CLogFile  
{
public:
	//日志级别枚举
	enum EnumLogLevel
	{
		LogLevelAll = 0,    //所有信息都写日志
		LogLevelMid,        //写错误、警告信息
		LogLevelNormal,     //只写错误信息
		LogLevelStop        //不写日志
	};
public:
    //默认构造函数
    CLogFile();
    //构造函数
    CLogFile(const char * strLogPath);
    //析构函数
    virtual ~CLogFile();
public:
    //写错误信息
    void TraceError(const char* strInfo, ...);
    //写警告信息
    void TraceWarning(const char * strInfo, ...);
    //写一般信息
    void TraceInfo(const char * strInfo, ...);
    //设置写日志级别
    void SetLogLevel(EnumLogLevel nLevel);

	int CreateFileDir(const char *sPathName);
private:
    string GetAppPath();
    //写文件操作
    void Trace(const char * strInfo);
    //获取当前系统时间
    char * GetCurrentTime();
    //创建日志文件名称
    void GenerateLogName();
	//创建日志目录
    //创建日志路径
    void CreateLogPath();
private:
    //写日志文件流
    FILE * m_pFileStream;
    //写日志级别
    EnumLogLevel m_nLogLevel;
    //日志的路径
    char m_strLogPath[MAX_STR_LEN];
    //日志的名称
    char m_strCurLogName[MAX_STR_LEN];
    //线程同步的临界区变量
    pthread_mutex_t m_cs;
};

#endif 
