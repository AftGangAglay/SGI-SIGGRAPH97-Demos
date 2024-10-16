<HTML><HEAD><TITLE>OpenGL - More Samples</TITLE></HEAD><BODY><CENTER>

<A HREF="../examples.html"><IMG SRC="../opengl.jpg"></A><BR>

<A HREF="./"><H1>more_samples/</H1></A>

<BR><IMG SRC="../divider.gif"><BR>

<BR>
<TABLE BORDER=3 WIDTH=520 CELLPADDING=6>
<TR>
 <TD><IMG SRC="chess.jpg"></TD>
 <TD VALIGN=TOP>
  <BR>
  Classic chessboard with (relatively) complicated pieces (pieces by Randy Brown).  Drag the middle mouse button to spin board.
  <BR>
  <BR> Source code: <A HREF="chess/">source directory</A>.<BR>
  <BR> Executable: <A HREF="chess.bat">chess.bat</A>.<BR>
  <BR> Snapshots: <A HREF="chess.jpg">chess (shown)</A>.<BR>
 </TD>
</TR>
</TABLE>

<BR>
<TABLE BORDER=3 WIDTH=520 CELLPADDING=6>
<TR>
 <TD><IMG SRC="csg.jpg"></TD>
 <TD VALIGN=TOP>
  <BR>
  Constructive Solid Geometry program based on information from the
  Advanced OpenGL Rendering course at SIGGRAPH '96 by David Blythe and
  Tom McReynolds.  Click right mouse button for menu.
  <BR>
  <BR> Source code: <A HREF="csg.c">csg.c</A>.<BR>
  <BR> Executable: <A HREF="csg.exe">csg.exe</A>.<BR>
  <BR> Snapshots: <A HREF="csg.jpg">Sphere sub Cube (shown)</A>, <A HREF="csg1.jpg">Cone sub Cube</A>.<BR>
 </TD>
</TR>
</TABLE>

<BR>
<TABLE BORDER=3 WIDTH=520 CELLPADDING=6>
<TR>
 <TD><IMG SRC="cube.jpg"></TD>
 <TD VALIGN=TOP>
  <BR>
  Very simple Win32 example of drawing a cube.
  <BR>
  <BR> Source code: <A HREF="cube.c">cube.c</A>.<BR>
  <BR> Executable: <A HREF="cube.exe">cube.exe</A>.<BR>
  <BR> Snapshots: <A HREF="cube.jpg">scene (shown)</A>.<BR>
 </TD>
</TR>
</TABLE>

<BR>
<TABLE BORDER=3 WIDTH=520 CELLPADDING=6>
<TR>
 <TD><IMG SRC="drip.jpg"></TD>
 <TD VALIGN=TOP>
  <BR>
  Neat little program that makes a drip wherever the mouse button is pressed.
  <BR>
  <BR> Source code: <A HREF="drip/">source directory</A>.<BR>
  <BR> Executable: <A HREF="drip/drip.exe">drip.exe</A>.<BR>
  <BR> Snapshots: <A HREF="drip.jpg">drops (shown)</A>.<BR>
 </TD>
</TR>
</TABLE>

<BR>
<TABLE BORDER=3 WIDTH=520 CELLPADDING=6>
<TR>
 <TD><IMG SRC="lorenz.jpg"></TD>
 <TD VALIGN=TOP>
  <BR>
 This program shows some particles stuck in a Lorenz attractor (the parameters
 used are r=28, b=8/3, sigma=10). The eye is attracted to the red particle,
 with a force directly proportionate to distance. A command line
 puts the whole mess inside a box made of hexagons. I think this helps to
 maintain the illusion of 3 dimensions, but it can slow things down.
 Other options allow you to play with the redraw rate and the number of new
 lines per redraw. So you can customize it to the speed of your machine.
  <BR>
  <BR> Source code: <A HREF="lorenz.c">lorenz.c</A>.<BR>
  <BR> Executable: <A HREF="lorenz.exe">lorenz.exe</A>.<BR>
  <BR> Snapshots: <A HREF="lorenz.jpg">start (shown)</A>.<BR>
 </TD>
</TR>
</TABLE>

<BR>
<TABLE BORDER=3 WIDTH=520 CELLPADDING=6>
<TR>
 <TD><IMG SRC="maze.jpg"></TD>
 <TD VALIGN=TOP>
  <BR>
 In this little demo the player navigates through a simple maze
 using the arrow keys.  The maze is defined by a 2D array where
 each element in the array indicates solid or empty space.  This
 program wraps polygon (quad) walls around the solid space and
 disallows the player to navigate into solid space during the demo.
 Note that all the walls are limited to being 90 degrees to each
 other - there are no "angled" features.  The purpose of this
 sample program is to show a beginning 3D game programmer some
 things they can do.

 One other cool thing that this program does is that it constucts
a single quad strip to draw all the walls by doing a recursive
 depth first search on the maze array data.
  <BR>
  <BR> Source code: <A HREF="maze.c">maze.c</A>.<BR>
  <BR> Executable: <A HREF="maze.exe">maze.exe</A>.<BR>
  <BR> Snapshots: <A HREF="maze.jpg">start (shown)</A>.<BR>
 </TD>
</TR>
</TABLE>

<BR>
<TABLE BORDER=3 WIDTH=520 CELLPADDING=6>
<TR>
 <TD><IMG SRC="meshview.jpg"></TD>
 <TD VALIGN=TOP>
  <BR>
  Simple program to visualize a mesh with a color ramp scale.
  Press 'h' for a help menu.  Specify a data file as a command 
  line argument.
  <BR>
  <BR> Source code: <A HREF="meshview/">source directory</A>.<BR>
  <BR> Executable: <A HREF="meshview.bat">meshview.bat</A>.<BR>
  <BR> Snapshots: <A HREF="meshview.jpg">sinc (shown)</A>.<BR>
 </TD>
</TR>
</TABLE>

<BR>
<TABLE BORDER=3 WIDTH=520 CELLPADDING=6>
<TR>
 <TD><IMG SRC="rainbow.jpg"></TD>
 <TD VALIGN=TOP>
  <BR>
  Demonstration of palette animation in color index mode.
  Press 'h' for a help menu, middle button spins.
  <BR>
  <BR> Source code: <A HREF="rainbow.c">rainbow.c</A>.<BR>
  <BR> Executable: <A HREF="rainbow.exe">rainbow.exe</A>.<BR>
  <BR> Snapshots: <A HREF="rainbow.jpg">rainbow (shown)</A>.<BR>
 </TD>
</TR>
</TABLE>

<BR>
<TABLE BORDER=3 WIDTH=520 CELLPADDING=6>
<TR>
 <TD><IMG SRC="sgiflag.jpg"></TD>
 <TD VALIGN=TOP>
  <BR>
  This program displays a waving flag with an SGI logo trimmed out of
  it.  The flag is a single nurbs surface (bicubic, bezier). It "waves" 
  by making it control point oscillate on a sine wave.
  The logo is cut from the flag using a combination of piecewise-linear 
  and bezier trim curves.
  <BR>
  <BR> Source code: <A HREF="sgiflag.c">sgiflag.c</A>.<BR>
  <BR> Executable: <A HREF="sgiflag.exe">sgiflag.exe</A>.<BR>
  <BR> Snapshots: <A HREF="sgiflag.jpg">sgiflag (shown)</A>.<BR>
 </TD>
</TR>
</TABLE>

<BR>
<TABLE BORDER=3 WIDTH=520 CELLPADDING=6>
<TR>
 <TD><IMG SRC="shadow.jpg"></TD>
 <TD VALIGN=TOP>
  <BR>
  Real-time soft shadows.  Press 'h' for a help menu.  Drag the
  middle mouse button to rotate the scene.
  <BR>
  <BR> Source code: <A HREF="shadow/">source directory</A>.<BR>
  <BR> Executable: <A HREF="shadow.bat">shadow.bat</A>.<BR>
  <BR> Snapshots: <A HREF="shadow.jpg">shadow (shown)</A>, <A HREF="shadow1.jpg">large w/multisample enabled (shown)</A>.<BR>
 </TD>
</TR>
</TABLE>

<BR>
<TABLE BORDER=3 WIDTH=520 CELLPADDING=6>
<TR>
 <TD><IMG SRC="signal.jpg"></TD>
 <TD VALIGN=TOP>
  <BR>
  Demonstration of picking and rendering luminous objects.  Drag the
  middle mouse button to spin the object.  Move the mouse over the
  bulbs to light them.
  <BR>
  <BR> Source code: <A HREF="signal.c">signal.c</A>.<BR>
  <BR> Executable: <A HREF="signal.exe">signal.exe</A>.<BR>
  <BR> Snapshots: <A HREF="signal.jpg">signal (shown)</A>.<BR>
 </TD>
</TR>
</TABLE>

<BR>
<TABLE BORDER=3 WIDTH=520 CELLPADDING=6>
<TR>
 <TD><IMG SRC="simplecap.jpg"></TD>
 <TD VALIGN=TOP>
  <BR>
  Simple demonstration of using OpenGL's stencil test to cap clipped objects.
  <BR>
  <BR> Source code: <A HREF="simplecap.c">simplecap.c</A>.<BR>
  <BR> Executable: <A HREF="simplecap.exe">simplecap.exe</A>.<BR>
  <BR> Snapshots: <A HREF="simplecap.jpg">capped (shown)</A>.<BR>
 </TD>
</TR>
</TABLE>

<BR>
<TABLE BORDER=3 WIDTH=520 CELLPADDING=6>
<TR>
 <TD><IMG SRC="simpleci.jpg"></TD>
 <TD VALIGN=TOP>
  <BR>
  Simple program that shows how to do multi-colored material lighting
  in color index mode.
  <BR>
  <BR> Source code: <A HREF="simpleci.c">simpleci.c</A>.<BR>
  <BR> Executable: <A HREF="simpleci.exe">simpleci.exe</A>.<BR>
  <BR> Snapshots: <A HREF="simpleci.jpg">torus (shown)</A>.<BR>
 </TD>
</TR>
</TABLE>

<BR>
<TABLE BORDER=3 WIDTH=520 CELLPADDING=6>
<TR>
 <TD><IMG SRC="texenv.jpg"></TD>
 <TD VALIGN=TOP>
  <BR>
 Demonstrates texture environment modes and internal image formats.
  <BR>
  <BR> Source code: <A HREF="texenv.c">texenv.c</A>.<BR>
  <BR> Executable: <A HREF="texenv.exe">texenv.exe</A>.<BR>
  <BR> Snapshots: <A HREF="texenv.jpg">scene (shown)</A>.<BR>
 </TD>
</TR>
</TABLE>

<BR>
<TABLE BORDER=3 WIDTH=520 CELLPADDING=6>
<TR>
 <TD><IMG SRC="vcull.jpg"></TD>
 <TD VALIGN=TOP>
  <BR>
  Simple program that demonstrates the vertex array extension.
  <BR>
  <BR> Source code: <A HREF="vcull.c">vcull.c</A>.<BR>
  <BR> Executable: <A HREF="vcull.exe">vcull.exe</A>.<BR>
  <BR> Snapshots: <A HREF="vcull.jpg">boxes (shown)</A>, <A HREF="vcull1.jpg">torus</A>.<BR>
 </TD>
</TR>
</TABLE>

<BR><IMG SRC="../divider.gif"><BR>

<H5>Copyright &copy; 1997 Silicon Graphics Incorporated.</H5>

</CENTER></BODY></HTML>
