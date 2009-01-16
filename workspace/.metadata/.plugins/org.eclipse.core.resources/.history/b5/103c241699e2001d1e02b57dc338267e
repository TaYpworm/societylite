package com.society;

import java.util.ArrayList;
import java.util.HashMap;

public class BackboneHash {
	private HashMap<ByteArray, ArrayList<Connection>> backboneHash;
	
	BackboneHash() {
		backboneHash = new HashMap<ByteArray, ArrayList<Connection>>();
	}
	
	synchronized void putBackbone(byte[] source, Connection connection) {
		ByteArray src = ByteArray.create(source);
		if (backboneHash.containsKey(src)) {
			backboneHash.get(src).add(connection);
		} else {
			ArrayList<Connection> value = new ArrayList<Connection>();
			value.add(connection);
			backboneHash.put(src, value);
		}
	}
	
	synchronized ArrayList<Connection> getBackbones(byte[] source) {
		return backboneHash.get(ByteArray.create(source));
	}
	
	synchronized int removeBackbones(byte[] source) {
		ArrayList<Connection> connects = backboneHash.remove(ByteArray.create(source));
		if (connects != null) {
			for (Connection conn : connects) {
				conn.shutdown();
			}
			return connects.size();
		}
		return 0;
	}
	 
	int numBackbones(byte[] source) {
		if (backboneHash.containsKey(ByteArray.create(source)))
			return backboneHash.get(ByteArray.create(source)).size();
		return 0;
	}
	
	int numServices() {
		return backboneHash.size();
	}
}
