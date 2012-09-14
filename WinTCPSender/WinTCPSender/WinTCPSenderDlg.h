
// WinTCPSenderDlg.h : header file
//

#pragma once

#include "DataType.h"

// CWinTCPSenderDlg dialog
class CWinTCPSenderDlg : public CDialogEx
{
// Construction
public:
	CWinTCPSenderDlg(CWnd* pParent = NULL);	// standard constructor
	~CWinTCPSenderDlg();

// Dialog Data
	enum { IDD = IDD_WINTCPSENDER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
    enum States { kDisconnected, kConnecting, kConnected };

    BOOL InitSocket();
    BOOL BindSocket();
    BOOL ConnetToServer();
    BOOL ResetSocket();
    void CloseSocket();

    BOOL LoadLocalIPAddr();
    BOOL AsyncSelect(HWND hWnd);
    
    BOOL TcpSend(SOCKADDR_IN &sockAddr, const char *buf, UINT iLen);
    BOOL TcpRecv(TRecvBuf *buf);
    void SendTcpDataByTimes();

    void UpdateBtnWindow(int nID, CString &strData);

    BOOL ShowSockErrorMsg(const char *msg);
    void WriteLogsToFile(const char *msg);

    void setState(States s) { m_connState = s; }

public:
    LRESULT OnRecvData(WPARAM wParam, LPARAM lParam);

    afx_msg void OnBnClickedBtnConnect();
    afx_msg void OnBnClickedBtnSend();
    afx_msg void OnBnClickedBtnBind();
    afx_msg void OnBnClickedCheckContinual();
    afx_msg void OnBnClickedCheckLog();
    afx_msg void OnBnClickedClearData();
    afx_msg void OnEnChangeRemoteAddr();
    afx_msg void OnEnChangeRemotePort();
    afx_msg void OnEnChangeLocalAddr();
    afx_msg void OnEnChangeLocalPort();
    afx_msg void OnEnChangeSendTimeout();
    afx_msg void OnEnChangeSendTimes();
    afx_msg void OnEnChangeSendData();
    afx_msg void OnTimer(UINT_PTR nIDEvent);

private:
    // Remote
    CString       m_strSendData;
    CString       m_strRemoteAddr;
    SOCKADDR_IN   m_sockaddrRemote;
    UINT          m_uRemotePort;
    UINT          m_uSendTimes;
    ULONG         m_ulTimeOut;
    ULONG         m_ulElapseTime;
    UINT_PTR      m_uSendTimer;
    UINT          m_uExecTimes;
    BOOL          m_bSendContinual;

    // Local
    CString       m_strLocalAddr;
    UINT          m_uLocalPort;
    UINT          m_uRecvCount;
    UINT          m_uSentCount;
    BOOL          m_bBindLocal;
    States        m_connState;

    // Statics and logs
    BOOL          m_bOpenLogs;
    SOCKET        m_socketfd;
    FILE*         m_fp;
};
