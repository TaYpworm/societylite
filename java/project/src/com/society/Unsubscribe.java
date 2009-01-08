package com.society;

import java.nio.channels.SocketChannel;

//import java.net.InetSocketAddress;

public class Unsubscribe extends PacketProcessor {

	public Unsubscribe() {
		super();
		// TODO Auto-generated constructor stub
	}

	@Override
	public void process(SocietyPacket inPacket, SocketChannel conn) {
		// TODO Auto-generated method stub
		shared.subscribers.removeSubscriber(conn);
	}

}
