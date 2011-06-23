/*
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


#ifndef _SPRITE_BUTTON
#define _SPRITE_BUTTON

#include "ofMain.h"
#include "ofxMSAInteractiveObject.h"

class spriteButton : public ofxMSAInteractiveObject {
	
	public:
		void setup(string sprite_path);
		void update();
		void draw();
		virtual void onRollOver(int x, int y);		
		virtual void onRollOut();
		virtual void onMouseMove(int x, int y);
		virtual void onDragOver(int x, int y, int button);
		virtual void onDragOutside(int x, int y, int button);
		virtual void onPress(int x, int y, int button);
		virtual void onRelease(int x, int y, int button);
		virtual void onReleaseOutside(int x, int y, int button);
		virtual void keyPressed(int key);
		virtual void keyReleased(int key);
		
		void setBlur(bool on);

		ofEvent<ofPoint> onReleaseEvent;
		
	protected:
		bool _blur;
		ofImage sprite;
		int sprite_height;
		int sprite_width;
		int button_height;
		int state;
		ofPoint p_image[4];  
		ofPoint p_window[4];

	
};

#endif