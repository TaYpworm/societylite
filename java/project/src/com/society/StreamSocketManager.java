package com.society;

import java.io.IOException;
import java.net.InetSocketAddress;
import java.nio.ByteBuffer;
import java.nio.channels.SelectionKey;
import java.nio.channels.Selector;
import java.nio.channels.ServerSocketChannel;
import java.nio.channels.SocketChannel;
import java.util.Iterator;

public class StreamSocketManager implements Runnable {
	private int recvBufferSize;
	private CommonObjects shared;
	private ServerSocketChannel server;
	private Selector selector;
	private SelectionKey serverKey;
	private byte[] recvData;
	private ByteBuffer recvBuffer;
	
	StreamSocketManager(CommonObjects shared) throws IOException {
		this.shared = shared;
		selector = Selector.open();
		
		server = ServerSocketChannel.open();
		server.configureBlocking(false);
		server.socket().bind(new InetSocketAddress(shared.getTCPListenPort()));
		recvBufferSize = server.socket().getReceiveBufferSize();
		
		serverKey = server.register(selector, SelectionKey.OP_ACCEPT);
		
		recvData = new byte[recvBufferSize];
		recvBuffer = ByteBuffer.wrap(recvData);
	}

	public void run() {
		int bytesRead, numPacks=0;
		Iterator iter;
		SelectionKey key, clientKey;
		SocketChannel client;
		SocketState tmpState;
		SocietyPacket packet;

		for ( ; ; ) {
			try {
				selector.select();
				iter = selector.selectedKeys().iterator();

				while (iter.hasNext()) {
					key = (SelectionKey) iter.next();
					iter.remove();

					if (key == serverKey) {
						if (key.isAcceptable()) {
							client = server.accept();
							client.configureBlocking(false);
							clientKey = client.register(selector, SelectionKey.OP_READ);
							// add VO
							// not enough info here.  Done below.
							clientKey.attach(new SocketState(Protocol.TCP, null, null));
						}
					} else {
						if (key.isValid() && key.isReadable()) {
							client = (SocketChannel) key.channel();
							tmpState = (SocketState) key.attachment();

							// this process can be optimized for Society packets
							// unfortunately, cannot because need to accept VO and Society packets
							recvBuffer.clear();
							bytesRead = client.read(recvBuffer);

							if (bytesRead == -1) {
								// find client in tables and remove
								client.close();
								key.cancel();
							} else {
								// wrong!  If multiple packets are sent in the same buffer (small packets)
								// then this receive algorithm will break.
								recvBuffer.flip();
								recvData = new byte[recvBuffer.remaining()];
								recvBuffer.get(recvData, 0, recvData.length);

								if (!tmpState.partial) {
									tmpState.packet = new SocietyPacket(recvData.length, recvData, Protocol.TCP);
									tmpState.bytesRemaining = tmpState.packet.payloadSize - (recvData.length - SocietyPacket.headerSize);
									tmpState.setPartial();
								} else {
									tmpState.packet.appendToPayload(recvData);
									tmpState.bytesRemaining -= recvData.length;
								}

								if (tmpState.bytesRemaining == 0) {
									numPacks++;
									System.out.println("numRecv: " + numPacks);
									packet = tmpState.clearPartial();
									if (packet.packetType == PacketType.VOSubscribe.ordinal()) {
										int offset = 20;
										byte type;
										byte[] source;
										String sourceString;
										VOConnectionType connType;
										SocketChannel voClient;
										
										System.out.println("connect to VO and subscribe");
										type = packet.data[offset];
										offset++;
										source = new byte[packet.payloadSize - offset];
										System.arraycopy(packet.data, offset, source, 0, packet.payloadSize - offset);
										System.out.println("Packet Contents: " + DevelUtils.byteArrayToHexString(source));
										sourceString = new String(source);
										
										System.out.println("Type = " + type);
										System.out.println("Type2 = " + VOConnectionType.values()[(short)type]);
										System.out.println("Size " + sourceString.length() + "sourceString = " + sourceString);
										
										connType = VOConnectionType.values()[(short)type];
										voClient = createVOSocketChannel(sourceString, connType);
										
										tmpState.type = Protocol.VO;
										tmpState.sourceHash = packet.source;
										tmpState.sourceString = sourceString;
										
										if (connType == VOConnectionType.read) {
											shared.societyMessageQueue.add(Pair.create(packet, new Connection(Protocol.VO, packet.source, client)));
										} else if (connType == VOConnectionType.write) {
											shared.societyMessageQueue.add(Pair.create(packet, new Connection(Protocol.VO, packet.source, voClient)));
										}
										
									} else if (packet.packetType == PacketType.VOUnsubscribe.ordinal()){
									} else {
										shared.societyMessageQueue.add(Pair.create(packet, new Connection(Protocol.TCP, packet.source, client)));
									}
								}
							}
						} 
					} 
				}
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			} catch (PacketException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
		}
	}	

	SocketChannel createVOSocketChannel(String sourceString, VOConnectionType type) throws IOException {
		String voSubscribeString;
		ByteBuffer buffer;
		SocketChannel channel = SocketChannel.open();
		channel.configureBlocking(false);
		channel.connect(shared.getVOAddress());
		while (!channel.finishConnect()) {}
		voSubscribeString = new String("request\n" + type + "\nascii\n" + sourceString + "\n");
		System.out.println(voSubscribeString);
		buffer = ByteBuffer.wrap(voSubscribeString.getBytes());
		channel.write(buffer);
		
		return channel;
	}
}