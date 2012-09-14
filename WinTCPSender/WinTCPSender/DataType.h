#ifndef DATATYPE_H_
#define DATATYPE_H_

// define
#define	WM_TCPRECVDATA		WM_USER+1


// const
const char NULL_CHAR           = '\0';
const int  kMaxHostNameLen     = 1024;
const int  kMaxRecvBufLen      = 65536;
const int  kMaxIPV4AddressLen  = 16;

// typedef
typedef struct RecvBuf 
{
    struct sockaddr_in	addr;
    int					addrSize;
    int					size;
    char				msgBuf[kMaxRecvBufLen];
}TRecvBuf;



#endif // DATATYPE_H_
