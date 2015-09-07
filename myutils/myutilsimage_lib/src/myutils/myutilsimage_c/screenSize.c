/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MultimediaTools. https://github.com/juanbarrios/multimedia_tools
 * MultimediaTools is made available under the terms of the BSD 2-Clause License.
 */

#include "screenSize.h"

#if WIN32
#include "wtypes.h"
#endif

// Get the horizontal and vertical screen sizes in pixel
void my_getScreenResolution(int *out_width, int *out_height) {
#if WIN32
	RECT desktop;
	// Get a handle to the desktop window
	const HWND hDesktop = GetDesktopWindow();
	// Get the size of screen to the variable desktop
	GetWindowRect(hDesktop, &desktop);
	// The top left corner will have coordinates (0,0)
	// and the bottom right corner will have coordinates
	// (horizontal, vertical)
	*out_width = desktop.right;
	*out_height = desktop.bottom;
#else
	/*
	 char *command = "xrandr | grep '*'";
	 FILE *fpipe = (FILE*) popen(command, "r");
	 char line[256];
	 while (fgets(line, sizeof(line), fpipe)) {
	 printf("%s", line);
	 }
	 pclose(fpipe);
	 */
	*out_width = 1024;
	*out_height = 768;
#endif
}
