package com.society;

public class ServiceExistsException extends AddressException {
	private static final long serialVersionUID = 1L;

	ServiceExistsException() {
	}
	
	ServiceExistsException(String message) {
		super(message);
	}
}
