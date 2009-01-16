package com.society;


abstract class PacketProcessor {
	CommonObjects shared;
	
	PacketProcessor() {}
	
	public void initVariables(CommonObjects shared) {
		this.shared = shared;
	}

	abstract public void process(SocietyPacket inPacket, Connection connection);
}
