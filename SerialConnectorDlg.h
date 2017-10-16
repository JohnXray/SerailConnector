
// SerialConnectorDlg.h : header file
//

#pragma once

#include "SerialPort.h"

// CSerialConnectorDlg dialog
class CSerialConnectorDlg : public CDialogEx
{
	// Construction
public:
	CSerialConnectorDlg(CWnd* pParent = NULL);	// standard constructor
	~CSerialConnectorDlg();	// standard destructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SERIALCONNECTOR_DIALOG };
#endif

private:
  static bool vExitThread;
	static CSerialConnectorDlg* vMyPointer;
	bool vThreadStarted;
	CWinThread * vWorkerThread1;
	CWinThread * vWorkerThread2;
	static UINT ThreadProc1(LPVOID param);
	static UINT ThreadProc2(LPVOID param);

	const COLORREF cRed = 0x0000FFFF; //COLORREF hexadecimal form 0x00bbggrr
	HBRUSH vMessageBrush; 

	CSerialPort vPort1;
	CSerialPort vPort2;

	CString vBaudRates[10];
	CString vDataBits[4];
	CString vParity[5];
	CString vStopBits[3];
	CString vFlowCtrl[3];

	CStatic m_MessageArea;
	CComboBox m_Port1Number;
	CComboBox m_Port1BaudRate;
	CComboBox m_Port1Parity;
	CComboBox m_Port1DataBits;
	CComboBox m_Port1StopBits;
	CComboBox m_Port1FlowCtrl;
	CButton m_Port1Open;
	CComboBox m_Port2Number;
	CComboBox m_Port2BaudRate;
	CComboBox m_Port2Parity;
	CComboBox m_Port2DataBits;
	CComboBox m_Port2StopBits;
	CComboBox m_Port2FlowCtrl;
	CButton m_Port2Open;
	CButton m_IDOK;

	int vPortVal;
	DWORD vBaudVal;
	CSerialPort::Parity vParityVal;
	BYTE vDataBitVal;
	CSerialPort::StopBits vStopBitVal;
	CSerialPort::FlowControl vFlowCtrlVal;
	BOOL vOverlappedVal;


	void SetupComParameterSelections();
	void GetSelectedText(CComboBox* ctrl, CString* text);
	void GetSerialParams(int portId);
	void GetPort1SerialParams();
	void GetPort2SerialParams();
	void DisplayUserMessage(CString msg);
	void CheckPortConnectivity();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnConnectClicked();
	afx_msg void OnPort1OpenClicked();
	afx_msg void OnPort2OpenClicked();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnMessageAreaClicked();
};
