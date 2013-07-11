// GerberPlotDlg.h : header file
//

#if !defined(AFX_GERBERPLOTDLG_H__5D55A5CE_F8BB_4396_8AD4_FAEA750DB847__INCLUDED_)
#define AFX_GERBERPLOTDLG_H__5D55A5CE_F8BB_4396_8AD4_FAEA750DB847__INCLUDED_

#include "command.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CGerberPlotDlg dialog

class CGerberPlotDlg : public CDialog
{
// Construction
public:
	CGerberPlotDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CGerberPlotDlg)
	enum { IDD = IDD_GERBERPLOT_DIALOG };
	CString	m_strInFile;
	CString	m_strOutFile;
	float	m_fPenSize;
	UINT	m_nPenSpeed;
	BOOL	m_bOutputGerber;
	BOOL	m_bForceOrigin;				// PT2005
	float	m_XOffset;					// PT2005
	float	m_YOffset;					// PT2005
	CString	m_strPortName;
	BOOL	m_bSendChunks;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGerberPlotDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	bool Convert( FILE *fpIn, FILE *fpOut, float fPenSize, unsigned int nPenSpeed );
	bool ReadCommand( FILE *fpIn, Command *cmd, FILE *fpOut );


	// Generated message map functions
	//{{AFX_MSG(CGerberPlotDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnConvert();
	afx_msg void OnSend();
	afx_msg void OnBrowsein();
	afx_msg void OnBrowseout();
	afx_msg void OnAbout();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GERBERPLOTDLG_H__5D55A5CE_F8BB_4396_8AD4_FAEA750DB847__INCLUDED_)
