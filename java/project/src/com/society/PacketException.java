package com.society;

public class PacketException extends Exception {
	private static final long serialVersionUID = 1L;

	PacketException() {
	}
	
	PacketException(String message) {
		super(message);
	}
}
