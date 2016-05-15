#include "log.h"
#include <iostream>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <stdio.h>
#include <sstream>
#include <unistd.h>
#include <sys/time.h>

#define SYNC_FILE_INTERVAL (3)
#define MAX_PATH_STR_LEN (1024)
#define MAX_PATH_DEPTH (10)

struct LogFileInfo
{
    uint64_t lastmodifytime;
    std::string filename;
    int32_t index;
    uint64_t size;
};

Log::Log() : m_current_file(""), m_path(""), m_base_file(""), m_file_index(0), m_file(NULL),
             m_last_synctime(0), m_level(DEBUG), m_current_size(0)
{

}

Log::~Log()
{ 
    if (m_file)
    {
        fclose(m_file);
        m_file = NULL;
    }
}

void Log::SetLogLevel(Level level)
{
    m_level = level;
}

const char* Log::LevelString(Level level)
{
    switch (level)
    {
        case DEBUG:
            return "DEBUG";
        case INFO:
            return "INFO";
        case ERROR:
            return "ERROR";
        case WARN:
            return "WARN";
        case TRACE:
            return "TRACE";
        default:
            return NULL;
    }
    return NULL;
}

int Log::MakeDirs(std::string path)
{
    char str[MAX_PATH_STR_LEN];
    int depth = 0;
    int plen = path.size();
    if (plen == 0 || plen >= (MAX_PATH_STR_LEN - 1))
    {
        std::cout<<"path:"<<path<<", length too long"<<std::endl;
        return -1;
    }
    int i = 0;
    for (; i < plen; ++i)
    {
        str[i] = path[i];
        if (str[i] == '/')
        {
            str[i + 1] = 0;
            if (access(str, F_OK) != 0)
            {
                int ret = mkdir(str, 0775);
                if (ret != 0)
                {
                    std::cout<<"mkdir "<<str<<" failed.errno:"<<errno<<std::endl;
                    return -1;
                }
            }
        }
    }
    str[i] = 0;
    if (access(str, F_OK) != 0)
    {
        int ret = mkdir(str, 0775);
        if (ret != 0)
        {
            std::cout<<"mkdir "<<str<<" failed.errno:"<<errno<<std::endl;
            return -1;
        }
    }
    return 0;
}

int Log::Init(std::string path, std::string base_file)
{
    m_path = path;
    m_base_file = base_file;
    if (MakeDirs(m_path) != 0)
    {
        return -1;
    }
    if (InitFileName() != 0)
    {
        return -1;
    }
    OpenLogFile("a");
    return 0;
}

bool Log::IsLogFile(const char* filename, int32_t* index)
{
    size_t basefile_len = m_base_file.size();
    int flen = strlen(filename);
    if (flen < basefile_len)
    {
        return false;
    }
    
    if (strncmp(filename, m_base_file.c_str(), basefile_len) != 0)
    {
        return false;
    }

    int sub = flen - basefile_len;
    if (sub == 0)
    {
        *index = 0;
        return true;
    }

    if (sub != 2)
    {
        return false;
    }

    if (filename[basefile_len] != '.')
    {
        return false;
    }
    int32_t counter = atoi(&filename[basefile_len + 1]);
    if (counter > MAX_FILE_NUM || counter < 1)
    {
        return false;
    }
    *index = counter;
    return true;
}

void Log::MakeFileNameAndIndex()
{
    m_file_index = ++m_file_index % (MAX_FILE_NUM + 1);
    if (m_file_index == 0)
    {
        m_current_file = AbsoluteFileName(m_base_file.c_str());
        return;
    }

    std::ostringstream oss;
    oss.str("");
    oss << m_base_file << "." << m_file_index;
    m_current_file = AbsoluteFileName(oss.str().c_str());
}

void Log::OpenLogFile(const char* mode)
{
    if (MakeDirs(m_path) != 0)
    {
        m_file = NULL;
        return;
    }
    m_file = fopen(m_current_file.c_str(), mode);
}

int Log::InitFileName()
{
    DIR* dir = opendir(m_path.c_str());
    if (dir == NULL)
    {
        std::cout<<"open "<<m_path<<" error, errno:"<<errno<<std::endl;
        return -1;
    }
    struct dirent* ent;
    LogFileInfo info;
    info.lastmodifytime = 0;
    info.filename = "";
    info.index = 0;
    info.size = 0;
    while ((ent = readdir(dir)) != NULL)
    {   
        int32_t index;
        if (!IsLogFile(ent->d_name, &index))
        {
            continue;
        }
        struct stat buf;
        buf.st_mtime = 0;
        std::string absolute_file = AbsoluteFileName(ent->d_name);
        stat(absolute_file.c_str(), &buf);
        if (buf.st_mtime > info.lastmodifytime)
        {
            info.lastmodifytime = buf.st_mtime;
            info.filename = absolute_file;
            info.index = index;
            info.size = buf.st_size;
        }
    }
    closedir(dir);
    if (info.lastmodifytime == 0 || info.filename == "")
    {
        info.filename = AbsoluteFileName(m_base_file.c_str());
        info.index = 0;
        info.size = 0;
    }
    m_current_file = info.filename;
    m_file_index = info.index;
    m_current_size = info.size;
    return 0;
}

std::string Log::AbsoluteFileName(const char* filename)
{
    if (m_path[m_path.size() - 1] == '/')
    {
        return m_path + filename;
    }
    return m_path + "/" + filename;
}

void Log::Write(Level level, const char* format, ...)
{
    if (level < m_level)
    {
        return;
    }
    if (!m_file)
    {
        return;
    }
    struct timeval tv;
    gettimeofday(&tv, NULL);
    char buf[1024];
    struct tm* timeinfo;
    timeinfo = localtime(&tv.tv_sec);
    int offset = snprintf(buf, sizeof(buf), "%04d%02d%02d %02d:%02d:%02d:%02ld", 
                          timeinfo->tm_year + 1900,  timeinfo->tm_mon + 1, timeinfo->tm_mday, 
                          timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, tv.tv_usec);
    offset += snprintf(buf + offset, sizeof(buf) - offset, " [%s]", LevelString(level));
    va_list vlist;
    va_start(vlist, format);
    offset += vsnprintf(buf + offset, sizeof(buf) - offset, format, vlist);
    va_end(vlist);
    if (offset <= 0)
    {
        return;
    }
    if (offset < sizeof(buf) - 1)
    {
        buf[offset] = '\n';
        buf[offset + 1] = '\0';
        offset += 1;
    }
    else
    {
        buf[offset - 1] = '\n';
        buf[offset] = '\0';
    }
    if (IsNeedSwitchFile(offset))
    {
        SwitchFile();
    }
    fwrite(buf, offset, 1, m_file);
    m_current_size += static_cast<uint64_t>(offset);
    if (tv.tv_sec - m_last_synctime >= SYNC_FILE_INTERVAL)
    {
        SyncFile();
    }
}

inline void Log::SyncFile()
{
    if (m_file)
    {
        fflush(m_file);
    }
}

void Log::Fini()
{
    SyncFile();
}

bool Log::IsNeedSwitchFile(int msg_size)
{
    if ((m_current_size + static_cast<uint64_t>(msg_size)) >= MAX_LOG_FILE_SIZE)
    {
        return true;
    }
    return false;
}

void Log::SwitchFile()
{
    if (m_file)
    {
        fclose(m_file);
        m_file = NULL;
    }
    MakeFileNameAndIndex();
    OpenLogFile("w");
    m_current_size = 0;
}

Log* GlobalLog()
{
    static Log log;
    return &log;
}

int LogInit(Log* log, std::string path, std::string base_file)
{
    if (!log)
    {
        return -1;
    }
    return log->Init(path, base_file);
}

void LogRefresh(Log* log, const LogConf& conf)
{
    if (!log)
    {
        return;
    }
    log->SetLogLevel(conf.level);
}

void LogFini(Log* log)
{
    if (log)
    {
        log->Fini();
    }
}
