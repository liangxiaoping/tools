
// WinTCPSenderDlg.cpp : implementation file
//

#include "stdafx.h"
#include "WinTCPSender.h"
#include "WinTCPSenderDlg.h"
#include "afxdialogex.h"

#include "DataType.h"

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <stdio.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CString g_strBuildTime(_T("WinTCPSender  Build Time : "));
CString g_strAppAbout(_T(""));

CString g_strVersion(_T("  Version 1.0\r\n"));
CString g_strCopyRight(_T("  Copyright (C) 2012 - 2013\r\n"));
CString g_strAuthor(_T("  Coded by Xiaoping Liang"));

void GeneBuildTime()
{
    g_strBuildTime += _T(__DATE__);
    g_strBuildTime += _T("-");
    g_strBuildTime += _T(__TIME__);
}

void GeneAppAbout()
{
    g_strAppAbout += g_strVersion;
    g_strAppAbout += g_strCopyRight;
    g_strAppAbout += g_strAuthor;
}


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
    CAboutDlg();

// Dialog Data
    enum { IDD = IDD_ABOUTBOX };

    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
    DECLARE_MESSAGE_MAP()
public:
    virtual BOOL OnInitDialog();
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
}

BOOL CAboutDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();
    // TODO:  Add extra initialization here
    SetDlgItemText(IDC_STATIC_BUILDTIME, g_strBuildTime);
    SetDlgItemText(IDC_STATIC_APP_ABOUT, g_strAppAbout);

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}
BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CWinTCPSenderDlg dialog



CWinTCPSenderDlg::CWinTCPSenderDlg(CWnd* pParent /*=NULL*/)
  : CDialogEx(CWinTCPSenderDlg::IDD, pParent)
, m_strSendData(_T("Enter data to send ..."))
, m_strRemoteAddr(_T("127.0.0.1"))
, m_uRemotePort(12345)
, m_uSendTimes(1)
, m_ulTimeOut(1000)
, m_ulElapseTime(0)
, m_uExecTimes(0)
, m_bSendContinual(FALSE)
, m_strLocalAddr(_T("127.0.0.1"))
, m_uLocalPort(12345)
, m_uRecvCount(0)
, m_uSentCount(0)
, m_ulRetryDelayMs(kInitRetryDelayMs)
, m_bBindLocal(FALSE)
, m_connState(kDisconnected)
, m_bOpenLogs(FALSE)
, m_socketfd(INVALID_SOCKET)
, m_fp(NULL)
{
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

CWinTCPSenderDlg::~CWinTCPSenderDlg()
{
    CloseSocket();

    if(NULL != m_fp)
    {
      fclose(m_fp);
    }
}
void CWinTCPSenderDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_REMOTE_ADDR, m_strRemoteAddr);
    DDX_Text(pDX, IDC_REMOTE_PORT, m_uRemotePort);
    DDV_MinMaxUInt(pDX, m_uRemotePort, 0, 65535);
    DDX_Text(pDX, IDC_SEND_TIMES, m_uSendTimes);
    DDV_MinMaxUInt(pDX, m_uSendTimes, 0, 100000);
    DDX_Text(pDX, IDC_LOCAL_ADDR, m_strLocalAddr);
    DDX_Text(pDX, IDC_LOCAL_PORT, m_uLocalPort);
    DDV_MinMaxUInt(pDX, m_uLocalPort, 0, 65535);
    DDX_Text(pDX, IDC_RECV_COUNT, m_uRecvCount);
    DDX_Text(pDX, IDC_SENT_COUNT, m_uSentCount);
    DDX_Text(pDX, IDC_SEND_TIMEOUT, m_ulTimeOut);
    DDX_Text(pDX, IDC_SEND_ELAPSED, m_ulElapseTime);
    DDX_Check(pDX, IDC_CHECK_CONTINUAL, m_bSendContinual);
    DDX_Check(pDX, IDC_CHECK_LOG, m_bOpenLogs);
    DDX_Text(pDX, IDC_SEND_DATA, m_strSendData);
}

BEGIN_MESSAGE_MAP(CWinTCPSenderDlg, CDialogEx)
    ON_WM_SYSCOMMAND()
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    ON_WM_TIMER()
    ON_MESSAGE(WM_TCPEVENT, &CWinTCPSenderDlg::OnTCPEvent)

    ON_BN_CLICKED(IDC_BTN_CONNECT, &CWinTCPSenderDlg::OnBnClickedBtnConnect)
    ON_BN_CLICKED(IDC_BTN_SEND, &CWinTCPSenderDlg::OnBnClickedBtnSend)
    ON_BN_CLICKED(IDC_BTN_BIND, &CWinTCPSenderDlg::OnBnClickedBtnBind)
    ON_BN_CLICKED(IDC_CHECK_CONTINUAL, &CWinTCPSenderDlg::OnBnClickedCheckContinual)
    ON_BN_CLICKED(IDC_CHECK_LOG, &CWinTCPSenderDlg::OnBnClickedCheckLog)
    ON_BN_CLICKED(IDC_CLEAR_DATA, &CWinTCPSenderDlg::OnBnClickedClearData)
    ON_EN_CHANGE(IDC_REMOTE_ADDR, &CWinTCPSenderDlg::OnEnChangeRemoteAddr)
    ON_EN_CHANGE(IDC_REMOTE_PORT, &CWinTCPSenderDlg::OnEnChangeRemotePort)
    ON_EN_CHANGE(IDC_LOCAL_ADDR, &CWinTCPSenderDlg::OnEnChangeLocalAddr)
    ON_EN_CHANGE(IDC_LOCAL_PORT, &CWinTCPSenderDlg::OnEnChangeLocalPort)
    ON_EN_CHANGE(IDC_SEND_TIMEOUT, &CWinTCPSenderDlg::OnEnChangeSendTimeout)
    ON_EN_CHANGE(IDC_SEND_TIMES, &CWinTCPSenderDlg::OnEnChangeSendTimes)
    ON_EN_CHANGE(IDC_SEND_DATA, &CWinTCPSenderDlg::OnEnChangeSendData)
END_MESSAGE_MAP()


// CWinTCPSenderDlg message handlers

BOOL CWinTCPSenderDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    // Add "About..." menu item to system menu.

    // IDM_ABOUTBOX must be in the system command range.
    ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
    ASSERT(IDM_ABOUTBOX < 0xF000);

    CMenu* pSysMenu = GetSystemMenu(FALSE);
    if (pSysMenu != NULL)
    {
      BOOL bNameValid;
      CString strAboutMenu;
      bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
      ASSERT(bNameValid);
      if (!strAboutMenu.IsEmpty())
      {
        pSysMenu->AppendMenu(MF_SEPARATOR);
        pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
      }
    }

    // Set the icon for this dialog.  The framework does this automatically
    //  when the application's main window is not a dialog
    SetIcon(m_hIcon, TRUE);     // Set big icon
    SetIcon(m_hIcon, FALSE);    // Set small icon

    // TODO: Add extra initialization here
    GeneBuildTime();
    GeneAppAbout();

    GetDlgItem(IDC_SEND_TIMEOUT)->EnableWindow(FALSE);
    LoadLocalIPAddr();
    m_strRemoteAddr = m_strLocalAddr;
    UpdateData(FALSE); // 变量传递给控件

    GetDlgItem(IDC_BTN_SEND)->EnableWindow(FALSE);

    if (!InitSocket())
    {
        return FALSE;
    }
  return TRUE;  // return TRUE  unless you set the focus to a control
}

void CWinTCPSenderDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
  if ((nID & 0xFFF0) == IDM_ABOUTBOX)
  {
    CAboutDlg dlgAbout;
    dlgAbout.DoModal();
  }
  else
  {
    CDialogEx::OnSysCommand(nID, lParam);
  }
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CWinTCPSenderDlg::OnPaint()
{
  if (IsIconic())
  {
    CPaintDC dc(this); // device context for painting

    SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

    // Center icon in client rectangle
    int cxIcon = GetSystemMetrics(SM_CXICON);
    int cyIcon = GetSystemMetrics(SM_CYICON);
    CRect rect;
    GetClientRect(&rect);
    int x = (rect.Width() - cxIcon + 1) / 2;
    int y = (rect.Height() - cyIcon + 1) / 2;

    // Draw the icon
    dc.DrawIcon(x, y, m_hIcon);
  }
  else
  {
    CDialogEx::OnPaint();
  }
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CWinTCPSenderDlg::OnQueryDragIcon()
{
    return static_cast<HCURSOR>(m_hIcon);
}

BOOL CWinTCPSenderDlg::InitSocket()
{
    m_socketfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (m_socketfd == INVALID_SOCKET)
    {
        ShowSockErrorMsg("Please setup TCP/IP(controlpanel->network)\r\n");
        return  FALSE;
    }

    BOOL flg = TRUE;    // Non Block
    if (ioctlsocket(m_socketfd, FIONBIO, (unsigned long *)&flg) != 0)
    {
        CloseSocket();
        ShowSockErrorMsg("ioctlsocket(nonblock)");
        return FALSE;
    }

    if (!AsyncSelect(m_hWnd, FD_CONNECT))
    {
        CloseSocket();
        ShowSockErrorMsg("AsyncSelect");
        return FALSE;
    }
    
    return TRUE;
}

void CWinTCPSenderDlg::CloseSocket()
{
    if (m_socketfd != INVALID_SOCKET)
    {
        closesocket(m_socketfd);

        m_socketfd = INVALID_SOCKET;
    }
}

BOOL CWinTCPSenderDlg::ResetSocket()
{
    CloseSocket();
    return InitSocket();
}

BOOL CWinTCPSenderDlg::ConnetToServer()
{
    UpdateData(TRUE); // 控件传递给变量
    
    sockaddr_in sinaddrRemote;
    sinaddrRemote.sin_family  = AF_INET;
    sinaddrRemote.sin_port    = htons(m_uRemotePort);

    int iRet = inet_pton(AF_INET, m_strRemoteAddr.GetBuffer(), &sinaddrRemote.sin_addr);
    if (0 == iRet)
    {
        MessageBox(_T("Remote Address is not a valid addres!"));
        return FALSE;
    }
    else if(-1 == iRet)
    {
        MessageBox(_T("inet_pton() error!"));
        return FALSE;
    }
        
    if(-1 == connect(m_socketfd, (SOCKADDR*)&sinaddrRemote, sizeof(sinaddrRemote)))
    {
        int err = WSAGetLastError();
        if(err != WSAEWOULDBLOCK && err != WSAEINVAL && err != WSAEINPROGRESS)
        {
             WriteLogsToFile("connect error");
             return FALSE;
        }
        else
        {
             WriteLogsToFile("connecting...");
        }
    }

    return TRUE;
}

BOOL CWinTCPSenderDlg::ReTryConnect(ULONG ulRetryDelay)
{
    setState(kDisconnected);
    // 开启定时器再重新连接
    UINT_PTR upInstallFlag = SetTimer(kConnectTimerID, ulRetryDelay, NULL);
    if(upInstallFlag == 0)
    {
        WriteLogsToFile("Can not install timer!");
        return FALSE;
    }
    return TRUE;
}

BOOL CWinTCPSenderDlg::BindSocket()
{
    UpdateData(TRUE); // 控件传递给变量
    
    sockaddr_in sinaddrLocal;
    sinaddrLocal.sin_family         = AF_INET;
    sinaddrLocal.sin_port           = htons(m_uLocalPort);
    //sinaddrLocal.sin_addr.s_addr    = INADDR_ANY;   // 如果bind本地地址不要用htonl

    int iRet = inet_pton(AF_INET, m_strLocalAddr.GetBuffer(), &sinaddrLocal.sin_addr);

    if (0 == iRet)
    {
        MessageBox(_T("Local Address is not a valid addres!"));
        return FALSE;
    }
    else if(-1 == iRet)
    {
        MessageBox(_T("inet_pton() error!"));
        return FALSE;
    }

    int iret = bind(m_socketfd, (SOCKADDR*)&sinaddrLocal, sizeof(sinaddrLocal));

    if (iret == SOCKET_ERROR)
    {
        ShowSockErrorMsg("bind");
        CloseSocket();
        return FALSE;
    }
    return TRUE;
}

BOOL CWinTCPSenderDlg::AsyncSelect(HWND hWnd, long lEvent)
{
    int iret = WSAAsyncSelect(m_socketfd, hWnd, WM_TCPEVENT, lEvent);
    if ( iret == SOCKET_ERROR)
    {
        ShowSockErrorMsg("WSAAsyncSelect");
        CloseSocket();
        return  FALSE;
    }

    return  TRUE;
}

BOOL CWinTCPSenderDlg::TcpSend(SOCKADDR_IN &sockAddr, const char *buf, UINT iLen)
{
    if (send(m_socketfd, buf, iLen, 0) == SOCKET_ERROR)
    {
        int err = WSAGetLastError();
        CString strMsg;
        strMsg.Format(_T("%s error = %d"), "send fail.", err);
        WriteLogsToFile(strMsg.GetBuffer());
        if (err != WSAEWOULDBLOCK)
        {
            ShowSockErrorMsg("send error");
        }
    }

    // TODO:如果发送缓冲区还有没发送完的数据，要关注FD_WRITE事件
    //if (!AsyncSelect(m_hWnd, FD_WRITE|FD_CLOSE))
    //{
    //    CloseSocket();
    //    ShowSockErrorMsg("AsyncSelect");
    //    return;
    //}

    return  TRUE;
}

BOOL CWinTCPSenderDlg::TcpRecv(TRecvBuf *buf)
{
    buf->addrSize = sizeof(buf->addr);

    buf->size = recv(m_socketfd, buf->msgBuf, sizeof(buf->msgBuf) -1, 0);

    if (buf->size == SOCKET_ERROR)
    {
        ShowSockErrorMsg("recv");
        return  FALSE;
    }

    buf->msgBuf[buf->size] = 0;

    return  TRUE;
}

void CWinTCPSenderDlg::SendTcpDataByTimes()
{
    if (m_strSendData.IsEmpty())
    {
        return;
    }

    const char * sendBuf = m_strSendData.GetBuffer();
    UINT  uSendLen       = m_strSendData.GetLength(); 

    for (UINT i = 0; i<m_uSendTimes; ++i)
    {
        if(TcpSend(m_sockaddrRemote, sendBuf, uSendLen))
        {
            m_uSentCount++;
            SetDlgItemInt(IDC_SENT_COUNT, m_uSentCount);
            WriteLogsToFile(sendBuf);
        }
    }
}

LRESULT CWinTCPSenderDlg::OnTCPEvent(WPARAM wParam, LPARAM lParam)
{
    int err = WSAGETSELECTERROR(lParam);
    
    switch (WSAGETSELECTEVENT(lParam))
    {
        case FD_CONNECT:
            if(err)
            {
                if(err == WSAEWOULDBLOCK  ||
                   err == WSAECONNREFUSED || 
                   err == WSAENETUNREACH  || 
                   err == WSAEADDRNOTAVAIL||
                   err == WSAEADDRINUSE)
                {
                    ReTryConnect(m_ulRetryDelayMs);
                }
                else
                {
                    ShowSockErrorMsg("connection refused.");
                    return  FALSE;
                }
                
                return TRUE;
            }
            setState(kConnected);
            KillTimer(kConnectTimerID);

            // 在连接成功后会收到携带FD_WRITE事件的消息，不关注
            //if (!AsyncSelect(m_hWnd, FD_WRITE|FD_CLOSE))
            if (!AsyncSelect(m_hWnd, FD_READ|FD_CLOSE))
            {
                CloseSocket();
                ShowSockErrorMsg("AsyncSelect");
                return FALSE;
            }

            SetDlgItemText(IDC_BTN_CONNECT, _T("Disconnect"));
            GetDlgItem(IDC_REMOTE_ADDR)->EnableWindow(FALSE);
            GetDlgItem(IDC_REMOTE_PORT)->EnableWindow(FALSE);
            GetDlgItem(IDC_BTN_SEND)->EnableWindow(TRUE);
            break;
          
        case FD_READ:
            // TODO:接收缓冲区
            if(!m_bBindLocal)
            {
                return TRUE;
            }
            
            if(err)
            {
                ShowSockErrorMsg("OnTCPEvent");
                return  FALSE;
            }
            
            TRecvBuf recvBuf;
            if (TcpRecv(&recvBuf) != TRUE || recvBuf.size == 0)
            {
                return  FALSE;
            }

            WriteLogsToFile(recvBuf.msgBuf);

            // 只显示最后一次接受的数据
            SetDlgItemText(IDC_RECV_DATA, recvBuf.msgBuf);

            m_uRecvCount++;
            SetDlgItemInt(IDC_RECV_COUNT, m_uRecvCount);
            break;

        case FD_WRITE:
            // FD_WRITE事件不同于FD_READ, 
            // 我们只能通过FD_WRITE事件判断是否可以向对方发送数据， 
            // 而不是在事件处理中向对方发数据
            //if (!AsyncSelect(m_hWnd, FD_READ|FD_CLOSE))
            //{
            //    CloseSocket();
            //    ShowSockErrorMsg("AsyncSelect");
            //    return FALSE;
            //}

            // TODO:发送缓冲区
            // TODO:将发送缓冲区的数据发送完
            // TODO:如果发送缓冲区已经没有数据了，要取消关注FD_WRITE事件
            break;

        case FD_CLOSE:
            setState(kDisconnected);
            ResetSocket();
            break;
          
        default:
            break;
    }

    return TRUE;
}

BOOL CWinTCPSenderDlg::LoadLocalIPAddr()
{
    char strHostName[kMaxHostNameLen] = {0};
    HOSTENT* pHost = NULL;

    if (gethostname(strHostName, sizeof(strHostName)) == 0)
    {
        pHost = gethostbyname(strHostName);
        m_strLocalAddr = inet_ntoa(*(struct in_addr *)pHost->h_addr_list[0]);
        return TRUE;
    }

    return FALSE;
}

void CWinTCPSenderDlg::WriteLogsToFile(const char *msg)
{
  if(!m_bOpenLogs || NULL == msg)
  {
    return;
  }

  CString strTime = CTime::GetCurrentTime().Format(_T("%Y/%m/%d %H:%M:%S"));
  CString strText;
  strText.Format(_T("%s: %s\n"), strTime, msg);

  if(NULL == m_fp)
  {
    errno_t err;
    err = fopen_s(&m_fp, "WinTCPSender.log", "a+");
    if(err!=0 || NULL == m_fp)
    {
      MessageBox(_T("Problem opening the file!"));
      return;
    }

    fprintf(m_fp, "%s", strText.GetBuffer());
  }
  else
  {
    fprintf(m_fp, "%s", strText.GetBuffer());
  }
}

BOOL CWinTCPSenderDlg::ShowSockErrorMsg(const char *msg)
{
    CString strMsg;
    strMsg.Format(_T("%s error = %d"), msg, WSAGetLastError());
    WriteLogsToFile(strMsg.GetBuffer());
    return  MessageBox(strMsg);
}

void CWinTCPSenderDlg::OnTimer(UINT_PTR nIDEvent)
{
    // TODO: Add your message handler code here and/or call default

    if (nIDEvent == kConnectTimerID)
    {
        m_ulRetryDelayMs = min(m_ulRetryDelayMs * 2, kMaxRetryDelayMs);
        ConnetToServer();
    }
    else if (nIDEvent == kSendTimerID)
    {
        SendTcpDataByTimes();
        m_uExecTimes++;
        m_ulElapseTime = m_uExecTimes * m_ulTimeOut;

        SetDlgItemInt(IDC_SEND_ELAPSED, m_ulElapseTime, true);
    }
    else
    {
        //
    }

    CDialogEx::OnTimer(nIDEvent);
}

void CWinTCPSenderDlg::OnBnClickedBtnConnect()
{
    // TODO: Add your control notification handler code here
    CString strBtnConnect = _T("");
    GetDlgItemText(IDC_BTN_CONNECT, strBtnConnect);

    if (strBtnConnect == "Disconnect")
    {
        setState(kDisconnected);
        ResetSocket();
        SetDlgItemText(IDC_BTN_CONNECT, _T("Connect"));
        GetDlgItem(IDC_LOCAL_ADDR)->EnableWindow(TRUE);
        GetDlgItem(IDC_LOCAL_PORT)->EnableWindow(TRUE);
        GetDlgItem(IDC_REMOTE_ADDR)->EnableWindow(TRUE);
        GetDlgItem(IDC_REMOTE_PORT)->EnableWindow(TRUE);
        GetDlgItem(IDC_BTN_SEND)->EnableWindow(FALSE);

        SetDlgItemText(IDC_BTN_BIND, _T("Bind"));
    }
    else
    {
        if(!ConnetToServer())
        {
            ShowSockErrorMsg("ConnetToServer");
            ResetSocket();
            return;
        }
    }
}

void CWinTCPSenderDlg::OnBnClickedBtnSend()
{
    // TODO: Add your control notification handler code here
    if (m_connState != kConnected)
    {
        ShowSockErrorMsg("not connected.");
        return;
    }
    
    CString strBtnSend = _T("");
    GetDlgItemText(IDC_BTN_SEND, strBtnSend);

    if (strBtnSend == "Stop")
    {
        KillTimer(kSendTimerID);
        GetDlgItem(IDC_SEND_DATA)->EnableWindow(TRUE);
        GetDlgItem(IDC_SEND_TIMES)->EnableWindow(TRUE);
        GetDlgItem(IDC_SEND_TIMEOUT)->EnableWindow(TRUE);
        GetDlgItem(IDC_SEND_ELAPSED)->EnableWindow(TRUE);
        GetDlgItem(IDC_CHECK_CONTINUAL)->EnableWindow(TRUE);
        //GetDlgItem(IDC_CHECK_LOG)->EnableWindow(TRUE);
        GetDlgItem(IDC_BTN_BIND)->EnableWindow(TRUE);
        GetDlgItem(IDC_BTN_CONNECT)->EnableWindow(TRUE);
        
        SetDlgItemText(IDC_BTN_SEND, _T("Send"));
    } 
    else if (strBtnSend == "Send")
    {
        UpdateData(TRUE); // 控件传递给变量
        
        if (0 == m_uRemotePort)
        {
            MessageBox(_T("LocalPort is 0!"));
            return;
        }

        m_sockaddrRemote.sin_family = AF_INET;
        m_sockaddrRemote.sin_port   = htons(m_uRemotePort);

        int iRet = inet_pton(AF_INET, m_strRemoteAddr.GetBuffer(), &m_sockaddrRemote.sin_addr);

        if (0 == iRet)
        {
            MessageBox(_T("Remote Address is not a valid addres!"));
            return;
        }
        else if(-1 == iRet)
        {
            MessageBox(_T("inet_pton() error!"));
            return;
        }

        if (m_strSendData.IsEmpty())
        {
            return;
        }

        const char * sendBuf = m_strSendData.GetBuffer();
        UINT  uSendLen       = m_strSendData.GetLength(); 

        if (m_bSendContinual)
        {
            // 开启定时器,连续发送
            UINT_PTR upInstallFlag = SetTimer(kSendTimerID, m_ulTimeOut, NULL);  //定时m_ulTimeOut(ms)
            if(upInstallFlag == 0)
            {
                MessageBox(_T("Can not install timer!"));
            }

            GetDlgItem(IDC_REMOTE_ADDR)->EnableWindow(FALSE);
            GetDlgItem(IDC_REMOTE_PORT)->EnableWindow(FALSE);
            GetDlgItem(IDC_SEND_DATA)->EnableWindow(FALSE);
            GetDlgItem(IDC_SEND_TIMES)->EnableWindow(FALSE);
            GetDlgItem(IDC_SEND_TIMEOUT)->EnableWindow(FALSE);
            GetDlgItem(IDC_SEND_ELAPSED)->EnableWindow(FALSE);
            GetDlgItem(IDC_CHECK_CONTINUAL)->EnableWindow(FALSE);
            GetDlgItem(IDC_BTN_BIND)->EnableWindow(FALSE);
            GetDlgItem(IDC_BTN_CONNECT)->EnableWindow(FALSE);
            //GetDlgItem(IDC_CHECK_LOG)->EnableWindow(FALSE);

            SetDlgItemText(IDC_BTN_SEND, _T("Stop"));
        } 
        else
        {
            // 直接发送
            SendTcpDataByTimes();
        }
    }
    else
    {
        //
    }
}

void CWinTCPSenderDlg::UpdateBtnWindow(int nID, CString &strData)
{
    if (strData.IsEmpty())
    {
        GetDlgItem(nID)->EnableWindow(FALSE);
    } 
    else
    {
        GetDlgItem(nID)->EnableWindow(TRUE);
    }
}

void CWinTCPSenderDlg::OnBnClickedBtnBind()
{
    // TODO: Add your control notification handler code here
    if (!ResetSocket())
    {
        ShowSockErrorMsg("ResetSocket");
        return;
    }

    GetDlgItem(IDC_REMOTE_ADDR)->EnableWindow(TRUE);
    GetDlgItem(IDC_REMOTE_PORT)->EnableWindow(TRUE);
    GetDlgItem(IDC_BTN_SEND)->EnableWindow(FALSE);
    SetDlgItemText(IDC_BTN_CONNECT, _T("Connect"));

    CString strBtnBind = _T("");
    GetDlgItemText(IDC_BTN_BIND, strBtnBind);

    if (strBtnBind == _T("UnBind"))
    {
        m_bBindLocal = FALSE;
        SetDlgItemText(IDC_BTN_BIND, _T("Bind"));
        GetDlgItem(IDC_LOCAL_ADDR)->EnableWindow(TRUE);
        GetDlgItem(IDC_LOCAL_PORT)->EnableWindow(TRUE);
    } 
    else if(strBtnBind == _T("Bind"))
    {
        if (!BindSocket())
        {
            ShowSockErrorMsg("BindSocket");
            return;
        }
        m_bBindLocal = TRUE;
        SetDlgItemText(IDC_BTN_BIND, _T("UnBind"));
        GetDlgItem(IDC_LOCAL_ADDR)->EnableWindow(FALSE);
        GetDlgItem(IDC_LOCAL_PORT)->EnableWindow(FALSE);
    }
}


void CWinTCPSenderDlg::OnBnClickedCheckContinual()
{
    // TODO: Add your control notification handler code here
    UpdateData(TRUE); // 控件传递给变量

    if (m_bSendContinual)
    {
        GetDlgItem(IDC_SEND_TIMEOUT)->EnableWindow(TRUE);
    } 
    else
    {
        GetDlgItem(IDC_SEND_TIMEOUT)->EnableWindow(FALSE);
    }
}


void CWinTCPSenderDlg::OnBnClickedCheckLog()
{
    // TODO: Add your control notification handler code here
    UpdateData(TRUE); // 控件传递给变量

    if(!m_bOpenLogs)
    {
      fflush(m_fp);
      fclose(m_fp);
    }
}


void CWinTCPSenderDlg::OnBnClickedClearData()
{
    // TODO: Add your control notification handler code here
    m_uRecvCount = 0;
    m_uSentCount = 0;
    SetDlgItemInt(IDC_RECV_COUNT, m_uRecvCount);
    SetDlgItemInt(IDC_SENT_COUNT, m_uSentCount);
    SetDlgItemText(IDC_RECV_DATA, _T(""));
}


void CWinTCPSenderDlg::OnEnChangeRemoteAddr()
{
    // TODO:  If this is a RICHEDIT control, the control will not
    // send this notification unless you override the CDialogEx::OnInitDialog()
    // function and call CRichEditCtrl().SetEventMask()
    // with the ENM_CHANGE flag ORed into the mask.

    // TODO:  Add your control notification handler code here
    CString strText = _T("");
    GetDlgItemText(IDC_REMOTE_ADDR, strText);

    UpdateBtnWindow(IDC_BTN_SEND, strText);
}


void CWinTCPSenderDlg::OnEnChangeRemotePort()
{
    // TODO:  If this is a RICHEDIT control, the control will not
    // send this notification unless you override the CDialogEx::OnInitDialog()
    // function and call CRichEditCtrl().SetEventMask()
    // with the ENM_CHANGE flag ORed into the mask.


    // TODO:  Add your control notification handler code here
    CString strText = _T("");
    GetDlgItemText(IDC_REMOTE_PORT, strText);

    UpdateBtnWindow(IDC_BTN_SEND, strText);
}


void CWinTCPSenderDlg::OnEnChangeLocalAddr()
{
    // TODO:  If this is a RICHEDIT control, the control will not
    // send this notification unless you override the CDialogEx::OnInitDialog()
    // function and call CRichEditCtrl().SetEventMask()
    // with the ENM_CHANGE flag ORed into the mask.

    // TODO:  Add your control notification handler code here
    CString strText = _T("");
    GetDlgItemText(IDC_LOCAL_ADDR, strText);

    UpdateBtnWindow(IDC_BTN_BIND, strText);
}


void CWinTCPSenderDlg::OnEnChangeLocalPort()
{
    // TODO:  If this is a RICHEDIT control, the control will not
    // send this notification unless you override the CDialogEx::OnInitDialog()
    // function and call CRichEditCtrl().SetEventMask()
    // with the ENM_CHANGE flag ORed into the mask.

    // TODO:  Add your control notification handler code here
    CString strText = _T("");
    GetDlgItemText(IDC_LOCAL_PORT, strText);

    UpdateBtnWindow(IDC_BTN_BIND, strText);
}


void CWinTCPSenderDlg::OnEnChangeSendTimeout()
{
    // TODO:  If this is a RICHEDIT control, the control will not
    // send this notification unless you override the CDialogEx::OnInitDialog()
    // function and call CRichEditCtrl().SetEventMask()
    // with the ENM_CHANGE flag ORed into the mask.

    // TODO:  Add your control notification handler code here
    CString strText = _T("");
    GetDlgItemText(IDC_SEND_TIMEOUT, strText);

    UpdateBtnWindow(IDC_BTN_SEND, strText);
}


void CWinTCPSenderDlg::OnEnChangeSendTimes()
{
    // TODO:  If this is a RICHEDIT control, the control will not
    // send this notification unless you override the CDialogEx::OnInitDialog()
    // function and call CRichEditCtrl().SetEventMask()
    // with the ENM_CHANGE flag ORed into the mask.

    // TODO:  Add your control notification handler code here
    CString strText = _T("");
    GetDlgItemText(IDC_SEND_TIMES, strText);

    UpdateBtnWindow(IDC_BTN_SEND, strText);
}


void CWinTCPSenderDlg::OnEnChangeSendData()
{
    // TODO:  If this is a RICHEDIT control, the control will not
    // send this notification unless you override the CDialogEx::OnInitDialog()
    // function and call CRichEditCtrl().SetEventMask()
    // with the ENM_CHANGE flag ORed into the mask.

    // TODO:  Add your control notification handler code here
    CString strText = _T("");
    GetDlgItemText(IDC_SEND_DATA, strText);

    UpdateBtnWindow(IDC_BTN_SEND, strText);
}
