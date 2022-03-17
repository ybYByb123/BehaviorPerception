#include "LogFile.h"

//////////////////////////////////////////////////////////////////////
static const char *ERRORPREFIX = " 错误: ";
static const char *WARNINGPREFIX = " 警告: ";
static const char *INFOPREFIX = " 信息: ";
//////////////////////////////////////////////////////////////////////
//默认构造函数
CLogFile::CLogFile()
{
    //初始化
    memset(m_strLogPath, 0, MAX_STR_LEN);
    memset(m_strCurLogName, 0, MAX_STR_LEN);
    m_pFileStream = NULL;
    //设置默认的写日志级别
    m_nLogLevel = EnumLogLevel::LogLevelNormal;

    setlocale(LC_ALL, "");

    CreateLogPath();
    //初始化临界区变量
    pthread_mutex_init(&m_cs, NULL);
    //创建日志文件名
    GenerateLogName();
}

//构造函数
CLogFile::CLogFile(const char *strLogPath)
{
    //初始化
    m_pFileStream = NULL;
    strcpy(m_strLogPath, strLogPath);
    //初始化临界区变量
    pthread_mutex_init(&m_cs, NULL);
    CreateLogPath();
    GenerateLogName();
}

//析构函数
CLogFile::~CLogFile()
{
    //释放临界区
    pthread_mutex_destroy(&m_cs);
    //关闭文件流
    if (m_pFileStream)
        fclose(m_pFileStream);
}

//写错误信息
void CLogFile::TraceError(const char *strInfo, ...)
{
    //判断当前的写日志级别，若设置为不写日志则函数返回
    if (!strInfo)
        return;
    char pTemp[MAX_STR_LEN] = {0};
    strcpy(pTemp, GetCurrentTime());
    strcat(pTemp, ERRORPREFIX);
    va_list arg_ptr;
    va_start(arg_ptr, strInfo);
    vsprintf(pTemp + strlen(pTemp), strInfo, arg_ptr);
    va_end(arg_ptr);
    Trace(pTemp);
    arg_ptr;
}

//写警告信息
void CLogFile::TraceWarning(const char *strInfo, ...)
{
    //判断当前的写日志级别，若设置为只写错误信息则函数返回
    if (!strInfo)
        return;
    char pTemp[MAX_STR_LEN] = {0};
    strcpy(pTemp, GetCurrentTime());
    strcat(pTemp, WARNINGPREFIX);
    va_list arg_ptr;
    va_start(arg_ptr, strInfo);
    vsprintf(pTemp + strlen(pTemp), strInfo, arg_ptr);
    va_end(arg_ptr);
    Trace(pTemp);
    arg_ptr;
}

//写一般信息
void CLogFile::TraceInfo(const char *strInfo, ...)
{
    //判断当前的写日志级别，若设置只写错误和警告信息则函数返回
    if (!strInfo)
        return;
    char pTemp[MAX_STR_LEN] = {0};
    strcpy(pTemp, GetCurrentTime());
    strcat(pTemp, INFOPREFIX);
    va_list arg_ptr;
    va_start(arg_ptr, strInfo);
    vsprintf(pTemp + strlen(pTemp), strInfo, arg_ptr);
    va_end(arg_ptr);
    Trace(pTemp);
    arg_ptr;
}

//获取系统当前时间
char *CLogFile::GetCurrentTime()
{
    time_t curTime;
    struct tm *pTimeInfo = NULL;
    time(&curTime);
    pTimeInfo = localtime(&curTime);
    char temp[MAX_STR_LEN] = {0};
    sprintf(temp, "%02d:%02d:%02d", pTimeInfo->tm_hour, pTimeInfo->tm_min, pTimeInfo->tm_sec);
    char *pTemp = temp;
    return pTemp;
}

//设置写日志级别
void CLogFile::SetLogLevel(EnumLogLevel nLevel)
{
    m_nLogLevel = nLevel;
}

int CLogFile::CreateFileDir(const char *sPathName)
{
    char DirName[256];
    strcpy(DirName, sPathName);
    int i, len = strlen(DirName);
    for (i = 1; i < len; i++)
    {
        if (DirName[i] == '/')
        {
            DirName[i] = 0;
            if (access(DirName, NULL) != 0)
            {
                if (mkdir(DirName, 0755) == -1)
                {
                    printf("mkdir error\n");
                    return -1;
                }
            }
            DirName[i] = '/';
        }
    }
    return 0;
}

//写文件操作
void CLogFile::Trace(const char *strInfo)
{
    if (!strInfo)
        return;
    try
    {
        //进入临界区
        // EnterCriticalSection(&m_cs);
        pthread_mutex_lock(&m_cs);
        GenerateLogName();
        //若文件流没有打开，则重新打开
        if (!m_pFileStream)
        {
            char temp[1024] = {0};
            strcat(temp, m_strLogPath);
            strcat(temp, m_strCurLogName);
            m_pFileStream = fopen(temp, "a+");
            if (!m_pFileStream)
            {
                return;
            }
        }
        //写日志信息到文件流
        fprintf(m_pFileStream, "%s\r\n", strInfo);

        fflush(m_pFileStream);
        //离开临界区
        pthread_mutex_unlock(&m_cs);
    }
    //若发生异常，则先离开临界区，防止死锁
    catch (...)
    {
        pthread_mutex_unlock(&m_cs);
    }
}

//创建日志文件的名称
void CLogFile::GenerateLogName()
{
    time_t curTime;
    struct tm *pTimeInfo = NULL;
    time(&curTime);
    pTimeInfo = localtime(&curTime);
    char temp[1024] = {0};
    //日志的名称如：2013-01-01.log
    sprintf(temp, "%04d-%02d-%02d.log", pTimeInfo->tm_year + 1900, pTimeInfo->tm_mon + 1, pTimeInfo->tm_mday);
    if (0 != strcmp(m_strCurLogName, temp))
    {
        strcpy(m_strCurLogName, temp);
        if (m_pFileStream)
            fclose(m_pFileStream);
        char temp[1024] = {0};
        strcat(temp, m_strLogPath);
        strcat(temp, m_strCurLogName);
        //以追加的方式打开文件流
        m_pFileStream = fopen(temp, "a+");
    }
}

string CLogFile::GetAppPath()
{
    char result[MAX_PATH];
    ssize_t count = readlink("/proc/self/exe", result, MAX_PATH);
    string exePath = string(result, (count > 0) ? count : 0);
    size_t pos = exePath.find_last_of("\\/");
    return (string::npos == pos)
               ? ""
               : exePath.substr(0, pos);
}

//创建日志文件的路径
void CLogFile::CreateLogPath()
{
    string strPath;

    strPath = GetAppPath() + "/data/logs/";

    strcpy(m_strLogPath, strPath.c_str());

    if (access(strPath.c_str(), 0) == -1)
    {
        CreateFileDir(strPath.c_str());
    }
}