
// WinUDPSenderDlg.cpp : implementation file
//

#include "stdafx.h"
#include "WinUDPSender.h"
#include "WinUDPSenderDlg.h"
#include "afxdialogex.h"

#include "DataType.h"

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <stdio.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CString g_strBuildTime(_T("WinUDPSender  Build Time : "));
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


// CWinUDPSenderDlg dialog



CWinUDPSenderDlg::CWinUDPSenderDlg(CWnd* pParent /*=NULL*/)
: CDialogEx(CWinUDPSenderDlg::IDD, pParent)
, m_strSendData(_T("Enter data to send ..."))
, m_strRemoteAddr(_T("127.0.0.1"))
, m_uRemotePort(12345)
, m_uSendTimes(1)
, m_ulTimeOut(1000)
, m_uSendTimer(0)
, m_ulElapseTime(0)
, m_uExecTimes(0)
, m_bSendContinual(FALSE)
, m_strLocalAddr(_T("127.0.0.1"))
, m_uLocalPort(12345)
, m_uRecvCount(0)
, m_uSentCount(0)
, m_bBindLocal(FALSE)
, m_bOpenLogs(FALSE)
, m_socketfd(INVALID_SOCKET)
, m_fp(NULL)
{
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

CWinUDPSenderDlg::~CWinUDPSenderDlg()
{
  CloseSocket();

  if(NULL != m_fp)
  {
    fclose(m_fp);
  }
}

void CWinUDPSenderDlg::DoDataExchange(CDataExchange* pDX)
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

BEGIN_MESSAGE_MAP(CWinUDPSenderDlg, CDialogEx)
    ON_WM_SYSCOMMAND()
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()

    ON_WM_TIMER()
    ON_MESSAGE(WM_UDPRECVDATA ,&CWinUDPSenderDlg::OnRecvData)

    ON_BN_CLICKED(IDC_BTN_SEND, &CWinUDPSenderDlg::OnBnClickedBtnSend)
    ON_BN_CLICKED(IDC_BTN_BIND, &CWinUDPSenderDlg::OnBnClickedBtnBind)
    ON_BN_CLICKED(IDC_CHECK_CONTINUAL, &CWinUDPSenderDlg::OnBnClickedCheckContinual)
    ON_BN_CLICKED(IDC_CHECK_LOG, &CWinUDPSenderDlg::OnBnClickedCheckLog)
    ON_BN_CLICKED(IDC_CLEAR_DATA, &CWinUDPSenderDlg::OnBnClickedClearData)
    ON_EN_CHANGE(IDC_REMOTE_ADDR, &CWinUDPSenderDlg::OnEnChangeRemoteAddr)
    ON_EN_CHANGE(IDC_REMOTE_PORT, &CWinUDPSenderDlg::OnEnChangeRemotePort)
    ON_EN_CHANGE(IDC_LOCAL_ADDR, &CWinUDPSenderDlg::OnEnChangeLocalAddr)
    ON_EN_CHANGE(IDC_LOCAL_PORT, &CWinUDPSenderDlg::OnEnChangeLocalPort)
    ON_EN_CHANGE(IDC_SEND_TIMEOUT, &CWinUDPSenderDlg::OnEnChangeSendTimeout)
    ON_EN_CHANGE(IDC_SEND_TIMES, &CWinUDPSenderDlg::OnEnChangeSendTimes)
    ON_EN_CHANGE(IDC_SEND_DATA, &CWinUDPSenderDlg::OnEnChangeSendData)
END_MESSAGE_MAP()


// CWinUDPSenderDlg message handlers

BOOL CWinUDPSenderDlg::OnInitDialog()
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
    SetIcon(m_hIcon, TRUE);         // Set big icon
    SetIcon(m_hIcon, FALSE);        // Set small icon

    // TODO: Add extra initialization here
    GeneBuildTime();
    GeneAppAbout();

    GetDlgItem(IDC_SEND_TIMEOUT)->EnableWindow(FALSE);
    LoadLocalIPAddr();
    m_strRemoteAddr = m_strLocalAddr;
    UpdateData(FALSE); // 变量传递给控件

    if (!InitSocket())
    {
        return FALSE;
    }
    return TRUE;  // return TRUE  unless you set the focus to a control
}

void CWinUDPSenderDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CWinUDPSenderDlg::OnPaint()
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
HCURSOR CWinUDPSenderDlg::OnQueryDragIcon()
{
    return static_cast<HCURSOR>(m_hIcon);
}

BOOL CWinUDPSenderDlg::InitSocket()
{
    m_socketfd = socket(AF_INET,SOCK_DGRAM,0);
    if (m_socketfd == INVALID_SOCKET)
    {
        ShowSockErrorMsg("Please setup TCP/IP(controlpanel->network)\r\n");
        return  FALSE;
    }

    if (!BindSocket())
    {
        CloseSocket();
        ShowSockErrorMsg("BindSocket");
        return FALSE;
    }

    BOOL flg = TRUE;    // Non Block
    if (ioctlsocket(m_socketfd, FIONBIO, (unsigned long *)&flg) != 0)
    {
        CloseSocket();
        ShowSockErrorMsg("ioctlsocket(nonblock)");
        return FALSE;
    }

    if (!AsyncSelect(m_hWnd))
    {
        CloseSocket();
        ShowSockErrorMsg("AsyncSelect");
        return FALSE;
    }

    int iBufSize    = MAX_SOCKBUF; 
    int iMinBufSize = MAX_SOCKBUF / 2;
    if (setsockopt(m_socketfd, SOL_SOCKET, SO_SNDBUF, (char *)&iBufSize, sizeof(int)) != 0
        &&  setsockopt(m_socketfd, SOL_SOCKET, SO_SNDBUF, (char *)&iMinBufSize, sizeof(int)) != 0)
    {
        CloseSocket();
        ShowSockErrorMsg("setsockopt(sendbuf)");
        return FALSE;
    }

    iBufSize    = MAX_SOCKBUF;
    iMinBufSize = MAX_SOCKBUF / 2;
    if (setsockopt(m_socketfd, SOL_SOCKET, SO_RCVBUF, (char *)&iBufSize, sizeof(int)) != 0
        &&  setsockopt(m_socketfd, SOL_SOCKET, SO_RCVBUF, (char *)&iMinBufSize, sizeof(int)) != 0)
    {
        CloseSocket();
        ShowSockErrorMsg("setsockopt(recvbuf)");
        return FALSE;
    }

    return TRUE;
}

void CWinUDPSenderDlg::CloseSocket()
{
    if (m_socketfd != INVALID_SOCKET)
    {
        closesocket(m_socketfd);

        m_socketfd = INVALID_SOCKET;
    }
}

 BOOL CWinUDPSenderDlg::ResetSocket()
 {
     CloseSocket();
     return InitSocket();
 }

 BOOL CWinUDPSenderDlg::BindSocket()
{
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
        m_bBindLocal = TRUE;
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

        SetDlgItemText(IDC_BTN_BIND, _T("UnBind"));
        GetDlgItem(IDC_LOCAL_ADDR)->EnableWindow(FALSE);
        GetDlgItem(IDC_LOCAL_PORT)->EnableWindow(FALSE);
    }

    return TRUE;
}

BOOL CWinUDPSenderDlg::AsyncSelect(HWND hWnd)
{
    int iret = WSAAsyncSelect(m_socketfd, hWnd, WM_UDPRECVDATA, FD_READ);
    if ( iret == SOCKET_ERROR)
    {
        ShowSockErrorMsg("WSAAsyncSelect");
        CloseSocket();
        return  FALSE;
    }

    return  TRUE;
}

BOOL CWinUDPSenderDlg::UdpSend(SOCKADDR_IN &sockAddr, const char *buf, UINT iLen)
{
    if (sendto(m_socketfd, buf, iLen, 0, (LPSOCKADDR)&sockAddr, sizeof(sockAddr)) == SOCKET_ERROR)
    {
        int err = WSAGetLastError();
        CString strMsg;
        strMsg.Format(_T("%s error = %d"), "udpsend fail.", err);
        WriteLogsToFile(strMsg.GetBuffer());
        
        switch (err) 
        {
        case WSAENETDOWN:
            break;

        case WSAEHOSTUNREACH:
            return  FALSE;

        default:
            return  FALSE;
        }

        if (ResetSocket() != TRUE)
        {
            return  FALSE;
        }

        if ( AsyncSelect(m_hWnd) != TRUE)
        {
            return  FALSE;
        }

        if (sendto(m_socketfd, buf, iLen, 0, (LPSOCKADDR)&sockAddr, sizeof(sockAddr)) == SOCKET_ERROR)
        {
            return  FALSE;
        }
    }

    return  TRUE;
}

BOOL CWinUDPSenderDlg::UdpRecv(TRecvBuf *buf)
{
    buf->addrSize = sizeof(buf->addr);

    buf->size = recvfrom(m_socketfd, buf->msgBuf, sizeof(buf->msgBuf) -1, 0, 
                          (LPSOCKADDR)&buf->addr, &buf->addrSize);

    if (buf->size == SOCKET_ERROR)
    {
        ShowSockErrorMsg("recvfrom");
        return  FALSE;
    }

    buf->msgBuf[buf->size] = 0;

    return  TRUE;
}

void CWinUDPSenderDlg::SendUdpDataByTimes()
{
    if (m_strSendData.IsEmpty())
    {
        return;
    }

    const char * sendBuf = m_strSendData.GetBuffer();
    UINT  uSendLen       = m_strSendData.GetLength(); 

    for (UINT i = 0; i<m_uSendTimes; ++i)
    {
        if(UdpSend(m_sockaddrRemote, sendBuf, uSendLen))
        {
            m_uSentCount++;
            SetDlgItemInt(IDC_SENT_COUNT, m_uSentCount);
            WriteLogsToFile(sendBuf);
        }
    }
}


LRESULT CWinUDPSenderDlg::OnRecvData(WPARAM wParam, LPARAM lParam)
{
    if(!m_bBindLocal)
    {
      return TRUE;
    }
    
    if (WSAGETSELECTERROR(lParam))
    {
        ShowSockErrorMsg("OnRecvData");
        return  FALSE;
    }

    TRecvBuf recvBuf;
    if (UdpRecv(&recvBuf) != TRUE || recvBuf.size == 0)
    {
        return  FALSE;
    }

    WriteLogsToFile(recvBuf.msgBuf);

    // 只显示最后一次接受的数据
    SetDlgItemText(IDC_RECV_DATA, recvBuf.msgBuf);

    m_uRecvCount++;
    SetDlgItemInt(IDC_RECV_COUNT, m_uRecvCount);
    return TRUE;
}

BOOL CWinUDPSenderDlg::LoadLocalIPAddr()
{
    char strHostName[MAX_BUF] = {0};
    HOSTENT* pHost = NULL;

    if (gethostname(strHostName, sizeof(strHostName)) == 0)
    {
        pHost = gethostbyname(strHostName);
        m_strLocalAddr = inet_ntoa(*(struct in_addr *)pHost->h_addr_list[0]);
        return TRUE;
    }

    return FALSE;
}

void CWinUDPSenderDlg::WriteLogsToFile(const char *msg)
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
    err = fopen_s(&m_fp, "WinUDPSender.log", "a+");
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

BOOL CWinUDPSenderDlg::ShowSockErrorMsg(const char *msg)
{
    CString strMsg;
    strMsg.Format(_T("%s error = %d"), msg, WSAGetLastError());
    WriteLogsToFile(strMsg.GetBuffer());
    return  MessageBox(strMsg);
}

void CWinUDPSenderDlg::OnTimer(UINT_PTR nIDEvent)
{
    // TODO: Add your message handler code here and/or call default

    if (nIDEvent == m_uSendTimer)
    {
        SendUdpDataByTimes();
        m_uExecTimes++;
        m_ulElapseTime = m_uExecTimes * m_ulTimeOut;

        SetDlgItemInt(IDC_SEND_ELAPSED, m_ulElapseTime, true);
    }

    CDialogEx::OnTimer(nIDEvent);
}

void CWinUDPSenderDlg::OnBnClickedBtnSend()
{
    // TODO: Add your control notification handler code here
    CString strBtnSend = _T("");
    GetDlgItemText(IDC_BTN_SEND, strBtnSend);


    if (strBtnSend == "Stop")
    {
        //
        KillTimer(m_uSendTimer);

        GetDlgItem(IDC_REMOTE_ADDR)->EnableWindow(TRUE);
        GetDlgItem(IDC_REMOTE_PORT)->EnableWindow(TRUE);
        GetDlgItem(IDC_SEND_DATA)->EnableWindow(TRUE);
        GetDlgItem(IDC_SEND_TIMES)->EnableWindow(TRUE);
        GetDlgItem(IDC_SEND_TIMEOUT)->EnableWindow(TRUE);
        GetDlgItem(IDC_SEND_ELAPSED)->EnableWindow(TRUE);
        GetDlgItem(IDC_CHECK_CONTINUAL)->EnableWindow(TRUE);
        //GetDlgItem(IDC_CHECK_LOG)->EnableWindow(TRUE);
        GetDlgItem(IDC_BTN_BIND)->EnableWindow(TRUE);

        SetDlgItemText(IDC_BTN_SEND, _T("Send"));

        // UpdateData(FALSE); // 变量传递给控件
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
            UINT_PTR upInstallFlag = SetTimer(m_uSendTimer, m_ulTimeOut, NULL);  //定时m_ulTimeOut(ms)
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
            //GetDlgItem(IDC_CHECK_LOG)->EnableWindow(FALSE);

            SetDlgItemText(IDC_BTN_SEND, _T("Stop"));
        } 
        else
        {
            // 直接发送
            SendUdpDataByTimes();
        }
    }
    else
    {
        //
    }
}

void CWinUDPSenderDlg::UpdateBtnWindow(int nID, CString &strData)
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

void CWinUDPSenderDlg::OnBnClickedBtnBind()
{
    // TODO: Add your control notification handler code here
    UpdateData(TRUE); // 控件传递给变量

    if (!ResetSocket())
    {
        ShowSockErrorMsg("ResetSocket");
        return;
    }

    UpdateData(FALSE); // 变量传递给控件
}


void CWinUDPSenderDlg::OnBnClickedCheckContinual()
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


void CWinUDPSenderDlg::OnBnClickedCheckLog()
{
    // TODO: Add your control notification handler code here
    UpdateData(TRUE); // 控件传递给变量
}


void CWinUDPSenderDlg::OnBnClickedClearData()
{
    // TODO: Add your control notification handler code here
    m_uRecvCount = 0;
    m_uSentCount = 0;
    SetDlgItemInt(IDC_RECV_COUNT, m_uRecvCount);
    SetDlgItemInt(IDC_SENT_COUNT, m_uSentCount);
    SetDlgItemText(IDC_RECV_DATA, _T(""));
}


void CWinUDPSenderDlg::OnEnChangeRemoteAddr()
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


void CWinUDPSenderDlg::OnEnChangeRemotePort()
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


void CWinUDPSenderDlg::OnEnChangeLocalAddr()
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


void CWinUDPSenderDlg::OnEnChangeLocalPort()
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


void CWinUDPSenderDlg::OnEnChangeSendTimeout()
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


void CWinUDPSenderDlg::OnEnChangeSendTimes()
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


void CWinUDPSenderDlg::OnEnChangeSendData()
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
