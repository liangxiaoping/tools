#ifndef DATATYPE_H_
#define DATATYPE_H_

// define
#define	WM_UDPRECVDATA		WM_USER+1


// const
const char NULL_CHAR    = '\0';
const int  MAX_BUF      = 1024;
const int  MAX_SOCKBUF  = 65536;
const int  MAX_UDPBUF   = 32768;
const int  MaxIPV4AddressLength = 16;

// typedef
typedef struct RecvBuf 
{
    struct sockaddr_in	addr;
    int					addrSize;
    int					size;
    char				msgBuf[MAX_UDPBUF];
}TRecvBuf;



#endif // DATATYPE_H_
