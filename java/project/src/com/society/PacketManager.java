package com.society;

import java.io.IOException;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.*;
import java.security.NoSuchAlgorithmException;

public class PacketManager implements Runnable {
	private CommonObjects shared;
	private EnumMap<PacketType, Pair<PacketProcessor, Method>> packetProcessors;
	
	PacketManager(CommonObjects shared) throws NoSuchAlgorithmException, IOException, SecurityException, ClassNotFoundException, InstantiationException, IllegalAccessException, NoSuchMethodException, IllegalArgumentException, InvocationTargetException {
		this.shared = shared;
		packetProcessors = new EnumMap<PacketType, Pair<PacketProcessor,Method>>(PacketType.class);
		initPacketProcessors();
	}

	@SuppressWarnings("unchecked")
	private void initPacketProcessors() throws ClassNotFoundException, InstantiationException, IllegalAccessException, SecurityException, NoSuchMethodException, IllegalArgumentException, InvocationTargetException {
		Class processParams[] = { SocietyPacket.class, Connection.class };
		Class initParams[] = { CommonObjects.class };
		Object initArgs[] = { shared };
		for (int i = 0; i < PacketType.values().length; i++) {
			Class tmp = Class.forName("com.society." + PacketType.values()[i].toString());
			packetProcessors.put(PacketType.values()[i], 
					Pair.create((PacketProcessor) tmp.newInstance(), 
							tmp.getMethod("process", processParams)));
			Method initVars = tmp.getMethod("initVariables", initParams);
			initVars.invoke(packetProcessors.get(PacketType.values()[i]).getLeft(), initArgs);
		}
	}
	
	public void run() {
		Pair<SocietyPacket, Connection> packet;
		Pair<PacketProcessor, Method> packetProc;
		Object[] args = new Object[2];
		for ( ; ; ) {
			try {
				packet = shared.societyMessageQueue.take();
				packetProc = packetProcessors.get(PacketType.values()[packet.getLeft().packetType]);
				args[0] = packet.getLeft();
				args[1] = packet.getRight();
				packetProc.getRight().invoke(packetProc.getLeft(), args);
			} catch (InterruptedException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			} catch (IllegalArgumentException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			} catch (IllegalAccessException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			} catch (InvocationTargetException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
		}
	}
}