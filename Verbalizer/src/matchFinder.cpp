/*
 *  MatchFinder.cpp
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

#include "matchFinder.h"

void matchFinder::Init() {
	satisfying_match = 409976;
	min_val = satisfying_match + 1;
};

void matchFinder::setInput(ofImage &img) {
	input.clear();
	input.allocate(img.getWidth(), img.getHeight());
	input.setFromPixels(img.getPixels(), img.getWidth(), img.getHeight());
}

void matchFinder::setTpl(ofImage &img) {
	tpl.clear();
	tpl.allocate(img.getWidth(), img.getHeight());
	tpl.setFromPixels(img.getPixels(), img.getWidth(), img.getHeight());
}

ofPoint matchFinder::getPoint() {
	// get the size for our result image
	CvSize result_size = cvSize(input.getWidth() - tpl.getWidth() + 1,
								input.getHeight() - tpl.getHeight() + 1);
	
	// create the result image for the comparison
	IplImage *result_image = cvCreateImage(result_size, IPL_DEPTH_32F, 1);

	// make the comparison
	cvMatchTemplate(input.getCvImage(), tpl.getCvImage(), result_image, CV_TM_SQDIFF);
	
	// copy to ofCv image.
	IplImage *result_char = cvCreateImage(cvSize(result_image->width, result_image->height), 8, 1);
	
	ofcv_result_image.allocate(result_size.width, result_size.height);
	ofcv_result_image = result_char;
	
	// get the location of the best match
	CvPoint min_loc;
	CvPoint max_loc;
	cvMinMaxLoc(result_image, &min_val, &max_val, &min_loc, &max_loc, 0);
		
	// clean up
	cvReleaseImage(&result_image);
	
	// return value
	ofPoint p = ofPoint(min_loc.x, min_loc.y);
	return p;
}

bool matchFinder::hasSatisfyingMatch() {
	return min_val < satisfying_match;
}

ofxCvColorImage matchFinder::getOfCvResultImage() {
	return ofcv_result_image;
}

matchFinder::~matchFinder() {
	input.clear();
	tpl.clear();
}