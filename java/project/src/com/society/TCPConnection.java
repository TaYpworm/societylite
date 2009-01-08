package com.society;

import java.io.IOException;
import java.net.InetSocketAddress;
import java.nio.channels.SocketChannel;

public class TCPConnection extends Connection {
	SocketChannel channel;
	
	TCPConnection(byte[] service, SocketChannel channel) {
		super(Protocol.TCP, service);
		this.channel = channel;
		// TODO Auto-generated constructor stub
	}
	
	InetSocketAddress getAddress() {
		return new InetSocketAddress(channel.socket().getInetAddress(), channel.socket().getPort());
	}

	void shutdown() {
		try {
			channel.close();
			channel = null;
		} catch (IOException e) {
			// TODO Auto-generated catch block
		}
	}
	
	static TCPConnection create(byte[] service, SocketChannel channel) {
		return new TCPConnection(service, channel);
	}
}
