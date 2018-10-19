import java.util.List;

public class Texture {
	protected final String name;
	private final java.awt.Color texture;
	protected long hits;
	protected Texture(Texture texture) {
		this.name = new String(texture.name);
		this.texture = new java.awt.Color(texture.texture.getRGB());
		this.hits = hits;
	}
	protected Texture(JSON json) {
		this.name = json.elements[0].getS();
		this.texture = new java.awt.Color(
			json.elements[1].elements[0].getI(),
			json.elements[1].elements[1].getI(),
			json.elements[1].elements[2].getI()
		);
		this.hits = json.elements[2].getL();
	}
	protected int getRGB() {
		return this.texture.getRGB();
	}
	static protected int distance(int a, int b) {
		java.awt.Color ca = new java.awt.Color(a);
		java.awt.Color cb = new java.awt.Color(b);
		return Math.abs(ca.getRed() - cb.getRed()) +
			Math.abs(ca.getGreen() - cb.getGreen()) +
			Math.abs(ca.getBlue() - cb.getBlue());
	}
	static protected Texture get(List<Texture> textures, String name) {
		for(Texture t : textures) if(name.equals(t.name)) return t;
		return null;
	}
	static protected Texture get(List<Texture> textures, Texture texture) {
		for(Texture t : textures) if(texture.equals(t)) return t;
		return null;
	}
	static protected Texture get(List<Texture> textures, int value) {
		Texture texture = null;
		for(Texture t : textures) if((texture == null) ||
			(Texture.distance(t.getRGB(), value) <
			Texture.distance(texture.getRGB(), value))) texture = t;
		return texture;
	}
	public boolean equals(Object object) {
		return object != null && object instanceof Texture &&
			((Texture) object).texture.equals(this.texture);
	}
	public String toString() {
		return "[\"" + this.name + "\",[" +
			this.texture.getRed() + "," + this.texture.getGreen() + "," +
			this.texture.getBlue() + "]," + this.hits + "]";
	}
}
