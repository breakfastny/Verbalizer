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

#include "audioDeviceUtil.h"

void audioDeviceUtil::Init() {
	
	//Find audio inputs
	OSStatus result = noErr;
	UInt32 prop_size;
	
	// get the device list	
	AudioObjectPropertyAddress audio_prop = { kAudioHardwarePropertyDevices, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMaster };
	result = AudioObjectGetPropertyDataSize(kAudioObjectSystemObject, &audio_prop, 0, NULL, &prop_size);
	if (result) { printf("Error in AudioObjectGetPropertyDataSize: %d\n", result); }
	
	// Find out how many devices are on the system
	num_devices = prop_size / sizeof(AudioDeviceID);
	device_list = (AudioDeviceID*)calloc(num_devices, sizeof(AudioDeviceID));
	
	result = AudioObjectGetPropertyData(kAudioObjectSystemObject, &audio_prop, 0, NULL, &prop_size, device_list);
	if (result) { printf("Error in AudioObjectGetPropertyData 1: %d\n", result);  }
}


bool audioDeviceUtil::setInputDevice(char *name) {
	return setDevice(name, true);
}

bool audioDeviceUtil::setOutputDevice(char *name) {
	return setDevice(name, false);
}

void audioDeviceUtil::getInputDevice(char *buf) {
	UInt32 propsize = sizeof(AudioDeviceID);
	verify_noerr (AudioHardwareGetProperty(kAudioHardwarePropertyDefaultInputDevice, &propsize, &buf));
}

void audioDeviceUtil::getOutputDevice(char *buf) {
	UInt32 propsize = sizeof(AudioDeviceID);
	verify_noerr (AudioHardwareGetProperty(kAudioHardwarePropertyDefaultOutputDevice, &propsize, &buf));
}

bool audioDeviceUtil::setDevice(char *name, bool is_input) {
	
	OSStatus result = noErr;
	CFStringRef new_device_name = CFStringCreateWithCString(kCFAllocatorDefault, name, CFStringGetSystemEncoding());	
	CFStringRef device_name = NULL;
	UInt32 prop_size = sizeof(CFStringRef);
	bool found_new_device = false;
	
	AudioObjectPropertyAddress audio_prop;
	audio_prop.mSelector = kAudioObjectPropertyName;
	audio_prop.mScope = kAudioObjectPropertyScopeGlobal;
	audio_prop.mElement = kAudioObjectPropertyElementMaster;

	for (UInt32 i=0; i < num_devices; i++) {
		// get the device name
		
		result = AudioObjectGetPropertyData(device_list[i], &audio_prop, 0, NULL, &prop_size, &device_name);
		if (result) { 
			printf("Error in AudioObjectGetPropertyData 2: %d\n", result);
			break;
		}
		
		if (CFStringCompare(device_name, new_device_name, 0) == kCFCompareEqualTo) {
			
			// we found the device, now set it as the default output device
			if (is_input) {
				audio_prop.mSelector = kAudioHardwarePropertyDefaultInputDevice;
			}else{
				audio_prop.mSelector = kAudioHardwarePropertyDefaultOutputDevice;
			}
			
			result = AudioObjectSetPropertyData(kAudioObjectSystemObject, &audio_prop, 0, NULL, sizeof(AudioDeviceID), &device_list[i]);
			if (result) { 
				printf("Error in AudioObjectSetPropertyData 3: %d\n", result);
			}else{
				found_new_device = true;
				CFRelease(device_name); // no more need for this
				printf("Successfully changed audio device to %s\n", name);
				break;
			}
		}
		CFRelease(device_name);
	}
	CFRelease(new_device_name);
	return found_new_device;
}