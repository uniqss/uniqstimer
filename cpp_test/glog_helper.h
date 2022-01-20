#pragma once

#if defined(WIN32) || defined(WINDOWS) || defined(_WIN32)
#define GLOG_SYSTEM_TYPE_WINDOWS
#else
#define GLOG_SYSTEM_TYPE_LINUX
#endif

#ifdef GLOG_SYSTEM_TYPE_WINDOWS
#include <windows.h>
#include <io.h>
#include <direct.h>
#else
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#endif

#include <string.h>

#define GOOGLE_GLOG_DLL_DECL
#define GLOG_NO_ABBREVIATED_SEVERITIES
#include "glog/logging.h"

#include <chrono>
#include <vector>
#include <string>
using namespace std;


#ifdef GLOG_SYSTEM_TYPE_WINDOWS

#else
bool dirExists(const std::string& dirName_in) {
    return access(dirName_in.c_str(), F_OK) == 0;
}
#endif

#include <string>
#include <iostream>

void init_glog(const char* pname, const std::string& logDir);
