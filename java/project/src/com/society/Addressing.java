package com.society;

import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;

public class Addressing {
	private MessageDigest md;
	
	Addressing() throws NoSuchAlgorithmException {
		md = MessageDigest.getInstance("SHA-1");
	}
	
	byte[] digest(String service) {
		byte[] toDigest = service.getBytes();
		md.update(toDigest);
		byte[] digest = md.digest();
		md.reset();
		return digest;
	}
}
