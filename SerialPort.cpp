///////////////////////////////// Includes ////////////////////////////////////

#include "stdafx.h"
#include "SerialPort.h"

#ifndef __ATLBASE_H__
#pragma message("To avoid this message, please put atlbase.h in your pre compiled header (normally stdafx.h)")
#include <atlbase.h>
#endif //#ifndef __ATLBASE_H__


///////////////////////////////// Defines /////////////////////////////////////

#ifdef CSERIALPORT_MFC_EXTENSIONS
#ifdef _DEBUG
#define new DEBUG_NEW
#endif //#ifdef _DEBUG
#endif //#ifdef CSERIALPORT_MFC_EXTENSIONS


//////////////////////////////// Implementation ///////////////////////////////

#if _MSC_VER >= 1700
BOOL CSerialException::GetErrorMessage(_Out_z_cap_(nMaxError) LPTSTR lpszError, _In_ UINT nMaxError, _Out_opt_ PUINT pnHelpContext) const
#elif _MSC_VER >= 1500
BOOL CSerialException::GetErrorMessage(_Out_z_cap_(nMaxError) LPTSTR lpszError, _In_ UINT nMaxError, _Out_opt_ PUINT pnHelpContext) const
#else
BOOL CSerialException::GetErrorMessage(__out_ecount_z(nMaxError) LPTSTR lpszError, __in UINT nMaxError, __out_opt PUINT pnHelpContext) const
#endif //#if _MSC_VER >= 1700
{
  //Validate our parameters
  ATLASSERT(lpszError != NULL);
    
  if (pnHelpContext != NULL)
    *pnHelpContext = 0;
    
  //What will be the return value from this function (assume the worst)
  BOOL bSuccess = FALSE;

  LPTSTR lpBuffer;
  DWORD dwReturn = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                 NULL,  m_dwError, MAKELANGID(LANG_NEUTRAL, SUBLANG_SYS_DEFAULT),
                                 reinterpret_cast<LPTSTR>(&lpBuffer), 0, NULL);

  if (dwReturn == 0)
    *lpszError = _T('\0');
  else
  {
    bSuccess = TRUE;
    Checked::tcsncpy_s(lpszError, nMaxError, lpBuffer, _TRUNCATE);  
    LocalFree(lpBuffer);
  }

  return bSuccess;
}

#if _MSC_VER >= 1700
BOOL CSerialException::GetErrorMessage(_Out_z_cap_(nMaxError) LPTSTR lpszError, _In_ UINT nMaxError, _Out_opt_ PUINT pnHelpContext)
#elif _MSC_VER >= 1500
BOOL CSerialException::GetErrorMessage(_Out_z_cap_(nMaxError) LPTSTR lpszError, _In_ UINT nMaxError, _Out_opt_ PUINT pnHelpContext)
#else
BOOL CSerialException::GetErrorMessage(__out_ecount_z(nMaxError) LPTSTR lpszError, __in UINT nMaxError, __out_opt PUINT pnHelpContext)
#endif //#if _MSC_VER >= 1700
{
  //Call the const version of GetErrorMessage
  return const_cast<const CSerialException*>(this)->GetErrorMessage(lpszError, nMaxError, pnHelpContext);
}

#ifdef CSERIALPORT_MFC_EXTENSIONS
CString CSerialException::GetErrorMessage()
{
  CString rVal;
  LPTSTR pstrError = rVal.GetBuffer(4096);
  GetErrorMessage(pstrError, 4096, NULL);
  rVal.ReleaseBuffer();
  return rVal;
}
#endif //#ifdef CSERIALPORT_MFC_EXTENSIONS

CSerialException::CSerialException(_In_ DWORD dwError) : m_dwError(dwError)
{
}

#ifdef CSERIALPORT_MFC_EXTENSIONS
#ifdef _DEBUG
void CSerialException::Dump(_In_ CDumpContext& dc) const
{
  CObject::Dump(dc);

  dc << _T("m_dwError = ") << m_dwError << _T("\n");
}
#endif //#ifdef _DEBUG
#endif //#ifdef CSERIALPORT_MFC_EXTENSIONS


CSerialPort::CSerialPort() : m_hComm(INVALID_HANDLE_VALUE)
{
}

CSerialPort::~CSerialPort()
{
  Close();
}

void CSerialPort::ThrowSerialException(_In_ DWORD dwError)
{
  if (dwError == 0)
    dwError = ::GetLastError();

  ATLTRACE(_T("Warning: throwing CSerialException for error %d\n"), dwError);
#ifdef CSERIALPORT_MFC_EXTENSIONS
  CSerialException* pException = new CSerialException (dwError);
  THROW(pException);
 #else
  CSerialException e(dwError);
  throw e;
 #endif //#ifdef CSERIALPORT_MFC_EXTENSIONS
}

#ifdef CSERIALPORT_MFC_EXTENSIONS
#ifdef _DEBUG
void CSerialPort::Dump(CDumpContext& dc) const
{
  dc << _T("m_hComm = ") << m_hComm << _T("\n");
}
#endif //#ifdef _DEBUG
#endif //#ifdef CSERIALPORT_MFC_EXTENSIONS

void CSerialPort::Open(_In_z_ LPCTSTR pszPort, _In_ DWORD dwBaud, _In_ Parity parity, _In_ BYTE DataBits, _In_ StopBits stopBits, _In_ FlowControl fc, _In_ BOOL bOverlapped)
{
  Close(); //In case we are already open

  //Call CreateFile to open the comms port
  m_hComm = CreateFile(pszPort, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, bOverlapped ? FILE_FLAG_OVERLAPPED : 0, NULL);
  if (m_hComm == INVALID_HANDLE_VALUE)
  {
    DWORD dwLastError = GetLastError();
    ATLTRACE(_T("CSerialPort::Open, Failed to open the comms port, Error:%u\n"), dwLastError);
    ThrowSerialException(dwLastError);
  }

  //Get the current state prior to changing it
  DCB dcb;
  dcb.DCBlength = sizeof(DCB);
  GetState(dcb);

  //Setup the baud rate
  dcb.BaudRate = dwBaud; 

  //Setup the Parity
  switch (parity)
  {
    case EvenParity:  
    {
      dcb.Parity = EVENPARITY;
      break;
    }
    case MarkParity:  
    {
      dcb.Parity = MARKPARITY;
      break;
    }
    case NoParity:    
    {
      dcb.Parity = NOPARITY;
      break;
    }
    case OddParity:   
    {
      dcb.Parity = ODDPARITY;
      break;
    }
    case SpaceParity: 
    {
      dcb.Parity = SPACEPARITY;
      break;
    }
    default:          
    {
      ATLASSERT(FALSE);            
      break;
    }
  }

  //Setup the data bits
  dcb.ByteSize = DataBits;

  //Setup the stop bits
  switch (stopBits)
  {
    case OneStopBit:           
    {
      dcb.StopBits = ONESTOPBIT; 
      break;
    }
    case OnePointFiveStopBits: 
    {
      dcb.StopBits = ONE5STOPBITS;
      break;
    }
    case TwoStopBits:          
    {
      dcb.StopBits = TWOSTOPBITS;
      break;
    }
    default:                   
    {
      ATLASSERT(FALSE);
      break;
    }
  }

  //Setup the flow control 
  dcb.fDsrSensitivity = FALSE;
  switch (fc)
  {
    case NoFlowControl:
    {
      dcb.fOutxCtsFlow = FALSE;
      dcb.fOutxDsrFlow = FALSE;
      dcb.fOutX = FALSE;
      dcb.fInX = FALSE;
      break;
    }
    case CtsRtsFlowControl:
    {
      dcb.fOutxCtsFlow = TRUE;
      dcb.fOutxDsrFlow = FALSE;
      dcb.fRtsControl = RTS_CONTROL_HANDSHAKE;
      dcb.fOutX = FALSE;
      dcb.fInX = FALSE;
      break;
    }
    case CtsDtrFlowControl:
    {
      dcb.fOutxCtsFlow = TRUE;
      dcb.fOutxDsrFlow = FALSE;
      dcb.fDtrControl = DTR_CONTROL_HANDSHAKE;
      dcb.fOutX = FALSE;
      dcb.fInX = FALSE;
      break;
    }
    case DsrRtsFlowControl:
    {
      dcb.fOutxCtsFlow = FALSE;
      dcb.fOutxDsrFlow = TRUE;
      dcb.fRtsControl = RTS_CONTROL_HANDSHAKE;
      dcb.fOutX = FALSE;
      dcb.fInX = FALSE;
      break;
    }
    case DsrDtrFlowControl:
    {
      dcb.fOutxCtsFlow = FALSE;
      dcb.fOutxDsrFlow = TRUE;
      dcb.fDtrControl = DTR_CONTROL_HANDSHAKE;
      dcb.fOutX = FALSE;
      dcb.fInX = FALSE;
      break;
    }
    case XonXoffFlowControl:
    {
      dcb.fOutxCtsFlow = FALSE;
      dcb.fOutxDsrFlow = FALSE;
      dcb.fOutX = TRUE;
      dcb.fInX = TRUE;
      dcb.XonChar = 0x11;
      dcb.XoffChar = 0x13;
      dcb.XoffLim = 100;
      dcb.XonLim = 100;
      break;
    }
    default:
    {
      ATLASSERT(FALSE);
      break;
    }
  }
  
  //Now that we have all the settings in place, make the changes
  SetState(dcb);
}

void CSerialPort::Open(_In_ int nPort, _In_ DWORD dwBaud, _In_ Parity parity, _In_ BYTE DataBits, _In_ StopBits stopBits, _In_ FlowControl fc, _In_ BOOL bOverlapped)
{
  //Form the string version of the port number
  TCHAR szPort[12];
  _stprintf_s(szPort, sizeof(szPort)/sizeof(TCHAR), _T("\\\\.\\COM%d"), nPort);

  //Delegate the work to the other version of Open
  Open(szPort, dwBaud, parity, DataBits, stopBits, fc, bOverlapped);
}

void CSerialPort::Close()
{
  if (IsOpen())
  {
    //Close down the comms port
    CloseHandle(m_hComm);
    m_hComm = INVALID_HANDLE_VALUE;
  }
}

void CSerialPort::Attach(_In_ HANDLE hComm)
{
  Close();

  //Validate our parameters, now that the port has been closed
  ATLASSERT(m_hComm == INVALID_HANDLE_VALUE);

  m_hComm = hComm;  
}

HANDLE CSerialPort::Detach()
{
  //What will be the return value from this function
  HANDLE hComm = m_hComm;

  m_hComm = INVALID_HANDLE_VALUE;

  return hComm;
}

DWORD CSerialPort::Read(_Out_writes_bytes_(dwNumberOfBytesToRead) __out_data_source(FILE) void* lpBuffer, _In_ DWORD dwNumberOfBytesToRead)
{
  //Validate our parameters
  ATLASSERT(IsOpen());

  DWORD dwBytesRead = 0;
  if (!ReadFile(m_hComm, lpBuffer, dwNumberOfBytesToRead, &dwBytesRead, NULL))
  {
    DWORD dwLastError = GetLastError();
    ATLTRACE(_T("CSerialPort::Read, Failed in call to ReadFile, Error:%u\n"), dwLastError);
    ThrowSerialException(dwLastError);
  }

  return dwBytesRead;
}

void CSerialPort::Read(_Out_writes_bytes_to_opt_(dwNumberOfBytesToRead, *lpNumberOfBytesRead) __out_data_source(FILE) void* lpBuffer, _In_ DWORD dwNumberOfBytesToRead, _In_ OVERLAPPED& overlapped, _Inout_opt_ DWORD* lpNumberOfBytesRead)
{
  //Validate our parameters
  ATLASSERT(IsOpen());

  if (!ReadFile(m_hComm, lpBuffer, dwNumberOfBytesToRead, lpNumberOfBytesRead, &overlapped))
  {
    DWORD dwLastError = GetLastError();
    ATLTRACE(_T("CSerialPort::Read, Failed in call to ReadFile, Error:%u\n"), dwLastError);
    ThrowSerialException(dwLastError);
  }
}

void CSerialPort::ReadEx(_Out_writes_bytes_opt_(dwNumberOfBytesToRead) __out_data_source(FILE) LPVOID lpBuffer, _In_ DWORD dwNumberOfBytesToRead, _Inout_ LPOVERLAPPED lpOverlapped, _In_ LPOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine)
{
  //Validate our parameters
  ATLASSERT(IsOpen());

  if (!ReadFileEx(m_hComm, lpBuffer, dwNumberOfBytesToRead, lpOverlapped, lpCompletionRoutine))
  {
    DWORD dwLastError = GetLastError();
    ATLTRACE(_T("CSerialPort::ReadEx, Failed in call to ReadFileEx, Error:%u\n"), dwLastError);
    ThrowSerialException(dwLastError);
  }
}

DWORD CSerialPort::Write(_In_reads_bytes_opt_(dwNumberOfBytesToWrite) const void* lpBuffer, _In_ DWORD dwNumberOfBytesToWrite)
{
  //Validate our parameters
  ATLASSERT(IsOpen());

  DWORD dwBytesWritten = 0;
  if (!WriteFile(m_hComm, lpBuffer, dwNumberOfBytesToWrite, &dwBytesWritten, NULL))
  {
    DWORD dwLastError = GetLastError();
    ATLTRACE(_T("CSerialPort::Write, Failed in call to WriteFile, Error:%u\n"), dwLastError);
    ThrowSerialException(dwLastError);
  }

  return dwBytesWritten;
}

void CSerialPort::Write(_In_reads_bytes_opt_(dwNumberOfBytesToWrite) const void* lpBuffer, _In_ DWORD dwNumberOfBytesToWrite, _In_ OVERLAPPED& overlapped, _Out_opt_ DWORD* lpNumberOfBytesWritten)
{
  //Validate our parameters
  ATLASSERT(IsOpen());

  if (!WriteFile(m_hComm, lpBuffer, dwNumberOfBytesToWrite, lpNumberOfBytesWritten, &overlapped))
  {
    DWORD dwLastError = GetLastError();
    ATLTRACE(_T("CSerialPort::Write, Failed in call to WriteFile, Error:%u\n"), dwLastError);
    ThrowSerialException(dwLastError);
  }
}

void CSerialPort::WriteEx(_In_reads_bytes_opt_(dwNumberOfBytesToWrite) LPCVOID lpBuffer, _In_ DWORD dwNumberOfBytesToWrite, _Inout_ LPOVERLAPPED lpOverlapped, _In_ LPOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine)
{
  //Validate our parameters
  ATLASSERT(IsOpen());

  if (!WriteFileEx(m_hComm, lpBuffer, dwNumberOfBytesToWrite, lpOverlapped, lpCompletionRoutine))
  {
    DWORD dwLastError = GetLastError();
    ATLTRACE(_T("CSerialPort::WriteEx, Failed in call to WriteFileEx, Error:%u\n"), dwLastError);
    ThrowSerialException(dwLastError);
  }
}

void CSerialPort::GetOverlappedResult(_In_ OVERLAPPED& overlapped, _Out_ DWORD& dwBytesTransferred, _In_ BOOL bWait)
{
  //Validate our parameters
  ATLASSERT(IsOpen());

  if (!::GetOverlappedResult(m_hComm, &overlapped, &dwBytesTransferred, bWait))
  {
    DWORD dwLastError = GetLastError();
    ATLTRACE(_T("CSerialPort::GetOverlappedResult, Failed in call to GetOverlappedResult, Error:%u\n"), dwLastError);
    ThrowSerialException(dwLastError);
  }
}

void CSerialPort::CancelIo()
{
  //Validate our parameters
  ATLASSERT(IsOpen());

  if (!::CancelIo(m_hComm))
  {
    DWORD dwLastError = GetLastError();
    ATLTRACE(_T("Failed in call to CancelIO, Error:%u\n"), dwLastError);
    ThrowSerialException(dwLastError);
  }
}

DWORD CSerialPort::BytesWaiting()
{
  //Validate our parameters
  ATLASSERT(IsOpen());

  //Check to see how many characters are unread
  COMSTAT stat;
  GetStatus(stat);
  return stat.cbInQue;
}

void CSerialPort::TransmitChar(_In_ char cChar)
{
  //Validate our parameters
  ATLASSERT(IsOpen());

  if (!TransmitCommChar(m_hComm, cChar))
  {
    DWORD dwLastError = GetLastError();
    ATLTRACE(_T("CSerialPort::TransmitChar, Failed in call to TransmitCommChar, Error:%u\n"), dwLastError);
    ThrowSerialException(dwLastError);
  }
}

void CSerialPort::GetConfig(_Out_ COMMCONFIG& config)
{
  //Validate our parameters
  ATLASSERT(IsOpen());

  DWORD dwSize = sizeof(COMMCONFIG);
  if (!GetCommConfig(m_hComm, &config, &dwSize))
  {
    DWORD dwLastError = GetLastError();
    ATLTRACE(_T("CSerialPort::GetConfig, Failed in call to GetCommConfig, Error:%u\n"), dwLastError);
    ThrowSerialException(dwLastError);
  }
}

void CSerialPort::SetConfig(_In_ COMMCONFIG& config)
{
  //Validate our parameters
  ATLASSERT(IsOpen());

  DWORD dwSize = sizeof(COMMCONFIG);
  if (!SetCommConfig(m_hComm, &config, dwSize))
  {
    DWORD dwLastError = GetLastError();
    ATLTRACE(_T("CSerialPort::SetConfig, Failed in call to SetCommConfig, Error:%u\n"), dwLastError);
    ThrowSerialException(dwLastError);
  }
}

void CSerialPort::SetBreak()
{
  //Validate our parameters
  ATLASSERT(IsOpen());

  if (!SetCommBreak(m_hComm))
  {
    DWORD dwLastError = GetLastError();
    ATLTRACE(_T("CSerialPort::SetBreak, Failed in call to SetCommBreak, Error:%u\n"), dwLastError);
    ThrowSerialException(dwLastError);
  }
}

void CSerialPort::ClearBreak()
{
  //Validate our parameters
  ATLASSERT(IsOpen());

  if (!ClearCommBreak(m_hComm))
  {
    DWORD dwLastError = GetLastError();
    ATLTRACE(_T("CSerialPort::ClearBreak, Failed in call to SetCommBreak, Error:%u\n"), dwLastError);
    ThrowSerialException(dwLastError);
  }
}

void CSerialPort::ClearError(_Out_ DWORD& dwErrors)
{
  //Validate our parameters
  ATLASSERT(IsOpen());

  if (!ClearCommError(m_hComm, &dwErrors, NULL))
  {
    DWORD dwLastError = GetLastError();
    ATLTRACE(_T("CSerialPort::ClearError, Failed in call to ClearCommError, Error:%u\n"), dwLastError);
    ThrowSerialException(dwLastError);
  }
}

void CSerialPort::GetDefaultConfig(_In_ int nPort, _Out_ COMMCONFIG& config)
{
  //Create the device name as a string
  TCHAR szPort[12];
  _stprintf_s(szPort, sizeof(szPort)/sizeof(TCHAR), _T("COM%d"), nPort);

  return GetDefaultConfig(szPort, config);
}

void CSerialPort::GetDefaultConfig(_In_z_ LPCTSTR pszPort, _Out_ COMMCONFIG& config)
{
  DWORD dwSize = sizeof(COMMCONFIG);
  if (!GetDefaultCommConfig(pszPort, &config, &dwSize))
  {
    DWORD dwLastError = GetLastError();
    ATLTRACE(_T("CSerialPort::GetDefaultConfig, Failed in call to GetDefaultCommConfig, Error:%u\n"), dwLastError);
    ThrowSerialException(dwLastError);
  }
}

void CSerialPort::SetDefaultConfig(_In_ int nPort, _In_ COMMCONFIG& config)
{
  //Create the device name as a string
  TCHAR szPort[12];
  _stprintf_s(szPort, sizeof(szPort)/sizeof(TCHAR), _T("COM%d"), nPort);

  return SetDefaultConfig(szPort, config);
}

void CSerialPort::SetDefaultConfig(_In_z_ LPCTSTR pszPort, _In_ COMMCONFIG& config)
{
  DWORD dwSize = sizeof(COMMCONFIG);
  if (!SetDefaultCommConfig(pszPort, &config, dwSize))
  {
    DWORD dwLastError = GetLastError();
    ATLTRACE(_T("CSerialPort::SetDefaultConfig, Failed in call to SetDefaultCommConfig, Error:%u\n"), dwLastError);
    ThrowSerialException(dwLastError);
  }
}

void CSerialPort::GetStatus(_Out_ COMSTAT& stat)
{
  //Validate our parameters
  ATLASSERT(IsOpen());

  DWORD dwErrors;
  if (!ClearCommError(m_hComm, &dwErrors, &stat))
  {
    DWORD dwLastError = GetLastError();
    ATLTRACE(_T("CSerialPort::GetStatus, Failed in call to ClearCommError, Error:%u\n"), dwLastError);
    ThrowSerialException(dwLastError);
  }
}

void CSerialPort::GetState(_Out_ DCB& dcb)
{
  //Validate our parameters
  ATLASSERT(IsOpen());

  if (!GetCommState(m_hComm, &dcb))
  {
    DWORD dwLastError = GetLastError();
    ATLTRACE(_T("CSerialPort::GetState, Failed in call to GetCommState, Error:%u\n"), dwLastError);
    ThrowSerialException(dwLastError);
  }
}

void CSerialPort::SetState(_In_ DCB& dcb)
{
  //Validate our parameters
  ATLASSERT(IsOpen());

  if (!SetCommState(m_hComm, &dcb))
  {
    DWORD dwLastError = GetLastError();
    ATLTRACE(_T("CSerialPort::SetState, Failed in call to SetCommState, Error:%u\n"), dwLastError);
    ThrowSerialException(dwLastError);
  }
}

void CSerialPort::Escape(_In_ DWORD dwFunc)
{
  //Validate our parameters
  ATLASSERT(IsOpen());

  if (!EscapeCommFunction(m_hComm, dwFunc))
  {
    DWORD dwLastError = GetLastError();
    ATLTRACE(_T("CSerialPort::Escape, Failed in call to EscapeCommFunction, Error:%u\n"), dwLastError);
    ThrowSerialException(dwLastError);
  }
}

void CSerialPort::ClearDTR()
{
  Escape(CLRDTR);
}

void CSerialPort::ClearRTS()
{
  Escape(CLRRTS);
}

void CSerialPort::SetDTR()
{
  Escape(SETDTR);
}

void CSerialPort::SetRTS()
{
  Escape(SETRTS);
}

void CSerialPort::SetXOFF()
{
  Escape(SETXOFF);
}

void CSerialPort::SetXON()
{
  Escape(SETXON);
}

void CSerialPort::GetProperties(_Inout_ COMMPROP& properties)
{
  //Validate our parameters
  ATLASSERT(IsOpen());

  if (!GetCommProperties(m_hComm, &properties))
  {
    DWORD dwLastError = GetLastError();
    ATLTRACE(_T("CSerialPort::GetProperties, Failed in call to GetCommProperties, Error:%u\n"), dwLastError);
    ThrowSerialException(dwLastError);
  }
}

void CSerialPort::GetModemStatus(_Out_ DWORD& dwModemStatus)
{
  //Validate our parameters
  ATLASSERT(IsOpen());

  if (!GetCommModemStatus(m_hComm, &dwModemStatus))
  {
    DWORD dwLastError = GetLastError();
    ATLTRACE(_T("CSerialPort::GetModemStatus, Failed in call to GetCommModemStatus, Error:%u\n"), dwLastError);
    ThrowSerialException(dwLastError);
  }
}

void CSerialPort::SetMask(_In_ DWORD dwMask)
{
  //Validate our parameters
  ATLASSERT(IsOpen());

  if (!SetCommMask(m_hComm, dwMask))
  {
    DWORD dwLastError = GetLastError();
    ATLTRACE(_T("CSerialPort::SetMask, Failed in call to SetCommMask, Error:%u\n"), dwLastError);
    ThrowSerialException(dwLastError);
  }
}

void CSerialPort::GetMask(_Out_ DWORD& dwMask)
{
  //Validate our parameters
  ATLASSERT(IsOpen());

  if (!GetCommMask(m_hComm, &dwMask))
  {
    DWORD dwLastError = GetLastError();
    ATLTRACE(_T("CSerialPort::GetMask, Failed in call to GetCommMask, Error:%u\n"), dwLastError);
    ThrowSerialException(dwLastError);
  }
}

void CSerialPort::Flush()
{
  //Validate our parameters
  ATLASSERT(IsOpen());

  if (!FlushFileBuffers(m_hComm))
  {
    DWORD dwLastError = GetLastError();
    ATLTRACE(_T("CSerialPort::Flush, Failed in call to FlushFileBuffers, Error:%u\n"), dwLastError);
    ThrowSerialException(dwLastError);
  }
}

void CSerialPort::Purge(_In_ DWORD dwFlags)
{
  //Validate our parameters
  ATLASSERT(IsOpen());

  if (!PurgeComm(m_hComm, dwFlags))
  {
    DWORD dwLastError = GetLastError();
    ATLTRACE(_T("CSerialPort::Purge, Failed in call to PurgeComm, Error:%u\n"), dwLastError);
    ThrowSerialException(dwLastError);
  }
}

void CSerialPort::TerminateOutstandingWrites()
{
  Purge(PURGE_TXABORT);
}

void CSerialPort::TerminateOutstandingReads()
{
  Purge(PURGE_RXABORT);
}

void CSerialPort::ClearWriteBuffer()
{
  Purge(PURGE_TXCLEAR);
}

void CSerialPort::ClearReadBuffer()
{
  Purge(PURGE_RXCLEAR);
}

void CSerialPort::Setup(_In_ DWORD dwInQueue, _In_ DWORD dwOutQueue)
{
  //Validate our parameters
  ATLASSERT(IsOpen());

  if (!SetupComm(m_hComm, dwInQueue, dwOutQueue))
  {
    DWORD dwLastError = GetLastError();
    ATLTRACE(_T("CSerialPort::Setup, Failed in call to SetupComm, Error:%u\n"), dwLastError);
    ThrowSerialException(dwLastError);
  }
}

void CSerialPort::SetTimeouts(_In_ COMMTIMEOUTS& timeouts)
{
  //Validate our parameters
  ATLASSERT(IsOpen());

  if (!SetCommTimeouts(m_hComm, &timeouts))
  {
    DWORD dwLastError = GetLastError();
    ATLTRACE(_T("CSerialPort::SetTimeouts, Failed in call to SetCommTimeouts, Error:%u\n"), dwLastError);
    ThrowSerialException(dwLastError);
  }
}

void CSerialPort::GetTimeouts(_Out_ COMMTIMEOUTS& timeouts)
{
  //Validate our parameters
  ATLASSERT(IsOpen());

  if (!GetCommTimeouts(m_hComm, &timeouts))
  {
    DWORD dwLastError = GetLastError();
    ATLTRACE(_T("CSerialPort::GetTimeouts, Failed in call to GetCommTimeouts, Error:%u\n"), dwLastError);
    ThrowSerialException(dwLastError);
  }
}

void CSerialPort::Set0Timeout()
{
  COMMTIMEOUTS Timeouts;
  memset(&Timeouts, 0, sizeof(Timeouts));
  Timeouts.ReadIntervalTimeout = MAXDWORD;
  SetTimeouts(Timeouts);
}

void CSerialPort::Set0WriteTimeout()
{
  COMMTIMEOUTS Timeouts;
  GetTimeouts(Timeouts);
  Timeouts.WriteTotalTimeoutMultiplier = 0;
  Timeouts.WriteTotalTimeoutConstant = 0;
  SetTimeouts(Timeouts);
}

void CSerialPort::Set0ReadTimeout()
{
  COMMTIMEOUTS Timeouts;
  GetTimeouts(Timeouts);
  Timeouts.ReadIntervalTimeout = MAXDWORD;
  Timeouts.ReadTotalTimeoutMultiplier = 0;
  Timeouts.ReadTotalTimeoutConstant = 0;
  SetTimeouts(Timeouts);
}

void CSerialPort::WaitEvent(_Inout_ DWORD& dwMask)
{
  //Validate our parameters
  ATLASSERT(IsOpen());

  if (!WaitCommEvent(m_hComm, &dwMask, NULL))
  {
    DWORD dwLastError = GetLastError();
    ATLTRACE(_T("CSerialPort::WaitEvent, Failed in call to WaitCommEvent, Error:%u\n"), dwLastError);
    ThrowSerialException(dwLastError);
  }
}

BOOL CSerialPort::WaitEvent(_Inout_ DWORD& dwMask, _Inout_ OVERLAPPED& overlapped)
{
  //Validate our parameters
  ATLASSERT(IsOpen());
  ATLASSERT(overlapped.hEvent != NULL);

  BOOL bSuccess = WaitCommEvent(m_hComm, &dwMask, &overlapped);
  if (!bSuccess)
  {
    DWORD dwLastError = GetLastError();
    ATLTRACE(_T("CSerialPort::WaitEvent, Failed in call to WaitCommEvent, Error:%u\n"), dwLastError);
    ThrowSerialException(dwLastError);
  }

  return bSuccess;
}
