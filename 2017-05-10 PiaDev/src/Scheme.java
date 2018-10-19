import java.util.HashMap;
import java.util.LinkedList;
import java.util.stream.Collectors;

public class Scheme implements Comparable<Scheme> {
	protected final int id;
	protected int equilibrated;
	protected final Context context;
	protected final LinkedList<String> lAction;
	protected final Context expected;
	protected final Scheme sParent;
	protected final HashMap<Context, Integer> success;
	protected final HashMap<Context, Integer> failure;
	protected final LinkedList<Scheme> lScheme;
	private static int n = 0;
	protected Scheme() {
		this.id = Scheme.n++;
		this.equilibrated = 0;
		this.context = new Context();
		this.lAction = new LinkedList<String>();
		this.expected = new Context();
		this.success = new HashMap<Context, Integer>();
		this.failure = new HashMap<Context, Integer>();
		this.lScheme = new LinkedList<Scheme>();
		this.sParent = null;
	}
	protected Scheme(
		int equilibrated,
		Context context,
		LinkedList<String> lAction,
		Context expected,
		Scheme sParent
	) {
		this.id = Scheme.n++;
		this.equilibrated = equilibrated;
		this.context = new Context(context);
		this.lAction = new LinkedList<String>(lAction);
		this.expected = new Context(expected);
		this.success = new HashMap<Context, Integer>();
		this.failure = new HashMap<Context, Integer>();
		this.lScheme = new LinkedList<Scheme>();
		this.sParent = sParent;
	}
	protected Scheme(Scheme scheme, Scheme sParent) {
		this.id = Scheme.n++;
		this.equilibrated = scheme.equilibrated;
		this.context = new Context(scheme.context);
		this.lAction = new LinkedList<String>(scheme.lAction);
		this.expected = new Context(scheme.expected);
		this.success = new HashMap<Context, Integer>();
		for(HashMap.Entry<Context, Integer> e : scheme.success.entrySet())
			this.success.put(new Context(e.getKey()), new Integer(e.getValue()));
		this.failure = new HashMap<Context, Integer>();
		for(HashMap.Entry<Context, Integer> e : scheme.failure.entrySet())
			this.failure.put(new Context(e.getKey()), new Integer(e.getValue()));
		this.lScheme = new LinkedList<Scheme>();
		for(Scheme s0 : scheme.lScheme) this.lScheme.add(new Scheme(s0, this));
		this.sParent = sParent;
	}
	protected Scheme(
		JSON json,
		LinkedList<Color> lColor,
		LinkedList<Size> lSize,
		LinkedList<Texture> lTexture,
		Scheme sParent
	) {
		JSON j;
		this.id = Scheme.n++;
		j = json.get("equilibrated");
		this.equilibrated = j == null ? 0 : j.value.getI();
		j = json.get("context");
		this.context =
			j == null ? null : new Context(j.value, lColor, lSize, lTexture);
		this.lAction = new LinkedList<String>();
		if((j = json.get("lAction")) != null)
			for(JSON k : j.value.elements) this.lAction.add(k.getS());
		j = json.get("expected");
		this.expected = j == null ?
			new Context() : new Context(j.value, lColor, lSize, lTexture);
		this.sParent = sParent;
		this.success = new HashMap<Context, Integer>();
		if((j = json.get("success")) != null)
			for(JSON k : j.value.elements)
				this.success.put(
					new Context(k.elements[0], lColor, lSize, lTexture),
					k.elements[1].getI()
				);
		this.failure = new HashMap<Context, Integer>();
		if((j = json.get("failure")) != null)
			for(JSON k : j.value.elements)
				this.failure.put(
					new Context(k.elements[0], lColor, lSize, lTexture),
					k.elements[1].getI()
				);
		this.lScheme = new LinkedList<Scheme>();
		if((j = json.get("lScheme")) != null)
			for(JSON k : j.value.elements)
				this.lScheme.add(new Scheme(k, lColor, lSize, lTexture, this));
	}
	protected int getSuccessRate() {
		double sh = (double) this.getSuccessHits();
		double fh = (double) this.getFailureHits();
		return (int) ((sh + fh) == 0d ? 0d : 100d * sh / (sh + fh));
	}
	protected int getHits() {
		return this.getSuccessHits() + this.getFailureHits();
	}
	protected int getSuccessHits() {
		int h = 0;
		for(Integer i : this.success.values()) h += i;
		return h;
	}
	protected int getFailureHits() {
		int h = 0;
		for(Integer i : this.failure.values()) h += i;
		return h;
	}
	protected Scheme getRoot() {
		for(Scheme s = this; true; s = s.sParent)
			if(s.sParent == null || s.sParent.id == 0) return s;
	}
	protected double getRate() {
/*
		double r = this.lScheme.isEmpty() ? 1d : 0d;
		for(Scheme s : this.lScheme) r += (s.getRate() / this.lScheme.size());
		return ((double) (this.getPleasure() * this.getSuccessRate()) / 100d) * r;

		double r = this.equilibrated == -1 ? 0d : this.getPleasure();
		for(Scheme s : this.lScheme) {
			double sr = s.getRate();
			if(sr > r) r = sr;
		}

		double r = this.lScheme.isEmpty() ? this.getPleasure() : 0d;
		for(Scheme s : this.lScheme) {
			double sr = s.getRate();
			if(sr > r) r = sr;
		}
		return r;
*/
		//return this.getRoot().expected.getPleasure() + (1.0 / this.getHits());
		//int h = this.getHits();
		//return (((double) (100 - this.getSuccessRate())) / 100.0) *
		//	(h == 0 ? 1.0 : 1.0 / h);
		return this.isBasic() ? 0.0 : 1.0 / (1.0 + this.getHits());
	}
	private Context getMapContext(
		HashMap<Context, Integer> map, Context context
	) {
		for(Context c : map.keySet()) if(context.equals(c)) return c;
		return null;
	}
	protected LinkedList<Scheme> getSchemes() {
		LinkedList<Scheme> lScheme = new LinkedList<Scheme>();
		lScheme.add(this);
		for(Scheme s : this.lScheme) lScheme.addAll(s.getSchemes());
		return lScheme;
	}
	protected LinkedList<Scheme> getLeafs() {
		LinkedList<Scheme> lScheme = new LinkedList<Scheme>();
		if(this.lScheme.isEmpty()) lScheme.add(this);
		for(Scheme s : this.lScheme) lScheme.addAll(s.getLeafs());
		return lScheme;
	}
/*
	protected LinkedList<Scheme> getSchemes(Scheme sMemory) {
		LinkedList<Scheme> lScheme = new LinkedList<Scheme>();
		while(true) {
			sMemory = sMemory.lScheme.get(0);
			lScheme.addAll(this.getMatchedSchemes(sMemory));
			if(sMemory.lScheme.isEmpty()) break;
		}
		return lScheme;
	}
	private LinkedList<Scheme> getMatchedSchemes(Scheme sMemory) {
		LinkedList<Scheme> lScheme = new LinkedList<Scheme>();
		if(!sMemory.lScheme.isEmpty()) {
			Match m = Match.getMatch(this.context, sMemory.context);
			if(
				m.getTotal() == 100 &&
				m.getMatched().size() == sMemory.context.size() &&
				this.isSameActions(sMemory.lAction) &&
				this.expected.generalize().equals(sMemory.expected.generalize())
			)
				for(Scheme s : this.lScheme)
					lScheme.addAll(s.getMatchedSchemes(sMemory.lScheme.get(0)));
		} else lScheme.add(this);
		return lScheme;
	}
*/
	protected void addlAction(LinkedList<String> lAction) {
		for(String action : lAction)
			if(!this.lAction.contains(action)) this.lAction.add(action);
	}
	protected void addSuccess(Context context) {
		if(this.isBasic()) return;
		Context c = this.getMapContext(this.success, context);
		if(c == null) this.success.put(context, 1);
			else this.success.put(c, this.success.get(c) + 1);
	}
	protected void addFailure(Context context) {
		if(this.isBasic()) return;
		Context c = this.getMapContext(this.failure, context);
		if(c == null) this.failure.put(context, 1);
			else this.failure.put(c, this.failure.get(c) + 1);
	}
	protected void copyHits(Scheme scheme) {
		for(HashMap.Entry<Context, Integer> e : scheme.success.entrySet()) {
			Context ec = this.getMapContext(this.success, e.getKey());
			if(ec == null) this.success.put(new Context(e.getKey()), e.getValue());
				else this.success.put(ec, e.getValue() + this.success.get(ec));
		}
		for(HashMap.Entry<Context, Integer> e : scheme.failure.entrySet()) {
			Context ec = this.getMapContext(this.failure, e.getKey());
			if(ec == null) this.failure.put(new Context(e.getKey()), e.getValue());
				else this.failure.put(ec, e.getValue() + this.failure.get(ec));
		}
	}
	protected void copyMatchedHits(Scheme scheme) {
		for(HashMap.Entry<Context, Integer> e : scheme.success.entrySet()) {
			if(Match.getMatch(e.getKey(), this.context).getTotal() == 100) {
				Context ec = this.getMapContext(this.success, e.getKey());
				if(ec == null)
					this.success.put(new Context(e.getKey()), e.getValue());
				else
					this.success.put(ec, e.getValue() + this.success.get(ec));
			}
		}
		for(HashMap.Entry<Context, Integer> e : scheme.failure.entrySet()) {
			if(Match.getMatch(e.getKey(), this.context).getTotal() == 100) {
				Context ec = this.getMapContext(this.failure, e.getKey());
				if(ec == null)
					this.failure.put(new Context(e.getKey()), e.getValue());
				else
					this.failure.put(ec, e.getValue() + this.failure.get(ec));
			}
		}
	}
	protected void clearHits() {
		this.success.clear();
		this.failure.clear();
	}
	protected void takeHits(Scheme scheme) {
		this.copyHits(scheme);
		scheme.clearHits();
	}
	protected void removeHits(Scheme scheme) {
		for(Context c : scheme.success.keySet())
			this.success.remove(this.getMapContext(this.success, c));
		for(Context c : scheme.failure.keySet())
			this.failure.remove(this.getMapContext(this.failure, c));
	}
	protected boolean isBasic() {
		return this.expected.size() == 0 && this.equilibrated == -1;
	}
	protected boolean isCollector() {
		return this.expected.size() > 0 && this.equilibrated == -1;
	}
/*
	protected boolean isRoot() {
		return this.lScheme == this.lSParent;
	}
	protected boolean isLeaf() {
		return this.lScheme.isEmpty();
	}
*/
	protected boolean isSameActions(LinkedList<String> lAction) {
		return this.lAction.containsAll(lAction) &&
			lAction.containsAll(this.lAction);
	}
	protected boolean isExpectedMet(Context context, Context expected) {
		return Match.getMatch(this.expected, expected).getTotal() == 100 &&
			context.containsObjects(expected);
	}
/*
	protected boolean isEmpty() {
		for(Scheme s : this.lScheme) if(s.equilibrated != -1) return false;
		return true;
	}
	protected boolean contains(Scheme scheme) {
		if(!this.equals(scheme)) return false;
		for(Scheme s0 : scheme.lScheme) {
			boolean test = false;
			for(Scheme s1 : this.lScheme)
				if(test = s1.contains(s0)) break;
			if(!test) return false;
		}
		return false;
	}
*/
	protected Scheme toEmotional(Scheme sParent) {
		Scheme s = new Scheme(this, sParent);
		s.context.set(s.context.toEmotional());
		s.expected.set(s.expected.toEmotional());
		for(int i = 0; i < s.lScheme.size(); i++)
			s.lScheme.set(i, s.lScheme.get(i).toEmotional(s));
		return s;
	}
	protected LinkedList<Scheme> differentiate() {
		LinkedList<Scheme> lScheme = new LinkedList<Scheme>();
		for(HashMap.Entry<Context, Integer> e1 : this.success.entrySet())
			for(Context c : this.context.differentiate(e1.getKey())) {
				Scheme s0 =
					new Scheme(0, c, this.lAction, this.expected, this.sParent);
				if(lScheme.contains(s0)) continue;
				for(HashMap.Entry<Context, Integer> e2 : this.success.entrySet())
					if(Match.getMatch(s0.context, e2.getKey()).getTotal() == 100)
						s0.success.put(e2.getKey(), e2.getValue());
				for(HashMap.Entry<Context, Integer> e2 : this.failure.entrySet())
					if(Match.getMatch(s0.context, e2.getKey()).getTotal() == 100)
						s0.failure.put(e2.getKey(), e2.getValue());
				for(Scheme s1 : this.lScheme) s0.lScheme.add(new Scheme(s1, s0));
				lScheme.add(s0);
			}
		return lScheme;
	}
	protected HashMap<Context, Integer> getHits(Context context) {
		HashMap<Context, Integer> hits = new HashMap<Context, Integer>();
		for(HashMap.Entry<Context, Integer> e : this.success.entrySet())
			if(Match.getMatch(context, e.getKey()).getTotal() == 100)
				hits.put(new Context(e.getKey()), e.getValue());
		for(HashMap.Entry<Context, Integer> e : this.failure.entrySet())
			if(Match.getMatch(context, e.getKey()).getTotal() == 100)
				hits.put(new Context(e.getKey()), e.getValue());
		return hits;
	}
	protected int getSuccessRate(Context context) {
		double success = 0.0;
		double failure = 0.0;
		double rate;
		for(HashMap.Entry<Context, Integer> e : this.success.entrySet())
			if(Match.getMatch(context, e.getKey()).getTotal() == 100)
				success += (double) e.getValue();
		for(HashMap.Entry<Context, Integer> e : this.failure.entrySet())
			if(Match.getMatch(context, e.getKey()).getTotal() == 100)
				failure += (double) e.getValue();
		return success + failure > 0.0 ?
			(int) ((100.0 * success) / (success + failure)) : -1;
	}
	protected int generalize(Scheme scheme) {
		return Match.getMatch(this.context, scheme.context).getTotal() == 100 &&
			this.isSameActions(scheme.lAction) &&
			this.expected.equals(scheme.expected) ?
				this.getSuccessRate(scheme.context) : 0;
	}
	protected boolean isSameObject() {
		if(this.context.isEmpty()) return false;
		Context context = new Context();
		LinkedList<String> lAction = new LinkedList<String>(this.lAction);
		for(CEmotional ce : this.context)
			if(!this.expected.containsObject(ce)) context.add(ce);
		for(CEmotional ce : context)
			if(ce instanceof CVisual) {
				if(!lAction.remove("changeAttention V")) return false;
			} else if(ce instanceof CTactile) {
				if(!lAction.remove("changeAttention T")) return false;
			}
		return context.size() == 0;
	}
	protected boolean isPleasured() {
		boolean test = false;
		if(this.context.isEmpty()) return false;
		for(CEmotional ce : this.context) {
			CEmotional cEmotional = this.expected.getObject(ce);
			if(cEmotional == null || ce.pleasure > cEmotional.pleasure)
				return false;
			if(cEmotional != null && ce.pleasure < cEmotional.pleasure)
				test = true;
		}
		return test;
	}
	protected boolean isRecovered() {
		for(CEmotional ce : this.context)
			if(ce.pleasure == -1 && this.expected.removeLost().containsObject(ce))
				return true;
		return false;
	}
	protected boolean update(Scheme scheme) {
		boolean test = false;
		for(Scheme s0 : this.lScheme)
			for(Scheme s1 : scheme.lScheme)
				test = s0.update(s1) || test;
		if(test || this.lScheme.isEmpty()) {
			if(!this.isSameActions(scheme.lAction))
				return false;
			if(!this.expected.toEmotional().equals(scheme.expected.toEmotional()))
				return false;
			if(Match.getMatch(this.context, scheme.context).getTotal() == 100) {
				this.copyHits(scheme);
				return true;
			}
/*
			for(Context c0 : this.context.generalize()) {
				if(Match.getMatch(c0, scheme.context).getTotal() == 100) {
					this.context.set(c0);
					this.copyHits(scheme);
					return true;
				}
			}
*/
		}
		return false;
	}
	protected boolean merge(Scheme scheme) {
		if(
			this.context.equals(scheme.context) &&
			this.isSameActions(scheme.lAction) &&
			this.expected.equals(scheme.expected)
		) {
			for(HashMap.Entry<Context, Integer> e : scheme.success.entrySet()) {
				Context ec = this.getMapContext(this.success, e.getKey());
				if(ec == null)
					this.success.put(new Context(e.getKey()), e.getValue());
				else this.success.put(ec, e.getValue() + this.success.get(ec));
			}
			for(HashMap.Entry<Context, Integer> e : scheme.failure.entrySet()) {
				Context ec = this.getMapContext(this.failure, e.getKey());
				if(ec == null)
					this.failure.put(new Context(e.getKey()), e.getValue());
				else this.failure.put(ec, e.getValue() + this.failure.get(ec));
			}
			for(Scheme s0 : scheme.lScheme) {
				boolean test = false;
				for(Scheme s1 : this.lScheme)
					if(test = s1.merge(s0)) break;
				if(!test) this.lScheme.add(new Scheme(s0, this));
			}
			return true;
		} else return false;
	}
	public int compareTo(Scheme scheme) {
		if(this.isBasic() && scheme.isBasic()) return 0;
		if(!this.isBasic() && scheme.isBasic()) return -1;
		if(this.isBasic() && !scheme.isBasic()) return 1;
		return this.equilibrated - scheme.equilibrated;
	}
	public boolean equals(Object object) {
		if(!(object instanceof Scheme)) return false;
		Scheme s = (Scheme) object;
		if(!this.context.equals(s.context)) return false;
		if(!this.isSameActions(s.lAction)) return false;
		if(!this.expected.equals(s.expected)) return false;
		return true;
	}
	protected String toString1() {
		return "#" + this.id + " " +
			this.context + "-" + this.lAction + "->" + this.expected;
	}
	protected String toString2() {
		return "#" + this.id + " " + this.equilibrated + "e " +
			this.getSuccessRate() + "% " + this.getHits() + "h " +
			String.format("%.3f", this.getRate()) + "r " +
			this.context + "-" + this.lAction + "->" + this.expected;
	}
	protected String toStringTree1(int deep) {
		String s = this.toString1();
		for(int i = 0; i < deep; i++) s = "   " + s;
		if(!this.lScheme.isEmpty())
			s += this.lScheme.stream().map(
				i -> i.toStringTree1(deep + 1)
			).collect(Collectors.joining("\n", "\n", ""));
		return s;
	}
	protected String toStringTree2(int deep) {
		String s = this.toString2();
		for(int i = 0; i < deep; i++) s = "   " + s;
		if(!this.lScheme.isEmpty())
			s += this.lScheme.stream().map(
				i -> i.toStringTree2(deep + 1)
			).collect(Collectors.joining("\n", "\n", ""));
		return s;
	}
	public String toString() {
		return "{\n\t" +
			"\"id\":\"" + this.id + "\",\n\t" +
			"\"equilibrated\":" + this.equilibrated + ",\n\t" +
			"\"context\":" + this.context + ",\n\t" +
			"\"lAction\":" + this.lAction.stream().map(
					i -> "\"" + i + "\""
				).collect(Collectors.joining(", ", "[", "]")) + ",\n\t" +
			"\"expected\":" + this.expected + ",\n\t" +
			"\"success\":" + this.success.entrySet().stream().map(
					i -> "\n\t\t[" + i.getKey() + ", " + i.getValue() + "]"
				).collect(Collectors.joining(",", "[", "\n\t]")) + ",\n\t" +
			"\"failure\":" + this.failure.entrySet().stream().map(
					i -> "\n\t\t[" + i.getKey() + ", " + i.getValue() + "]"
				).collect(Collectors.joining(",", "[", "\n\t]")) + ",\n\t" +
			"\"lScheme\":" + this.lScheme.stream().map(
					i -> "\n\t\t" + i.toString().replaceAll("\n", "\n\t\t")
				).collect(Collectors.joining(",", "[", "\n\t]")) +
			"\n}";
	}
}
