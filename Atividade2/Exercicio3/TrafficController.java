import java.util.concurrent.Semaphore;
public class TrafficController {
    private Semaphore s;

    public TrafficController(Semaphore sem){
        this.s = sem;
    }

    public void enterLeft() {
        try{
            s.acquire();
        }catch (InterruptedException e) { System.out.println("Excecao");}
    }
    public void enterRight() {
        try{
            s.acquire();
        }catch (InterruptedException e) { System.out.println("Excecao");}
    }
    public void leaveLeft() {
        s.release();
    }
    public void leaveRight() {
        s.release();
    }
}