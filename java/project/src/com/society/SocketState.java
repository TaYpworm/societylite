package com.society;

public class SocketState {
	int bytesRemaining;
	boolean partial;
	SocietyPacket packet;
	
	
	SocketState() {
		bytesRemaining = 0;
		partial = false;
		packet = null;
	}
	
	void setPartial() {
		partial = true;
	}
	
	SocietyPacket clearPartial() {
		bytesRemaining = 0;
		partial = false;
		return packet;
	}
	
	boolean isPartial() {
		return partial;
	}
	
	void setBytesRemaining(int remaining) {
		bytesRemaining = remaining;
	}
	
	int getBytesRemaining(){
		return bytesRemaining;
	}
}
