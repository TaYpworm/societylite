package com.society;

public class Pair<L, R> implements Comparable {

	private final L left;
	private final R right;

	R getRight() {
		return right;
	}

	L getLeft() {
		return left;
	}

	Pair(final L left, final R right) {
		this.left = left;
		this.right = right;
	}

	static <A, B> Pair<A, B> create(A left, B right) {
		return new Pair<A, B>(left, right);
	}

	public final boolean equals(Object o) {
		if (!(o instanceof Pair))
			return false;

		final Pair<?, ?> other = (Pair) o;
		return equal(getLeft(), other.getLeft()) && equal(getRight(), other.getRight());
	}

	static final boolean equal(Object o1, Object o2) {
		if (o1 == null) {
			return o2 == null;
		}
		return o1.equals(o2);
	}

	public int hashCode() {
		int hLeft = getLeft() == null ? 0 : getLeft().hashCode();
		int hRight = getRight() == null ? 0 : getRight().hashCode();

		return hLeft + (57 * hRight);
	}

	public int compareTo(Object arg0) {
		// TODO Auto-generated method stub
		return 0;
	}

}
