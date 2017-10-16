
// SerialConnectorDlg.cpp : implementation file
//

#include "stdafx.h"
#include "atlconv.h"
#include "stdlib.h"
#include "SerialConnector.h"
#include "SerialConnectorDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CSerialConnectorDlg* CSerialConnectorDlg::vMyPointer;
bool CSerialConnectorDlg::vExitThread;

// CSerialConnectorDlg dialog
CSerialConnectorDlg::CSerialConnectorDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_SERIALCONNECTOR_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	vBaudRates[0] = "300"; vBaudRates[1] = "1200"; vBaudRates[2] = "2400"; vBaudRates[3] = "4800";
	vBaudRates[4] = "9600"; vBaudRates[5] = "19200"; vBaudRates[6] = "38400"; vBaudRates[7] = "57600";
	vBaudRates[8] = "115200"; vBaudRates[9] = "128000";
	vDataBits[0] = "5";	vDataBits[1] = "6";	vDataBits[2] = "7";	vDataBits[3] = "8";
	vParity[0] = "None"; vParity[1] = "Even"; vParity[2] = "Odd"; vParity[3] = "Mark"; vParity[4] = "Space";
	vStopBits[0] = "1";	vStopBits[1] = "1.5";	vStopBits[2] = "2";
	vFlowCtrl[0] = "Xon / Xoff"; vFlowCtrl[1] = "Hardware"; vFlowCtrl[2] = "None";
	vMessageBrush = ::CreateSolidBrush(cRed);
	vPort1Id = vPort2Id = -1;

	// Create listneer threads
	vExitThread = false;
	vMyPointer = this;
	vThreadStarted = false;
	vWorkerThread1 = AfxBeginThread(&CSerialConnectorDlg::ThreadProc1, 0, THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED, (LPSECURITY_ATTRIBUTES)0);
	vWorkerThread1->m_bAutoDelete = true;
	vWorkerThread2 = AfxBeginThread(&CSerialConnectorDlg::ThreadProc2, 0, THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED, (LPSECURITY_ATTRIBUTES)0);
	vWorkerThread2->m_bAutoDelete = true;
}

CSerialConnectorDlg::~CSerialConnectorDlg()
{
	vExitThread = true;
	DeleteObject(vMessageBrush);
	vPort1.CancelIo();
	vPort2.CancelIo();
	vWorkerThread1->ResumeThread();
	vWorkerThread2->ResumeThread();
	Sleep(100);
}

void CSerialConnectorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_MessageArea, m_MessageArea);
	DDX_Control(pDX, IDC_Port1Number, m_Port1Number);
	DDX_Control(pDX, IDC_Port1BaudRate, m_Port1BaudRate);
	DDX_Control(pDX, IDC_Port1Parity, m_Port1Parity);
	DDX_Control(pDX, IDC_Port1DataBits, m_Port1DataBits);
	DDX_Control(pDX, IDC_Port1StopBits, m_Port1StopBits);
	DDX_Control(pDX, IDC_Port1FlowCtrl, m_Port1FlowCtrl);
	DDX_Control(pDX, IDC_Port1Open, m_Port1Open);
	DDX_Control(pDX, IDC_Port2Number, m_Port2Number);
	DDX_Control(pDX, IDC_Port2BaudRate, m_Port2BaudRate);
	DDX_Control(pDX, IDC_Port2Parity, m_Port2Parity);
	DDX_Control(pDX, IDC_Port2DataBits, m_Port2DataBits);
	DDX_Control(pDX, IDC_Port2StopBits, m_Port2StopBits);
	DDX_Control(pDX, IDC_Port2FlowCtrl, m_Port2FlowCtrl);
	DDX_Control(pDX, IDC_Port2Open, m_Port2Open);
	DDX_Control(pDX, IDOK, m_IDOK);
}

BEGIN_MESSAGE_MAP(CSerialConnectorDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CSerialConnectorDlg::OnConnectClicked)
	ON_BN_CLICKED(IDC_Port1Open, &CSerialConnectorDlg::OnPort1OpenClicked)
	ON_BN_CLICKED(IDC_Port2Open, &CSerialConnectorDlg::OnPort2OpenClicked)
	ON_WM_CTLCOLOR()
	ON_STN_CLICKED(IDC_MessageArea, &CSerialConnectorDlg::OnMessageAreaClicked)
END_MESSAGE_MAP()


// CSerialConnectorDlg message handlers

BOOL CSerialConnectorDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	SetupComParameterSelections();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CSerialConnectorDlg::SetupComParameterSelections()
{
	// Setup COM port numbers
	CString comId;
	for (int indx = 1; indx < 256; indx++)
	{
		comId.Format(_T("%d"), indx);
		m_Port1Number.AddString(comId);
		m_Port2Number.AddString(comId);
	}
	m_Port1Number.SetCurSel(0);
	m_Port2Number.SetCurSel(0);

	// Setup baud rates
	for (int indx = 0; indx < 10; indx++)
	{
		m_Port1BaudRate.AddString(vBaudRates[indx]);
		m_Port2BaudRate.AddString(vBaudRates[indx]);
	}
	m_Port1BaudRate.SetCurSel(4);
	m_Port2BaudRate.SetCurSel(4);

	// Setup data bits
	for (int indx = 0; indx < 4; indx++)
	{
		m_Port1DataBits.AddString(vDataBits[indx]);
		m_Port2DataBits.AddString(vDataBits[indx]);
	}
	m_Port1DataBits.SetCurSel(3);
	m_Port2DataBits.SetCurSel(3);

	// Setup the parity
	for (int indx = 0; indx < 5; indx++)
	{
		m_Port1Parity.AddString(vParity[indx]);
		m_Port2Parity.AddString(vParity[indx]);
	}
	m_Port1Parity.SetCurSel(0);
	m_Port2Parity.SetCurSel(0);

	// Setup stop bits
	for (int indx = 0; indx < 3; indx++)
	{
		m_Port1StopBits.AddString(vStopBits[indx]);
		m_Port2StopBits.AddString(vStopBits[indx]);
	}
	m_Port1StopBits.SetCurSel(0);
	m_Port2StopBits.SetCurSel(0);

	// Setup flow control
	for (int indx = 0; indx < 3; indx++)
	{
		m_Port1FlowCtrl.AddString(vFlowCtrl[indx]);
		m_Port2FlowCtrl.AddString(vFlowCtrl[indx]);
	}
	m_Port1FlowCtrl.SetCurSel(2);
	m_Port2FlowCtrl.SetCurSel(2);

	m_IDOK.EnableWindow(false);
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CSerialConnectorDlg::OnPaint()
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

UINT CSerialConnectorDlg::ThreadProc1(LPVOID param)
{
	BYTE readBuff[32];
	DWORD bytesRead = 0;
	for (;!vExitThread;)
	{
		TRY
		{
			if ((bytesRead = vMyPointer->vPort1.Read(&readBuff, 1)) > 0)
				vMyPointer->vPort2.Write(&readBuff, bytesRead);
		}
			CATCH_ALL(e)
		{
			TCHAR buff[1024];
			e->GetErrorMessage((LPTSTR)(&buff[0]), 1022);
			e->Delete();
		}
		END_CATCH_ALL
	}
	return 0;
}

UINT CSerialConnectorDlg::ThreadProc2(LPVOID param)
{
	BYTE readBuff[32];
	DWORD bytesRead = 0;
	for (;!vExitThread;)
	{
		TRY
		{
		  if ((bytesRead = vMyPointer->vPort2.Read(&readBuff, 30)) > 0)
				vMyPointer->vPort1.Write(&readBuff, bytesRead);
		}
			CATCH_ALL(e)
		{
			TCHAR buff[1024];
			e->GetErrorMessage((LPTSTR)(&buff[0]), 1022);
			e->Delete();
		}
		END_CATCH_ALL
	}
	return 0;
}

void CSerialConnectorDlg::CheckPortConnectivity()
{
	if (vPort1.IsOpen() && vPort2.IsOpen())
	{
		m_IDOK.EnableWindow(true);
	}
	else
	{
		m_IDOK.SetWindowTextW(_T("Connect"));
		m_IDOK.EnableWindow(false);
	}
}

void CSerialConnectorDlg::GetSerialParams(int portId)
{
	if (portId == 1) GetPort1SerialParams();
	else if (portId == 2) GetPort2SerialParams();
}

void CSerialConnectorDlg::GetSelectedText(CComboBox* ctrl, CString* text)
{
	int selIndx = ctrl->GetCurSel();
	*text = ""; ctrl->GetLBText(selIndx, *text);
}

void CSerialConnectorDlg::GetPort1SerialParams()
{
	long valueNum;
	CString selText;
	vPortVal = m_Port1Number.GetCurSel() + 1;

	GetSelectedText(&m_Port1BaudRate, &selText);
	CT2A strVal(selText);
	valueNum = atol((const char*)strVal);
	vBaudVal = (DWORD)valueNum;

	GetSelectedText(&m_Port1DataBits, &selText);
	CT2A strVal1(selText);
	valueNum = atol((const char*)strVal1);
	vDataBitVal = (BYTE)valueNum;

	GetSelectedText(&m_Port1Parity, &selText);
	if (selText.Compare(_T("None")) == 0) vParityVal = CSerialPort::NoParity;
	else if (selText.Compare(_T("Even")) == 0) vParityVal = CSerialPort::EvenParity;
	else if (selText.Compare(_T("Odd")) == 0) vParityVal = CSerialPort::OddParity;
	else if (selText.Compare(_T("Mark")) == 0) vParityVal = CSerialPort::MarkParity;
	else if (selText.Compare(_T("Space")) == 0) vParityVal = CSerialPort::SpaceParity;

	GetSelectedText(&m_Port1StopBits, &selText);
	if (selText.Compare(_T("1")) == 0) vStopBitVal = CSerialPort::OneStopBit;
	else if (selText.Compare(_T("1.5")) == 0) vStopBitVal = CSerialPort::OnePointFiveStopBits;
	else if (selText.Compare(_T("2")) == 0) vStopBitVal = CSerialPort::TwoStopBits;

	GetSelectedText(&m_Port1FlowCtrl, &selText);
	if (selText.Compare(_T("Xon / Xoff")) == 0) vFlowCtrlVal = CSerialPort::XonXoffFlowControl;
	else if (selText.Compare(_T("Hardware")) == 0) vFlowCtrlVal = CSerialPort::CtsRtsFlowControl;
	else if (selText.Compare(_T("None")) == 0) vFlowCtrlVal = CSerialPort::NoFlowControl;

	vOverlappedVal = false;
}

void CSerialConnectorDlg::GetPort2SerialParams()
{
	long valueNum;
	CString selText;
	vPortVal = m_Port2Number.GetCurSel() + 1;

	GetSelectedText(&m_Port2BaudRate, &selText);
	CT2A strVal(selText);
	valueNum = atol((const char*)strVal);
	vBaudVal = (DWORD)valueNum;

	GetSelectedText(&m_Port2DataBits, &selText);
	CT2A strVal1(selText);
	valueNum = atol((const char*)strVal1);
	vDataBitVal = (BYTE)valueNum;

	GetSelectedText(&m_Port2Parity, &selText);
	if (selText.Compare(_T("None")) == 0) vParityVal = CSerialPort::NoParity;
	else if (selText.Compare(_T("Even")) == 0) vParityVal = CSerialPort::EvenParity;
	else if (selText.Compare(_T("Odd")) == 0) vParityVal = CSerialPort::OddParity;
	else if (selText.Compare(_T("Mark")) == 0) vParityVal = CSerialPort::MarkParity;
	else if (selText.Compare(_T("Space")) == 0) vParityVal = CSerialPort::SpaceParity;

	GetSelectedText(&m_Port2StopBits, &selText);
	if (selText.Compare(_T("1")) == 0) vStopBitVal = CSerialPort::OneStopBit;
	else if (selText.Compare(_T("1.5")) == 0) vStopBitVal = CSerialPort::OnePointFiveStopBits;
	else if (selText.Compare(_T("2")) == 0) vStopBitVal = CSerialPort::TwoStopBits;

	GetSelectedText(&m_Port2FlowCtrl, &selText);
	if (selText.Compare(_T("Xon / Xoff")) == 0) vFlowCtrlVal = CSerialPort::XonXoffFlowControl;
	else if (selText.Compare(_T("Hardware")) == 0) vFlowCtrlVal = CSerialPort::CtsRtsFlowControl;
	else if (selText.Compare(_T("None")) == 0) vFlowCtrlVal = CSerialPort::NoFlowControl;

	vOverlappedVal = false;
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CSerialConnectorDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CSerialConnectorDlg::DisplayUserMessage(CString msg)
{
	m_MessageArea.SetWindowTextW(msg);
	m_MessageArea.ShowWindow(SW_SHOW);
	m_Port1Number.EnableWindow(false);
	m_Port1BaudRate.EnableWindow(false);
	m_Port1Parity.EnableWindow(false);
	m_Port1DataBits.EnableWindow(false);
	m_Port1StopBits.EnableWindow(false);
	m_Port1FlowCtrl.EnableWindow(false);
	m_Port1Open.EnableWindow(false);
	m_Port2Number.EnableWindow(false);
	m_Port2BaudRate.EnableWindow(false);
	m_Port2Parity.EnableWindow(false);
	m_Port2DataBits.EnableWindow(false);
	m_Port2StopBits.EnableWindow(false);
	m_Port2FlowCtrl.EnableWindow(false);
	m_Port2Open.EnableWindow(false);
}

void CSerialConnectorDlg::OnMessageAreaClicked()
{
	m_Port1Number.EnableWindow(true);
	m_Port1BaudRate.EnableWindow(true);
	m_Port1Parity.EnableWindow(true);
	m_Port1DataBits.EnableWindow(true);
	m_Port1StopBits.EnableWindow(true);
	m_Port1FlowCtrl.EnableWindow(true);
	m_Port1Open.EnableWindow(true);
	m_Port2Number.EnableWindow(true);
	m_Port2BaudRate.EnableWindow(true);
	m_Port2Parity.EnableWindow(true);
	m_Port2DataBits.EnableWindow(true);
	m_Port2StopBits.EnableWindow(true);
	m_Port2FlowCtrl.EnableWindow(true);
	m_Port2Open.EnableWindow(true);
	m_MessageArea.ShowWindow(SW_HIDE);
}

void CSerialConnectorDlg::OnConnectClicked()
{// Close the ports before the application stack starts unwinding.
	if (vThreadStarted == false)
	{
		vWorkerThread1->ResumeThread();
		vWorkerThread2->ResumeThread();
		m_IDOK.SetWindowTextW(_T("Disconnect"));
		vThreadStarted = true;
	}
	else
	{
		vPort1.CancelIo();
		vPort2.CancelIo();
		vWorkerThread1->SuspendThread();
		vWorkerThread2->SuspendThread();
		m_IDOK.SetWindowTextW(_T("Connect"));
		vThreadStarted = false;
	}
}

void CSerialConnectorDlg::OnPort1OpenClicked()
{
	if (vPort1.IsOpen())
	{
		vPort1.CancelIo();
		if(vPort2.IsOpen()) vPort2.CancelIo();
		vWorkerThread1->SuspendThread();
		vWorkerThread2->SuspendThread();
		vPort1.Close();
		m_Port1Open.SetWindowTextW(_T("Open"));
		vPort1Id = -1;
	}
	else
	{
		GetSerialParams(1);
		if(vPortVal == vPort2Id)
		{
		  CString ErrorMessage; ErrorMessage.Format(_T("COM%d is in use by Port2."), vPortVal);
			DisplayUserMessage(ErrorMessage);
			return;
		}
		if (vPort1.IsOpen()) vPort1.Close();
		TRY
		{
			vPort1.Open(vPortVal, vBaudVal, vParityVal, vDataBitVal, vStopBitVal, vFlowCtrlVal, vOverlappedVal);
		  vPort1Id = vPortVal;
		}
			CATCH_ALL(e)
		{
			vPort1.Close();
			e->Delete();
		}
		END_CATCH_ALL
			if (!vPort1.IsOpen())
			{
				CString ErrorMessage; ErrorMessage.Format(_T("Could not open Port1 on COM%d."), vPortVal);
				DisplayUserMessage(ErrorMessage);
			}
			else
			{
				m_Port1Open.SetWindowTextW(_T("Close"));
			}
	}
	CheckPortConnectivity();
}

void CSerialConnectorDlg::OnPort2OpenClicked()
{
	if (vPort2.IsOpen())
	{
		if(vPort1.IsOpen()) vPort1.CancelIo();
		vPort2.CancelIo();
		vWorkerThread1->SuspendThread();
		vWorkerThread2->SuspendThread();
		vPort2.Close();
		m_Port2Open.SetWindowTextW(_T("Open"));
		vPort2Id = -1;
	}
	else
	{
		GetSerialParams(2);
		if (vPortVal == vPort1Id)
		{
			CString ErrorMessage; ErrorMessage.Format(_T("COM%d is in use by Port1."), vPortVal);
			DisplayUserMessage(ErrorMessage);
			return;
		}
		if (vPort2.IsOpen()) vPort2.Close();
		TRY
		{
			vPort2.Open(vPortVal, vBaudVal, vParityVal, vDataBitVal, vStopBitVal, vFlowCtrlVal, vOverlappedVal);
			vPort2Id = vPortVal;
		}
			CATCH_ALL(e)
		{
			vPort2.Close();
			e->Delete();
		}
		END_CATCH_ALL
			if (!vPort2.IsOpen())
			{
				CString ErrorMessage; ErrorMessage.Format(_T("Could not open Port2 on COM%d."), vPortVal);
				DisplayUserMessage(ErrorMessage);
			}
			else
			{
				m_Port2Open.SetWindowTextW(_T("Close"));
			}
	}
	CheckPortConnectivity();
}

HBRUSH CSerialConnectorDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hBr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);
	UINT CtrlId = pWnd->GetDlgCtrlID();
	if (CtrlId == IDC_MessageArea)
	{
		pDC->SetBkMode(TRANSPARENT);
		hBr = vMessageBrush;
	}
	return hBr;
}

