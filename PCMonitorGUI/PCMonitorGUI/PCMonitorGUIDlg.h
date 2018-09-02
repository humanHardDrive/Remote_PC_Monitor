
// PCMonitorGUIDlg.h : header file
//
#include <queue>
#include <vector>
#include <utility>

#include <thread>
#include <mutex>

#include "NetworkThread.h"
#include "Commander.h"

#pragma once

#define MAX_EVENTS		100

// CPCMonitorGUIDlg dialog
class CPCMonitorGUIDlg : public CDialogEx
{
// Construction
public:
	CPCMonitorGUIDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PCMONITORGUI_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;
	std::queue<CString> m_EventQ;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

	void LogEvent(CString event);

	void BuildSensorList();
	void UpdateSensorList();
	void SendFile();

	void UpdateConnectionStatus(bool connected);
	void UpdateSensorListBox();
	void UpdateParameterListBox();

	void AddData(CListCtrl & ctrl, int row, int col, LPWSTR str);
	uint32_t ParseHexFile(CString path, uint8_t* Binary, uint32_t maxsize);

	public:
	afx_msg void OnBnClickedConnectbtn();

	private:
	CString m_LoggingText;

	CString m_IPAddress;
	CString m_Port;

	NetworkThread m_NetworkThread;
	Commander m_Commander;

	std::vector<std::pair<CString, float>> m_SensorList;
	std::vector<std::pair<CString, uint16_t>> m_ParameterList;

	bool m_BuildingSensorList, m_UpdatingSensorList, m_SensorListBuilt;
	std::thread m_BuildSensorListThread;
	std::thread m_UpdateSensorListThread;
	std::mutex m_SensorListLock;

	bool m_SendingFile;
	CString m_FileToSend;
	std::thread m_SendFileThread;

	CButton m_ConnectedChkBox;
	CButton m_ConnectBtn;
	CButton m_DisconnectBtn;

	CButton m_SendFileBtn;
	CButton m_ValidateFileBtn;
	CButton m_ListSensorsBtn;
	CButton m_UpdateSensorsBtn;

	CProgressCtrl m_FileProgressBar;

	CListCtrl m_SensorListBox;
	CListCtrl m_ParameterListBox;

	public:
	afx_msg void OnBnClickedDisconnectbtn();
	afx_msg void OnBnClickedLstsnsbtn();
	afx_msg void OnBnClickedSndfilebtn();
	afx_msg void OnBnClickedValidfilebtn();
	afx_msg void OnBnClickedUpdatesnsbtn();
	afx_msg void OnClose();
};
