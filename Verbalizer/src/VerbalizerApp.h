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
 
#pragma once

#include <IOBluetooth/IOBluetoothUserLib.h>
#include <IOBluetooth/IOBluetoothUtilities.h>

#include "ofMain.h"
#include "ofEvents.h"
#include "ofxOpenCv.h"
#include "ofxXmlSettings.h"

#include "screenCap.h"
#include "matchFinder.h"
#include "mouseCtrl.h"
#include "BTUtil.h"
#include "spriteButton.h"
#include "audioDeviceUtil.h"
#include "CBTimer.h"

static bool bluetoothConnected;

// Commands
// The commands and data protocol has been kept extremly basic to
// make it easy for users to do listen and send data.
// Therfore each command consists of a single character from 8bit ASCII. 
// We are deliberatly avoiding characters from 7bit ASCII since that could 
// be sent from the BTM112 bluetooth module.

const char CMD_ACTIVATE		= 0x80; // Fired from Arudino to activate voice search
const char CMD_FINISHED		= 0x81; // Finished activation of voice search (not implemented)
const char STATUS_ACTIVATED	= 0x82; // Sent to Arduino if activation come from elsewhere (i.e. desktop app).
const char STATUS_READY		= 0x83; // Found the Mic icon on site and ready to click it.
const char STATUS_DONE		= 0x84; // Voice search session is over.
const char STATUS_CONNECTED	= 0x85; // BT connected 
const char ACK_OK			= 0x86; // Command acknowledged.
const char ACK_BAD			= 0x87; // Command acknowledged but something went wrong.
const char HEARTBEAT        = 0x88; // BT serial link still alive
const char STATUS_DISCONNECT= 0x89; // BT disconnected

//========================================================================
class VerbalizerApp : public ofBaseApp {

	public:
		~VerbalizerApp();
		void setup();
		void update();
		void draw();
		void connectBluetooth();
		void disconnectBluetooth();
		void activateVoiceSearch();
		void findAndClickIcon();
		void playStartedSound(bool only_load);
		void playReadySound(bool only_load);
		void sendCommand(char cmd, bool first_call);
		void sendSerialData(char cmd);

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y);
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
	
		void launchBrowser();
	
		void onConnectBtn(ofPoint &p);
		void onDisconnectBtn(ofPoint &p);
		void onSelectDeviceBtn(ofPoint &p);
		
		void onBluetoothCreateConnection(bool &success);
		void onBluetoothConnect(IOBluetoothObjectRef &dev);
		void onBluetoothSerialFlow(IOBluetoothRFCOMMFlowControlStatus &status);
		void onBluetoothDisconnect(IOBluetoothObjectRef &dev);
		void onSerialData(IOBluetoothRFCOMMDataBlock &data);
		void handleSerialDataCommand(int cmd);
	
		void onTimerCheckPage(ofEventArgs &args);
		void onTimerActivateMic(ofEventArgs &args);
		void onTimerCommandAck(ofEventArgs &args);
		void onTimerClearLastInString(ofEventArgs &args);
		void onTimerSearchDone(ofEventArgs &args);
		void onTimerSendHeartbeat(ofEventArgs &args);
		void onTimerAutoConnect(ofEventArgs &args);
		
		string xml_file;
		ofxXmlSettings XML;
		string url;
		ofSoundPlayer sound_started;
		ofSoundPlayer sound_ready;
		bool sound_started_loaded;
		bool sound_ready_loaded;
		CBTimer timer_check_page;
		CBTimer timer_click;
		CBTimer timer_search_done;
		CBTimer timer_heartbeat;
		CBTimer timer_autoconnect;
		int page_load_check_counter;
		int heartbeat_interval;
	
	
		bool bt_audio_in;
		bool bt_audio_out;
		bool activated;
	
		// UI
		ofTrueTypeFont 	txt_device;
		ofTrueTypeFont 	txt_status;
		ofTrueTypeFont	txt_error;
		ofImage bg;
		spriteButton connect_btn;
		spriteButton disconnect_btn;
		spriteButton select_device_btn;
		string status_msg;
		string error_msg;
		
		// MatchFinder
		screenCap sc;
		ofImage imgScreenShot;
		matchFinder mf;
		ofImage imgTpl;
		ofPoint point;
		mouseCtrl mouse;
		
		// Bluetooth
		BTUtil btutil;
		IOBluetoothDeviceRef device;
		IOBluetoothRFCOMMChannelRef channel;
		char * device_name;
		char * device_address;
		bool waiting_for_ack;
		int command_send_tries;
		CBTimer timer_command_ack;
		char active_cmd;
		int last_cmd;
		CBTimer timer_clear_last_in_s;
	
		// Audio contrl
		audioDeviceUtil audio_util;
};
