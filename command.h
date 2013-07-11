#if !defined(_COMMAND_H_INCLUDED_)
#define _COMMAND_H_INCLUDED_

typedef enum {
	atCircle,
	atRectangle,
	atOval,
} eApertureType;

typedef struct Aperture {
	eApertureType	Type;
	int				DCode;

	float			XSize;	// basic size. If the type only has one dimension
	float			YSize;	// then it is in XSize - PT2005 - but YSize always copied from XSize

	bool			bHasHole;
	bool			bRoundHole;

	float			HoleXSize;
	float			HoleYSize;

	// Next aperture in the list
	struct Aperture		*pNext;
} Aperture;

typedef enum {
	cUnknownCommand,
	cNoOperation,			// used by state update commands.
	cFormatSpecifier,		// FS...
	cModeCommand,			// MO<MM|IN>
	cProgramStop,			// M00
	cOptionalStop,			// M01
	cEndOfProgram,			// M02
	cApertureDefinition,	// ADD...
	cDrawLine,				// D01
	cMove,					// D02
	cFlashAperture,			// D03
	cSelectAperture,		// D10-D999
} eCmdType;

/*
 * Define the structure to store a command
 */
typedef struct {
	eCmdType	Type;				// The type of command
	float		X, Y;				// The position if any
	float		prevX, prevY;		// The previous position
	Aperture	*pCurrentAperture;	// The apeture to use

	// State information for the parser/converter
	// Dimension formats
	bool		bMetric;			// Are we using millimeters or inches?
	bool		bTrailingZeroOmit;	// Are leading or trailing zeros omitted?
	bool		bAbsolutePosition;	// Are we using absolute or relative position?
	int			XFmtNumBeforeDP;	// X number format
	int			XFmtNumAfterDP;
	int			YFmtNumBeforeDP;	// Y number format
	int			YFmtNumAfterDP;

	// apertures
	Aperture	*pApertures;

} Command;

bool InitialiseCommand( Command *cmd );
void DestroyCommand( Command *cmd );
Aperture *FindApertureInList( int DCode, Aperture *pList );
void CheckCommandXYPosition(Command *cmd);

#endif /* _COMMAND_H_INCLUDED_ */