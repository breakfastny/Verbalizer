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

#include "spriteButton.h"

#define NORMAL_POS	 0
#define OVER_POS	 1
#define CLICK_POS	 2
#define DISABLED_POS 3

void spriteButton::setup(string sprite_path) {
	sprite.loadImage(sprite_path);
	sprite_height = sprite.getHeight();
	sprite_width = sprite.getWidth();
	button_height = sprite_height / 4;
	state = NORMAL_POS;
	setSize(sprite_width, button_height);
	_blur = false;
	enableMouseEvents();
}

void spriteButton::setBlur(bool blur) {
	_blur = blur;
	if(blur) {
		disableMouseEvents();
	}else{
		enableMouseEvents();
	}
}

void spriteButton::update() {
	if(_blur) {
		state = DISABLED_POS;
	}else if(isMouseDown() && isMouseOver()) {
		state = CLICK_POS;
	}else if(isMouseOver()) {
		state = OVER_POS;
	}else{
		state = NORMAL_POS;
	}
	
	p_image[0].set(0, state*button_height);
	p_image[1].set(sprite_width, state*button_height);
	p_image[2].set(sprite_width, state*button_height+button_height);
	p_image[3].set(0, state*button_height+button_height);
	
	p_window[0].set(x, y);
	p_window[1].set(x+sprite_width, y);
	p_window[2].set(x+sprite_width, y+button_height);
	p_window[3].set(x, y+button_height);
}

void spriteButton::draw() {
	sprite.getTextureReference().bind();
	glBegin(GL_QUADS);
	for(int i = 0; i < 4; i++) {
		glTexCoord2f(p_image[i].x, p_image[i].y);
		glVertex2f(p_window[i].x, p_window[i].y);
	}
	glEnd();
	sprite.getTextureReference().unbind();
}

void spriteButton::onRollOver(int x, int y) { }

void spriteButton::onRollOut() { }

void spriteButton::onMouseMove(int x, int y) { }

void spriteButton::onDragOver(int x, int y, int button) { }

void spriteButton::onDragOutside(int x, int y, int button) { }

void spriteButton::onPress(int x, int y, int button) { }

void spriteButton::onRelease(int x, int y, int button) {
	ofPoint p = ofPoint(x, y, button);
	ofNotifyEvent(onReleaseEvent, p, this);
}

void spriteButton::onReleaseOutside(int x, int y, int button) { }

void spriteButton::keyPressed(int key) { }

void spriteButton::keyReleased(int key) { }
