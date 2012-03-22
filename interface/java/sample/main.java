import uk.co.attie.libxbee.*;
import java.lang.*;

class main {
	static libxbee xbee;

	public static void main(String[] args) {
		try {
			xbee = new libxbee("xbee1", "/dev/ttyUSB0", 57600);
		} catch (libxbee_err ret) {
			System.out.println("Error = " + ret.ret);
		}
	}
	
	static {
		System.loadLibrary("xbee_java");
	}
}
