import java.util.LinkedList;

public class CVisual extends CEmotional {
	protected final Color color;
	protected final Size size;
	protected final Boolean moving;
	protected final Integer area;
	protected CVisual(
		Integer pleasure, Color color, Size size, Boolean moving, Integer area
	) {
		super(pleasure);
		this.color = color == null ? null : new Color(color);
		this.size = size == null ? null : new Size(size);
		this.moving = moving == null ? null : new Boolean(moving);
		this.area = area == null ? null : new Integer(area);
	}
	protected CVisual(CVisual cVisual) {
		super(cVisual.pleasure);
		this.color = cVisual.color == null ?
			null : new Color(cVisual.color);
		this.size = cVisual.size == null ?
			null : new Size(cVisual.size);
		this.moving = cVisual.moving == null ?
			null : new Boolean(cVisual.moving);
		this.area = cVisual.area == null ?
			null : new Integer(cVisual.area);
	}
	protected CVisual(
		JSON json, LinkedList<Color> lColor, LinkedList<Size> lSize
	) {
		super(json);
		this.color = json.elements[2].getS().isEmpty() ?
			null : Color.get(lColor, json.elements[2].getS());
		this.size = json.elements[3].getS().isEmpty() ?
			null : Size.get(lSize, json.elements[3].getS());
		this.moving = json.elements[4].getS().isEmpty() ?
			null : json.elements[4].getS().equals("moving");
		this.area = json.elements[5].getS().isEmpty() ?
			null : json.elements[5].getI();
	}
	protected CVisual toEmotional() {
		return new CVisual(super.pleasure, null, null, null, null);
	}
	protected double getInterest(Mind mind) {
		double interest = 0;
		double lColorHits = 0;
		double lSizeHits = 0;
		/*
		Color c = Color.get(mind.lColor, this.color);
		for(Color color : mind.lColor) lColorHits += color.hits;
		for(Size size : mind.lSize) lSizeHits += size.hits;
		interest += this.moving ? 1 : 0;
		interest += Color.distance(
			this.color.getRGB(), java.awt.Color.black.getRGB()
		) / (255 * 3);
		*/
		interest += this.size.size;
		/*
		interest += lColorHits == 0 ? 0 :
			(lColorHits - c.hits) / lColorHits;
		interest += lSizeHits == 0 ? 0 :
			(lSizeHits - this.size.hits) / lSizeHits;
		*/
		return (interest + super.getInterest(mind)) / 2.0;
	}
	protected int getMatch(CEmotional cEmotional) {
		if(cEmotional instanceof CVisual) {
			CVisual cVisual = (CVisual) cEmotional;
			int m = 0;
			if(super.pleasure == null || cVisual.pleasure == null ||
				cVisual.pleasure.equals(super.pleasure)) m++;
			if(this.color == null || cVisual.color == null ||
				cVisual.color.equals(this.color)) m++;
			if(this.size == null || cVisual.size == null ||
				cVisual.size.equals(this.size)) m++;
			if(this.moving == null || cVisual.moving == null ||
				cVisual.moving.equals(this.moving)) m++;
			if(this.area == null || cVisual.area == null ||
				cVisual.area.equals(this.area)) m++;
			return (m * 100) / 5;
		} else if(cEmotional instanceof CTactile) return 0;
			else return super.getMatch(cEmotional);
	}
	protected CVisual getLost() {
		return new CVisual(
			-1,
			this.color == null ? null : new Color(this.color),
			this.size == null ? null : new Size(this.size),
			this.moving == null ? null : new Boolean(this.moving),
			this.area == null ? null : new Integer(this.area)
		);
	}
	protected boolean isSameObject(CEmotional cEmotional) {
		return cEmotional instanceof CVisual && this.color != null &&
			this.color.equals(((CVisual) cEmotional).color);
	}
	protected boolean isSameColor(Color color) {
		return (this.color == null && color == null) ||
			(this.color != null && this.color.equals(color));
	}
	protected boolean isSameSize(Size size) {
		return (this.size == null && size == null) ||
			(this.size != null && this.size.equals(size));
	}
	protected boolean isSameMoving(Boolean moving) {
		return (this.moving == null && moving == null) ||
			(this.moving != null && this.moving.equals(moving));
	}
	protected boolean isSameArea(Integer area) {
		return (this.area == null && area == null) ||
			(this.area != null && this.area.equals(area));
	}
	protected LinkedList<CEmotional> differentiate(CEmotional cEmotional) {
		LinkedList<CEmotional> lCEmotional = new LinkedList<CEmotional>();
		CVisual cVisual = (CVisual) cEmotional;
		if(this.area == null)
			lCEmotional.add(new CVisual(
				super.pleasure, this.color, this.size, this.moving, cVisual.area
			));
		if(this.color == null)
			lCEmotional.add(new CVisual(
				super.pleasure, cVisual.color, this.size, this.moving, this.area
			));
		if(this.size == null)
			lCEmotional.add(new CVisual(
				super.pleasure, this.color, cVisual.size, this.moving, this.area
			));
		if(this.moving == null)
			lCEmotional.add(new CVisual(
				super.pleasure, this.color, this.size, cVisual.moving, this.area
			));
/*
		if(super.pleasure == null)
			lCEmotional.add(new CVisual(
				cVisual.pleasure, this.color, this.size, this.moving, this.area
			));
*/
		return lCEmotional;
	}
	protected LinkedList<CEmotional> generalize() {
		LinkedList<CEmotional> lCEmotional = new LinkedList<CEmotional>();
		if(this.color != null) {
			CVisual cVisual = new CVisual(
				super.pleasure, null, this.size, this.moving, this.area
			);
			lCEmotional.add(cVisual);
		}
		if(this.size != null) {
			CVisual cVisual = new CVisual(
				super.pleasure, this.color, null, this.moving, this.area
			);
			lCEmotional.add(cVisual);
		}
		if(this.moving != null) {
			CVisual cVisual = new CVisual(
				super.pleasure, this.color, this.size, null, this.area
			);
			lCEmotional.add(cVisual);
		}
		if(this.area != null) {
			CVisual cVisual = new CVisual(
				super.pleasure, this.color, this.size, this.moving, null
			);
			lCEmotional.add(cVisual);
		}
		return lCEmotional;
	}
	protected int getTn() {
		int t = 0;
		if(this.color == null) t++;
		if(this.size == null) t++;
		if(this.moving == null) t++;
		if(this.area == null) t++;
		return t;
	}
	protected boolean isTn() {
		return this.getTn() == 4;
	}
	public boolean equals(Object object) {
		if(!(object instanceof CVisual)) return false;
		CVisual cVisual = (CVisual) object;
		return super.isSamePleasure(cVisual.pleasure) &&
			this.isSameColor(cVisual.color) &&
			this.isSameSize(cVisual.size) &&
			this.isSameMoving(cVisual.moving) &&
			this.isSameArea(cVisual.area);
	}
	public String toString() {
		return "[\"V\"," + (this.pleasure == null ? "" : this.pleasure) + "," +
			(this.color == null ? "" : "\"" + this.color.name + "\"") + "," +
			(this.size == null ? "" : "\"" + this.size.name + "\"") + "," +
			(this.moving == null ? "" :
				"\"" + (this.moving ? "moving" : "static") + "\"") + "," +
			(this.area == null ? "" : this.area) + "]";
	}
}
