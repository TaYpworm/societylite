package com.society;


public class Unsubscribe extends PacketProcessor {

	public Unsubscribe() {
		super();
		// TODO Auto-generated constructor stub
	}

	@Override
	public void process(SocietyPacket inPacket, Connection conn) {
		// TODO Auto-generated method stub
		shared.subscribers.removeSubscriber(conn);
	}

}
