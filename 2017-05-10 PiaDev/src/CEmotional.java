import java.util.LinkedList;

public class CEmotional {
	protected final Integer pleasure;
	protected CEmotional(Integer pleasure) {
		this.pleasure = pleasure;
	}
	protected CEmotional(CEmotional cEmotional) {
		this.pleasure = cEmotional.pleasure;
	}
	protected CEmotional(JSON json) {
		this.pleasure = json.elements[1].getI();
	}
	protected CEmotional toEmotional() {
		return new CEmotional(this.pleasure);
	}
	protected double getInterest(Mind mind) {
		return (this.pleasure + 1.0) / 3.0;
	}
	protected int getMatch(CEmotional cEmotional) {
		if(cEmotional instanceof CVisual) {
			return this.isSamePleasure(cEmotional.pleasure) ? 100 : (100 * 4) / 5;
		} else if(cEmotional instanceof CTactile) {
			return this.isSamePleasure(cEmotional.pleasure) ? 100 : (100 * 2) / 3;
		} else return this.isSamePleasure(cEmotional.pleasure) ? 100 : 0;
	}
	protected CEmotional getLost() {
		return new CEmotional(-1);
	}
	protected boolean isSameObject(CEmotional cEmotional) {
		return false;
	}
	protected boolean isSamePleasure(Integer pleasure) {
		return (this.pleasure == null && pleasure == null) ||
			(this.pleasure != null && this.pleasure.equals(pleasure));
	}
	protected LinkedList<CEmotional> differentiate(CEmotional cEmotional) {
		return new LinkedList<CEmotional>();
	}
	protected LinkedList<CEmotional> generalize() {
		LinkedList<CEmotional> lCEmotional = new LinkedList<CEmotional>();
		lCEmotional.add(new CEmotional(this));
		return lCEmotional;
	}
	protected int getTn() {
		if(this instanceof CVisual) return 4;
		if(this instanceof CTactile) return 2;
		return 0;
	}
	protected boolean isTn() {
		return true;
	}
	public boolean equals(Object object) {
		if(!(object instanceof CEmotional)) return false;
		CEmotional cEmotional = (CEmotional) object;
		return object != null && object instanceof CEmotional && (
			(this.pleasure == null && cEmotional.pleasure == null) ||
			(this.pleasure != null && this.pleasure.equals(cEmotional.pleasure))
		);
	}
	public String toString() {
		return "[," + (this.pleasure == null ? "" : this.pleasure) + "]";
	}
}
