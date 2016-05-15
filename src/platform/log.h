#ifndef _LOG_H_
#define _LOG_H_

#include <string>
#include <vector>
#include <stdint.h>

#define MAX_LOG_FILE_SIZE (1024 * 1024 * 20)
#define MAX_FILE_NUM (9)

struct LogFileInfo;
enum Level
{
    DEBUG,
    TRACE,
    INFO,
    WARN,
    ERROR
};

struct LogConf
{
    Level level;
};

class Log
{
public:
    Log();
    ~Log();
    int Init(std::string path, std::string base_file);
    void Write(Level level, const char* format, ...);
    void SetLogLevel(Level level);
    void Fini();
private:
    Log(const Log&);
    Log& operator= (const Log&);
    int InitFileName();
    void MakeFileNameAndIndex();
    bool IsLogFile(const char* filename, int32_t* index);
    bool IsNeedSwitchFile(int msg_size);
    void SwitchFile();
    const char* LevelString(Level level);
    void SyncFile();
    int MakeDirs(std::string path);
    std::string AbsoluteFileName(const char* filename);
    void OpenLogFile(const char* mode);

    std::string m_current_file;
    std::string m_path;
    std::string m_base_file;
    int32_t m_file_index;
    FILE* m_file;
    uint64_t m_last_synctime;
    Level m_level;
    uint64_t m_current_size;
};

Log* GlobalLog();
int LogInit(Log* log, std::string path, std::string base_file);
void LogRefresh(Log* log, const LogConf& conf);
void LogFini(Log* log);

#define LogDebug(format, args...) \
    do \
    { \
        if (GlobalLog()) \
        { \
            GlobalLog()->Write(DEBUG, "[%s:%u %s]:" format, __FILE__, __LINE__, __FUNCTION__, ##args); \
        } \
    } while(0)

#define LogInfo(format, args...) \
    do \
    { \
        if (GlobalLog()) \
        { \
            GlobalLog()->Write(INFO, "[%s:%u %s]:" format, __FILE__, __LINE__, __FUNCTION__, ##args); \
        } \
    } while(0)

#define LogTrace(format, args...) \
    do \
    { \
        if (GlobalLog()) \
        { \
            GlobalLog()->Write(TRACE, "[%s:%u %s]:" format, __FILE__, __LINE__, __FUNCTION__, ##args); \
        } \
    } while(0)

#define LogWarn(format, args...) \
    do \
    { \
        if (GlobalLog()) \
        { \
            GlobalLog()->Write(WARN, "[%s:%u %s]:" format, __FILE__, __LINE__, __FUNCTION__, ##args); \
        } \
    } while(0)

#define LogError(format, args...) \
    do \
    { \
        if (GlobalLog()) \
        { \
            GlobalLog()->Write(ERROR, "[%s:%u %s]:" format, __FILE__, __LINE__, __FUNCTION__, ##args); \
        } \
    } while(0)

#endif
