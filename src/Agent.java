import com.eteks.sweethome3d.model.Camera;
import com.eteks.sweethome3d.model.HomeEnvironment;
import com.eteks.sweethome3d.model.HomeFurnitureGroup;
import com.eteks.sweethome3d.model.HomePieceOfFurniture;
import com.eteks.sweethome3d.model.Room;
import java.awt.Graphics2D;
import java.awt.Image;
import java.awt.image.BufferedImage;
import java.util.LinkedList;
import java.util.Random;
import java.util.StringJoiner;

public class Agent {
	protected final PiaWorld piaWorld;
	private final Mind mind;
	protected final Camera camera;
	protected BufferedImage bufferedImage;
	private HomePieceOfFurniture head;
	private final HomePieceOfFurniture cHead;
	private HomePieceOfFurniture hand;
	private final HomePieceOfFurniture cHand;
	protected final LinkedList<String> lAction;
	protected boolean success;
	protected final Random random;
	private final String[] lKey = {
		"headUp", "headDown", "headLeft", "headRight", "headLeftUp",
		"headLeftDown", "headRightUp", "headRightDown", "handRight",
		"handLeft", "handUp", "handDown", "handBackwards", "handForward",
		"handClose", "handOpen"
	};
	int Hyn;
	int Han;
	int hxn;
	int hyn;
	int hzn;
	int hon;
	protected Agent(PiaWorld piaWorld) {
		JSON json = JSON.getJ("Agent");
		this.piaWorld = piaWorld;
		this.mind = new Mind(this);
		this.camera =
			new Camera(0, 0, 0, 0, 0, JSON.getF("Agent/head/vision/field"));
		this.bufferedImage = null;
		for(HomePieceOfFurniture p : piaWorld.lPiece)
			if(p.getName().equals(JSON.getS("Agent/head/name"))) {
				this.head = p;
				break;
			}
		for(HomePieceOfFurniture p : piaWorld.lPiece)
			if(p.getName().equals(JSON.getS("Agent/hand/name"))) {
				this.hand = p;
				break;
			}
		this.cHead = head.clone();
		this.cHand = hand.clone();
		this.lAction = new LinkedList<String>();
		this.success = true;
		this.random = new Random();
		this.Hyn = 0;
		this.Han = 0;
		this.hxn = 0;
		this.hyn = 0;
		this.hzn = 0;
		this.hon = 0;
	}
	protected void update() {
		/*
		this.mind.next(this.getContext());
		LinkedList<String> lAction;
		switch((int) this.piaWorld.step) {
			case 1:
				lAction = new LinkedList<String>();
				lAction.add("headUp");
				this.act(lAction);
				break;
		}
		*/
		this.act(this.mind.next(this.getContext()));
	}
	protected Context getContext() {
		Context cInterested = this.mind.cInterested.getCVisual();
		Context context = new Context();
		LinkedList<Color> lColor = new LinkedList<Color>(this.mind.lColor);
		Color color = null;
		BufferedImage bi = this.see();
		int w = bi.getWidth() - 1;
		int h = bi.getHeight() - 1;
		int vh = JSON.getI("Agent/head/vision/horizontal");
		int vv = JSON.getI("Agent/head/vision/vertical");
		int black = java.awt.Color.black.getRGB();
		int white = java.awt.Color.white.getRGB();
		int[][] rgb = new int[w + 1][h + 1];
		boolean match;
		lColor.add(new Color("c0", java.awt.Color.black, 0L));
		for(int x = 0; x <= w; x++)
			for(int y = 0; y <= h; y++) {
				rgb[x][y] = Color.get(lColor, bi.getRGB(x, y)).getRGB();
				bi.setRGB(x, y, rgb[x][y]);
			}
		for(int x = 0; x <= w; x++) {
			for(int y = 0; y <= h; y++) {
				if(rgb[x][y] != black) {
					double X = 0; double Y = 0; double S = 0;
					match = true;
					color = Color.get(this.mind.lColor, rgb[x][y]);
					for(int i = 0; match; i++) {
						match = false;
						int l = x - i; if(l < 0) l = 0;
						int r = x + i; if(r > w) r = w;
						int u = y - i; if(u < 0) u = 0;
						int d = y + i; if(d > h) d = h;
						for(int j = l; j <= r; j++) {
							if(color.getRGB() == rgb[j][u])
								{ rgb[j][u] = black; match = true; X+=j; Y+=u; S++; }
							if(color.getRGB() == rgb[j][d])
								{ rgb[j][d] = black; match = true; X+=j; Y+=d; S++; }
						}
						for(int j = u; j <= d; j++) {
							if(color.getRGB() == rgb[l][j])
								{ rgb[l][j] = black; match = true; X+=l; Y+=j; S++; }
							if(color.getRGB() == rgb[r][j])
								{ rgb[r][j] = black; match = true; X+=r; Y+=j; S++; }
						}
					}
					bi.setRGB((int) (X / S), (int) (Y / S), white);
					double relativeX = (X / S) / (double) w;
					double relativeY = (Y / S) / (double) h;
					int areaX =
						relativeX > 0 ? (int) Math.ceil(relativeX * (double) vh) : 1;
					int areaY =
						relativeY > 0 ? (int) Math.ceil(relativeY * (double) vv) : 1;
					boolean isCenter =
						(1 < areaX) && (areaX < vh) && (1 < areaY) && (areaY < vv);
					int area = areaX + (int) (vh * (areaY - 1));
					Size size = Size.get(this.mind.lSize, (S / (double) (w * h)));
					boolean moving = false;
					for(CEmotional ce : cInterested) {
						CVisual cVisual = (CVisual) ce;
						if(cVisual.isSameColor(color) && (
							!cVisual.isSameSize(size) ||
							!cVisual.isSameArea(area)
						)) moving = true;
					}
					context.add(
						new CVisual(isCenter ? 2 : 1, color, size, moving, area)
					);
				}
			}
		}
		Graphics2D graphics2D = bi.createGraphics();
		graphics2D.setColor(java.awt.Color.white);
		for(int i = 1; i < vh; i++)
			graphics2D.drawLine((i * w) / vh, 0, (i * w) / vh, h);
		for(int i = 1; i < vv; i++)
			graphics2D.drawLine(0, (i * h) / vv, w, (i * h) / vv);
		double f = JSON.getF("Agent/head/vision/factor");
		Image image = this.bufferedImage.getScaledInstance(
			(int) ((float) this.bufferedImage.getWidth() / f),
			(int) ((float) this.bufferedImage.getHeight() / f),
			Image.SCALE_FAST
		);
		this.bufferedImage = new BufferedImage(
			image.getWidth(null), image.getHeight(null),
			this.bufferedImage.getType()
		);
		this.bufferedImage.getGraphics().drawImage(image, 0, 0, null);
		for(Ball b : this.palpate())
			context.add(new CTactile(
				b.getSet() == null ? 1 : 2,
				Texture.get(this.mind.lTexture, b.getPiece().getColor()),
				this.hand.getHeight() == this.cHand.getHeight()
			));
		return context;
	}
	protected BufferedImage see() {
		this.camera.setX(this.head.getX());
		this.camera.setY(this.head.getY());
		this.camera.setZ(this.head.getElevation());
		this.camera.setYaw(this.head.getAngle());
		HomeEnvironment he1 = this.piaWorld.home.getEnvironment();
		HomeEnvironment he2 = this.piaWorld.home.getEnvironment().clone();
		he1.setWallsAlpha(1f);
		he1.setSkyColor(0);
		he1.setGroundColor(0);
		for(HomePieceOfFurniture p : this.piaWorld.lPiece)
			if(p.getName().indexOf("Ball") == 0) p.setShininess(0f);
				else p.setVisible(false);
		for(Room r : this.piaWorld.home.getRooms()) {
			r.setFloorVisible(false);
			r.setCeilingVisible(false);
		}
		this.hand.setVisible(true);
		this.hand.setColor(java.awt.Color.blue.getRGB());
		this.hand.setShininess(0f);
		this.piaWorld.home.setCamera(this.camera);
		float f = JSON.getF("Agent/head/vision/factor");
		try {
			BufferedImage bufferedImage =
				this.piaWorld.homeComponent3D.getOffScreenImage(
					(int) ((float) this.piaWorld.jLabel.getWidth() * f),
					(int) ((float) this.piaWorld.jLabel.getHeight() * f)
				);
			this.bufferedImage = bufferedImage;
		} catch(Exception exception) {
			this.success = false;
			System.out.println("Error:");
			System.out.println(this.piaWorld);
			System.out.println(this.piaWorld.homeComponent3D);
			System.out.println(this.piaWorld.jLabel);
			exception.printStackTrace(System.err);
		}
		he1.setWallsAlpha(0f);
		he1.setSkyColor(he2.getSkyColor());
		he1.setGroundColor(he2.getGroundColor());
		for(HomePieceOfFurniture p : this.piaWorld.lPiece)
			if(p.getName().indexOf("Ball") == 0) p.setShininess(null);
			else p.setVisible(true);
		for(Room r : this.piaWorld.home.getRooms()) {
			r.setFloorVisible(true);
			r.setCeilingVisible(true);
		}
		this.hand.setColor(null);
		this.hand.setShininess(null);
		return bufferedImage;
	}
	protected LinkedList<Ball> palpate() {
		LinkedList<Ball> lBall = new LinkedList<Ball>();
		for(Ball b : this.piaWorld.lBall) {
			if(
				Math.abs(this.hand.getX() - b.getPiece().getX()) <
					Math.max(this.hand.getWidth(), b.getPiece().getDepth()) / 2f &&
				Math.abs(this.hand.getY() - b.getPiece().getY()) <
					Math.max(this.hand.getDepth(), b.getPiece().getWidth()) / 2f &&
				Math.abs(this.hand.getElevation() - b.getPiece().getElevation()) <
					Math.max(this.hand.getHeight(), b.getPiece().getHeight()) / 2f
			) {
				lBall.add(b);
				if(this.lAction.contains("handOpen")) b.setSet(null);
				if(this.lAction.contains("handClose")) b.setSet(this.hand);
			} else b.setSet(null);
			b.update();
		}
		return lBall;
	}
	protected void act(LinkedList<String> lAction) {
		float
			Hy = JSON.getF("Agent/head/step/y"),
			Ha = JSON.getF("Agent/head/step/a"),
			hx = JSON.getF("Agent/hand/step/x"),
			hy = JSON.getF("Agent/hand/step/y"),
			hz = JSON.getF("Agent/hand/step/z"),
			ho = JSON.getF("Agent/hand/step/o");
		int
			Hyn = JSON.getI("Agent/head/step/yn"),
			Han = JSON.getI("Agent/head/step/an"),
			hxn = JSON.getI("Agent/hand/step/xn"),
			hyn = JSON.getI("Agent/hand/step/yn"),
			hzn = JSON.getI("Agent/hand/step/zn"),
			hon = JSON.getI("Agent/hand/step/on");
		boolean
			bHy = false,
			bHa = false,
			bhx = false,
			bhy = false,
			bhz = false,
			bho = false;
		this.lAction.clear();
		this.success = true;
		for(String s : lAction) {
			String a = s;
			this.success = false;
			do {
				boolean headUp = this.Hyn < Hyn && !bHy;
				boolean headDown = -this.Hyn < Hyn && !bHy;
				boolean headLeft = -this.Han < Han && !bHa;
				boolean headRight = this.Han < Han && !bHa;
				boolean handUp = this.hyn < hyn && !bhy;
				boolean handDown = -this.hyn < hyn && !bhy;
				boolean handLeft = -this.hxn < hxn && !bhx;
				boolean handRight = this.hxn < hxn && !bhx;
				boolean handOpen = this.hon > 0 && !bho;
				boolean handClose = this.hon < hon && !bho;
				if(s.equals("random"))
					a = this.lKey[this.random.nextInt(this.lKey.length)];
				if(a.equals("headUp") && headUp) {
					this.head.setElevation(
						this.cHead.getElevation() + (((float) ++this.Hyn) * Hy)
					); bHy = true; this.success = true;
				}
				if(a.equals("headDown") && headDown) {
					this.head.setElevation(
						this.cHead.getElevation() + (((float) --this.Hyn) * Hy)
					); bHy = true; this.success = true;
				}
				if(a.equals("headLeft") && headLeft) {
					this.head.setAngle(
						this.cHead.getAngle() + (((float) --this.Han) * Ha)
					); bHa = true; this.success = true;
				}
				if(a.equals("headRight") && headRight) {
					this.head.setAngle(
						this.cHead.getAngle() + (((float) ++this.Han) * Ha)
					); bHa = true; this.success = true;
				}
				if(a.equals("headLeftUp") && headLeft && headUp) {
					this.head.setAngle(
						this.cHead.getAngle() + (((float) --this.Han) * Ha)
					); bHa = true;
					this.head.setElevation(
						this.cHead.getElevation() + (((float) ++this.Hyn) * Hy)
					); bHy = true;
					this.success = true;
				}
				if(a.equals("headLeftDown") && headLeft && headDown) {
					this.head.setAngle(
						this.cHead.getAngle() + (((float) --this.Han) * Ha)
					); bHa = true;
					this.head.setElevation(
						this.cHead.getElevation() + (((float) --this.Hyn) * Hy)
					); bHy = true;
					this.success = true;
				}
				if(a.equals("headRightUp") && headRight && headUp) {
					this.head.setAngle(
						this.cHead.getAngle() + (((float) ++this.Han) * Ha)
					); bHa = true;
					this.head.setElevation(
						this.cHead.getElevation() + (((float) ++this.Hyn) * Hy)
					); bHy = true;
					this.success = true;
				}
				if(a.equals("headRightDown") && headRight && headDown) {
					this.head.setAngle(
						this.cHead.getAngle() + (((float) ++this.Han) * Ha)
					); bHa = true;
					this.head.setElevation(
						this.cHead.getElevation() + (((float) --this.Hyn) * Hy)
					); bHy = true;
					this.success = true;
				}
				if(a.equals("handUp") && handUp) {
					this.hand.setElevation(
						this.cHand.getElevation() + (((float) ++this.hyn) * hy)
					); bhy = true; this.success = true;
				}
				if(a.equals("handDown") && handDown) {
					this.hand.setElevation(
						this.cHand.getElevation() + (((float) --this.hyn) * hy)
					); bhy = true; this.success = true;
				}
				if(a.equals("handLeft") && handLeft) {
					float A = ((float) --this.hxn) * hx + (float) Math.PI;
					float ohx = this.cHead.getX() - this.cHand.getX();
					float ohy = this.cHead.getY() - this.cHand.getY();
					float rhx = this.cHead.getX() + this.rotateX(ohx, ohy, A);
					float rhy = this.cHead.getY() + this.rotateY(ohx, ohy, A);
					this.hand.setX(rhx);
					this.hand.setY(rhy);
					bhx = true;
					this.success = true;
				}
				if(a.equals("handRight") && handRight) {
					float A = ((float) ++this.hxn) * hx + (float) Math.PI;
					float ohx = this.cHead.getX() - this.cHand.getX();
					float ohy = this.cHead.getY() - this.cHand.getY();
					float rhx = this.cHead.getX() + this.rotateX(ohx, ohy, A);
					float rhy = this.cHead.getY() + this.rotateY(ohx, ohy, A);
					this.hand.setX(rhx);
					this.hand.setY(rhy);
					bhx = true;
					this.success = true;
				}
/*
				if(a.equals("handBackwards") && handBackwards) {
					this.hand.setY(
						this.cHand.getElevation() + (((float) --this.hzn) * hz)
					); bhy = true; this.success = true;
				}
				if(a.equals("handForward") && handForward) {
					this.hand.setY(
						this.cHand.getElevation() + (((float) ++this.hzn) * hz)
					); bhy = true; this.success = true;
				}
*/
				if(a.equals("handOpen") && handOpen) {
					this.hon--;
					this.hand.setHeight(
						this.cHand.getHeight()
					); bho = true; this.success = true;
				}
				if(a.equals("handClose") && handClose) {
					this.hand.setHeight(
						this.cHand.getHeight() -
						(((float) ++this.hon) * ho * this.cHand.getHeight())
					); bho = true; this.success = true;
				}
				if(this.success) this.lAction.add(a);
			} while(s.equals("random") && !this.success);
		}
	}
	private float rotateX(float x, float y, float angle) {
		return (float)
			(((double) x) * Math.cos(angle) - ((double) y) * Math.sin(angle));
	}
	private float rotateY(float x, float y, float angle) {
		return (float)
			(((double) x) * Math.sin(angle) + ((double) y) * Math.cos(angle));
	}
	public String toString() {
		return this.mind.toString();
	}
}
