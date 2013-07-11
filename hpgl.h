#if !defined( _HPGL_H_INCLUDED_ )
#define _HPGL_H_INCLUDED_

struct HPGLOutput {

	// MUST BE SET by the caller
	
	// Entry Points
	static bool InitialiseOutput( FILE *fp, float fPenWidthMM, unsigned int nPenSpeed, bool bOutputGerberCommands );
	static bool FinaliseOutput( FILE *fp );
	static bool WriteCommand( FILE *fp, Command *cmd );

	// Helper Functions
	static bool WriteMoveCommand( FILE *fp, Command *cmd );
	static bool WriteDrawLineCommand( FILE *fp, Command *cmd );
	static bool WriteFlashApertureCommand( FILE *fp, Command *cmd );

	static bool OutputCircle( FILE *fp, long radius );
	static bool OutputFilledCircle( FILE *fp, long radius, long hole );
	static bool OutputRectangle( FILE *fp, long width, long height );
	static bool OutputFilledRectangle( FILE *fp, long width, long height, bool hashole, bool roundhole, long holewidth, long holeheight );
	static bool OutputOval( FILE *fp, long width, long height );
	static bool OutputFilledOval( FILE *fp, long width, long height );
	static void UpdateExtentsRelative( float x, float y );
	static float GetPenWidth(void);						// PT2005

	// Variables
	static float	s_fCurX, s_fCurY;
	static float    s_minCurX, s_minCurY;
	static bool		s_bOutputGerber;
	static float	s_fPenWidth;

};

#endif /* _HPGL_H_INCLUDED_ */