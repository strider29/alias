/*
*  Useful functions used in debug
*/

#ifndef DEBUGUTIL_H_
#define DEBUGUTIL_H_

#include <functional>
#include <vector>

#include <core/logger/LoggerManager.h>
#include <core/logger/Loggers.h>


#define _CONFIG_BASIC_LOGGERS \
      typedef std::shared_ptr<core::Logger> LoggerPtr;\
      std::vector<LoggerPtr> _basic_loggers_loggers;\
      _basic_loggers_loggers.push_back(LoggerPtr(new core::ConsoleLogger));\
      for (unsigned int i = 0; i < _basic_loggers_loggers.size(); ++i) {\
          core::LoggerManager::instance().addLogger(_basic_loggers_loggers[i].get());\
      }\
      core::LoggerManager::instance().configureLevel(core::LogLevel::LL_0);\


#if defined(_WIN32) || defined(CYGWIN)

#elif defined(linux) || defined(_linux) || defined(__linux) || defined(__linux__) || defined(__APPLE__)
#  ifndef _GNU_SOURCE
#    define _GNU_SOURCE
#  endif
#  include <cstring>
#else
#  error "Unsupported platform, aborting compilation."
#endif
// Relative file name string, i.e. without path.
#ifndef __FILENAME__
#   define  __FILENAME__  (basename(__FILE__))
#endif


#if defined(DEBUG) || 1 || defined(DEBUG_LOG_ENABLE)

    #include <assert.h>
    #include <iostream>
    #include <stdio.h>
    #include <cstdio>


// shared buffer for the debugg information. THIS IS NOT THREAD SAFE!!!!!
// be carefoul when logging from different threads...
    #define _SHRD_DEBUG_BUFF_SIZE 8192
    static char _shrd_debug_buff[_SHRD_DEBUG_BUFF_SIZE+1];


    #define LOG_INFO(format, ...) {\
                    const int _len = snprintf(_shrd_debug_buff, _SHRD_DEBUG_BUFF_SIZE, "[%s, %s, %d]: " format, \
                     __FILE__, __FUNCTION__, __LINE__, ## __VA_ARGS__);\
                     if (_len < _SHRD_DEBUG_BUFF_SIZE && _len >= 0) {\
                        core::LoggerManager::instance().logMessage(_shrd_debug_buff, core::LogType::LT_INFO);\
                     }\
                    }

    #define LOG_OPTIMIZATION(format, ...) {\
                    const int _len = snprintf(_shrd_debug_buff, _SHRD_DEBUG_BUFF_SIZE, "[%s, %s, %d]: " format, \
                     __FILE__, __FUNCTION__, __LINE__, ## __VA_ARGS__);\
                     if (_len < _SHRD_DEBUG_BUFF_SIZE && _len >= 0) {\
                     core::LoggerManager::instance().logMessage(_shrd_debug_buff, core::LogType::LT_OPTIMIZATION);\
                     }\
                    }

    #define LOG_ERROR(format, ...) {\
                    const int _len = snprintf(_shrd_debug_buff, _SHRD_DEBUG_BUFF_SIZE, "[%s, %s, %d]: " format, \
                     __FILE__, __FUNCTION__, __LINE__, ## __VA_ARGS__);\
                     if (_len < _SHRD_DEBUG_BUFF_SIZE && _len >= 0) {\
                     core::LoggerManager::instance().logMessage(_shrd_debug_buff, core::LogType::LT_ERROR);\
                     }\
                    }

    #define LOG_WARNING(format, ...) {\
                    const int _len = snprintf(_shrd_debug_buff, _SHRD_DEBUG_BUFF_SIZE, "[%s, %s, %d]: " format, \
                     __FILE__, __FUNCTION__, __LINE__, ## __VA_ARGS__);\
                     if (_len < _SHRD_DEBUG_BUFF_SIZE && _len >= 0) {\
                     core::LoggerManager::instance().logMessage(_shrd_debug_buff, core::LogType::LT_WARNING);\
                     }\
                    }

    #define LOG_TODO(format, ...) {\
                    const int _len = snprintf(_shrd_debug_buff, _SHRD_DEBUG_BUFF_SIZE, "[%s, %s, %d]: " format, \
                     __FILE__, __FUNCTION__, __LINE__, ## __VA_ARGS__);\
                     if (_len < _SHRD_DEBUG_BUFF_SIZE && _len >= 0) {\
                     core::LoggerManager::instance().logMessage(_shrd_debug_buff, core::LogType::LT_TODO);\
                     }\
                    }

    #define LOG_STATUS(format, ...) {\
                     const int _len = snprintf(_shrd_debug_buff, _SHRD_DEBUG_BUFF_SIZE, "[%s, %s, %d]: " format, \
                     __FILE__, __FUNCTION__, __LINE__, ## __VA_ARGS__);\
                     if (_len < _SHRD_DEBUG_BUFF_SIZE && _len >= 0) {\
                     core::LoggerManager::instance().logMessage(_shrd_debug_buff, core::LogType::LT_STATUS);\
                     }\
                    }





// common stuff
//
#define ASSERT(x)   {const bool condition = (x); if(!condition){LOG_ERROR("Assert failed " #x "\n"); assert(false);}}
#define ASSERT_PTR(x) ASSERT(x != nullptr)


#else
    #define ASSERT(x)
    #define ASSERT_PTR(x)
    #define LOG_OPTIMIZATION(format, ...)
    #define LOG_ERROR(format, ...)
    #define LOG_WARNING(format, ...)
    #define LOG_TODO(format, ...)
    #define LOG_INFO(format, ...)
    #define LOG_STATUS(format, ...)
#endif

#endif
