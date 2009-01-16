package com.society;

import java.net.InetSocketAddress;
import java.util.concurrent.PriorityBlockingQueue;

public class CommonObjects {
	private static CommonObjects instance = null;
	private int TCPListenPort;
	private boolean terminateFlag;
	private boolean initialized = false;
	private InetSocketAddress VOAddress; 
	
	SubscriberHash subscribers;
	
	PriorityBlockingQueue<Pair<SocietyPacket, Connection>> societyMessageQueue;
	
	protected CommonObjects() {}
	
	public static CommonObjects getInstance() {
		if (instance == null) {
			instance = new CommonObjects();
		}
		return instance;
	}
	
	public static CommonObjects getInstance(int TCPListenPort, String VOServerHostname, int VOServerPort) {
		CommonObjects tmp = getInstance();
		tmp.initialize(TCPListenPort, VOServerHostname, VOServerPort);
		return tmp;
	}
	
	void initialize(int TCPListenPort, String VOServerHostname, int VOServerPort) {
		if (!initialized) {
			this.TCPListenPort = TCPListenPort;
			this.VOAddress = new InetSocketAddress(VOServerHostname, VOServerPort);
			terminateFlag = false;
			subscribers = new SubscriberHash();
			societyMessageQueue = new PriorityBlockingQueue<Pair<SocietyPacket, Connection>>();
			initialized = true;
		}
	}
	
	int getTCPListenPort() {
		return TCPListenPort;
	}
	
	InetSocketAddress getVOAddress() {
		return VOAddress;
	}
	
	boolean getTerminateFlag() {
		return terminateFlag;
	}
	
	void setTerminateFlag(boolean terminateFlag) {
		this.terminateFlag = terminateFlag;
	}
}
