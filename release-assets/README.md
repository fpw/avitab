# Release assets

This directory contains static files that must be included in release packages
but are not produced by the build.

Required contents:
- release-assets/readme.txt
- release-assets/AircraftIntegration/readme.txt
- release-assets/AircraftIntegration/sample.zip
- release-assets/MapTiles/EPSG-3857/readme.txt
- release-assets/MapTiles/GeoTIFFs/readme.txt
- release-assets/MapTiles/Mercator/readme.txt

Packaging behavior:
- The build workflow copies these assets into the release package.
- res/icons, res/online-maps, and res/config.json still come from the build.
- res/charts still comes from the build.
