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

#include "VerbalizerApp.h"

//--------------------------------------------------------------
void VerbalizerApp::setup() {
	
	// data folder is in our .app bundle
	ofSetDataPathRoot("../Resources/");
		
	// Load settings from our XML file
	xml_file = "settings.xml";
	XML.loadFile(xml_file);
	url = XML.getValue("settings:url", "http://www.google.com/");
	
	// UI setup
	ofSetWindowTitle("Verbalizer");
	device_name = "No device selected";
	status_msg = "";
	error_msg = "";

	bg.loadImage(ofToDataPath("bg.png"));
	txt_device.loadFont(ofToDataPath("LucidaGrande.ttc"), 10, true, false, false);
	txt_status.loadFont(ofToDataPath("LucidaGrande.ttc"), 10, true, false, false);
	txt_error.loadFont(ofToDataPath("LucidaGrande.ttc"), 10, true, false, false);
	
	connect_btn.setup(ofToDataPath("connect_btn.png"));
	connect_btn.setBlur(true);
	connect_btn.enabled = true;
	connect_btn.setPos(593, 316);
	ofAddListener(connect_btn.onReleaseEvent, this, &VerbalizerApp::onConnectBtn);
	
	disconnect_btn.setup(ofToDataPath("disconnect_btn.png"));
	disconnect_btn.setPos(582, 316);
	disconnect_btn.enabled = false;
	ofAddListener(disconnect_btn.onReleaseEvent, this, &VerbalizerApp::onDisconnectBtn);
	
	select_device_btn.setup(ofToDataPath("select_device_btn.png"));
	select_device_btn.setPos(348, 175);
	ofAddListener(select_device_btn.onReleaseEvent, this, &VerbalizerApp::onSelectDeviceBtn);
	
	activated = false;
	
	// Start listening to BT connections
	waiting_for_ack = false;
	bluetoothConnected = false;
	btutil.registerListenConnect();
	ofAddListener(btutil.createConnectionEvent, this, &VerbalizerApp::onBluetoothCreateConnection);
	ofAddListener(btutil.connectEvent, this, &VerbalizerApp::onBluetoothConnect);
	ofAddListener(btutil.disconnectEvent, this, &VerbalizerApp::onBluetoothDisconnect);
	ofAddListener(btutil.serialDataEvent, this, &VerbalizerApp::onSerialData);
	ofAddListener(btutil.serialFlowEvent, this, &VerbalizerApp::onBluetoothSerialFlow);
	
	// Load our image to search for
	imgTpl.loadImage(ofToDataPath("voice_input.png"));
	mf.Init();
	mf.setTpl(imgTpl);
	
	// Initialize screenCap
	sc.Init();
	
	// Initialize audio utils
	audio_util.Init();
	sound_started_loaded = false;
	sound_ready_loaded = false;
	
	bt_audio_in = XML.getValue("settings:bt_audio_in", true);
	bt_audio_out = XML.getValue("settings:bt_audio_out", true);
	
	// Timers
	ofAddListener(timer_check_page.time_up, this, &VerbalizerApp::onTimerCheckPage);
	ofAddListener(timer_click.time_up, this, &VerbalizerApp::onTimerActivateMic);
	ofAddListener(timer_command_ack.time_up, this, &VerbalizerApp::onTimerCommandAck);
	ofAddListener(timer_clear_last_in_s.time_up, this, &VerbalizerApp::onTimerClearLastInString);
	ofAddListener(timer_search_done.time_up, this, &VerbalizerApp::onTimerSearchDone);
	ofAddListener(timer_heartbeat.time_up, this, &VerbalizerApp::onTimerSendHeartbeat);
	
	heartbeat_interval = XML.getValue("settings:timer_heartbeat", 1500);
}

//--------------------------------------------------------------
void VerbalizerApp::update() {
	ofSoundUpdate();
}

//--------------------------------------------------------------
void VerbalizerApp::draw() {
	
	// Add our background and text
	bg.draw(0, 0);
	
	// Draw UI text elements
	ofPushStyle();
	ofSetColor(0, 0, 0);
	txt_device.drawString(device_name, 348, 159);
	txt_status.drawString(status_msg, 348, 230);
	ofPopStyle();
	
	ofPushStyle();
	ofSetColor(255, 0, 0);
	txt_error.drawString(error_msg, 348, 270);
	ofPopStyle();	
}

//--------------------------------------------------------------
void VerbalizerApp::disconnectBluetooth() {
	if(device != NULL && btutil.isConnected(device)) {
		printf("Closing BT base band connection\n");
		// send the disconnect status.
		sendSerialData(STATUS_DISCONNECT);
		usleep(1000 * 250); // quart second.
		btutil.closeConnection(device);
	}
}

//--------------------------------------------------------------
void VerbalizerApp::activateVoiceSearch() {
	
	if(activated) {
		return;
	}

	activated = true;
	
	printf("Activating browser voice search\n");

	// Launch the browser with focus
	ofLaunchBrowser(url);
	
	// Start timer for checking page load status
	page_load_check_counter = XML.getValue("settings:cv_min_loc_tries", 5);
	timer_check_page.start(XML.getValue("settings:cv_min_loc_timeout", 1000));
	
	// Play first sound nofying that we've got the command to commence voice search.
	playStartedSound(false);
	
	// HELP!
	// TODO: set timeout here, not sleep.
	usleep(XML.getValue("settings:click_delay", 10));
	
}

//--------------------------------------------------------------
void VerbalizerApp::playStartedSound(bool only_load) {
	if (!sound_started_loaded) {
		sound_started.loadSound(XML.getValue("settings:mp3_file_started", ""), false);
		sound_started_loaded = true;
	}
	if (!only_load) {
		sound_started.play();
	}
}

//--------------------------------------------------------------
void VerbalizerApp::playReadySound(bool only_load) {
	if (!sound_ready_loaded) {
		sound_ready.loadSound(XML.getValue("settings:mp3_file_ready", ""), false);
		sound_ready_loaded = true;
	}
	if (!only_load) {
		sound_ready.play();
	}
}

//--------------------------------------------------------------
void VerbalizerApp::sendCommand(char cmd, bool first_call) {
	if (channel == NULL) { return; }
	waiting_for_ack = true;
	active_cmd = cmd;
	sendSerialData(cmd);
	// setup a timer for looking for ack
	timer_command_ack.start(XML.getValue("settings:command_retry_interval", 500));
	if (first_call) {
		command_send_tries = XML.getValue("settings:command_number_of_retries", 2);
	}
}

//--------------------------------------------------------------
void VerbalizerApp::sendSerialData(char cmd) {
	if (channel == NULL || !bluetoothConnected) { return; }
	char c_arr[2];
	c_arr[0] = cmd;
	btutil.sendRFCOMMData(channel, c_arr, 1);
}

//--------------------------------------------------------------
void VerbalizerApp::onTimerCommandAck(ofEventArgs &args) {
	if (command_send_tries > 0) {
		// ok we should try sending one more time.
		printf("No ack for [%i] yet. Trying again..\n", active_cmd);
		command_send_tries--;
		sendCommand(active_cmd, false);
	}else{
		// We never got an ack for this command.
		// we don't really have much to do here other than perhaps 
		// Letting the app know by updating the status message?
		//status_msg = "Didn't get ack from user";
		printf("Didn't receive ack for command [%i]. Giving up.\n", active_cmd);
	}
}

//--------------------------------------------------------------
void VerbalizerApp::onTimerCheckPage(ofEventArgs &args) {
	printf("onTimerCheckPage\n");
	page_load_check_counter--;
	
	// Take a new screen grab
	imgScreenShot = sc.snap(XML.getValue("settings:tmp_file", "/tmp/voice_search.png"));
	mf.setInput(imgScreenShot);
	
	// Find our point
	point = mf.getPoint();
	
	// Not good?
	if (!mf.hasSatisfyingMatch()) {
		printf("Can't find matchTemplate. Tries left: %i \n", page_load_check_counter);
		if (page_load_check_counter > 0) {
			timer_check_page.start(XML.getValue("settings:cv_min_loc_timeout", 500));
		}else{
			status_msg = "Could not find voice input\n on " +  XML.getValue("settings:url", "http://google.com");
			activated = false;
		}
		return;
	}
	
	// We've found the template so let's play the Ready sound.
	playReadySound(false);
	
	// Tell board we're activating the mic
	sendCommand(STATUS_READY, true);
	usleep(1000 * 100); // wait half sec
	// Start the timer for clicking on the template
	timer_click.start(XML.getValue("settings:activation_delay", 200));
	activated = false;
}

//--------------------------------------------------------------
void VerbalizerApp::onTimerActivateMic(ofEventArgs &args) {
	// Now we'll click to enable voice search
	printf("Clicking %fx%f \n", point.x, point.y);
	mouse.leftClick(point.x+(imgTpl.width/2), point.y+(imgTpl.height/2));
	
	// Timeout for telling the board when the search is done.
	// TODO: poll screenshot for updates and figure out when 
	// the user search is actually done.
	timer_search_done.start(XML.getValue("settings:timer_search_done", 7000));
}

//--------------------------------------------------------------
void VerbalizerApp::onTimerClearLastInString(ofEventArgs &args) {
	last_cmd = '0';
}

//--------------------------------------------------------------
void VerbalizerApp::onTimerSearchDone(ofEventArgs &args) {
	printf("Sending done command\n");
	sendCommand(STATUS_DONE, true);
}

//--------------------------------------------------------------
void VerbalizerApp::onTimerSendHeartbeat(ofEventArgs &args) {
	sendSerialData(HEARTBEAT);
	timer_heartbeat.start(heartbeat_interval); // loop heartbeat
}

//--------------------------------------------------------------
void VerbalizerApp::onBluetoothConnect (IOBluetoothObjectRef &dev) {
	// Setup a comm cannel to the BT device that was picked.

	if (dev == device) {
		
		// Set the BT device as default in and output for audio.
		if(bt_audio_in) {
			printf("Setting BT device as default audio input\n");
			if(!audio_util.setInputDevice(btutil.deviceName(device))) {
				error_msg = "Error, could not connect onboard microphone!\nPlease try restarting bluetooth and connect again.";
				disconnectBluetooth();
				return;
			}
		}
		
		if(bt_audio_out) {
			printf("Setting BT device as default audio output\n");
			audio_util.setOutputDevice(btutil.deviceName(device));
		}
		
		printf("Opening RFCOMM channel\n");
		channel = btutil.openRFCOMMChannel(device);
		
		sendCommand(STATUS_CONNECTED, true);
		timer_heartbeat.start(heartbeat_interval);
		
	}else{
		printf("Unknown BT device connected, ignoring it.\n");
	}
}

//--------------------------------------------------------------
void VerbalizerApp::onBluetoothCreateConnection (bool &success) {
	if (success) {
		status_msg = "Waiting for device..";
		printf("Baseband connected\n");
		connect_btn.enabled = false;
		disconnect_btn.enabled = true;
		bluetoothConnected = true;
	} else {
		status_msg = "Could not connect";
		printf("Could not establish connection.\n");
		connect_btn.enabled = true;
		connect_btn.setBlur(false);
		disconnect_btn.enabled = false;
	}
}

//--------------------------------------------------------------
void VerbalizerApp::onBluetoothDisconnect (IOBluetoothObjectRef &dev) {
	printf("Device disconnected\n");
	status_msg = "Device disconnected";
	connect_btn.enabled = true;
	connect_btn.setBlur(false);
	disconnect_btn.enabled = false;
	bluetoothConnected = false;
	timer_heartbeat.stop();
}

//--------------------------------------------------------------
void VerbalizerApp::onSerialData (IOBluetoothRFCOMMDataBlock &data) {
	
	if (data.dataSize == NULL || data.dataSize == 0 
				|| data.dataPtr == NULL || data.dataPtr == 0) {
		printf("onSerialData. No dataSize\n");
		return;
	}
	
	int size = data.dataSize+1;
	char my_data[size];
	strcpy(my_data, (const char *)data.dataPtr);
	
	// We're getting some nasty values in data.dataSize sometime, where the size is 
	// reported as extremly high or very low. A temporary fix is to test for the size 
	// here and abort if it's very high.
	// TODO: figure out the real reason why data.dataSize is reporting abnormal values.
	if (size > 1024 || size < 0) {
		printf("!! onSerialData: Aborting since we're getting sizes way higher than we're expecting.");
		return;
	}
	
	for (int i = 0; i < size - 1; i++) {
		handleSerialDataCommand(my_data[i]);
	}
}

//--------------------------------------------------------------
void VerbalizerApp::handleSerialDataCommand(int cmd) {
	
	printf("handleSerialDataCommand %i\n", cmd);
		
	// if we have the same string as we got last time,
	// let's bail.
	if (cmd == last_cmd) {
		return;
	}
	
	// ACTIVATE VOICE SEARCH
	if (cmd == CMD_ACTIVATE) {
		activateVoiceSearch();
		sendSerialData(ACK_OK);
	}
	
	// ACTIVATION FINISHED or SUBMITTED
	else if (cmd == CMD_FINISHED) {
		// not implemented
		sendSerialData(ACK_OK);
	}
	
	// ACK
	else if (cmd == ACK_OK) {
		// not implemented
		printf("Got ACK, excellent!\n");
		waiting_for_ack = false;
		timer_command_ack.stop();
	}
	
	// ACK (bad)
	else if (cmd == ACK_BAD) {
		// not implemented
		waiting_for_ack = false;
		timer_command_ack.stop();
		printf("Error ack from board.\n");
	}
	
	// save in string in case we're getting hammered.
	last_cmd = cmd;
	timer_clear_last_in_s.start(500);

}


//--------------------------------------------------------------
void VerbalizerApp::onBluetoothSerialFlow(IOBluetoothRFCOMMFlowControlStatus &status) {
	printf("RFCOMM channel open\n");
	status_msg = "Bluetooth device connected";
	
	// Now is a good time to preload sounds since we're done with
	// establishing the BT connection with RFCOMM channel.
	printf("Preloading sounds\n");
	playStartedSound(true);
	playReadySound(true);
	
	//
}

//--------------------------------------------------------------
void VerbalizerApp::onConnectBtn(ofPoint &p) {
	printf("Click Connect\n");
	error_msg = "";
	if(btutil.connect(device)) {
		printf("Trying to connect...\n");
		status_msg = "Connecting..";
		connect_btn.setBlur(true);
	}else{
		printf("error!\n");
		status_msg = "Could not initiate connection attempt";
	}
}

//--------------------------------------------------------------
void VerbalizerApp::onDisconnectBtn(ofPoint &p) {
	printf("Click Disconnect\n");
	sendSerialData(STATUS_DONE);
	disconnectBluetooth();
}

void VerbalizerApp::onSelectDeviceBtn(ofPoint &p) {

	// Make sure we're not already connected since this can cause trouble.
	if (bluetoothConnected) {
		printf("Already connected.\n");
		disconnectBluetooth();
		return;
	}
	
	device = BTUtil::selectBTDevice();
	if (device != NULL) {
		disconnectBluetooth();
		connect_btn.setBlur(false);
		device_name = btutil.deviceName(device);
		device_address = btutil.deviceAddress(device);
		printf("Selected device\n");
	}else{
		printf("No device selected\n");
	}
}

//--------------------------------------------------------------
void VerbalizerApp::keyPressed(int key) {
	
}

//--------------------------------------------------------------
void VerbalizerApp::keyReleased(int key) {
	
	// Various key presses to test functionality.
	if (key == 't' && channel != NULL) {
		char * c = (char *)"Sending test message to BT device";
		printf("%s\n", c);
		IOBluetoothRFCOMMChannelWriteAsync(channel, c, strlen(c), this);
	}else if (key == 'a') {
		activateVoiceSearch();
		sendSerialData(STATUS_ACTIVATED);
	}else if(key == 's') {
		playStartedSound(false);
	}else if(key == 'c') {
		imgScreenShot = sc.snap(XML.getValue("settings:tmp_file", "/tmp/voice_search.png"));
		mf.setInput(imgScreenShot);
		point = mf.getPoint();
	}else if(key == 'p') {
		sendSerialData(STATUS_READY);
	}
}

//--------------------------------------------------------------
void VerbalizerApp::mouseMoved(int x, int y) {

}

//--------------------------------------------------------------
void VerbalizerApp::mouseDragged(int x, int y, int button) {

}

//--------------------------------------------------------------
void VerbalizerApp::mousePressed(int x, int y, int button) {
	
}

//--------------------------------------------------------------
void VerbalizerApp::mouseReleased(int x, int y, int button) {

}

//--------------------------------------------------------------
void VerbalizerApp::windowResized(int w, int h) {

}

VerbalizerApp::~VerbalizerApp() {
	printf("** Killing app **\n");
	if (bluetoothConnected) {
		disconnectBluetooth();
	}
}
