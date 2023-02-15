# wxpolygon - Create and manipulate OpenSCAD polygons

wxpolygon was created to support making polygons for OpenSCAD extrusions.  It provides a XY two-dimensional space within 
which to plot points, move them around, and either save to a file or copy/paste into a OpenSCAD script.  It also optionally 
supports adding a radius component to each point for use by the Round-Anything OpenSCAD extension library, found at:

https://github.com/Irev-Dev/Round-Anything

## Building

wxpolygon uses the wxWidgets GUI library.  I have the autoconf configuration set up to use the wx-config script of a hand-compiled
wxWidgets instantiation, and the following instructions make use of it:

```
$ autoconf
$ mkdir build
$ cd build
$ ../configure --with-wx-config=/path/to/wx-config
$ make
$ sudo make install
```

The install target installs to /usr/local.  Note that it doesn't move wxpolygon.conf; you have to do that yourself.

## Usage

When you start the program, you're presented with a window that has X and Y axes, and a point at 0,0.  Also, as of version 1.1, there's a table on the right-hand side for display/editing the polygon points.  When you left-click in the left window, 
one of three things will happen:

1. If you click in an empty part of the window, a point is added to the polygon, between the last point in the list and the 0,0 point.
2. If you click on a line between two previously defined points, a point is added to the polygon between those two points.
3. If you click on a point, it is selected.  If you keep the mouse button down, you can drag the point around the window.

You can delete the selected point with the DEL button.  Pressing the 'n' button will bring up a message box containing the point list.
Right-clicking on a point will bring up an edit dialog with all three values available for changing.  This is where you'd put in radii
for Round-Anything's polyRound() module.

The mouse wheel can be used to zoom in or out of the polygon view.  Before starting a polygon, you want to make sure the window scale will support the polygon's dimensions; use the mouse wheel to change the scale accordingly.  Also, if you open a polygon file or paste a point set and you can't see the polygon, zoom out with the mousewheel until the polygon comes into view.

Note that wxpolygon always displays a closed polygon, closed with a line segment between the last point in the list and the first point.

The point table grows as you add points.  You can edit any coordinate in the table and the polygon window will update.  When you select a point in the polygon pane, the corresponding entry in the table will select.  Note that three columns are always displayed in the table, x, y, and r.  r is used for clipboard copy and file saving if the polyround property is set to 1; if the property is set to 0, the table still displays r but it isn't used for clipboard copy and file saving.

## File Opening/Saving

The menu selection File->Open will provide an open dialog for you to select a OpenSCAD script file containing a point array definition. 
If the script file contains other things, wxpolygon will find and use the first declared point array.

The menu selection File->Save will provide, if a file wasn't previously opened, a dialog for you to specify a OpenSCAD script file to 
save the polygon definition.  **Note that this operation will overwrite any contents that previously existed in the file, and replace 
it with just the polygon definition, so don't use it to replace polygons contained in larger scripts.**  See the **fileformat** 
property below for more information on saving polygon definition files. 

You can also open a file by dragging it from your file explorer to the wxpolygon window.  

And, you can also open a file by specifying its path on the wxpolygon command line.  This
method allows you to set up a desktop icon pointing to wxpolygon and then drag files to that
icon.

Notes:

- Saving a polygon definition as a file with a .scad extension is necessary for using that file in an OpenSCAD include directive.
- If wxpolygon saves a .scad file that is referenced in an opened OpenSCAD script with an include directive, the OpenSCAD window will update.  Neat!

## Copy/Paste

Edit->Copy places the point list in the clipboard.  Note that it is just the points, not the surrounding code to make a polygon definition.

Edit->Paste will parse the clipboard contents and if a polygon definition is found, will use the points to populate the wxpolygon point list.

## Properties

If a file called wxpolygon.conf is in the same directory as the wxpolygon executable, it is read on startup and the properties contained therein
are used by the program.  You can change the properties by using the menu, Edit->Properties..., and a dialog is displayed where you can add, 
edit, and delete properties.  Here are the properties wxpolygon recognizes:

- **maxvalue**: Sets the maximum value when the program is open.  This number is used to scale points to the window. Default: 10.0
- **precision**: Sets the display/export precision of floating point numbers.  Default: 3
- **polyround**: 0|1, sets the use of the third component in the polygon points, for use by the RoundAnything library. If =1, clipboard copy and file save contents will have three columns, x, y, and r; if =0, then just x and y. The state of this property is indicated in the status bar.  Default: 0 (only x and y)
- **filepath**: Sets the default path when the program is started.  Default: current working directory
 - **fileformat**: Sets the format of the saved file contents.  Default: "filename = [ %s ];".  Must contain a %s for the place to put the points. "filename" tells wxpolygon to use the filename minus the extension for the name of the polygon point array; any other value will be used literally as the array name.


## Resources:

https://openscad.org/

https://github.com/Irev-Dev/Round-Anything


