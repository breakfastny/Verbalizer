/*
 *  screenCap.cpp
 *
 *  Copyright (c) 2011, BREAKFAST LLC
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *      * Redistributions of source code must retain the above copyright
 *        notice, this list of conditions and the following disclaimer.
 *      * Redistributions in binary form must reproduce the above copyright
 *        notice, this list of conditions and the following disclaimer in the
 *        documentation and/or other materials provided with the distribution.
 *      * Neither the name of the BREAKFAST LLC nor the
 *        names of its contributors may be used to endorse or promote products
 *        derived from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 *  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 *  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *  DISCLAIMED. IN NO EVENT SHALL BREAKFAST LLC BE LIABLE FOR ANY
 *  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include "screenCap.h"

void screenCap::Init () {
	// Get screen size so we can allocate memory for ofImage.
	CGRect screenFrame = CGDisplayBounds(kCGDirectMainDisplay);
	screenSize  = screenFrame.size;
	
	// Allocate some memory for or screenshot
	img.allocate(screenSize.width, screenSize.height, OF_IMAGE_COLOR);
}

ofImage screenCap::snap(string path) {
	tmpPath = path;
	
	// Take a screenshot using OS X screencapture.
	// TODO: This could be improved by not spinning up the 
	// screencapture process and hitting disk. Perhaps looking
	// into some other libraries that does this.
	string command = "screencapture -x -m -tpng " + tmpPath;
	system(command.c_str());
	
	img.loadImage(tmpPath);
	return img;
}

void screenCap::removeTmpImage() {
	// TODO: clean up
}

screenCap::~screenCap () {
	// release here.
	//removeTmpImage();
}