package uk.co.attie.libxbee;

public class libxbee_err extends Exception {
	public static int ret;

	public libxbee_err(int ret) {
		this.ret = ret;
	}
}
