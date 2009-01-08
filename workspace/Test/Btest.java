import java.nio.ByteBuffer;


public class Btest {

	/**
	 * @param args
	 */
	public static void main(String[] args) {
		ByteBuffer blah = ByteBuffer.allocate(5);
		blah.limit(8);
		System.out.println("limit: " + blah.limit());
	}

}
