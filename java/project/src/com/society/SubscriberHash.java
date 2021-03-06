package com.society;

import java.nio.channels.SocketChannel;
import java.util.ArrayList;
import java.util.HashMap;

public class SubscriberHash {
	private HashMap<ByteArray, ArrayList<SocketChannel>> subscriberHash;
	private HashMap<SocketChannel, ArrayList<ByteArray>> removalHash;
	
	SubscriberHash() {
		subscriberHash = new HashMap<ByteArray, ArrayList<SocketChannel>>();
		removalHash = new HashMap<SocketChannel, ArrayList<ByteArray>>();
	}
	
	@SuppressWarnings("unchecked")
	synchronized void putSubscriber(byte[] source, SocketChannel sub) {
		// check to see if subscriber is already inserted
		ByteArray src = ByteArray.create(source);
		if (!subscriberHash.containsKey(src)) {
			ArrayList value = new ArrayList<SocketChannel>();
			value.add(sub);
			subscriberHash.put(src, value);
		} else {
			subscriberHash.get(src).add(sub);
		}
		if (!removalHash.containsKey(sub)) {
			ArrayList<ByteArray> tmp = new ArrayList<ByteArray>();
			tmp.add(src);
			removalHash.put(sub, tmp);
		} else {
			removalHash.get(sub).add(src);
		}
	}
	
//	synchronized int putSubscriber(Collection<byte[]> sources, Connection sub) throws NoSuchAlgorithmException, UnknownHostException {
//		int numPut = 0;
//		for (byte[] source : sources) {
//			// try + catch here
//			putSubscriber(source, sub);
//			numPut++;
//		}
//		return numPut;
//	}
	
	@SuppressWarnings("unchecked")
	synchronized private ArrayList<SocketChannel> getConnections(byte[] source) {
		ByteArray digest = ByteArray.create(source);
		if (subscriberHash.containsKey(digest)) {
			return subscriberHash.get(digest);
		}
		return null;
	}
	
//	synchronized ArrayList<SocketChannel> getTCPConnections(byte[] source) throws UnknownHostException, NoSuchAlgorithmException {
//		return getConnections(source, Protocol.TCP);
//	}
	
	@SuppressWarnings("unchecked")
	synchronized ArrayList<SocketChannel> getSubscribers(byte[] source) {
		return getConnections(source);
	}
	
	synchronized int removeSubscriber(SocketChannel sub) {
		int numRemoved = 0;
		ArrayList<ByteArray> removeFrom = removalHash.remove(sub);
		if (removeFrom != null) {
			ArrayList<SocketChannel> connects;
			for (ByteArray rem : removeFrom) {
				connects = getConnections(rem.getArray());
				while(connects.remove(sub)) {
					numRemoved++;
				}
			}
		}
		return numRemoved;
	}
	
	synchronized int numSubscribers(byte[] service) {
		return getConnections(service).size();
	}
	
	synchronized int numServices() {
		return subscriberHash.size();
	}
}
