package com.society;

public class AddressException extends Exception {
	private static final long serialVersionUID = 1L;

	AddressException() {
	}
	
	AddressException(String message) {
		super(message);
	}
}
