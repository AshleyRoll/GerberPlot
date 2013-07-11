/*************************************************************************
 * SerialPort.h
 *
 * Serial Port Handler Abstract Base Class.
 *
 * This object manages the serial port for an application. Derive from this
 * object and implement the OnReceive virtual method to process received data.
 *
 * Author: Ashley Roll.		27 Sept, 2001.
 * Copyright 2001, Digital Nemesis Pty Ltd. All Rights Reserved.
 * www.digitalnemesis.com
 *************************************************************************/

#if !defined(_SERIALPORT_H_INCLUDED_)
#define _SERIALPORT_H_INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class CSerialPort  
{
public:
	CSerialPort();
	virtual ~CSerialPort();

//------------------------------------------------------------------------
// Constants
//------------------------------------------------------------------------

	// Define the max receive buffer size for each "read" operation.
	static const int MAX_RX_BUFFER_LEN;


//------------------------------------------------------------------------
// Public Interface
//------------------------------------------------------------------------
public:
	/*
	 * Open the Serial Port and start the receiver thread.
	 *
	 * tszPort is the name of the port. eg "COM1"
	 * tszSettings is the settings for BuildCommDCB(). 
	 * eg "baud=19200 parity=N data=8 stop=1"
	 */
	bool	OpenPort( LPCTSTR tszPort, LPCTSTR tszSettings );

	/*
	 * Close the serial port, stopping the receiver thread if needed
	 */
	void	ClosePort( void );

	/*
	 * Send data out the serial port.
	 *
	 * We will timeout if the write doesn't complete in dwTimeout milliseconds time.
	 * This is important the serial port may be configured with hardware flow control and
	 * the device not attached or not responding. We don't want to halt the program if 
	 * this is the case.
	 */
	bool	SendData( char* pData, int nLen, DWORD dwTimeout = 500 );

	/*
	 * Virtual Call back for receiving data
	 *
	 * You must derive a class from this one and provide this function. 
	 * it is called by the receiver thread when data is received. Note that
	 * this function should do minimum work as it stops the receive process
	 * while it is running and so could result in missed data if the serial
	 * port buffer overflows.
	 */
	virtual void	OnReceive( char* pData, DWORD dwDataLen );

//------------------------------------------------------------------------
// Private Methods
//------------------------------------------------------------------------
private:
	// Start the receiver thread
	bool	StartReceiver( void );

	// Stop the receiver thread. This is called by ClosePort() for us
	void	StopReceiver( DWORD dwTimeout );

	// Thread procedure
	static DWORD WINAPI	ReceiverProc( void* pData );

//------------------------------------------------------------------------
// Private Variables
//------------------------------------------------------------------------
private:
	HANDLE	m_hComm;			// Handle to the serial port
	HANDLE	m_hRxThread;		// Handle to the receiver thread
	DWORD	m_dwThreadID;		// Thread ID of the reveiver thread
	bool	m_bRxThreadStop;	// Flag to stop the receiver thread
};

#endif // !defined(_SERIALPORT_H_INCLUDED_)
