import java.util.LinkedList;

public class CTactile extends CEmotional {
	protected final Texture texture;
	protected final Boolean open;
	protected CTactile(Integer pleasure, Texture texture, Boolean open) {
		super(pleasure);
		this.texture = texture == null ? null : new Texture(texture);
		this.open = open == null ? null : new Boolean(open);
	}
	protected CTactile(CTactile cTactile) {
		super(cTactile.pleasure);
		this.texture = cTactile.texture == null ?
			null : new Texture(cTactile.texture);
		this.open = cTactile.open == null ?
			null : new Boolean(cTactile.open);
	}
	protected CTactile(
		JSON json, LinkedList<Texture> lTexture
	) {
		super(json);
		this.texture = json.elements[2].getS().isEmpty() ?
			null : Texture.get(lTexture, json.elements[2].getS());
		this.open = json.elements[3].getS().isEmpty() ?
			null : json.elements[3].getS().equals("handOpen");
	}
	protected CTactile toEmotional() {
		return new CTactile(super.pleasure, null, null);
	}
	protected double getInterest(Mind mind) {
		double interest = 0;
		double lTextureHits = 0;
		for(Texture texture : mind.lTexture) lTextureHits += texture.hits;
/*
		interest += Texture.distance(
			this.texture.getRGB(), java.awt.Color.black.getRGB()
		) / (255 * 3);
		interest += this.open ? 1 : 0;
		interest += lTextureHits == 0 ? 0 :
			(lTextureHits - this.texture.hits) / lTextureHits;
		return ((100 * interest) + super.getInterest(mind)) / 6;

*/

		Texture t = Texture.get(mind.lTexture, this.texture);
		interest += lTextureHits == 0 ? 0 :
			(lTextureHits - t.hits) / lTextureHits;

		//return 100 * interest;

		return (interest + super.getInterest(mind)) / 2.0;
	}
	protected int getMatch(CEmotional cEmotional) {
		if(cEmotional instanceof CTactile) {
			CTactile cTactile = (CTactile) cEmotional;
			int m = 0;
			if(super.pleasure == null || cTactile.pleasure == null ||
				cTactile.pleasure.equals(super.pleasure)) m++;
			if(this.texture == null || cTactile.texture == null ||
				cTactile.texture.equals(this.texture)) m++;
			if(this.open == null || cTactile.open == null ||
				cTactile.open.equals(this.open)) m++;
			return (100 * m) / 3;
		} else if(cEmotional instanceof CVisual) return 0;
			else return super.getMatch(cEmotional);
	}
	protected CTactile getLost() {
		return new CTactile(
			-1,
			this.texture == null ? null : new Texture(this.texture),
			this.open == null ? null : new Boolean(this.open)
		);
	}
	protected boolean isSameObject(CEmotional cEmotional) {
		return cEmotional instanceof CTactile && this.texture != null &&
			this.texture.equals(((CTactile) cEmotional).texture);
	}
	protected boolean isSameTexture(Texture texture) {
		return (this.texture == null && texture == null) ||
			(this.texture != null && this.texture.equals(texture));
	}
	protected boolean isSameOpen(Boolean open) {
		return (this.open == null && open == null) ||
			(this.open != null && this.open.equals(open));
	}
	protected LinkedList<CEmotional> differentiate(CEmotional cEmotional) {
		LinkedList<CEmotional> lCEmotional = new LinkedList<CEmotional>();
		CTactile cTactile = (CTactile) cEmotional;
		if(this.texture == null)
			lCEmotional.add(new CTactile(
				super.pleasure, cTactile.texture, this.open
			));
		if(this.open == null)
			lCEmotional.add(new CTactile(
				super.pleasure, this.texture, cTactile.open
			));
/*
		if(super.pleasure == null)
			lCEmotional.add(new CTactile(
				cTactile.pleasure, this.texture, this.open
			));
*/
		return lCEmotional;
	}
	protected LinkedList<CEmotional> generalize() {
		LinkedList<CEmotional> lCEmotional = new LinkedList<CEmotional>();
		if(this.texture != null) {
			CTactile cTactile = new CTactile(
				super.pleasure, null, this.open
			);
			lCEmotional.add(cTactile);
		}
		if(this.open != null) {
			CTactile cTactile = new CTactile(
				super.pleasure, this.texture, null
			);
			lCEmotional.add(cTactile);
		}
		return lCEmotional;
	}
	protected int getTn() {
		int t = 0;
		if(this.texture == null) t++;
		if(this.open == null) t++;
		return t;
	}
	protected boolean isTn() {
		return this.getTn() == 2;
	}
	public boolean equals(Object object) {
		if(!(object instanceof CTactile)) return false;
		CTactile cTactile = (CTactile) object;
		return super.isSamePleasure(cTactile.pleasure) &&
			this.isSameTexture(cTactile.texture) &&
			this.isSameOpen(cTactile.open);
	}
	public String toString() {
		return "[\"T\"," + (this.pleasure == null ? "" : this.pleasure) + "," +
			(this.texture == null ? "" : "\"" + this.texture.name + "\"") + "," +
			(this.open == null ? "" :
				"\"hand" + (this.open ? "Open" : "Closed") + "\"") + "]";
	}
}
