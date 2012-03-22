package uk.co.attie.libxbee;

public class libxbee {
	static {
		System.loadLibrary("xbee_java");
	}
	
	private static long xbee_handle;
	private native int xbee_setup(Long xbee_handle); //, String mode, Object ... arguments);
	
	
	public libxbee(String mode, Object ... arguments) throws libxbee_err {
		int ret;
		Long hnd = new Long(0);

		//if ((ret = xbee_setup(xbee_handle, mode, arguments)) != 0) throw new libxbee_err(ret);
		if ((ret = xbee_setup(hnd)) != 0) throw new libxbee_err(ret);

		System.out.println("hnd:         " + hnd.longValue());
		System.out.println("xbee_handle: " + xbee_handle);
	}

	public void finalize() {
		System.out.println("finalize");
	}
	public void close() {
		System.out.println("close");
	}
}
