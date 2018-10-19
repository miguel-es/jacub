import java.util.List;

public class Color {
	protected final String name;
	private final java.awt.Color color;
	protected long hits;
	protected Color(String name, java.awt.Color color, long hits) {
		this.name = name;
		this.color = color;
		this.hits = hits;
	}
	protected Color(Color color) {
		this.name = new String(color.name);
		this.color = new java.awt.Color(color.color.getRGB());
		this.hits = hits;
	}
	protected Color(JSON json) {
		this.name = json.elements[0].getS();
		this.color = new java.awt.Color(
			json.elements[1].elements[0].getI(),
			json.elements[1].elements[1].getI(),
			json.elements[1].elements[2].getI()
		);
		this.hits = Long.parseLong(json.elements[2].getS());
	}
	protected int getRGB() {
		return this.color.getRGB();
	}
	static protected int distance(int a, int b) {
		java.awt.Color ca = new java.awt.Color(a);
		java.awt.Color cb = new java.awt.Color(b);
		return Math.abs(ca.getRed() - cb.getRed()) +
			Math.abs(ca.getGreen() - cb.getGreen()) +
			Math.abs(ca.getBlue() - cb.getBlue());
	}
	static protected Color get(List<Color> colors, String name) {
		for(Color c : colors) if(name.equals(c.name)) return c;
		return null;
	}
	static protected Color get(List<Color> colors, Color color) {
		for(Color c : colors) if(color.equals(c)) return c;
		return null;
	}
	static protected Color get(List<Color> colors, int value) {
		Color color = null;
		for(Color c : colors) if((color == null) ||
			(Color.distance(c.getRGB(), value) <
			Color.distance(color.getRGB(), value))) color = c;
		return color;
	}
	public boolean equals(Object object) {
		return object != null && object instanceof Color &&
			((Color) object).color.equals(this.color);
	}
	public String toString() {
		return "[\"" + this.name + "\",[" +
			this.color.getRed() + "," + this.color.getGreen() + "," +
			this.color.getBlue() + "]," + this.hits + "]";
	}
}
