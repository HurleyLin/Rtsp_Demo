//============================================================================
// Name        : comm.h
// Author      : Hurley
// Mail		   : 1150118636@qq.com
// Version     : 1.0.0
// Create On   : Oct 13, 2018
// Copyright   : Copyright (c) 2018 Hurley All rights reserved.
// Description : Hello World in C++, Ansi-style
//============================================================================

#ifndef COMM_H_
#define COMM_H_

#include <stdio.h>

#define dbg(fmt,arg ...) \
do{\
	printf("\033[1;33m[DEBUG %s:%s:%d]:"fmt,__FILE__,__FUNCTION__,__LINE__,##arg);\
	printf("\033[0;39m\n");\
}while(0)

#define info(fmt,arg ...) \
do{\
	printf("\033[1;32m[INFO %s:%s:%d]:"fmt,__FILE__,__FUNCTION__,__LINE__,##arg);\
	printf("\033[0;39m\n");\
}while(0)

#define warn(fmt,arg ...) \
do{\
	printf("\033[1;34m[WARN %s:%s:%d]:"fmt,__FILE__,__FUNCTION__,__LINE__,##arg);\
	printf("\033[0;39m\n");\
}while(0)

#define ERR(fmt,arg ...) \
do{\
	printf("\033[1;31m[ERROR %s:%s:%d]:"fmt,__FILE__,__FUNCTION__,__LINE__,##arg);\
	printf("\033[0;39m\n");\
}while(0)


#define __LINUX__ 1


#ifdef __WIN32__
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

#endif /* COMM_H_ */
