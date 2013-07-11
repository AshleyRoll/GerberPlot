; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=CGerberPlotDlg
LastTemplate=CDialog
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "GerberPlot.h"

ClassCount=3
Class1=CGerberPlotApp
Class2=CGerberPlotDlg

ResourceCount=4
Resource2=IDD_GERBERPLOT_DIALOG
Resource1=IDR_MAINFRAME
Resource3=IDD_GERBERPLOT_DIALOG (English (U.S.))
Class3=CAboutBox
Resource4=IDD_ABOUT (English (U.S.))

[CLS:CGerberPlotApp]
Type=0
HeaderFile=GerberPlot.h
ImplementationFile=GerberPlot.cpp
Filter=N

[CLS:CGerberPlotDlg]
Type=0
HeaderFile=GerberPlotDlg.h
ImplementationFile=GerberPlotDlg.cpp
Filter=D
BaseClass=CDialog
VirtualFilter=dWC
LastObject=IDC_PORTNAME



[DLG:IDD_GERBERPLOT_DIALOG]
Type=1
ControlCount=3
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816
Control3=IDC_STATIC,static,1342308352
Class=CGerberPlotDlg

[DLG:IDD_GERBERPLOT_DIALOG (English (U.S.))]
Type=1
Class=CGerberPlotDlg
ControlCount=18
Control1=IDC_STATIC,static,1342308866
Control2=IDC_EDIT1,edit,1350631552
Control3=IDC_STATIC,static,1342308866
Control4=IDC_EDIT2,edit,1350631552
Control5=IDC_CONVERT,button,1342242816
Control6=IDC_STATIC,static,1342308866
Control7=IDC_PENSIZE,edit,1350639744
Control8=IDC_STATIC,static,1342308866
Control9=IDC_PENSPEED,edit,1350639744
Control10=IDC_BROWSEIN,button,1342242816
Control11=IDC_BROWSEOUT,button,1342242816
Control12=IDC_ABOUT,button,1342242816
Control13=IDC_OUTPUT_GERBER,button,1342242819
Control14=IDC_STATIC,button,1342177287
Control15=IDC_SEND,button,1342242816
Control16=IDC_STATIC,static,1342308866
Control17=IDC_SENDCHUNKS,button,1342242819
Control18=IDC_PORTNAME,combobox,1344348226

[DLG:IDD_ABOUT (English (U.S.))]
Type=1
Class=CAboutBox
ControlCount=2
Control1=IDOK,button,1342242817
Control2=IDC_DETAILS,edit,1353779204

[CLS:CAboutBox]
Type=0
HeaderFile=AboutBox.h
ImplementationFile=AboutBox.cpp
BaseClass=CDialog
Filter=D
VirtualFilter=dWC
LastObject=IDC_DETAILS

