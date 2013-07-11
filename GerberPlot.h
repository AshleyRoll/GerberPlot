// GerberPlot.h : main header file for the GERBERPLOT application
//

#if !defined(AFX_GERBERPLOT_H__67E569CE_85A3_4A9F_8FCF_543A913040F8__INCLUDED_)
#define AFX_GERBERPLOT_H__67E569CE_85A3_4A9F_8FCF_543A913040F8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CGerberPlotApp:
// See GerberPlot.cpp for the implementation of this class
//

class CGerberPlotApp : public CWinApp
{
public:
	CGerberPlotApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGerberPlotApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CGerberPlotApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GERBERPLOT_H__67E569CE_85A3_4A9F_8FCF_543A913040F8__INCLUDED_)
