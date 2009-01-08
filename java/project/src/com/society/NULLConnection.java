package com.society;

import java.net.InetSocketAddress;

public class NULLConnection extends Connection {
	
	NULLConnection(byte[] service) {
		super(Protocol.NULL, service);
	}

	@Override
	InetSocketAddress getAddress() {
		// TODO Auto-generated method stub
		return null;
	}
	
	void shutdown() {
//		Nothing to be done
	}

	static NULLConnection create(byte[] service) {
		return new NULLConnection(service);
	}
}