package com.society;

import java.nio.channels.SocketChannel;

public class Subscribe extends PacketProcessor {

	public Subscribe() {
		// TODO Auto-generated constructor stub
		super();
	}

	@Override
	public void process(SocietyPacket inPacket, SocketChannel conn) {
		byte[] service = new byte[20];
		
		if (inPacket.payloadSize % 20 == 0) {
			System.arraycopy(inPacket.data, 0, service, 0, 20);
				shared.subscribers.putSubscriber(service, conn);
				System.out.println("numSubscribers " + shared.subscribers.getSubscribers(service).size());
		} else {
			System.err.println("Subscription packet size error");
		}
		inPacket = null;
	}
}
