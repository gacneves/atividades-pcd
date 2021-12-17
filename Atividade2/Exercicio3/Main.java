import java.util.concurrent.Semaphore;
public class Main {

    private static void nap(int ms) {
	try {
	    Thread.sleep(ms);
	} catch (InterruptedException e) {}
    }


    public static void main(String[] a) {
	Semaphore s = new Semaphore(1, true);
	final CarWindow win = new CarWindow(s);

	win.pack();
	win.setVisible(true);

	new Thread(new Runnable() {
		public void run() {
		    while (true) {
			nap(25);
			win.repaint();
		    }
		}
	    }).start();



    }

}
