# Avitab for Microsoft Flight Simulator

An Avitab port for Microsoft Flight Simulator (Avitab-msfs) is available as a standalone
application (exe). It should be noted that this is a bare-minimum adaptation and does
not offer the seamless integration that X-Plane aviator's will be accustomed to.

The MSFS port of Avitab is maintained by mjh65 in between other more time-consuming
projects. Many of the obvious shortcomings are already on the TODO list, but feel
free to report any less obvious issues!

## Packaging

The MSFS variant of Avitab is provided as a zip package containing a Windows desktop
application and additional required files. The zip file will be uploaded to github
alongside the usual release artefacts.

## Installation

After extraction the top-level package directory can be moved anywhere as desired,
but the contents should be kept together.

## Running

Double-click Avitab-msfs.exe to run it. The app will connect to the MSFS simulation and
can then be used as normal. Since Avitab-msfs runs as a desktop app it will be required to
shift focus to interact with it.

Avitab-msfs can be used in VR as an overlay. Toggling focus will still be required to
switch between the simulator and the tablet, which might be found cumbersome.

A better integration (as an in-game panel) is on the TODO list.

## Navigation Database

If, on launching, the window shows 'Loading nav data ...' then this indicates that there
is an X-Plane installation and its navigation database is being used. If this message does 
not appear then it is either using a pre-built SQLite navigation database, or no database at all.

There are instructions (in the HOWTO file in the navdb folder) for generating a database
from the MSFS installation that will be used instead of the default.

Integrated loading of the navigation database is on the TODO list.
