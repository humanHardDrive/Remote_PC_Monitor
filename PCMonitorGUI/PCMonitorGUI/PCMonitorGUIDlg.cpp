
// PCMonitorGUIDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PCMonitorGUI.h"
#include "PCMonitorGUIDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CPCMonitorGUIDlg dialog



CPCMonitorGUIDlg::CPCMonitorGUIDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_PCMONITORGUI_DIALOG, pParent)
	, m_Port(_T(""))
	, m_LoggingText(_T(""))
	, m_IPAddress(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CPCMonitorGUIDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_LOGBOX, m_LoggingText);
	DDX_Text(pDX, IDC_IPBOX2, m_IPAddress);
	DDX_Text(pDX, IDC_PORTBOX, m_Port);
}

BEGIN_MESSAGE_MAP(CPCMonitorGUIDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_CONNECTBTN, &CPCMonitorGUIDlg::OnBnClickedConnectbtn)
	ON_BN_CLICKED(IDC_DISCONNECTBTN, &CPCMonitorGUIDlg::OnBnClickedDisconnectbtn)
	ON_BN_CLICKED(IDC_LSTSNSBTN, &CPCMonitorGUIDlg::OnBnClickedLstsnsbtn)
	ON_BN_CLICKED(IDC_SNDFILEBTN, &CPCMonitorGUIDlg::OnBnClickedSndfilebtn)
	ON_BN_CLICKED(IDC_VALIDFILEBTN, &CPCMonitorGUIDlg::OnBnClickedValidfilebtn)
END_MESSAGE_MAP()


// CPCMonitorGUIDlg message handlers

BOOL CPCMonitorGUIDlg::OnInitDialog()
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
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	ShowWindow(SW_SHOW);

	m_Commander.SetNetworkThread(&m_NetworkThread);

	// TODO: Add extra initialization here

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CPCMonitorGUIDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CPCMonitorGUIDlg::OnPaint()
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
HCURSOR CPCMonitorGUIDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CPCMonitorGUIDlg::LogEvent(CString event)
{
	m_LoggingText.Empty();
	m_EventQ.push(event);

	if(m_EventQ.size() > MAX_EVENTS)
		m_EventQ.pop();

	std::queue<CString> tempQ = m_EventQ;
	for(unsigned int i = 0; i < m_EventQ.size(); i++)
	{
		m_LoggingText.AppendFormat(_T("%s\r\n"), tempQ.front());
		tempQ.pop();
	}

	//UpdateData(FALSE);
}



void CPCMonitorGUIDlg::OnBnClickedConnectbtn()
{
	LogEvent(_T("Attempting to connect..."));

	UpdateData(TRUE);

	m_NetworkThread.SetIP(m_IPAddress);
	m_NetworkThread.SetPort(m_Port);

	if(m_NetworkThread.Connect())
		LogEvent(_T("Success"));
	else
		LogEvent(_T("Failed"));
}


void CPCMonitorGUIDlg::OnBnClickedDisconnectbtn()
{
	m_NetworkThread.Disconnect();
}


void CPCMonitorGUIDlg::OnBnClickedLstsnsbtn()
{
	CString test = _T("HELLO");

	m_Commander.ListSensors(0, test);
}


void CPCMonitorGUIDlg::OnBnClickedSndfilebtn()
{
	// TODO: Add your control notification handler code here
}


void CPCMonitorGUIDlg::OnBnClickedValidfilebtn()
{
	// TODO: Add your control notification handler code here
}
