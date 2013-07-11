/*************************************************************************
 * SerialPort.cpp
 *
 * Serial Port Handler object.
 *
 * This object manages the serial port for an application
 *
 * Author: Ashley Roll.		27 Sept, 2001.
 * Copyright 2001, Digital Nemesis Pty Ltd. All Rights Reserved.
 * www.digitalnemesis.com
 *************************************************************************/

#include "stdafx.h"
#include "SerialPort.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//------------------------------------------------------------------------
// Constants
//------------------------------------------------------------------------

// Define the max receive buffer size for each "read" operation.
const int CSerialPort::MAX_RX_BUFFER_LEN = 250;

//------------------------------------------------------------------------
// Construction/Destruction
//------------------------------------------------------------------------

CSerialPort::CSerialPort()
{
	m_hComm = INVALID_HANDLE_VALUE;
	m_hRxThread = NULL;
}

CSerialPort::~CSerialPort()
{

}


//------------------------------------------------------------------------
// Public Interface
//------------------------------------------------------------------------

/*
 * Open the Serial Port.
 *
 * tszPort is the name of the port. eg "COM1"
 * tszSettings is the settings for BuildCommDCB(). 
 * eg "baud=19200 parity=N data=8 stop=1"
 */
bool CSerialPort::OpenPort( LPCTSTR tszPort, LPCTSTR tszSettings )
{
	DCB				dcb;
	COMMTIMEOUTS	timeouts;
	bool			bSuccess;

	// fail by default
	bSuccess = false;

	// Attempt to open the port
	m_hComm = CreateFile( tszPort, 
						GENERIC_READ | GENERIC_WRITE,
						0,								// comm devices must be opened w/exclusive-access
						NULL,							// no security attributes
						OPEN_EXISTING,					// comm devices must use OPEN_EXISTING
						FILE_FLAG_OVERLAPPED,			// overlapped I/O
						NULL							// hTemplate must be NULL for comm devices
                     );

	if( INVALID_HANDLE_VALUE == m_hComm )
		return false;

	// Get the timeouts and state of the COM port so we can modify them
	if( GetCommState( m_hComm, &dcb ) && GetCommTimeouts( m_hComm, &timeouts ) ) {
		
		// Modify the state and timeouts based on the settings string
		if( BuildCommDCB( tszSettings, &dcb ) ) {
			
			// Adjust the timeouts here so that the receiver will work properly
			timeouts.ReadIntervalTimeout = 2;			// 2 ms between characters
			timeouts.ReadTotalTimeoutMultiplier = 0;
			timeouts.ReadTotalTimeoutConstant = 200;	// wait up to 200 ms for data
			timeouts.WriteTotalTimeoutConstant = 0;
			timeouts.WriteTotalTimeoutMultiplier = 0;

			// Set the new timeouts and DCB
			if( SetCommTimeouts( m_hComm, &timeouts ) && SetCommState( m_hComm, &dcb ) ) {
				// empty the buffers on the port
				PurgeComm( m_hComm, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR );

				// start the receiver thread
				if( StartReceiver() )
					bSuccess = true;		
			}
		}
	} 

	// Check if we succeded, if not then we have to be sure to close the
	// serial port so we can try again.
	if( ! bSuccess ) {
		// failed to get configuration, close the handle and exit
		ClosePort();
		return false;
	} 


	// we have succeded.
	return true;
}

/*
 * Close the serial port, stopping the receiver thread if needed
 */
void CSerialPort::ClosePort( void )
{
	// stop the receiver thread
	StopReceiver( 1000 );

	// if the serial port is open, close it.
	if( INVALID_HANDLE_VALUE != m_hComm ) {
		CloseHandle( m_hComm );
		m_hComm = INVALID_HANDLE_VALUE;
	}
}

/*
 * Send data out the serial port.
 *
 * We will timeout if the write doesn't complete in dwTimeout milliseconds time.
 * This is important the serial port may be configured with hardware flow control and
 * the device not attached or not responding. We don't want to halt the program if 
 * this is the case.
 */
bool CSerialPort::SendData( char* pData, int nLen, DWORD dwTimeout )
{
	_ASSERTE( NULL != m_hComm );

	OVERLAPPED	oWrite = {0};
	DWORD		dwWritten;
	DWORD		dwRes;
	bool		bRes;

   // Create this write operation's OVERLAPPED structure's hEvent.
   oWrite.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
   if (oWrite.hEvent == NULL)
      return false;

	// Issue write.
	if( !WriteFile(m_hComm, pData, nLen, &dwWritten, &oWrite) ) {
		// check if it failed or is pending 
		if ( GetLastError() != ERROR_IO_PENDING ) { 
			// WriteFile failed, but isn't delayed. Report error and abort.
			bRes = false;
		} else {
			// Write is pending.
			dwRes = WaitForSingleObject(oWrite.hEvent, dwTimeout);
			if( WAIT_OBJECT_0 == dwRes)	{
				// OVERLAPPED structure's event has been signaled. 
				if ( !GetOverlappedResult(m_hComm, &oWrite, &dwWritten, FALSE) )
					bRes = false;
				else
					// Write operation completed successfully.
					bRes = true;
			} else if( WAIT_TIMEOUT == dwRes ) {
				// our timeout expired, fail and abort our write operation
				CancelIo( m_hComm );
				bRes = false;
			} else {
				// An error has occurred in WaitForSingleObject.
				// This usually indicates a problem with the
				// OVERLAPPED structure's event handle.
                bRes = false;
			}
		}
	} else {
		// WriteFile completed immediately.
		bRes = true;
	}

   CloseHandle(oWrite.hEvent);
   return bRes;
}


void CSerialPort::OnReceive( char* pData, DWORD dwDataLen )
{
	// do nothing..
}


//------------------------------------------------------------------------
// Private Methods
//------------------------------------------------------------------------

/*
 * Start the thread that handles receive events and errors
 */
bool CSerialPort::StartReceiver( void )
{
	_ASSERTE( NULL == m_hRxThread );

	// ensure the stop flag is cleared
	m_bRxThreadStop = false;

	// create our worker thread
	m_hRxThread = CreateThread( NULL, 0, CSerialPort::ReceiverProc, (void*)this, 0, & m_dwThreadID );

	// did we suceed?
	return NULL != m_hRxThread;
}

/*
 * Stop the receiver thread. This method signals the receiver thread
 * to exist and wait up to dwTimeout milliseconds for it to exit.
 */
void CSerialPort::StopReceiver( DWORD dwTimeout )
{
	DWORD dwRet = 0;

	// ensure that there is a thread running
	if( NULL != m_hRxThread ) {
		// signal to the worker to stop
		m_bRxThreadStop = true;

		// wait for the thread to exit
		dwRet = WaitForSingleObject( m_hRxThread, dwTimeout );

		// mark the thread as deleted.
		m_hRxThread = NULL;
	} 
}

/*
 * The static procedure which is run by the receive thread to process
 * events. This is the Thread procedure. pData is a pointer to the
 * CSerialPort object that created the thread.
 */
DWORD WINAPI CSerialPort::ReceiverProc( void* pData )
{
	_ASSERTE( pData != NULL );

	char		buffer[MAX_RX_BUFFER_LEN];
	CSerialPort	*me = (CSerialPort*)pData;
	DWORD		dwBytesRead, dwRes;
	OVERLAPPED	oReader = {0};
	bool		bWaitingOnRead = false;

   // Create this read operation's OVERLAPPED structure's hEvent.
   oReader.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
   if (oReader.hEvent == NULL)
      return 1;

	// loop until we are asked to stop
	while( !me->m_bRxThreadStop ) {
		// if we aren't currently waiting on a read operation to complete,
		// we will issue one
		if( ! bWaitingOnRead ) {
			// Issue read operation.
			if( !ReadFile(me->m_hComm, buffer, MAX_RX_BUFFER_LEN, &dwBytesRead, &oReader)) {
				// is this an error or is the read pending
				if( GetLastError() != ERROR_IO_PENDING ) {    // read not delayed?
					// ERROR, EXIT
					return 2;
				} else {
					bWaitingOnRead = true;
				}
			} else {    
				// read completed immediately
				if( dwBytesRead > 0 )
					me->OnReceive( buffer, dwBytesRead );
			}
		} else {
			// a read is still pending so lets wait for it
			dwRes = WaitForSingleObject(oReader.hEvent, 500);
			if( WAIT_OBJECT_0 == dwRes ) {
				// Read completed.
				if( !GetOverlappedResult(me->m_hComm, &oReader, &dwBytesRead, FALSE)) {
					// ERROR, Exit
					return 3;
				} else {
					// Read completed successfully.
					if( dwBytesRead > 0 )
						me->OnReceive( buffer, dwBytesRead );
				}
				// Reset flag so that another opertion can be issued.
				bWaitingOnRead = false;
			}
		}
	}

	return 0;
}

