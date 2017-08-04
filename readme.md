# trapper
## a simple mouse trapper for windows


### usage: trapper [options]

### options:
	--help		prints this explanation
	--keep-focus	periodically tries to make sure we still have focus
	--aggressive	additionally uses low-level hooks to keep the mouse trapped


### examples:
	trapper					just the basic mouse trap
	trapper --keep-focus			mouse trap with periodic checks
	trapper --aggressive			trap mouse, prevent tabbing, disable windows keys etc.
	trapper --keep-focus --aggressive	all at once - trap mouse, check periodically, prevent tabbing

### notes:
    - use either the command line or create an appropriate shortcut [for example C:\your\folder\trapper.exe --keep-focus]
    - click target window, use HOME to toggle the trap, use END to kill the trapper
    - needs elevated rights (i.e. to be run as admin) to trap the mouse in privileged applications
    - the mouse will be trapped in an area slightly smaller than the target window
