#include "stdafx.h"
#include "command.h"
#include "hpgl.h"

#include <Math.h>
#include <stdio.h>


/*
 * Declare our static variables
 */
float HPGLOutput::s_fCurX = 0.0f;
float HPGLOutput::s_fCurY = 0.0f;
float HPGLOutput::s_fPenWidth = 0.2f;
bool HPGLOutput::s_bOutputGerber = false;


/*
 * Convert a size in millimetres to plotter units
 */
static long ConvertMMToPlotterUnits( float fVal )
{
	return (long)(fVal * 40.0f);
}

/*
 * Convert the given position in command units to plotter units 
 * A plotter unit is 0.025mm. This does not look at relative or 
 * absolute position, the caller much work that out.
 */
static long ConvertToPlotterUnits( float fPos, Command *cmd )
{
	// Convert to metric if needed
	if( !cmd->bMetric )
		fPos *= 25.4f;

	// Convert to plotter units. one PU = 0.025mm = 40 PU per mm
	return ConvertMMToPlotterUnits(fPos);
}


/*
 * Send any initialisation data to the plotter file.
 *
 * We will be operating in relative positioning so that 
 * we can position a "start point".
 */
bool HPGLOutput::InitialiseOutput( FILE *fp, float fPenWidthMM, unsigned int nPenSpeed, bool bOutputGerberCommands )
{
	s_fCurX = 0.0f;
	s_fCurY = 0.0f;
	s_bOutputGerber = bOutputGerberCommands;
//	s_bOriginSet = false;
	s_fPenWidth = fPenWidthMM;

	fprintf( fp, ";IN;SP1;VS%u;PU;PA0,0;\n", nPenSpeed );
	return true;
}

/*
 * Send any finialisation data to the plotter file.
 *
 */
bool HPGLOutput::FinaliseOutput( FILE *fp )
{
	fprintf( fp, "PU;SP0;\n" );
	return true;
}

/*
 * Write out the given command to the HPGL output file
 */
bool HPGLOutput::WriteCommand( FILE *fp, Command *cmd )
{
	// Movement is absolute, all plotting is done relative to the 
	// current position estabilished by the previous movement.


	// we are not supporting relative mode
	if(! cmd->bAbsolutePosition )
		return false;

	// only do something about the interesting commands.
	switch( cmd->Type ) {
		case cMove:
			if( !WriteMoveCommand( fp, cmd ) )
				return false;
			break;

		case cDrawLine:
			if( !WriteDrawLineCommand( fp, cmd ) )
				return false;
			break;

		case cFlashAperture:
			if( !WriteFlashApertureCommand( fp, cmd ) )
				return false;
			break;
	}


	return true;

}


/*
 * Write out a move command
 */
bool HPGLOutput::WriteMoveCommand( FILE *fp, Command *cmd )
{
	long x, y;

	x = ConvertToPlotterUnits( cmd->X, cmd );
	y = ConvertToPlotterUnits( cmd->Y, cmd );

	// update our position
	s_fCurX = cmd->X;
	s_fCurY = cmd->Y;

	// send the command
	fprintf( fp, "PU;PA%ld,%ld;PR0,0;\n", x, y );

	return true;
}

/*
 * Write out a draw line command
 */
bool HPGLOutput::WriteDrawLineCommand( FILE *fp, Command *cmd )
{
	long x, y;
	long PenWidth;
	long ApertureWidth;
	long dX, dY, numSteps, i;
	long startx, starty;
	long accX, accY;


	// absolute, we need to convert to relative by using the previous 
	// location data in the command.
	x = ConvertToPlotterUnits( cmd->X - s_fCurX, cmd );
	y = ConvertToPlotterUnits( cmd->Y - s_fCurY, cmd );

	// we need to accumulate our movements so we can return to 
	// our starting point without loosing position. Note that we
	// are accumulating relative to our destination so we can move to
	// it when we finish
	accX = -x;
	accY = -y;

	// determine if the width of the aperture is less then the pen width
	PenWidth = ConvertMMToPlotterUnits( s_fPenWidth );
	if( NULL != cmd->pCurrentAperture )
		ApertureWidth = ConvertToPlotterUnits( cmd->pCurrentAperture->XSize, cmd );
	else
		ApertureWidth = 0; // default to smaller then the pen

	if( ApertureWidth < PenWidth * 2 ) {
		// the pen is larger then the aperture, we will
		// just draw a line and hope the user knows what they are doing
		fprintf( fp, "PD%ld,%ld;\n", x, y );
	} else {
		// we need to fill the line to make it as wide as the aperture

		// first fill a circle at the start if the aperture size is more then
		// 4 times the pen size
		if( ApertureWidth > PenWidth *4 )
			OutputFilledCircle( fp, ApertureWidth/2, 0 );
		
		// are we vertical
		if( x == 0 ) {
			// calculate the number of steps requried and the starting position
			// and the step deltas
			dX = PenWidth - (PenWidth / 4);
			numSteps = ApertureWidth / dX;
			dY = 0;
			starty = 0;
			startx = - (numSteps * dX)/2;
			// If the number of strokes is even we want to offset by a further 1/2 pen width
			if( numSteps % 2 == 0 ) {
				startx += PenWidth / 2;
			}
		} else {
			// horizontal or diagnal
			dY = PenWidth - (PenWidth / 4);
			numSteps = ApertureWidth / dY;
			dX = 0;
			startx = 0;
			starty = - (numSteps * dY)/2;
			// If the number of strokes is even we want to offset by a further 1/2 pen width
			if( numSteps % 2 == 0) {
				starty += PenWidth / 2;
			}
		}

		// Move to the starting point
		fprintf( fp, "PU%ld,%ld;", startx, starty );
		accX += startx;
		accY += starty;

		// start drawing
		for( i = 0; i < numSteps; i++ ) {
			// draw the line, odd loop numbers move in the
			// direction of -x&-y, even loop numbers move in the
			// x&y direction
			if( i % 2 == 0 ) {
				// even
				fprintf( fp, "PD%ld,%ld;", x, y );
				accX += x;
				accY += y;
			} else {
				// odd
				fprintf( fp, "PD%ld,%ld;", -x, -y );
				accX += -x;
				accY += -y;
			}

			// move to new start point
			fprintf( fp, "PU%ld,%ld;", dX, dY );
			accX += dX;
			accY += dY;
		}

		// reposition at the destination point
		fprintf( fp, "PU%ld,%ld;\n", -accX, -accY );


		// fill a circle at the destination end if the aperture size is more then
		// 4 times the pen size
		if( ApertureWidth > PenWidth * 4 )
			OutputFilledCircle( fp, ApertureWidth/2, 0 );
	}

	// Make sure we are really where we should be
	x = ConvertToPlotterUnits( cmd->X, cmd );
	y = ConvertToPlotterUnits( cmd->Y, cmd );

	// send the command
	fprintf( fp, "PU;PA%ld,%ld;PR0,0;\n", x, y );

	// update our position
	s_fCurX = cmd->X;
	s_fCurY = cmd->Y;

	return true;
}

/*
 * Write out a flash aperture command
 */
bool HPGLOutput::WriteFlashApertureCommand( FILE *fp, Command *cmd )
{
	long	x, y, hx, hy, radius;
	bool	bRet;

	// send the command
	if( NULL != cmd->pCurrentAperture ) {
		// determine the type of aperture mark to make
		switch( cmd->pCurrentAperture->Type ) {
			case atCircle:
				bRet = WriteMoveCommand( fp, cmd );			// CL7777: make certain the plotter knows where we are

				// we will only do circular cutouts so we will use the largest hole dimension
				// in a circle
				x = ConvertToPlotterUnits( cmd->pCurrentAperture->HoleXSize, cmd );
				y = ConvertToPlotterUnits( cmd->pCurrentAperture->HoleYSize, cmd );

				if( cmd->pCurrentAperture->bHasHole ) {
					// is it rectangular, we use the max on x&y,
					// otherwise just use the x dimension
					if( !cmd->pCurrentAperture->bRoundHole ) {
						if( y > x )
							x = y;
					}
				} else {
					// no hole
					x = 0;
				}
				radius = ConvertToPlotterUnits( cmd->pCurrentAperture->XSize / 2.0f, cmd );
				bRet = OutputFilledCircle( fp, radius, x );
				break;

			case atRectangle:
				bRet = WriteMoveCommand( fp, cmd );			// CL7777: make certain the plotter knows where we are

				x = ConvertToPlotterUnits( cmd->pCurrentAperture->XSize, cmd );
				y = ConvertToPlotterUnits( cmd->pCurrentAperture->YSize, cmd );
				hx = ConvertToPlotterUnits( cmd->pCurrentAperture->HoleXSize, cmd );
				hy = ConvertToPlotterUnits( cmd->pCurrentAperture->HoleYSize, cmd );
				bRet = OutputFilledRectangle( fp, x, y, cmd->pCurrentAperture->bHasHole, cmd->pCurrentAperture->bRoundHole, hx, hy );
				break;

			case atOval:
				bRet = WriteMoveCommand( fp, cmd );			// CL7777: make certain the plotter knows where we are

				x = ConvertToPlotterUnits( cmd->pCurrentAperture->XSize, cmd );
				y = ConvertToPlotterUnits( cmd->pCurrentAperture->YSize, cmd );
				bRet = OutputFilledOval( fp, x, y );
				break;

			default:
				bRet = false;
				break;
		}


	}

	return bRet;
}

/*
 * Output a Circle with the origin at the current pen position. The Radius
 * specifies the outer size and pen width is taken into account so the edge 
 * of the circle is positioned accurately.
 */
bool HPGLOutput::OutputCircle( FILE *fp, long radius )
{
	long PenOffset, r;
	
	PenOffset = ConvertMMToPlotterUnits( s_fPenWidth / 2.0f );

	r = radius - PenOffset;


	// Make sure we aren't trying to do a negative radius
	if( r > 0 ) {
		// Move to a position on the circle along the X axis
		fprintf( fp, "PU%ld,0;", r );

		// Draw an arc with relative position of the origin specified
		fprintf( fp, "PD;AR-%ld,0,360.0;", r );

		// Move the pen back to the origin
		fprintf( fp, "PU;PR-%ld,0;\n", r );					// PT2005 Modified
	}
	return true;
}

/*
 * Output a Filled Circle with the origin at the current pen position. The Radius
 * specifies the outer size and pen width is taken into account so the edge 
 * of the circle is positioned accurately.
 *
 * If hole is > 0, a circular hole of that size is left open
 */
bool HPGLOutput::OutputFilledCircle( FILE *fp, long radius, long hole )
{
	long PenOffset, PenOverlap;
	

	hole /= 2;						// PT2005
	PenOffset = ConvertMMToPlotterUnits( s_fPenWidth / 2.0f );
	PenOverlap = ConvertMMToPlotterUnits( s_fPenWidth / 4.0f );

	// PT2005
	// Ensure we have a hole of exactly the size we asked for !
	// By Drawing a circle whose internal diameter is the
	// diameter of the hole.
	if(hole && (hole + PenOffset < radius))
		OutputCircle(fp, hole + PenOffset);

	// draw concentric circles from the outside in until we come within 
	// a pens offset of the centre
	// PT2005 - Fixed a bug whereby if the hole radius was
	//          less than circle radius - pen width, no circle
	//          was drawn - we do need at least to draw something
	//          when this occurs
	 while( radius > hole + PenOffset) {
		OutputCircle( fp, radius );
		// decrement the radius
		radius = radius - PenOffset + PenOverlap;
		};


	

	return true;
}

/*
 * Output a Rectangle with the centre at the current pen position. 
 * Width is the size in the X direction, Height is the size in the
 * Y direction. The width of the pen is taken into account so the 
 * edge of the rectange is positioned correctly.
 */
bool HPGLOutput::OutputRectangle( FILE *fp, long width, long height )
{
	long PenOffset;
	long startx, starty;
	
	PenOffset = ConvertMMToPlotterUnits( s_fPenWidth );

	// adjust for pen size
	width -= PenOffset;
	height -= PenOffset;


	// are we tring for a negative width or height?
	if( width > 0 && height > 0 ) {
		// calculate the box coordinates. Done so we don't loose
		// position with odd sizes.
		starty = -(height / 2);
		startx = -(width / 2);

		// Move to the bottom left position
		fprintf( fp, "PU%ld,%ld;", startx, starty );

		// Draw the box
		fprintf( fp, "PD0,%ld;", height );	// up
		fprintf( fp, "PD%ld,0;", width );	// right
		fprintf( fp, "PD0,-%ld;", height );	// down
		fprintf( fp, "PD-%ld,0;", width );	// left

		// restore the starting location 
		fprintf( fp, "PU%ld,%ld;\n", -startx, -starty );
	}

	return true;
}

/*
 * Output a Rectangle with the centre at the current pen position. 
 * Width is the size in the X direction, Height is the size in the
 * Y direction. The width of the pen is taken into account so the 
 * edge of the rectange is positioned correctly.
 *
 * Holes are not supported in rectangles 
 * PT2005 - oh yes they are :-) - well round ones anyway !
 * PT2005 - this routine changed significantly
 */
bool HPGLOutput::OutputFilledRectangle( FILE *fp, long width, long height, bool hashole, bool roundhole, long holewidth, long holeheight )
{
	long PenOffset, PenOverlap;
	long x, y;
	long maxheight, maxwidth;							// PT2005

	PenOffset = ConvertMMToPlotterUnits( s_fPenWidth );
	PenOverlap = ConvertMMToPlotterUnits( s_fPenWidth / 4.0f );

	x = width;
	y = height;

	// Always output a rectangle which is the requested size, regardless

	OutputRectangle(fp, x, y);							// PT2005
	x -= PenOffset;										// PT2005
	x += PenOverlap;									// PT2005
	y -= PenOffset;										// PT2005
	y -= PenOverlap;									// PT2005
	


	// PT2005
	// Firstly, if the rectangle has a round hole
	// draw it. Then draw a rectangle which has the 
	// same INSIDE dimensions as the hole,
	// then modify the rectangle drawing algorithm
	// so that it does not "fill in" the hole
	if(hashole)											// PT2005
		{
		if(roundhole)
			{
			OutputCircle(fp, (holewidth/2) + PenOffset);
			// PT2005, because of the way in which this function works, it is
			// POSSIBLE, where there is a relatively large hole in a rectangular pad
			// to have part of the rectangle outside the hole not
			// filled in correctly, so we will see if we can plot another
			// circle larger and just overlapping the outer edge of the one we have written,
			// without exceeding the maximum rectangle size. If we can then we will
			// do so in an attempt to fill in those areas which would otherwise
			// be missed
			if(holewidth + PenOffset * 4 < width && holewidth + PenOffset * 4 < height)
					OutputCircle(fp, holewidth/2 + PenOffset * 1.5);
			OutputRectangle(fp, holewidth + (PenOffset * 2), holewidth + (PenOffset * 2));
			}
		maxheight = holeheight + PenOffset * 2;
		maxwidth  = holewidth  + PenOffset * 2;
		}
	else
		{
		maxheight = 0;
		maxwidth = 0;
		}


	// PT2005
	// In order to support all rectangular pads (rather than just the special
	// case of a square, we need to plot 2 lots of concentric rectangles :
	// one set with a decreasing height and one set with a decreasing width
	// this will increase the plot time, but will produce better output
	
	// output the rectangles with decreasing height
	while (y > maxheight) 
		{														// PT2005
			OutputRectangle( fp, x, y );
			y -= PenOffset;
			y += PenOverlap;
		};
	
	y = height;													// PT2005 - Reset the height
	// output the rectangles with decreasing width
	while (x > maxwidth) 
		{														// PT2005
			OutputRectangle( fp, x, y );
			x -= PenOffset;
			x += PenOverlap;
		};

	return true;
}

/*
 * Output an Oval with the centre at the current pen position. 
 * Width is the size in the X direction, Height is the size in the
 * Y direction. The width of the pen is taken into account so the 
 * edge of the oval is positioned correctly.
 *
 * This actually draws a rectangle with the the smallest ends being
 * rounded.
 */
bool HPGLOutput::OutputOval( FILE *fp, long width, long height )
{
	long PenOffset;
	long startx, starty;
	long radius;
	
	PenOffset = ConvertMMToPlotterUnits( s_fPenWidth );
	
	// adjust for pen size
	width -= PenOffset;
	height -= PenOffset;

	// make sure that there is a height and width greater then 0
	if( width > 0 && height > 0 ) {
		// determine the orientation
		if( height < width ) {
			radius = height / 2;
			
			startx = -((width - height) / 2);
			starty = -radius;

			// Move to the bottom left position
			fprintf( fp, "PU%ld,%ld;", startx, starty );

			// Draw the left semicircle to the top
			fprintf( fp, "PD;AR0,%ld,-180.0;", radius );

			// Draw the top line
			fprintf( fp, "PD%ld,0;", width - height );

			// Draw the right semicircle to the bottom
			fprintf( fp, "AR0,-%ld,-180.0;", radius );

			// Draw the bottom line
			fprintf( fp, "PD-%ld,0;", width - height );

			// restore the starting location 
			fprintf( fp, "PU%ld,%ld;\n", -startx, -starty );
		} else {
			radius = width / 2;
			
			starty = -((height - width) / 2);
			startx = -radius;

			// Move to the bottom left position
			fprintf( fp, "PU%ld,%ld;", startx, starty );

			// Draw the vertical left line
			fprintf( fp, "PD0,%ld;", height - width );

			// Draw the top semicircle
			fprintf( fp, "AR%ld,0,-180.0;", radius );

			// Draw the vertical right line
			fprintf( fp, "PD0,-%ld;", height - width );

			// Draw the bottom semicircle
			fprintf( fp, "AR-%ld,0,-180.0;", radius );

			// restore the starting location 
			fprintf( fp, "PU%ld,%ld;\n", -startx, -starty );
		}
	}
	
	return true;
}

/*
 * Output a Filled Oval with the centre at the current pen position. 
 * Width is the size in the X direction, Height is the size in the
 * Y direction. The width of the pen is taken into account so the 
 * edge of the oval is positioned correctly.
 *
 * This actually draws a rectangle with the the smallest ends being
 * rounded.
 *
 * Holes are not supported in ovals
 */
bool HPGLOutput::OutputFilledOval( FILE *fp, long width, long height )
{
	long PenOffset, PenOverlap;
	long x, y;

	PenOffset = ConvertMMToPlotterUnits( s_fPenWidth );
	PenOverlap = ConvertMMToPlotterUnits( s_fPenWidth / 4.0f );

	x = width;
	y = height;

	
	// we will simply draw concentric ovals, but we monitor the smallest
	// access to determine when to stop
	if( height < width ) {
		// monitor the y axis
		while( y > PenOffset - PenOverlap ) {
			OutputOval( fp, x, y );
			x -= PenOffset + PenOverlap;
			y -= PenOffset + PenOverlap;
		}
	} else {
		// monitor the x axis
		while( x > PenOffset - PenOverlap ) {
			OutputOval( fp, x, y );
			x -= PenOffset + PenOverlap;
			y -= PenOffset + PenOverlap;
		}
	}

	return true;
}

/*
 * Simply return the pen width to the caller	PT2005
*/
float HPGLOutput::GetPenWidth(void)
{
	return HPGLOutput::s_fPenWidth;
}