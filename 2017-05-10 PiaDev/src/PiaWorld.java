import com.eteks.sweethome3d.model.Camera;
import com.eteks.sweethome3d.model.Home;
import com.eteks.sweethome3d.model.HomeFurnitureGroup;
import com.eteks.sweethome3d.model.HomePieceOfFurniture;
import com.eteks.sweethome3d.io.DefaultHomeInputStream;
import com.eteks.sweethome3d.swing.HomeComponent3D;
import java.awt.event.MouseEvent;
import java.awt.event.MouseListener;
import java.io.File;
import java.io.FileInputStream;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.LinkedList;
import java.util.StringJoiner;
import javax.imageio.ImageIO;
import javax.swing.ImageIcon;
import javax.swing.JFrame;
import javax.swing.JLabel;

public class PiaWorld extends JFrame implements Runnable, MouseListener {
	protected long time;
	protected final JLabel jLabel;
	protected final Home home;
	protected final HomeComponent3D homeComponent3D;
	private final LinkedList<Camera> lCamera;
	protected final LinkedList<HomePieceOfFurniture> lPiece;
	protected final LinkedList<Ball> lBall;
	private final Agent agent;
	protected long step;
	private PiaWorld(String path) throws Exception {
		this.time = System.currentTimeMillis();
		JSON.load(path);
		super.setTitle("PiaDev");
		super.setSize(320, 240);
		super.setLocationRelativeTo(null);
		super.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		super.add(this.jLabel = new JLabel(
			new ImageIcon(ImageIO.read(new File("files/splash.png")))
		));
		super.setExtendedState(JFrame.MAXIMIZED_BOTH);
		super.addMouseListener(this);
		super.setVisible(true);
		this.home = new DefaultHomeInputStream(
			new FileInputStream(JSON.getS("sh3d"))
		).readHome();
		this.homeComponent3D = new HomeComponent3D(this.home);
		this.lPiece =
			new LinkedList<HomePieceOfFurniture>(this.home.getFurniture());
		this.lBall = new LinkedList<Ball>();
		for(int i = 0; i < this.lPiece.size(); i++) {
			HomePieceOfFurniture p = this.lPiece.get(i);
			if(p.getName().indexOf("Ball") == 0) this.lBall.add(new Ball(p));
			if(p instanceof HomeFurnitureGroup)
				this.lPiece.addAll(((HomeFurnitureGroup) p).getFurniture());
		}
		this.agent = new Agent(this);
		this.lCamera = new LinkedList<Camera>();
		this.lCamera.add(null);
		this.lCamera.add(this.agent.camera);
		for(JSON j : JSON.getJ("lCamera").elements)
			this.lCamera.add(new Camera(
				j.get("x").value.getF(),
				j.get("y").value.getF(),
				j.get("z").value.getF(),
				j.get("yaw").value.getF(),
				j.get("pitch").value.getF(),
				j.get("fieldOfView").value.getF()
			));
		this.step = 0L;
		new Thread(this).start();
	}
	public void run() {
		long latency = JSON.getL("latency");
		boolean photos = JSON.getB("photos");
		if(this.step == 0L) while(true) try {
			this.step++;
			for(Ball b : this.lBall) b.update();
			this.agent.update();
			this.home.setCamera(this.lCamera.peek());
			((ImageIcon) this.jLabel.getIcon()).setImage(
				this.lCamera.peek() == null ?
				this.agent.bufferedImage :
				this.homeComponent3D.getOffScreenImage(
					this.jLabel.getWidth(), this.jLabel.getHeight()
				)
			);
			if(photos)
				ImageIO.write(
					this.agent.bufferedImage,
					"jpg",
					new File("img/" + this.step + ".jpg")
				);
			Files.write(
				Paths.get(this.time + ".json"),
				this.toString().getBytes()
			);
			this.repaint();
			Thread.sleep(latency);
		} catch(Exception exception) {
			exception.printStackTrace(System.err);
			break;
		}
	}
	public void mouseExited(MouseEvent mouseEvent) {}
	public void mouseEntered(MouseEvent mouseEvent) {}
	public void mouseReleased(MouseEvent mouseEvent) {}
	public void mousePressed(MouseEvent mouseEvent) {}
	public void mouseClicked(MouseEvent mouseEvent) {
		this.lCamera.offer(this.lCamera.poll());
	}
	public String toString() {
		StringJoiner sj1 = new StringJoiner(",\n\t", "{\n\t", "\n}");
		StringJoiner sj2 = null;
		sj1.add("\"sh3d\":\"" + JSON.getS("sh3d") + "\"");
		sj1.add("\"latency\":" + JSON.getS("latency"));
		sj1.add("\"photos\":" + JSON.getS("photos"));
			sj2 = new StringJoiner(",\n\t\t", "[\n\t\t", "\n\t]");
			for(Camera c : this.lCamera)
				if(c != null && !c.equals(this.agent.camera))
					sj2.add(
						"{\"x\":" + c.getX() +
						",\"y\":" + c.getY() +
						",\"z\":" + c.getZ() +
						",\"yaw\":" + c.getYaw() +
						",\"pitch\":" + c.getPitch() +
						",\"fieldOfView\":" + c.getFieldOfView() + "}"
					);
		sj1.add("\"lCamera\":" + sj2);
			sj2 = new StringJoiner(",\n\t\t", "{\n\t\t", "\n\t}");
			for(Ball b : this.lBall)
				sj2.add(b.toString().replaceAll("\\n", "\n\t\t"));
		sj1.add("\"lBall\":" + sj2);
		sj1.add("\"Agent\":" + this.agent.toString().replaceAll("\\n", "\n\t"));
		return sj1.toString();
	}
	static public void main(String[] arguments) throws Exception {
		if(arguments.length == 1)
			Runtime.getRuntime().addShutdownHook(
				new Thread(new PiaWorld(arguments[0]))
			);
		else System.err.println("java PiaWorld Agent.json");
	}
}
