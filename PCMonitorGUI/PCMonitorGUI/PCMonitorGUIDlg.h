
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

	public:
	afx_msg void OnBnClickedConnectbtn();

	private:
	CString m_LoggingText;

	CString m_IPAddress;
	CString m_Port;

	NetworkThread m_NetworkThread;
	Commander m_Commander;

	std::vector<std::pair<CString, float>> m_SensorList;

	bool m_BuildingSensorList, m_UpdatingSensorList, m_SensorListBuilt;
	std::thread m_BuildSensorListThread;
	std::thread m_UpdateSensorListThread;
	std::mutex m_SensorListLock;

	bool m_SendingFile;
	CString m_FileToSend;
	std::thread m_SendFileThread;

	public:
	afx_msg void OnBnClickedDisconnectbtn();
	afx_msg void OnBnClickedLstsnsbtn();
	afx_msg void OnBnClickedSndfilebtn();
	afx_msg void OnBnClickedValidfilebtn();
	afx_msg void OnBnClickedUpdatesnsbtn();
	afx_msg void OnClose();
};
