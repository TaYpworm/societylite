package com.society;

import java.io.IOException;
import java.security.NoSuchAlgorithmException;
import java.lang.reflect.InvocationTargetException;

import org.apache.commons.cli.*;

public class Society {
	
	/**
	 * @param args
	 * @throws IOException 
	 * @throws NoSuchAlgorithmException 
	 * @throws NoSuchMethodException 
	 * @throws IllegalAccessException 
	 * @throws InstantiationException 
	 * @throws ClassNotFoundException 
	 * @throws SecurityException 
	 * @throws InvocationTargetException 
	 * @throws IllegalArgumentException 
	 */
	@SuppressWarnings("static-access")
	public static void main(String[] args) throws IOException, NoSuchAlgorithmException, SecurityException, ClassNotFoundException, InstantiationException, IllegalAccessException, NoSuchMethodException, IllegalArgumentException, InvocationTargetException {
		int TCPPortNumber = 5555;
		// default VO port
		// int VOServerPort = 4694;
		int VOServerPort = 20001;
		String VOServerHostname = "localhost";
//		int numWorkers = 3;
		
		Option help = new Option( "help", "print this message" );
		Option TCPPort = OptionBuilder.withArgName( "tcpport" )
		.hasArg()
		.withDescription( "set listen port" )
		.create( "tcpport" );
		Option VOHostname = OptionBuilder.withArgName( "vohost" )
		.hasArg()
		.withDescription( "set VO Server hostname" )
		.create( "vohost" );
		Option VOPort = OptionBuilder.withArgName( "voport" )
		.hasArg()
		.withDescription( "set VO Server port" )
		.create( "voport" );		
		
		Options options = new Options();
		options.addOption(help);
		options.addOption(TCPPort);
		options.addOption(VOHostname);
		options.addOption(VOPort);

		CommandLineParser parser = new GnuParser();
	    try {
	        // parse the command line arguments
	        CommandLine line = parser.parse( options, args );
	        if (line.hasOption("help")) {
	        	HelpFormatter formatter = new HelpFormatter();
		        formatter.printHelp( "society", options );
		        return;
	        }
	        if (line.hasOption("tcpport")) {
	        	TCPPortNumber = Integer.parseInt(line.getOptionValue("tcpport"));
	        }
	        if (line.hasOption("vohost")) {
	        	VOServerHostname = line.getOptionValue("vohost");
	        }
	        if (line.hasOption("VOPort")) {
	        	VOServerPort = Integer.parseInt(line.getOptionValue("voport"));
	        }
	    }
	    catch( ParseException exp ) {
	        System.err.println( "Parsing failed.  Reason: " + exp.getMessage() );
	    }

	    ThreadGroup societyThreads = new ThreadGroup("Society Threads");
		CommonObjects comObjs = CommonObjects.getInstance(TCPPortNumber, VOServerHostname, VOServerPort);

		PacketManager packetManager = new PacketManager(comObjs);
		StreamSocketManager streamManager = new StreamSocketManager(comObjs);

		Thread packetThread = new Thread(societyThreads, packetManager);
		Thread streamThread = new Thread(societyThreads, streamManager);
		
		packetThread.start();
		streamThread.start();
		
		try {
			packetThread.join();
			streamThread.join();
		} catch (InterruptedException e) {
			// Do something here
			e.printStackTrace();
		}
	}
}
