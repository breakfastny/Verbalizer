/*
 *  BTUtil.cpp
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

#include "BTUtil.h"

//-------------------------------------------------------------------
// STATIC CALLBACK FUNCTIONS
//-------------------------------------------------------------------

// Callback for disconnects
void bluetoothDisconnect_c(void *userRefCon, IOBluetoothUserNotificationRef inRef, IOBluetoothObjectRef objectRef) {
	// Stop listening to BT disconnects
	IOBluetoothUserNotificationUnregister(inRef);
	BTUtil * btutil = static_cast <BTUtil *>(userRefCon);
	ofNotifyEvent(btutil->disconnectEvent, objectRef);
}

// Callback function for connections
void bluetoothConnect_c(void *userRefCon, IOBluetoothUserNotificationRef inRef, IOBluetoothObjectRef objectRef) {
	BTUtil * btutil = static_cast <BTUtil *>(userRefCon);
	// Start listening to BT disconnects
	IOBluetoothDeviceRegisterForDisconnectNotification(objectRef, &bluetoothDisconnect_c, btutil);
	ofNotifyEvent(btutil->connectEvent, objectRef);
}

// Callback function create-connection
void bluetoothCreateConnection_c(void *userRefCon, IOBluetoothDeviceRef deviceRef, IOReturn status) {
	BTUtil * btutil = static_cast <BTUtil *>(userRefCon);
	// Start listening to BT disconnects
	//IOBluetoothDeviceRegisterForDisconnectNotification(objectRef, &bluetoothDisconnect_c, btutil);
	//ofNotifyEvent(btutil->connectEvent, deviceRef);
	bool success;
	success = (status == kIOReturnSuccess);
	ofNotifyEvent(btutil->createConnectionEvent, success, deviceRef);
}

// Callback for RFCOMM events
void rfcommEventListener_c (IOBluetoothRFCOMMChannelRef channel_ref, void *userRefCon, IOBluetoothRFCOMMChannelEvent *event) {
	BTUtil * btutil = static_cast <BTUtil *>(userRefCon);
	switch (event->eventType) {
		case kIOBluetoothRFCOMMNewDataEvent: // we have new client data.
			ofNotifyEvent(btutil->serialDataEvent, event->u.newData, &channel_ref);
			break;
		case kIOBluetoothRFCOMMFlowControlChangedEvent:
			ofNotifyEvent(btutil->serialFlowEvent, event->u.flowStatus, &channel_ref);
			break;
		case kIOBluetoothRFCOMMChannelTerminatedEvent:
			ofNotifyEvent(btutil->serialCloseEvent, event->u.terminatedChannel);
			break;
	}
}

//-------------------------------------------------------------------
// MEMBER FUNCTIONS
//-------------------------------------------------------------------

// Show's a list of BT devices that the user can pick from
IOBluetoothObjectRef BTUtil::selectBTDevice() {
	IOBluetoothDeviceRef dev = NULL;
	IOBluetoothDeviceSelectorControllerRef device_selector = IOBluetoothGetDeviceSelectorController();	
	CFArrayRef arr = IOBluetoothDeviceSelectorRunPanelWithAttributes(device_selector, NULL);
	if (arr == NULL) { return dev; }
	void* arr_value = const_cast <void *>(CFArrayGetValueAtIndex(arr, 0));
	dev = reinterpret_cast<IOBluetoothDeviceRef>(arr_value);
	return dev;
}

bool BTUtil::connect(IOBluetoothObjectRef dev) {
	return IOBluetoothDeviceOpenConnection(dev, &bluetoothCreateConnection_c, this) == kIOReturnSuccess;
}

// Start listening to connections from devices.
void BTUtil::registerListenConnect(){
	IOBluetoothRegisterForDeviceConnectNotifications(&bluetoothConnect_c, this);
}

bool BTUtil::isConnected(IOBluetoothObjectRef dev) {
	return IOBluetoothDeviceIsConnected(dev);
}

// Establish new RFCOMM/SPP connection channel
IOBluetoothRFCOMMChannelRef BTUtil::openRFCOMMChannel(IOBluetoothObjectRef dev) {
	//printf("in openRFCOMMChannel\n");
	CFArrayRef device_services = IOBluetoothDeviceGetServices(dev);
	//printf("Getting SDP service record\n");
	IOBluetoothSDPServiceRecordRef service_record = (IOBluetoothSDPServiceRecordRef) CFArrayGetValueAtIndex(device_services, 0);
	UInt8 channel_id;
	IOBluetoothRFCOMMChannelRef	channel_ref;
	//printf("Finding channel ID\n");
	IOBluetoothSDPServiceRecordGetRFCOMMChannelID(service_record, &channel_id);
	// Open channel and listen for RFCOMM data
	IOBluetoothDeviceOpenRFCOMMChannelAsync(dev, &channel_ref, channel_id, &rfcommEventListener_c, this);
	//	printf("Releasing service record\n");
	//IOBluetoothObjectRelease(service_record);
	return channel_ref;
}

bool BTUtil::sendRFCOMMData(IOBluetoothRFCOMMChannelRef channel_ref, void* buf, UInt32 length) {
	return (IOBluetoothRFCOMMChannelWriteAsync(channel_ref, buf, length, this) == kIOReturnSuccess); 
}

// Close the channel..
void BTUtil::closeChannel(IOBluetoothRFCOMMChannelRef channel_ref) {
	IOBluetoothRFCOMMChannelCloseChannel(channel_ref);
	IOBluetoothObjectRelease(channel_ref);
}

// and BT connection.
void BTUtil::closeConnection(IOBluetoothObjectRef dev) {
	IOBluetoothDeviceCloseConnection(dev);
}

// Get the name for the device
char *BTUtil::deviceName(IOBluetoothObjectRef dev) {
	CFStringRef dev_name = IOBluetoothDeviceGetName(dev);
	char *buf = const_cast<char *>(CFStringGetCStringPtr(dev_name, kCFStringEncodingMacRoman));
	return buf;
}

char *BTUtil::deviceAddress(IOBluetoothObjectRef dev) {
	CFStringRef dev_address = IOBluetoothDeviceGetAddressString(dev);
	char *buf = const_cast<char *>(CFStringGetCStringPtr(dev_address, kCFStringEncodingMacRoman));
	return buf;	
}

// Returns a Bluetooth device object for the c string address.
IOBluetoothObjectRef BTUtil::deviceForAddress(const char *address) {
	CFStringRef btAddressString = CFStringCreateWithCString(kCFAllocatorDefault, address, kCFStringEncodingUTF8);
	BluetoothDeviceAddress *btAddressPtr;
	IOBluetoothCFStringToDeviceAddress(btAddressString, btAddressPtr);
	CFRelease(btAddressString);
	IOBluetoothDeviceRef device = IOBluetoothDeviceCreateWithAddress(btAddressPtr);
}

