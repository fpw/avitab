AviTab can be integrated into 3D cockpits. To do that, assign a region in
the panel.png to AviTab's screen and specify these coordinates in a file called
AviTab.json that has to be put next to the ACF file. See sample.zip as an example
done by @nhadrian.

AviTab can optionally be controlled via the following datarefs:
 * avitab/panel_enabled: whether AviTab should draw on the assigned texture or not
 * avitab/panel_powered: whether the tablet is powered or not - setting it to 0 will make AviTab fill the texture in black
 * avitab/brightness: the total brightness of the tablet
 * avitab/is_in_menu: 1 if the main menu is shown, 0 if an app is shown
 * avitab/panel_left (and _width, _bottom, _height): To change panel location at runtime

The following commands are available to switch between apps:
 * AviTab/app_charts
 * AviTab/app_airports
 * AviTab/app_routes
 * AviTab/app_maps
 * AviTab/app_navigraph
 * AviTab/app_plane_manual
 * AviTab/app_notes
 * AviTab/app_about

By default, panel_enabled is false, i.e. the integration must enable the tablet on demand.
If this is not desired, set "enabled": true in the JSON file.

By default, AviTab will create an invisible capture window if it is integrated into a 3D cockpit.
If this interferes with existing windows of your aircraft (e.g. SASL2), you can set "disable_capture_window": true
in the JSON file. If that is set, AviTab will not create a window at all. Instead, forward clicks using the commands AviTab/click_left,
AviTab/wheel_up and AviTab/wheel_down. AviTab will figure out the coordinates by itself, you only need to send the commands to forward
clicks to AviTab.
 
If you don't want to use datarefs, you can additionally always enable AviTab via
the JSON file, see the sample for an example.

