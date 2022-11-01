# AviTab - The Aviator's Tablet

AviTab is a plugin for the [X-Plane](http://www.x-plane.com/) flight simulator.
It displays a tablet like computer with a few apps in the cockpit. It is mainly
used for flying in virtual reality.

![](screenshots/launcher.jpg)

## Purpose
When flying in a simulator, one often needs to lookup details in PDF charts, plane manuals,
checklists or other documents. Using a PDF reader breaks the immersion because the virtual aviator
either has to take off of their HMD or use other tools that can move windows into VR.

AviTab tries to solve this problem by offering a PDF reader inside a native plugin for X-Plane.
Using a plugin also opens possibilites for more apps inside the tablet, for example to display information
about the plane status or aid in navigation.

## Features
Let the screenshots speak! Note that these were taken using the standalone version for better resolution,
but all of this is available right inside your VR cockpit.

![](screenshots/charts.jpg)
![](screenshots/map.jpg)
![](screenshots/airports.jpg)

More screenshots here: [Screenshots](screenshots/)

## Installation

* Download the latest release from the [release page](https://github.com/fpw/avitab/releases/latest)
* Extract the archive and move the folder into your ``X-Plane/Resources/Plugins`` directory
* Start X-Plane
* Find a new menu inside the ``plugins`` directory to toggle the tablet
* Optionally, you can assign a key for this command (I prefer the right-hand lower trigger)

## Usage

### General
When hovering over the edges of the tablet from a short distance, X-Plane will display green bars around the tablet.
The _upper_ bar can be used to grab the window and move it around inside the cockpit.

### Charts Viewer
The charts viewer displays PDF files inside the ``charts/`` subdirectory of the ``plugin/`` directoy. You can add your
charts there, including subdirectories.

## Limitations

* PDFs containing [CJK fonts](https://en.wikipedia.org/wiki/List_of_CJK_fonts)
  or ancient fonts such [Linear B](https://en.wikipedia.org/wiki/Linear_B) are not supported.
  This is mainly due to font sizes - a version containing all fonts would be 40 Megabytes in size.

## Compiling

* If you would like to contribute to AviTab's development, a script called `setup.sh` has been added to automatically
  download and setup your environment with the needed dependencies.
  * Windows, Linux, and macOS are supported with their respective dependency managers.  If you do not have the
    proper dependency manager installed, the script will prompt you to install it before proceeding.
  * X-Plane 12, Make sure you have downloaded the latest [X-Plane 4 Beta SDK](https://developer.x-plane.com/sdk/plugin-sdk-downloads). (Needed for running the plugin on Apple Silicon hardware.)
* There is another script called `teardown.sh` that will delete the folders where the dependencies were
  placed to compile AviTab.

## Donate
If you like AviTab and want to support its further development, you can donate.

[![Donate](https://img.shields.io/badge/Donate-PayPal-green.svg)](https://www.paypal.com/cgi-bin/webscr?cmd=_donations&business=folke%2ewill%40gmail%2ecom&lc=US&item_name=AviTab&no_note=0&cn=Message%20to%20the%20developer%3a&no_shipping=1&currency_code=EUR&bn=PP%2dDonationsBF%3abtn_donateCC_LG%2egif%3aNonHosted)
