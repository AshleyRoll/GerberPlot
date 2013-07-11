#include "stdafx.h"
#include "command.h"



float s_minCurX = -1000000;			// PT2005
float s_minCurY = -1000000;			// PT2005

/*
 * Initialise a command for a new file
 */
bool InitialiseCommand( Command *cmd )
{
	memset( cmd, 0, sizeof(Command) );
	cmd->bTrailingZeroOmit = false;
	cmd->bAbsolutePosition = true;
	cmd->bMetric = false;
	cmd->XFmtNumBeforeDP = 4;
	cmd->XFmtNumAfterDP = 2;
	cmd->YFmtNumBeforeDP = 4;
	cmd->YFmtNumAfterDP = 2;
	cmd->pApertures = NULL;
	cmd->pCurrentAperture = NULL;
	cmd->X = -1.0;
	cmd->Y = -1.0;
	cmd->prevX = -1.0;
	cmd->prevY = -1.0;
	return true;
}

/*
 * Destroy Command
 */
void DestroyCommand( Command *cmd )
{
	Aperture	*p, *t;

	// destroy the aperture list
	p = cmd->pApertures;
	while( NULL != p ) {
		t = p;
		p = p->pNext;
		delete t;
	}
}

/*
 * Attempt to locate a given aperture in the list.
 */
Aperture *FindApertureInList( int DCode, Aperture *pList )
{
	while( NULL != pList ) {
		if( pList->DCode == DCode )
			return pList;

		pList = pList->pNext;
	}

	return NULL;
}

/* PT2005 - Add this whole function
*
* Check to see if the current command requires the plotter position
* to move to a lower X and / or Y value than any previous command
* Note that the first time this routine is called, the variables 
* s_minCurX and s_minCurY will be -1000000 and the X and Y values from the 
* command will be copied to them
*/
static void CheckCommandXYPosition(Command *cmd)
{
if(s_minCurX == 1000000)
	{
	s_minCurX = cmd->X;
	s_minCurY = cmd->Y;
	}
else
	{
	if(1)
		{
		}
	}
}