package com.society;

public class SocketState {
	int bytesRemaining;
	boolean partial;
	byte[] sourceHash;
	String sourceString;
	SocietyPacket packet;
	Protocol type;
	
	
	SocketState(Protocol type, byte[] sourceHash, String sourceString) {
		this.bytesRemaining = 0;
		this.partial = false;
		this.packet = null;
		this.sourceHash = sourceHash;
		this.sourceString = sourceString;
		this.type = type;
	}
	
	void setPartial() {
		partial = true;
	}
	
	SocietyPacket clearPartial() {
		bytesRemaining = 0;
		partial = false;
		return packet;
	}
}
