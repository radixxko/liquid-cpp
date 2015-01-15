#ifndef __liquid_cpp__core__includes__net__
#define __liquid_cpp__core__includes__net__

#include <liquid-cpp/core/defines.hpp>

#ifdef PLATFORM_UNIX

    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <netdb.h>
    #include <strings.h>
    #include <sys/wait.h>
    #include <unistd.h>

    typedef int SOCKET;
    #define INVALID_SOCKET (SOCKET)(~0)
    #define SOCKET_ERROR -1
    #define closesocket(s) ::close(s)
    #define _sock_err errno
    #define __sleep(v) usleep(v * 1000)

#endif

#endif