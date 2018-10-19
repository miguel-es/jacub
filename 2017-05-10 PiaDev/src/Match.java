import java.util.LinkedList;
import java.util.StringJoiner;

public class Match {
	private Context a;
	private Context b;
	private LinkedList<Integer> value;
	private int total;
	private Match() {
		this.a = new Context();
		this.b = new Context();
		this.value = new LinkedList<Integer>();
		this.total = 0;
	}
	protected static Match getMatch(Context c1, Context c2) {
		Match match = new Match();
		for(CEmotional ce1 : c1) {
			for(CEmotional ce2 : c2) {
				Context c_1 = new Context(c1);
				Context c_2 = new Context(c2);
				c_1.remove(ce1);
				c_2.remove(ce2);
				Match m = c_1.isEmpty() ? new Match() : Match.getMatch(c_1, c_2);
				m.add(ce1, ce2, ce2.getMatch(ce1));
				if(m.total > match.total) match = m;
			}
		}
		Context c_1 = new Context(c1);
		for(CEmotional ce1 : match.a) if(ce1 != null) c_1.remove(ce1);
		for(CEmotional ce1 : c_1) match.add(ce1, null, 0);
		Context c_2 = new Context(c2);
		for(CEmotional ce2 : match.b) if(ce2 != null) c_2.remove(ce2);
		for(CEmotional ce2 : c_2) match.add(null, ce2, 0);
		return match;
	}
	private void add(CEmotional ce1, CEmotional ce2, int value) {
		this.a.add(ce1);
		this.b.add(ce2);
		this.value.add(value);
		this.total = (int) this.value.stream().mapToInt(
			i -> i.intValue()
		).average().getAsDouble();
	}
	protected Context getMatched() {
		Context c = new Context();
		for(int i = 0; i < this.value.size(); i++)
			if(this.b.get(i) != null && this.a.get(i) != null)
				c.add(this.a.get(i));
		return c;
	}
	protected CEmotional getCEmotionalA(int i) {
		return this.a.get(i);
	}
	protected CEmotional getCEmotionalB(int i) {
		return this.b.get(i);
	}
	protected int getSize() {
		return this.value.size();
	}
	protected int getTotal() {
		return this.total;
	}
	public String toString() {
		StringJoiner sj = new StringJoiner(", ", "[", "]");
		for(int i = 0; i < this.value.size(); i++)
			sj.add(
				this.a.get(i) + " : " + this.b.get(i) + " -> " + this.value.get(i)
			);
		return this.total + "% -> " + sj.toString();
	}
}
