
// PCMonitorGUIDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PCMonitorGUI.h"
#include "PCMonitorGUIDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


enum HEX_PARSING_STATE
{
	START_OF_LINE = 0,
	BYTE_COUNT_0,
	BYTE_COUNT_1,
	ADDRESS_0,
	ADDRESS_1,
	ADDRESS_2,
	ADDRESS_3,
	RECORD_TYPE_0,
	RECORD_TYPE_1,
	DATA_0,
	DATA_1,
	CHECKSUM_0,
	CHECKSUM_1
};

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
	ON_BN_CLICKED(IDC_UPDATESNSBTN, &CPCMonitorGUIDlg::OnBnClickedUpdatesnsbtn)
	ON_WM_CLOSE()
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
	
	m_SensorListBuilt = false;
	m_BuildingSensorList = false;
	m_UpdatingSensorList = false;

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

void CPCMonitorGUIDlg::BuildSensorList()
{
	uint8_t failedmessagecount = 0, index = 0;
	CString name;
	bool eolfound = false;

	while (!eolfound && failedmessagecount < 10)
	{
		name.Empty();
		if (m_Commander.ListSensors(index, name))
		{
			failedmessagecount = 0;
			if (name.GetLength() > 0)
			{
				std::pair<CString, float> sensor;

				sensor.first = name;
				if (m_Commander.UpdateSensor(index, sensor.second))
				{
					m_SensorListLock.lock();
					m_SensorList.push_back(sensor);
					m_SensorListLock.unlock();
				}
			}
			else
				eolfound = true;
			index++;
		}
		else
			failedmessagecount++;

		std::this_thread::yield();
	}

	m_SensorListBuilt = true;
	m_BuildingSensorList = false;
}

void CPCMonitorGUIDlg::UpdateSensorList()
{
	uint8_t failedmessagecount = 0, index = 0;

	while (index < m_SensorList.size() && failedmessagecount < 10)
	{
		m_SensorListLock.lock();
		if (m_Commander.UpdateSensor(index, m_SensorList[index].second))
			failedmessagecount = 0;
		else
			failedmessagecount++;
		m_SensorListLock.unlock();

		index++;

		std::this_thread::yield();
	}

	m_UpdatingSensorList = false;
}

void CPCMonitorGUIDlg::SendFile()
{
	uint8_t failedmessagecount = 0;
	uint32_t index = 0;
	uint8_t ack = 1;

	std::vector<uint8_t> data;
	std::vector<FILE_INFO> info;

	ParseHexFile(info, data);

	m_SendingFile = false;
}

static uint8_t HexDigitToDecimal(char c)
{
	switch (c)
	{
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
		return (uint8_t)(c - '0');
		break;

	case 'A':
	case 'a':
		return 10;
		break;

	case 'B':
	case 'b':
		return 11;
		break;

	case 'C':
	case 'c':
		return 12;
		break;

	case 'D':
	case 'd':
		return 13;
		break;

	case 'E':
	case 'e':
		return 14;
		break;

	case 'F':
	case 'f':
		return 15;
		break;
	}

	return 0;
}

void CPCMonitorGUIDlg::ParseHexFile(std::vector<FILE_INFO>& infos, std::vector<uint8_t>& data)
{
	HEX_PARSING_STATE ParseState = START_OF_LINE;
	uint8_t LineByteCount = 0, LineChecksum = 0, LineAddress = 0, LineRecordType = 0;
	uint8_t ByteCount = 0, Checksum = 0, Data[32], buf[256];
	uint32_t expectedaddress = 0;

	CFile file;
	uint16_t bytesread;

	data.clear();
	infos.clear();

	if (file.Open(m_FileToSend, CFile::modeRead | CFile::typeBinary))
	{
		bytesread = file.Read(buf, 256);

		while (bytesread > 0)
		{
			for (uint16_t i = 0; i < bytesread; i++)
			{
				switch (ParseState)
				{
				case START_OF_LINE:
					if (buf[i] == ':')
					{
						LineByteCount = LineChecksum = LineAddress = LineRecordType = 0;
						ByteCount = Checksum = 0;
						memset(Data, 0, sizeof(Data));

						ParseState = BYTE_COUNT_0;
					}
					break;

				case BYTE_COUNT_0:
					LineByteCount = HexDigitToDecimal(buf[i]);

					ParseState = BYTE_COUNT_1;
					break;

				case BYTE_COUNT_1:
					LineByteCount *= 16;
					LineByteCount += HexDigitToDecimal(buf[i]);

					Checksum += LineByteCount;

					ParseState = ADDRESS_0;
					break;

				case ADDRESS_0:
					LineAddress = HexDigitToDecimal(buf[i]);

					ParseState = ADDRESS_1;
					break;

				case ADDRESS_1:
					LineAddress *= 16;
					LineAddress += HexDigitToDecimal(buf[i]);

					ParseState = ADDRESS_2;
					break;

				case ADDRESS_2:
					LineAddress *= 16;
					LineAddress += HexDigitToDecimal(buf[i]);

					ParseState = ADDRESS_3;
					break;

				case ADDRESS_3:
					LineAddress *= 16;
					LineAddress += HexDigitToDecimal(buf[i]);

					Checksum += LineAddress;

					ParseState = RECORD_TYPE_0;
					break;

				case RECORD_TYPE_0:
					LineRecordType = HexDigitToDecimal(buf[i]);

					ParseState = RECORD_TYPE_1;
					break;

				case RECORD_TYPE_1:
					LineRecordType *= 16;
					LineRecordType += HexDigitToDecimal(buf[i]);

					Checksum += LineRecordType;

					if (LineByteCount > 0)
						ParseState = DATA_0;
					else
						ParseState = CHECKSUM_0;
					break;

				case DATA_0:
					Data[ByteCount] = HexDigitToDecimal(buf[i]);

					ParseState = DATA_1;
					break;

				case DATA_1:
					Data[ByteCount] *= 16;
					Data[ByteCount] += HexDigitToDecimal(buf[i]);

					Checksum += Data[ByteCount];

					ByteCount++;
					if (ByteCount >= LineByteCount)
						ParseState = CHECKSUM_0;
					else
						ParseState = DATA_0;
					break;

				case CHECKSUM_0:
					LineChecksum = HexDigitToDecimal(buf[i]);

					ParseState = CHECKSUM_1;
					break;

				case CHECKSUM_1:
					LineChecksum *= 16;
					LineChecksum += HexDigitToDecimal(buf[i]);

					Checksum = ~Checksum;
					Checksum++;

					if (Checksum == LineChecksum && LineRecordType == 0)
					{
						//do something
						if(expectedaddress != LineAddress)
					}

					ParseState = START_OF_LINE;
					break;
				}
			}
		}

		file.Close();
	}
}



void CPCMonitorGUIDlg::OnBnClickedConnectbtn()
{
	UpdateData(TRUE);

	m_NetworkThread.SetIP(m_IPAddress);
	m_NetworkThread.SetPort(m_Port);

	if (m_NetworkThread.Connect())
	{
		LogEvent(_T("Success"));
		m_Commander.StartListener();
	}
	else
		LogEvent(_T("Failed"));
}


void CPCMonitorGUIDlg::OnBnClickedDisconnectbtn()
{
	m_NetworkThread.Disconnect();
}


void CPCMonitorGUIDlg::OnBnClickedLstsnsbtn()
{
	if (m_BuildingSensorList || m_UpdatingSensorList)
		return;

	if (m_BuildSensorListThread.joinable())
		m_BuildSensorListThread.join();

	m_SensorList.clear();
	m_BuildingSensorList = true;
	m_SensorListBuilt = false;

	m_BuildSensorListThread = std::thread(&CPCMonitorGUIDlg::BuildSensorList, this);
}


void CPCMonitorGUIDlg::OnBnClickedSndfilebtn()
{
	if (m_SendingFile)
		return;

	if (m_SendFileThread.joinable())
		m_SendFileThread.join();

	CFileDialog dlg(true, _T("hex"), NULL, OFN_HIDEREADONLY, _T("HEX Files (*.hex)|*.hex|All Files (*.*)|*.*||"), this);
	if (dlg.DoModal() == IDOK)
	{
		m_FileToSend.Empty();

		m_FileToSend.Append(dlg.GetFolderPath());
		m_FileToSend.Append(_T("\\"));
		m_FileToSend.Append(dlg.GetFileName());
		m_SendingFile = true;
		m_SendFileThread = std::thread(&CPCMonitorGUIDlg::SendFile, this);
	}
}


void CPCMonitorGUIDlg::OnBnClickedValidfilebtn()
{
	//Handled by background thread
}


void CPCMonitorGUIDlg::OnBnClickedUpdatesnsbtn()
{
	if (m_BuildingSensorList || m_UpdatingSensorList || !m_SensorListBuilt)
		return;

	if (m_UpdateSensorListThread.joinable())
		m_UpdateSensorListThread.join();

	m_UpdatingSensorList = true;

	m_UpdateSensorListThread = std::thread(&CPCMonitorGUIDlg::UpdateSensorList, this);
}


void CPCMonitorGUIDlg::OnClose()
{
	m_NetworkThread.Disconnect();

	if (m_BuildSensorListThread.joinable())
		m_BuildSensorListThread.join();

	if (m_UpdateSensorListThread.joinable())
		m_UpdateSensorListThread.join();

	if (m_SendFileThread.joinable())
		m_SendFileThread.join();

	CDialogEx::OnClose();
}
