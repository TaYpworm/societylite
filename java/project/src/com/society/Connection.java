package com.society;

import java.io.IOException;
import java.net.InetAddress;
import java.nio.channels.SocketChannel;

public class Connection {
	Protocol type;
	byte[] service;
	SocketChannel channel;
	
	Connection(Protocol type, byte[] service, SocketChannel channel) {
		this.type = type;
		this.service = service;
		this.channel = channel;
	}
	
	Protocol getType() {
		return type;
	}
	
	byte[] getService() {
		return service;
	}
	
	SocketChannel getChannel() {
		return channel;
	}
	
	InetAddress getAddress() {
		return channel.socket().getInetAddress();
	}
	void shutdown() throws IOException {
		channel.close();
	}
}
