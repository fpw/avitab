Place directories containing EPSG-3857 sets here. These are also called "Slippy Tiles".
It is advised to put every map into its own directory so that the directory looks something like this:

-- EPSG-3857
  \-Country 1
   \--5
   |--6
   |...
  \-Country 2
   \--5
   |--6
   |...

You can get EPSG-3857 tiles for Europe from https://openflightmaps.org/
Their ZIP files contain multiple versions, I suggest using the merged\256@2x version.
For example, the ZIP file for Switzerland contains the clip\merged\256@2x\latest directory.
Copy that directory into the EPSG-3857 folder and name it "Switzerland" so that this folder contains the numbered directories.
If you don't need zoomlevel 12, use the 256 directory to save disk space. 512 is not supported.

It is also possible to merge all these maps into one big map of Europe. To do that, click on the "Settings" link on the
OpenFlightMaps download page and select "tiles not clipped". The non-clipped downloads can then all be copied into a single
directory, for example called "Europe". That directory should contain the numbered directories from the OpenFlightMaps downloads.
Duplicate files can simply be overwritten.
