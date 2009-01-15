package com.society;

import java.io.IOException;
import java.nio.channels.SocketChannel;
//import java.net.InetAddress;
//import java.net.MulticastSocket;
import java.security.NoSuchAlgorithmException;
import java.util.ArrayList;
import java.util.concurrent.PriorityBlockingQueue;

public class CommonObjects {
	private static CommonObjects instance = null;
	private int TCPListenPort;
	private boolean terminateFlag;
	private boolean initialized = false;
	
	SubscriberHash subscribers;
	
//	get rid of connection
	PriorityBlockingQueue<Pair<SocietyPacket, SocketChannel>> societyMessageQueue;
	ArrayList<SocketChannel> activeConnections;
	
	protected CommonObjects() {}
	
	public static CommonObjects getInstance() {
		if (instance == null) {
			instance = new CommonObjects();
		}
		return instance;
	}
	
	void initialize(int TCPListenPort) throws NoSuchAlgorithmException, IOException {
		if (!initialized) {
			this.TCPListenPort = TCPListenPort;
			terminateFlag = false;
			subscribers = new SubscriberHash();
			societyMessageQueue = new PriorityBlockingQueue<Pair<SocietyPacket, SocketChannel>>();
			activeConnections = new ArrayList<SocketChannel>();
			initialized = true;
		}
	}
	
	int getTCPListenPort() {
		return TCPListenPort;
	}
	
	boolean getTerminateFlag() {
		return terminateFlag;
	}
	
	void setTerminateFlag(boolean terminateFlag) {
		this.terminateFlag = terminateFlag;
	}
}
