#ifndef __HP_RTMP_DEFINE_H__
#define __HP_RTMP_DEFINE_H__

#ifdef WIN32
#define RTMP_EXPORT
#ifdef RTMP_EXPORT
#define RTMP_API _declspec(dllexport)
#elif RTMP_DLL
#define RTMP_API _declspec(dllimport)
#else
#define RTMP_API
#endif

#else
#define RTMP_API
#define TRACE  printf
#endif

#include <map>
#include <string>
using namespace std;


















#endif