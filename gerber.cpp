#include "stdafx.h"
#include "command.h"
#include "gerber.h"

#include <Math.h>


/*
 * Attempt to parse the string in the szBuf and fill out the
 * command as needed
 */
bool GerberParser::ParseGerberCommand( char* szBuf, Command *cmd )
{
	char *p = szBuf;

	// While there are still characters to process, parse the command
	// block and update the command structure.
	while( '\0' != *p ) {
		// Determine the command type
		switch( *p ) {
			case 'A':	// Aperture definition
				return ParseGerberApertureDefinition( p, cmd );
				break;
			
			case 'D':	// D-Codes
				if( !ParseGerberDCommand( p, cmd ) )
					return false;
				break;

			case 'F':	// FS - Format Statement
				// the format statement is a full line command so
				// just return if it succeded.
				return ParseGerberFormatStatement( p, cmd );
				break;

			case 'S':	// S? command
				// the format statement is a full line command
				if( 0 == strncmp( p, "SF", 2 ) ) {
					while( '\0' != *p) p++;			//skip to the end of the command
					return true;
				} else {
					return false;
				}
				break;

			case 'I':	// I? command
				// the format statement is a full line command
				if( 0 == strncmp( p, "IP", 2 ) || 0 == strncmp( p, "IN", 2) ) {
					while( '\0' != *p) p++;			//skip to the end of the command
					return true;
				} else {
					return false;
				}
				break;

			case 'L':	// L? command
				// the format statement is a full line command
				if( ( 0 == strncmp( p, "LN", 2 ) ) || ( 0 == strncmp( p, "LP", 2 ) ) ) {
					while( '\0' != *p) p++;			//skip to the end of the command
					return true;
				} else {
					return false;
				}
				break;

			case 'G':	// G-Commands
				if( !ParseGerberGCommand( p, cmd ) )	// CL7777: Added
					return false;
				break;
			
			case 'M':	// Mode/Mirror Image/M-Code
				// the M* commands are full commands so just return
				return ParseGerberMCommand( p, cmd );
				break;
			case 'O':									// Offset command
				return ParseGerberOCommand( p, cmd );
				break;

			case 'X':	// set new X location
				if( ! ParseGerberXCommand( p, cmd ) )
					return false;
				break;

			case 'Y':	// set new Y location
				if( ! ParseGerberYCommand( p, cmd ) )
					return false;
				break;

			default:
				cmd->Type = cUnknownCommand;
				return false;
		}
	}

	return true;
}

/*
 * Read a number string from p into szBuf and null terminate
 * This stops copying when it gets to the next non-numeric
 * character. Note that p is left pointing to the last digit, not
 * the next command character.
 */
void GerberParser::ParseGerberNumber( char *&p, char *pszBuf )
{
	// we want numbers and the +/- characters.
	while( '\0' != *p && (*p == '-' || *p == '+' || (*p >= '0' && *p <= '9')) )
		*pszBuf++ = *p++;

	// null terminate
	*pszBuf = '\0';

	// back off p 
	p--;
}

/*
 * Gerber Command: FS<L|T><A|I>[Nn][Gn]<Xba><Yba>[Dn][Mn]
 *		[L = omit leading zeros, T = omit trailing zeros]
 *		A = Absolute Coordinates, I = Incremental Coordinates
 *		[Nn, Gn, Dn, Mm = specifiy the size for sequence specifiers. Not used]
 *		Xba = X Axis format specifier. b = # before decimal, a = # after decimal
 *		Yba = Y Axis format specifier. b = # before decimal, a = # after decimal
 *
 * This command has no "sub commands", we use the entire command block.
 */
bool GerberParser::ParseGerberFormatStatement( char *&p, Command *cmd )
{
//	char szBuf[50];

	// skip the "FS"
	p += 2;

	// go through the command and process it as we go
	while( '\0' != *p ) {
		switch( *p ) {
			case 'D':	// omit leading zeros.
				cmd->bTrailingZeroOmit = false;
				break;
			case 'L':	// omit leading zeros.
				cmd->bTrailingZeroOmit = false;
				break;

			case 'T':	// omit trailing zeros.
				cmd->bTrailingZeroOmit = true;
				break;

			case 'A':	// absolute position
				cmd->bAbsolutePosition = true;
				break;

			case 'I':	// incremental position
				cmd->bAbsolutePosition = false;
				break;

/*			case 'N':	// Ignore these
			case 'G':
			case 'D':
			case 'M':
				p++;
				ParseGerberNumber( p, szBuf );
				break;
*/			
			case 'X':	// X Axis format specifier
				// get the integer part (before decimal)
				p++;
				if( *p != '\0' && *p >= '0' && *p <= '9' ) {
					cmd->XFmtNumBeforeDP = *p - '0';
				} else {
					// Parse Error
					return false;
				}
				// get the decimal part (after decimal)
				p++;
				if( *p != '\0' && *p >= '0' && *p <= '9' ) {
					cmd->XFmtNumAfterDP = *p - '0';
				} else {
					// Parse Error
					return false;
				}
				break;

			case 'Y':	// Y Axis format specifier
				// get the integer part (before decimal)
				p++;
				if( *p != '\0' && *p >= '0' && *p <= '9' ) {
					cmd->YFmtNumBeforeDP = *p - '0';
				} else {
					// Parse Error
					return false;
				}
				// get the decimal part (after decimal)
				p++;
				if( *p != '\0' && *p >= '0' && *p <= '9' ) {
					cmd->YFmtNumAfterDP = *p - '0';
				} else {
					// Parse Error
					return false;
				}
				break;

			default:	// Parse Error.
				return false;
		}

		// Next character
		p++;
	}

	cmd->Type = cFormatSpecifier;
	return true;
}


/*
 * Gerber Command: G*			CL7777: Added whole section
 *
 *  G1  - General move command
 *  G4  - Comment
 * G54  - Select aperture
 */
bool GerberParser::ParseGerberGCommand( char *&p, Command *cmd )
{
	char		szBuf[50];
	int			GCode;

	cmd->Type = cNoOperation;
	// skip the G code
	p++;
	ParseGerberNumber( p, szBuf );
	if( strlen(szBuf) > 0 ) 
		{
		GCode = atoi(szBuf);
		
		// determine what command this is
		if( 1 == GCode ) 
			{
			// draw line
			cmd->Type = cNoOperation;
			p++;
			} 
		else if( 4 == GCode ) 
			{
			// Comment
			cmd->Type = cNoOperation;
			while( '\0' != *p) p++;		//skip to the end of the command
			} 
		else if( 54 == GCode ) 
			{
			// choose aperture
			cmd->Type = cNoOperation;
			p++;
			} 
		else {
			// Parse Error
			//cmd->Type = cUnknownCommand;
			//return false;
			// Ignore all other command blocks - in other words, treat them as if they were a G04
			cmd->Type = cNoOperation;
			while( '\0' != *p) p++;		//skip to the end of the command
			}
		} 
	else {
		// Parse Error
		cmd->Type = cUnknownCommand;
		return false;
		}
	return true;
}


/*
 * Gerber Command: M*
 *
 * This command has no "sub commands", we use the entire command block.
 */
bool GerberParser::ParseGerberMCommand( char *&p, Command *cmd )
{
	// skip the first M
	p++;

	// what type of command is it?
	switch( *p ) {
		case 'O':	// mode command
			p++;
			if( 0 == strncmp( p, "MM", 2 ) ) {
				// in millimeters
				cmd->Type = cModeCommand;
				cmd->bMetric = true;

				// absorb the two characters
				p += 2;
			} else if( 0 == strncmp( p, "IN", 2 ) ) {
				// in inches
				cmd->Type = cModeCommand;
				cmd->bMetric = false;

				// absorb the two characters
				p += 2;
			} else {
				// parse error
				cmd->Type = cUnknownCommand;
				return false;
			}
			break;
		
		//case 'I':	// Mirror Command
		//	break;

		// "M" Codes - double characters (leading 0)
		case '0':
			if((0 == strncmp( p, "00", 2 )) || (0 == strncmp( p, "0;", 2 ))) {
				cmd->Type = cProgramStop;
				p += 2;						// absorb the two characters
				return true;

			} else if( 0 == strncmp( p, "01", 2 ) ) {
				cmd->Type = cOptionalStop;
				p += 2;						// absorb the two characters
				return true;

			} else if( 0 == strncmp( p, "02", 2 ) ) {
				cmd->Type = cEndOfProgram;
				p += 2;						// absorb the two characters
				return true;

			} else {
				// parse error
				cmd->Type = cUnknownCommand;
				return false;
			}
			break;

		case '1':
			cmd->Type = cOptionalStop;
			p += 1;							// absorb the character
			return true;
			break;

		case '2':
			cmd->Type = cEndOfProgram;
			p += 1;							// absorb the character
			return true;
			break;

		default:	// Parse Error.
			cmd->Type = cUnknownCommand;
			return false;
	}

	return true;
}

/*
 * Gerber Command: O*
 *
 * This command has no "sub commands", we just ignore the command
 */
bool GerberParser::ParseGerberOCommand( char *&p, Command *cmd )
{
	// Just Skip the whole command
	cmd->Type = cNoOperation;
	while( '\0' != *p) p++;		//skip to the end of the command
	return true;
}


/*
 * Read an Aperture Parameter
 */
bool GerberParser::ParseGerberApertureParam( char *&p, char *pszBuf )
{
	if( '\0' == *p )
		return false;

	// we want all numbers and decimal points. 
	while( '\0' != *p && (*p == '.' || (*p >= '0' && *p <= '9')) )
		*pszBuf++ = *p++;

	// null terminate
	*pszBuf = '\0';

	// absorb the parameter seperator if that stopped us this time
	if( 'X' == *p )
		p++;

	return true;
}

/*
 * Aperture Definitions
 *	ADD<n><C|R|O><param>[X<param>]...
 */
bool GerberParser::ParseGerberApertureDefinition( char *&p, Command *cmd )
{
	char		szBuf[50];
	Aperture	*pAper = NULL;
	int			DCode;

	// make sure we're dealing with an aperture definition
	if( 0 == strncmp( p, "ADD", 3 ) ) {
		// skip the ADD and get the aperture number
		p += 3;
		ParseGerberNumber( p, szBuf );
		DCode = atoi( szBuf );

		// attempt to find the specified aperture
		pAper = FindApertureInList( DCode, cmd->pApertures );

		// if we didn't find it, create one and add it to the list
		if( NULL == pAper ) {
			pAper = new Aperture;
			pAper->pNext = cmd->pApertures;
			cmd->pApertures = pAper;
			pAper->DCode =  DCode;
		}

		// get the type of the aperature
		p++;

		switch( *p ) {
			case 'C':		// Circle
				pAper->Type = atCircle;
				pAper->bHasHole = false;
				pAper->bRoundHole = true;

				p += 2;		// skip past comma

				// read diameter
				if( !ParseGerberApertureParam( p, szBuf ) ) {
					// parse error
					cmd->Type = cUnknownCommand;
					return false;
				}
				pAper->XSize = (float)atof(szBuf);
				pAper->YSize = pAper->XSize;					// PT 2005 - ensure YSize is set
																// PT2005 - I know its a circle - but this helps
																// PT2005 - believe me !

				// look for hole X dimension
				if( ParseGerberApertureParam( p, szBuf ) ) {
					pAper->bHasHole = true;
					pAper->bRoundHole = true;
					pAper->HoleXSize = (float)atof(szBuf);
					pAper->HoleYSize = pAper->HoleXSize;		// PT2005 - force the Y size

					// look for hole Y dimension
					if( ParseGerberApertureParam( p, szBuf ) ) {
						pAper->HoleYSize = (float)atof(szBuf);
						if(pAper->HoleYSize != pAper->HoleXSize)	// PT2005 - modified
							pAper->bRoundHole = false;				// PT2005
					}	
				} 
				break;

			case 'R':		// Rectangle
				pAper->Type = atRectangle;
				pAper->bHasHole = false;
				pAper->bRoundHole = false;	// CL7777: was true;

				p += 2;		// skip past comma

				// read X Dimension
				if( !ParseGerberApertureParam( p, szBuf ) ) {
					// parse error
					cmd->Type = cUnknownCommand;
					return false;
				}
				pAper->XSize = (float)atof(szBuf);

				// read Y Dimension
				if( !ParseGerberApertureParam( p, szBuf ) ) {
					// parse error
					cmd->Type = cUnknownCommand;
					return false;
				}
				pAper->YSize = (float)atof(szBuf);

				// look for hole X dimension
				if( ParseGerberApertureParam( p, szBuf ) ) {
					pAper->bHasHole = true;
					pAper->bRoundHole = true;
					pAper->HoleXSize = (float)atof(szBuf);
					pAper->HoleYSize = pAper->HoleXSize;			// PT2005 - force the YSize

					// look for hole Y dimension
					if( ParseGerberApertureParam( p, szBuf ) ) {
						pAper->bRoundHole = false;
						pAper->HoleYSize = (float)atof(szBuf);
					}	
				} 
				break;
			
			case 'O':		// Oval
				pAper->Type = atOval;
				pAper->bHasHole = false;
				pAper->bRoundHole = true;

				p += 2;		// skip past comma

				// read X Dimension
				if( !ParseGerberApertureParam( p, szBuf ) ) {
					// parse error
					cmd->Type = cUnknownCommand;
					return false;
				}
				pAper->XSize = (float)atof(szBuf);

				// read Y Dimension
				if( !ParseGerberApertureParam( p, szBuf ) ) {
					// parse error
					cmd->Type = cUnknownCommand;
					return false;
				}
				pAper->YSize = (float)atof(szBuf);

				// look for hole X dimension
				if( ParseGerberApertureParam( p, szBuf ) ) {
					pAper->bHasHole = true;
					pAper->bRoundHole = true;
					pAper->HoleXSize = (float)atof(szBuf);
					pAper->HoleYSize = pAper->HoleXSize;			// PT2005 - force the YSize

					// look for hole Y dimension
					if( ParseGerberApertureParam( p, szBuf ) ) {
						pAper->bRoundHole = false;
						pAper->HoleYSize = (float)atof(szBuf);
					}	
				} 
				break;

			default:
				return false;
				break;
		}
			
	} else {
		cmd->Type = cNoOperation;
		while( '\0' != *p) p++;			//skip to the end of the command
		return true;
	}

	cmd->Type = cApertureDefinition;
	return true;
}

/*
 * Parse a D command
 *	D01 - exposure on (draw line)
 *	D02 - exposure off (move)
 *	D03 - Flash Aperture
 *	D10-D999 Select Aperture
 */
bool GerberParser::ParseGerberDCommand( char *&p, Command *cmd )
{
	char		szBuf[50];
	int			DCode;

	// skip the D and get the code
	p++;
	ParseGerberNumber( p, szBuf );
	if( strlen(szBuf) > 0 ) {
		DCode = atoi(szBuf);
		
		// determine what command this is
		if( 1 == DCode ) {
			// draw line
			cmd->Type = cDrawLine;
			p++;
		} else if( 2 == DCode ) {
			// move
			cmd->Type = cMove;
			p++;
		} else if( 3 == DCode ) {
			// flash aperture
			cmd->Type = cFlashAperture;
			p++;
		} else if( DCode >= 10 ) {
			// select the aperture
			cmd->Type = cSelectAperture;
			cmd->pCurrentAperture = FindApertureInList( DCode, cmd->pApertures );
			p++;

			if( NULL == cmd->pCurrentAperture ) {
				// unknown aperture.				
				cmd->Type = cUnknownCommand;
				return false;
			}
		} else {
			// Parse Error
			cmd->Type = cUnknownCommand;
			return false;
		}
	} else {
		// Parse Error
		cmd->Type = cUnknownCommand;
		return false;
	}

	return true;
}

/*
 * In an effort to save data when Photoplotters ran on punch tape, 
 * they compressed the dimension data by omitting either leading or
 * trailing zeros and the decimal point.
 *
 * So we have to reconstruct them from settings global to the file.
 */
float GerberParser::InterpretDimension( char *pszNumber, int nDigBefore, int nDigAfter, bool bOmitTrailingZeros )
{
	char	szBuf[50];
	char	*p = szBuf;
	double	fDim = 0.0;
	double	fSign = 1.0;

	// check for negative / positive sign
	if( *pszNumber == '-' ) {
		fSign = -1.0;
		pszNumber++;
	} else if( *pszNumber == '+' ) {
		pszNumber++;
	}

	// get the length of the string
	int nLen = strlen( pszNumber );
	int nPad = nLen < nDigBefore + nDigAfter;

	// do we add leading zeros?
	if( !bOmitTrailingZeros ) {
		while(nPad-- > 0)
			*p++ = '0';
	}
	
	// copy the numbers
	while( nLen-- > 0 )
		*p++ = *pszNumber++;

	// do we add trailing zeros?
	if( bOmitTrailingZeros ) {
		while(nPad-- > 0)
			*p++ = '0';
	}

	// null terminate
	*p = '\0';

	// convert the number to a double and divide to put the decimal in the right place
	fDim = atof(szBuf) * fSign;
	fDim = fDim / pow(10.0, (double)nDigAfter);

	return (float)fDim;
}

/*
 * Parse the X location setting command
 */
bool GerberParser::ParseGerberXCommand( char *&p, Command *cmd )
{
	char		szBuf[50];

	// skip the X and get the code
	p++;
	ParseGerberNumber( p, szBuf );
	p++;
	if( strlen(szBuf) > 0 ) {
		cmd->prevX = cmd->X; 
		cmd->Type = cNoOperation;
		cmd->X = InterpretDimension( szBuf, cmd->XFmtNumBeforeDP, cmd->XFmtNumAfterDP, cmd->bTrailingZeroOmit );
	} else {
		// Parse Error
		cmd->Type = cUnknownCommand;
		return false;
	}
	return true;
}


/*
 * Parse the Y location setting command
 */
bool GerberParser::ParseGerberYCommand( char *&p, Command *cmd )
{
	char		szBuf[50];

	// skip the Y and get the code
	p++;
	ParseGerberNumber( p, szBuf );
	p++;
	if( strlen(szBuf) > 0 ) {
		cmd->prevY = cmd->Y; 
		cmd->Type = cNoOperation;
		cmd->Y = InterpretDimension( szBuf, cmd->YFmtNumBeforeDP, cmd->YFmtNumAfterDP, cmd->bTrailingZeroOmit );
	} else {
		// Parse Error
		cmd->Type = cUnknownCommand;
		return false;
	}
	return true;
}


