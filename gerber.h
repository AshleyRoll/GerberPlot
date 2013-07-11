#if !defined( _GERBER_H_INCLUDED_ )
#define _GERBER_H_INCLUDED_

struct GerberParser {

	static bool ParseGerberCommand( char* szBuf, Command *cmd );
	static void ParseGerberNumber( char *&p, char *pszBuf );
	static bool ParseGerberFormatStatement( char*& p, Command *cmd );
	static bool ParseGerberGCommand( char *&p, Command *cmd );
	static bool ParseGerberMCommand( char *&p, Command *cmd );
	static bool ParseGerberOCommand( char *&p, Command *cmd );
	static bool ParseGerberApertureDefinition( char *&p, Command *cmd );
	static bool ParseGerberApertureParam( char *&p, char *pszBuf );
	static bool ParseGerberDCommand( char *&p, Command *cmd );
	static bool ParseGerberXCommand( char *&p, Command *cmd );
	static bool ParseGerberYCommand( char *&p, Command *cmd );
	static float InterpretDimension( char *pszNumber, int nDigBefore, int nDigAfter, bool bOmitTrailingZeros );

};

#endif /* _GERBER_H_INCLUDED_ */