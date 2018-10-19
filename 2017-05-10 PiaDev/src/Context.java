import java.util.LinkedList;

public class Context extends LinkedList<CEmotional> {
	protected Context() {
		super();
	}
	protected Context(Context context) {
		super();
		for(CEmotional ce : context) {
			if(ce instanceof CVisual) super.add(new CVisual((CVisual) ce));
			else if(ce instanceof CTactile) super.add(new CTactile((CTactile) ce));
			else super.add(new CEmotional(ce));
		}
	}
	protected Context(
		JSON json,
		LinkedList<Color> lColor,
		LinkedList<Size> lSize,
		LinkedList<Texture> lTexture
	) {
		super();
		for(JSON j : json.elements)
			if(j.elements[0].getS().isEmpty())
				this.add(new CEmotional(j));
			else if(j.elements[0].getS().equals("V"))
				this.add(new CVisual(j, lColor, lSize));
			else if(j.elements[0].getS().equals("T"))
				this.add(new CTactile(j, lTexture));
	}
	protected int getPleasure() {
		int p = 0;
		for(CEmotional ce : this) p += ce.pleasure;
		return p;
	}
	protected Context toEmotional() {
		Context context = new Context();
		for(CEmotional ce : this) context.add(ce.toEmotional());
		return context;
	}
	protected boolean isTn() {
		for(CEmotional ce : this) if(!ce.isTn()) return false;
		return true;
	}
	protected Context getCVisual() {
		Context context = new Context();
		for(CEmotional ce : this) if(ce instanceof CVisual) context.add(ce);
		return context;
	}
	protected Context getCTactile() {
		Context context = new Context();
		for(CEmotional ce : this) if(ce instanceof CTactile) context.add(ce);
		return context;
	}
	protected CEmotional getObject(CEmotional cEmotional) {
		for(CEmotional ce : this) if(ce.isSameObject(cEmotional)) return ce;
		return null;
	}
	protected void set(Context context) {
		this.clear();
		this.addAll(new Context(context));
	}
	protected boolean containsObject(CEmotional cEmotional) {
		for(CEmotional ce : this) if(cEmotional.isSameObject(ce)) return true;
		return false;
	}
	protected boolean containsObjects(Context context) {
		for(CEmotional ce1 : this) {
			boolean test = false;
			for(CEmotional ce2 : context)
				if(test = ce1.isSameObject(ce2)) break;
			if(!test) return false;
		}
		return true;
	}
	protected Context removeLost() {
		Context context = new Context();
		for(CEmotional ce : this) if(ce.pleasure != -1) context.add(ce);
		return context;
	}
	protected Context removeCVisual() {
		Context context = new Context();
		for(CEmotional ce : this) if(!(ce instanceof CVisual)) context.add(ce);
		return context;
	}
	protected Context removeCTactile() {
		Context context = new Context();
		for(CEmotional ce : this) if(!(ce instanceof CTactile)) context.add(ce);
		return context;
	}
	protected boolean removeObject(CEmotional cEmotional) {
		boolean test = false;
		for(CEmotional ce : this)
			if(ce.isSameObject(cEmotional)) test |= this.remove(ce);
		return test;
	}
	protected LinkedList<Context> differentiate(Context context) {
		LinkedList<Context> lContext = new LinkedList<Context>();
		Match m = Match.getMatch(this, context);
		for(int i = 0; i < m.getSize(); i++) {
			CEmotional ceA = m.getCEmotionalA(i);
			CEmotional ceB = m.getCEmotionalB(i);
			for(CEmotional ce : ceA.differentiate(ceB)) {
				Context c = new Context(this);
				c.remove(ceA);
				c.add(ce);
				lContext.add(c);
			}
		}
		return lContext;
	}
	protected LinkedList<Context> generalize() {
		LinkedList<Context> lContext = new LinkedList<Context>();
		for(CEmotional ce0 : this) {
			Context c0 = new Context(this);
			c0.remove(ce0);
			for(CEmotional ce1 : ce0.generalize()) {
				Context c1 = new Context(c0);
				c1.add(ce1);
				lContext.add(c1);
			}
		}
		return lContext;
	}
	protected Context merge(Context context) {
		Context c = new Context(this);
		c.addAll(context);
		return c;
	}
	public boolean equals(Object object) {
		return object instanceof Context &&
			this.containsAll((Context) object) &&
			((Context) object).containsAll(this);
	}
}
