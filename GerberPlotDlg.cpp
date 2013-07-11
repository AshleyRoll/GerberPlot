// GerberPlotDlg.cpp : implementation file
//

#include "stdafx.h"
#include "GerberPlot.h"
#include "GerberPlotDlg.h"
#include "AboutBox.h"

#include "gerber.h"
#include "hpgl.h"

#include "SerialPort.h"

#include <stdio.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGerberPlotDlg dialog

CGerberPlotDlg::CGerberPlotDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CGerberPlotDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CGerberPlotDlg)
	m_strInFile = _T("");
	m_strOutFile = _T("");
	m_fPenSize = 0.40f;
	m_nPenSpeed = 2;
	m_bOutputGerber = FALSE;
	m_bForceOrigin = FALSE;
	m_XOffset=0.0f;
	m_YOffset = 0.0f;
	m_strPortName = _T("LPT1:");
	m_bSendChunks = FALSE;
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CGerberPlotDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGerberPlotDlg)
	DDX_Text(pDX, IDC_EDIT1, m_strInFile);
	DDX_Text(pDX, IDC_EDIT2, m_strOutFile);
	DDX_Text(pDX, IDC_PENSIZE, m_fPenSize);
	DDX_Text(pDX, IDC_PENSPEED, m_nPenSpeed);
	DDX_Text(pDX, IDC_XOFFSET, m_XOffset);					// PT2005
	DDX_Text(pDX, IDC_YOFFSET, m_YOffset);					// PT2005
	DDX_Check(pDX, IDC_OUTPUT_GERBER, m_bOutputGerber);
	DDX_Check(pDX, IDC_FORCE_ORIGIN, m_bForceOrigin);		// PT2005
	DDX_Text(pDX, IDC_PORTNAME, m_strPortName);
	DDX_Check(pDX, IDC_SENDCHUNKS, m_bSendChunks);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CGerberPlotDlg, CDialog)
	//{{AFX_MSG_MAP(CGerberPlotDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_CONVERT, OnConvert)
	ON_BN_CLICKED(IDC_SEND, OnSend)
	ON_BN_CLICKED(IDC_BROWSEIN, OnBrowsein)
	ON_BN_CLICKED(IDC_BROWSEOUT, OnBrowseout)
	ON_BN_CLICKED(IDC_ABOUT, OnAbout)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CGerberPlotDlg message handlers

BOOL CGerberPlotDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CGerberPlotDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

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
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CGerberPlotDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CGerberPlotDlg::OnConvert() 
{
	FILE		*fpIn, *fpOut;
	CString		msg;
	
	// Update the settings
	UpdateData(TRUE);

	// Attempt to open the input file
	fpIn = fopen( m_strInFile, "r" );
	if( NULL == fpIn ) {
		MessageBox( _T("Failed to open the input file"), _T("File Error") );
		return;
	}

	// attempt to open the output file
	fpOut = fopen( m_strOutFile, "w" );
	if( NULL == fpOut ) {
		MessageBox( _T("Failed to open the output file"), _T("File Error") );
		fclose( fpIn );
		return;
	}

	if( Convert( fpIn, fpOut, m_fPenSize, m_nPenSpeed ) ) {
		MessageBox( _T("Successfully converted.\r\n") );
	} else {
		MessageBox( _T("Conversion failed.\r\n") );
	}

	// clean up
	fclose( fpIn );
	fclose( fpOut );

	// update the display
	UpdateData(FALSE);
}

void CGerberPlotDlg::OnSend() 
{
	CSerialPort sp;
	FILE		*fpIn;
	FILE		*fpOut;
	char		szBuf[250];
	int			byteCount;
	
	// Update the settings
	UpdateData(TRUE);

	// Attempt to open the HPGL file
	fpIn = fopen( m_strOutFile, "r" );
	if( NULL == fpIn ) {
		MessageBox( _T("Failed to open the input file"), _T("File Error") );
		return;
	}
	
	if(0 == strncmp( m_strPortName, "COM", 3 ) ) {
		// attempt to open the serial port
		if( !sp.OpenPort( m_strPortName, _T("baud=9600 parity=N data=8 stop=1") ) ) {
			MessageBox(_T("Failed to open the serial port."), _T("Communication Error") );
			fclose( fpIn );
			return;
		} 

		BeginWaitCursor();

		// read the input file one line at a time and send it to the printer.
		byteCount = 0;
		while( !feof( fpIn ) ) {
			fgets( szBuf, 250, fpIn );
			if( !sp.SendData( szBuf, strlen(szBuf), INFINITE ) ) {
				MessageBox(_T("Send Error."), _T("Communication Error") );
				break;
			}

			if( m_bSendChunks == TRUE ) {
				// For some reason the buffering sucks.. we will only send 4K chunks at a time
				// (the plotter has 5K). Then we will wait for the user to say it has finished that 
				// chunk
				byteCount += strlen(szBuf);
				if( byteCount > 4096 ) {
					if( IDCANCEL == MessageBox(_T("Press OK When this chunk of data as finished plotting"), _T("Wait for Buffer"), MB_OKCANCEL) )
						break;
					byteCount = 0;
				}
			}
		}

		EndWaitCursor();

		MessageBox( _T("Sent.") );

		// clean up
		sp.ClosePort();

	} else if(0 == strncmp( m_strPortName, "LPT", 3 ) ) {

		// attempt to open the output file
		fpOut = fopen( m_strPortName, "w" );
		if( NULL == fpOut ) {
			MessageBox( _T("Failed to open the parallel port"), _T("File Error") );
		}

		// read the input file one line at a time and send it to the printer.
		byteCount = 0;
		while( !feof( fpIn ) ) {
			fgets( szBuf, 250, fpIn );
			fputs( szBuf, fpOut );

			if( m_bSendChunks == TRUE ) {
				// For some reason the buffering sucks.. we will only send 4K chunks at a time
				// (the plotter has 5K). Then we will wait for the user to say it has finished that 
				// chunk
				byteCount += strlen(szBuf);
				if( byteCount > 4096 ) {
					if( IDCANCEL == MessageBox(_T("Press OK When this chunk of data as finished plotting"), _T("Wait for Buffer"), MB_OKCANCEL) )
						break;
					byteCount = 0;
				}
			}
		}
		fclose( fpOut );
	}
	fclose( fpIn );

	// update the display
	UpdateData(FALSE);
}


/*
 * Convert the file
 */
bool CGerberPlotDlg::Convert( FILE *fpIn, FILE *fpOut, float fPenSize, unsigned int nPenSpeed )	
{
	Command	cmd;
	bool	bRet = false;
	float	minX = 1000000;
	float	minY = 1000000;
	float	YSize;						// PT2005
	float	PenWidth;					// PT2005

	// initialise the command
	InitialiseCommand( &cmd );

	// now read through the input file looking for the minimum values of
	// the x&y positions, such that if it is plotted with an X and Y offset
	// of Zero in the dialog box, it will plot right up against the bottom
	// left hand edge of the plotters output area. Note that I need to take
	// account of any apetures which are plotted as they may cause the 
	// pen to go outside the plotting area. Also need to take account of the 
	// width of the pen !

	if(!m_bForceOrigin)								// PT2005
		{
		minX=0;
		minY=0;
		}
	else
		{
		while( ReadCommand( fpIn, &cmd, fpOut))		// PT2005
			{
			if(cmd.Type == cDrawLine  || cmd.Type == cMove || cmd.Type == cFlashAperture)
				{
				// Deal with only those commands which can affect the
				// Minimum X & Y position that is required to be plotted
				
				if(cmd.X < minX)
					minX = cmd.X;
				if(cmd.Y < minY)
					minY = cmd.Y;
				if( cmd.Type == cFlashAperture)
					{
					if( (cmd.X - (cmd.pCurrentAperture->XSize/2)) < minX)
						{
						minX = (cmd.X - (cmd.pCurrentAperture->XSize/2));
						}
					// If YSize is zero, then it is the same as the XSize,
					// So force the YSize used to the same as XSize
					if(!cmd.pCurrentAperture->YSize)
						YSize = cmd.pCurrentAperture->XSize;
					else
						YSize = cmd.pCurrentAperture->YSize;	

					if( (cmd.Y - (YSize/2)) < minY)
						{
						minY = (cmd.Y - (YSize/2));
						}
					}
				// PT2005
				// If the command was cDrawLine and the Aperture size in
				// the command block is bigger than the pen, we
				// need to make a further adjustment to the X or Y position
				// if the line is vertical or horizontal, to ensure it
				// plots against the edge of the board.
				if(cmd.Type == cDrawLine && (cmd.X == cmd.prevX || cmd.Y == cmd.prevY))
					{
					if(NULL != cmd.pCurrentAperture)			// if an aperture is supplied
						{
						PenWidth = HPGLOutput::GetPenWidth();
						if(!cmd.bMetric)						// If in inches
							PenWidth /= (float)25.4;					// Convert PenWidth
						if(cmd.pCurrentAperture->XSize > PenWidth)	// If line is wider than the pen
							{
							
							if(cmd.X == cmd.prevX && cmd.X == minX)					// Adjust X
								minX -= ((cmd.pCurrentAperture->XSize - PenWidth) / 2);
							if(cmd.Y == cmd.prevY && cmd.Y == minY)					// Adjust Y
								minY -= ((cmd.pCurrentAperture->YSize - PenWidth) / 2);
							}
						}
					}
				}
			cmd.prevX = cmd.X;
			cmd.prevY = cmd.Y;
			};										// PT2005

		// Rewind the input file to the beginning
	rewind(fpIn);							// PT2005

	DestroyCommand( &cmd);					// PT2005


	// initialise the command again			
	InitialiseCommand( &cmd );				// PT2005
	}

	// Take account of any user specified X and Y offset
	if(cmd.bMetric)								// PT2005
		{
		minX -= float(m_XOffset/40);
		minY -= float(m_YOffset/40);
		}
	else
		{
		minX -= float(m_XOffset / 1016);
		minY -= float (m_YOffset / 1016);
		}

	HPGLOutput::InitialiseOutput( fpOut, fPenSize, nPenSpeed, m_bOutputGerber );

	while( bRet = ReadCommand( fpIn, &cmd, fpOut ) ) {
		// Correct the offset to ensure board plots with an origin of 0,0

		if(cmd.Type == cDrawLine  || cmd.Type == cMove || cmd.Type == cFlashAperture)	// PT2005
			{
			if(cmd.X != cmd.prevX)
				cmd.X -= minX;						// PT2005
			if(cmd.Y != cmd.prevY)
				cmd.Y -= minY;						// PT2005
			}
		if( cmd.Type == cUnknownCommand || !HPGLOutput::WriteCommand( fpOut, &cmd ) )
			return false;
		cmd.prevX = cmd.X;							// PT2005
		cmd.prevY = cmd.Y;							// PT2005
		if( cmd.Type == cEndOfProgram || cmd.Type == cProgramStop || cmd.Type == cOptionalStop )
			break;
	};
	
	HPGLOutput::FinaliseOutput( fpOut );

	DestroyCommand( &cmd );
	return bRet;
}




/*
 * Read a single Block/Command, return true if successful,
 * false otherwise. Note that we rely on the SAME cmd
 * structure being passed back unchanged to support the
 * "modal" or sticky values
 */
bool CGerberPlotDlg::ReadCommand( FILE *fpIn, Command *cmd, FILE *fpOut)
{
	char	szBuf[250];
	int		i;
	int		c;
	

	// read up to a block seperator, ignorning the
	// parameter block delimeters
	i = 0;
	do {
		c = fgetc( fpIn );

		// ignore the param block specifiers
		if( '%' == c || '\r' == c || '\n' == c || ' ' == c )
			continue;

		if( '*' != c ) {
			szBuf[i++] = c;
		} else {
			break;
		}	
	} while( c != EOF );

	// null terminate the new command string
	szBuf[i] = '\0';

	if( c == EOF )
		return false;


	if( strlen(szBuf) > 0 ) {

	/* debug - echo the original Gerber command in OutFile	CL7777 - Added
	*/
		if( HPGLOutput::s_bOutputGerber ){
			fprintf( fpOut, "CO " );
			fputc( '"', fpOut );
			fprintf( fpOut, szBuf );
			fputc( '"', fpOut );
			fprintf( fpOut, ";\n" );
		}
		return GerberParser::ParseGerberCommand( szBuf, cmd );
	} else return true;		// in case of an empty line
}

void CGerberPlotDlg::OnBrowsein() 
{
	CFileDialog *d;
	
	// setup the dialogue
	d = new CFileDialog(TRUE, NULL, NULL, 
		OFN_ENABLESIZING | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST,
		"All Files (*.*)|*.*||"	);

	// do it..
	if( IDOK == d->DoModal() ) {
		// copy out the data
		m_strInFile = d->GetPathName();
		UpdateData(FALSE);
	}

	delete d;
}

void CGerberPlotDlg::OnBrowseout() 
{
	CFileDialog *d;
	
	// setup the dialogue
	d = new CFileDialog(FALSE, NULL, NULL, 
		OFN_ENABLESIZING | OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST,
		"All Files (*.*)|*.*|HPGL Files (*.hpg;*.hpgl)|*.hpg;*.hpgl||"	);

	// do it..
	if( IDOK == d->DoModal() ) {
		// copy out the data
		m_strOutFile = d->GetPathName();
		UpdateData(FALSE);
	}

	delete d;
}

void CGerberPlotDlg::OnAbout() 
{
	CAboutBox ab;

	ab.m_strAbout =  "BETA 6\r\n";
	ab.m_strAbout += "Gerber to HPGL converter by Ashley Roll.\r\n";
	ab.m_strAbout += "ash@digitalnemesis.com\r\n\r\n";

	ab.m_strAbout += "Additional programming by Peter Talbot.\r\n";

	ab.m_strAbout += "(C) Copyright 2002,2005, Digital Nemesis Pty Ltd. All Rights Reserved.\r\n";
	ab.m_strAbout += "Parts of this code are (C) Copyright Peter Talbot. All Rights Reserved.\r\n";
	ab.m_strAbout += "Free for non-commercial and private use.\r\n\r\n";

	ab.m_strAbout += "This is a rough and ready program. It works enough for me to play with\r\n";
	ab.m_strAbout += "but there are some minor problems. However it does a better job then\r\n";
	ab.m_strAbout += "using the standard plotter drivers with your CAD program.\r\n\r\n";

	ab.m_strAbout += " - Currently supports serial (9600,N,1,8) or parallel communication with the plotter.\r\n";
	ab.m_strAbout += " - From Beta 6 support is provided for holes in apertures.\r\n";
	ab.m_strAbout += " - Thick lines can get a little rough on the corners.\r\n\r\n";

	ab.m_strAbout += "Please read the READ_ME.TXT file for more info.\r\n\r\n";
	
	ab.m_strAbout += "I'm interested to hear how you go with it and if you have any problems.\r\n\r\n";
	ab.m_strAbout += "see http://www.digitalnemesis.com/ash/projects/GerberPlot for more info.\r\n";
	
	ab.m_strAbout += "\r\nAshley Roll. ash@digitalnemesis.com";

	ab.DoModal();
}
