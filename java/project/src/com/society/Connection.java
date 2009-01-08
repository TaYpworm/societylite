package com.society;

import java.net.InetSocketAddress;

public abstract class Connection {
	Protocol type;
	byte[] service;
	
	Connection(Protocol type, byte[] service) {
		this.type = type;
		this.service = service;
	}
	
	Protocol getType() {
		return type;
	}
	
	byte[] getService() {
		return service;
	}
	
	abstract InetSocketAddress getAddress();
	abstract void shutdown();
}
