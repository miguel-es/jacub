import java.util.List;

public class Size {
	protected final String name;
	protected final double size;
	protected long hits;
	protected Size(Size size) {
		this.name = new String(size.name);
		this.size = size.size;
		this.hits = size.hits;
	}
	protected Size(JSON json) {
		this.name = json.elements[0].getS();
		this.size = json.elements[1].getD();
		this.hits = json.elements[2].getL();
	}
	static protected Size get(List<Size> sizes, String name) {
		for(Size s : sizes) if(name.equals(s.name)) return s;
		return null;
	}
	static protected Size get(List<Size> sizes, double value) {
		Size size = null;
		for(Size s : sizes) if((size == null) ||
			(Math.abs(s.size - value) < Math.abs(size.size - value))) size = s;
		return size;
	}
	public boolean equals(Object object) {
		return object != null && object instanceof Size &&
			((Size) object).size == this.size;
	}
	public String toString() {
		return "[\"" + this.name + "\"," + this.size + "," + this.hits + "]";
	}
}
