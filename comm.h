/*************************************************************************
	File Name: comm.h
	Author: Hurley
	Mail: 11508118636@qq.com
	Created Time: Oct 13, 2018
 ************************************************************************/

#ifndef __COMM_H__
#define __COMM_H__

#include <stdio.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>
#include <ctype.h>

#define __LINUX__ 1

#define dbg(fmt, ...) do {printf("[DEBUG %s:%d:%s] " fmt, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__);} while(0)
#define info(fmt, ...) do {printf("[INFO  %s:%d:%s] " fmt, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__);} while(0)
#define warn(fmt, ...) do {printf("[WARN  %s:%d:%s] " fmt, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__);} while(0)
#define err(fmt, ...) do {printf("[ERROR %s:%d:%s] " fmt, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__);} while(0)

#ifdef __WIN32__
//#include <windows.h>
#include <winsock2.h>
#define MSG_DONTWAIT 0
#define usleep(x) Sleep((x)/1000)
#define snprintf _snprintf
typedef int socklen_t;
#endif

#ifdef __LINUX__
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#define closesocket(x) close(x)
#endif 

#endif

