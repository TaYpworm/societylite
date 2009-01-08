package com.society;
import java.util.Arrays;


public class ByteArray {
	private byte[] array;
	
	ByteArray(byte[] inArray) {
		array = inArray;
	}
	
	byte[] getArray() {
		return array;
	}

	@Override
	public int hashCode() {
		final int PRIME = 31;
		int result = 1;
		result = PRIME * result + Arrays.hashCode(array);
		return result;
	}
	
	static ByteArray create(byte[] inArray) {
		return new ByteArray(inArray);
	}
	
	@Override
	public boolean equals(Object obj) {
		if (this == obj)
			return true;
		if (obj == null)
			return false;
		if (getClass() != obj.getClass())
			return false;
		final ByteArray other = (ByteArray) obj;
		if (!Arrays.equals(array, other.array))
			return false;
		return true;
	}
	
}
