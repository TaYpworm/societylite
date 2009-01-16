package com.society;

import java.io.IOException;
//import java.net.InetSocketAddress;
//import java.net.UnknownHostException;
//import java.security.NoSuchAlgorithmException;
import java.util.ArrayList;
import java.nio.ByteBuffer;
import java.nio.channels.SocketChannel;

public class Publish extends PacketProcessor {
	
	public Publish() {
		// TODO Auto-generated constructor stub
		super();
	}

	@Override
	public void process(SocietyPacket inPacket, Connection conn) {
		int sendBufferSize;
		byte[] sendData;
		ByteBuffer sendBuffer;
		ArrayList<Connection> subscribers;
		SocketChannel tmpChannel;
		
		subscribers = shared.subscribers.getSubscribers(inPacket.source);
		if (subscribers != null) {
			try {
				sendData = inPacket.serialize();
				sendBuffer = ByteBuffer.wrap(sendData);
				for (Connection sub : subscribers) {
					tmpChannel = sub.getChannel();
					if (tmpChannel.isOpen()) {
						sendBuffer.clear();
						sendBufferSize = tmpChannel.socket().getSendBufferSize();
						
						if (sendBuffer.capacity() > sendBufferSize) {
							sendBuffer.limit(sendBufferSize);
						}						
						while (sendBuffer.position() != sendBuffer.capacity()) {
//							could send less bytes than buffer size
							tmpChannel.write(sendBuffer);
							
							if ((sendBuffer.limit() + sendBufferSize) <= sendBuffer.capacity()) {
								sendBuffer.limit(sendBuffer.limit() + sendBufferSize);
							} else {
								sendBuffer.limit(sendBuffer.capacity());
							}
						}

					} else {
						shared.subscribers.removeSubscriber(sub);
					}
				}
			} catch (IOException e) {
				e.printStackTrace();
			}
		} else {
			System.out.println("no subscribers");
		}
		inPacket = null;
	}
}
