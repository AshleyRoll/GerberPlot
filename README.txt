GerberPlot

This release (6) supports the following Gerber commands:

FS<L|T|D>AXabYcd	(a+b)<= 6, (c+d)<=6 
MO<IN|MM>
ADDnn<C|R|O>,eXfXgXh	10<=nn<=99,
			e=Dim1 (Diameter if Circle, Width if Rectangle|Oval),
			f=Dim2 (Height if Rectangle|Oval)
			g=Dim3 (Hole Diameter if round hole, Hole Width if rectangle hole)
			h=Dim4 (Hole Height if rectangle hole)
G1 or G01
G4 or G04
G54
D1 or D01
D2 or D02
D3 or D03
X......			...... in the format specified by ab
Y......			...... in the format specified by cd
Dnn
M0 or M00
M1 or M01
M2 or M02


These commands are ignored
<empty block>
SF
AD<empty Dnn>
IN			- added in beta 6
IPPOS
LPD
LN
Gxx - other than thoseabove	- added in beta 6

Options / Inputs
==========
1. Output Gerber Commands as comments
   Echoes the input Gerber Commands to the output HPGL file

2. Force Gerber Origin to 0.0
   Some Gerber files may include an offset, causing the output to plot other than at the very edge of the board.
   This option removes this offset and forces the plot to be against the edge of the plot area (in fact the minimum
   X plotted is set to zero and the minimum Y plotted is set to zero). The code takes account of apertures and line
   widths, so the plot should accurately plot againt the 0,0 edges of the plotter's output area.

3. X Offset (PLU's) and Y Offset (PLU's)
   Allows you to specify an offset to be applied to the plot (in Plotter Units or PLU's). This allows you to adjust the exact position
   of the output, which is useful if you are attempting to align for plotting a double sided PCB. Positioning can be adjusted to better than
   1/1000th inch !

1 Plotter unit = 0.025mm (in other words 40 per mm). There are 1016 PLU's per inch.