import com.eteks.sweethome3d.model.HomePieceOfFurniture;
import java.util.StringJoiner;

public class Ball {
	private final HomePieceOfFurniture piece;
	private HomePieceOfFurniture set;
	private final float[][] movement;
	private int step;
	protected Ball(HomePieceOfFurniture piece) {
		this.piece = piece;
		this.set = null;
		JSON json = JSON.getJ("lBall/" + piece.getName());
		this.movement = new float[json.elements.length][3];
		for(int i = 0; i < this.movement.length; i++) {
			this.movement[i][0] = json.elements[i].elements[0].getF();
			this.movement[i][1] = json.elements[i].elements[1].getF();
			this.movement[i][2] = json.elements[i].elements[2].getF();
		}
		this.step = 0;
	}
	protected void update() {
//		if(this.set == null) {
			this.step = this.step % this.movement.length;
			this.piece.setX(this.movement[this.step][0]);
			this.piece.setY(this.movement[this.step][1]);
			this.piece.setElevation(this.movement[this.step][2]);
			this.step++;
//		} else {
//			this.step = 0;
//			this.piece.setX(this.set.getX());
//			this.piece.setY(this.set.getY());
//			this.piece.setElevation(this.set.getElevation());
//		}
	}
	protected HomePieceOfFurniture getPiece() {
		return this.piece;
	}
	protected HomePieceOfFurniture getSet() {
		return this.set;
	}
	protected void setSet(HomePieceOfFurniture piece) {
		this.set = piece;
	}
	public String toString() {
		StringJoiner sj = new StringJoiner(",\n", "[\n", "\n]");
		for(float[] f : this.movement)
			sj.add("\t[" + f[0] + "," + f[1] + "," + f[2] + "]");
		return "\"" + this.piece.getName() + "\":" + sj.toString();
	}
}
