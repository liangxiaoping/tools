#ifndef DATATYPE_H_
#define DATATYPE_H_

// define
#define	WM_TCPEVENT		  WM_USER+1

// const
const char NULL_CHAR           = '\0';
const int  kMaxHostNameLen     = 1024;
const int  kMaxRecvBufLen      = 65536;
const int  kMaxIPV4AddressLen  = 16;

const int kMaxRetryDelayMs  = 30*1000;
const int kInitRetryDelayMs = 500;

const int kConnectTimerID = 1;
const int kSendTimerID    = 2;


// typedef
typedef struct RecvBuf 
{
    struct sockaddr_in	addr;
    int					addrSize;
    int					size;
    char				msgBuf[kMaxRecvBufLen];
}TRecvBuf;



#endif // DATATYPE_H_
