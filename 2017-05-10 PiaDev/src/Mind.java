import java.nio.file.Files;
import java.nio.file.Paths;
import java.nio.file.StandardOpenOption;
import java.util.Collections;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.StringJoiner;

public class Mind {
	private final Agent agent;
	protected final LinkedList<Color> lColor;
	protected final LinkedList<Size> lSize;
	protected final LinkedList<Texture> lTexture;
	private final LinkedList<String> lAction;
	private final Scheme sMemory;
	private Scheme sActual;
	private Scheme sExperience;
	private Scheme sPerception;
	private final Scheme sRandom;
	protected final Context cInterested;
	protected final LinkedList<Context> lInterested;
	private boolean success;
	private boolean bored;
	private boolean curious;
	private boolean surprised;
	private boolean lose;
	private long stable;
	private long stableMaximum;
	protected Mind(Agent agent) {
		String author = "Mind -> Loading";
		this.agent = agent;
		this.lColor = new LinkedList<Color>();
		for(JSON j : JSON.getJ("Agent/lColor").elements)
			this.lColor.add(new Color(j));
		this.lSize = new LinkedList<Size>();
		for(JSON j : JSON.getJ("Agent/lSize").elements)
			this.lSize.add(new Size(j));
		this.lTexture = new LinkedList<Texture>();
		for(JSON j : JSON.getJ("Agent/lTexture").elements)
			this.lTexture.add(new Texture(j));
		this.lAction = new LinkedList<String>();
		this.sMemory = new Scheme();
		for(JSON j : JSON.getJ("Agent/lScheme").elements)
			this.add(new Scheme(j, lColor, lSize, lTexture, this.sMemory), author);
		this.sActual = new Scheme(new Scheme(), this.sMemory);
		this.sActual.equilibrated = -1;
		this.sExperience = new Scheme();
		this.sPerception = new Scheme(this.sActual, null);
		this.sRandom = new Scheme(new Scheme(), this.sMemory);
		this.sRandom.equilibrated = -1;
		this.sRandom.lAction.add("random");
		this.cInterested = new Context();
		this.lInterested = new LinkedList<Context>();
		this.success = true;
		this.curious = false;
		this.bored = false;
		this.surprised = false;
		this.lose = false;
		this.stable = 0L;
		this.stableMaximum = 0L;
	}
	protected LinkedList<String> next(Context context) {
		this.log(0, 3, 1, "\nPiaWorld (" + this.agent.piaWorld.step + ")\n");
		this.perception(context);
		this.accommodation();
		if(!this.engagement()) this.reflection();
		this.equilibrium();
		return this.act();
	}
	private void perception(Context context) {
		Context c = new Context();
		HashMap<CVisual, Double> mCVisual = new HashMap<CVisual, Double>();
		HashMap<CTactile, Double> mCTactile = new HashMap<CTactile, Double>();
		LinkedList<CVisual> lCVisual = new LinkedList<CVisual>();
		LinkedList<CTactile> lCTactile = new LinkedList<CTactile>();
		boolean aVisual = JSON.getB("Agent/cognition/attention/visual");
		double aVMSize = JSON.getD("Agent/cognition/attention/visualMinimumSize");
		boolean aTactile = JSON.getB("Agent/cognition/attention/tactile");
		double aScalar = JSON.getD("Agent/cognition/attention/scalar");
		this.bored = this.isBored();
		this.log(0, 4, 1, "Proprioception:");
		this.log(1, 3, 0, "Hyn " + this.agent.Hyn);
		this.log(1, 3, 0, "Han " + this.agent.Han);
		this.log(1, 3, 0, "hxn " + this.agent.hxn);
		this.log(1, 3, 0, "hyn " + this.agent.hyn);
		this.log(1, 3, 0, "hzn " + this.agent.hzn);
		this.log(1, 3, 0, "hon " + this.agent.hon);
		this.log(0, 4, 1, "Perception -> Actions:");
		this.log(1, 3, 0, "Tried " + this.sActual.lAction);
		this.log(2, 3, 0, "Mind " + this.lAction +
			(this.success ? " success." : " fail."));
		this.log(2, 3, 0, "Body " + this.agent.lAction +
			(this.agent.success ? " success." : " fail."));
		if(this.bored) {
			int b = JSON.getI("Agent/cognition/attention/bored");
			for(Color color : this.lColor)
				color.hits = color.hits >= b ? 1L : 0L;
			for(Texture texture : this.lTexture)
				texture.hits = texture.hits >= b ? 1L : 0L;
			this.lose = true;
		}
		this.log(0, 4, 1, "Perception -> Rate attention:");
		for(CEmotional ce : context) {
			double i = (this.cInterested.containsObject(ce) ? aScalar : 0.0) +
				(ce.getInterest(this) * (1.0 - aScalar));
			if(ce instanceof CVisual && aVisual) {
				CVisual cVisual = (CVisual) ce;
				if(cVisual.size.size >= aVMSize) {
					boolean test = true;
					for(HashMap.Entry<CVisual, Double> e : mCVisual.entrySet())
						if(cVisual.isSameObject(e.getKey())) {
							if(i > e.getValue()) {
								mCVisual.remove(e.getKey());
							} else test = false;
							break;
						}
					if(test) mCVisual.put(cVisual, i);
				} else i = 0.0;
			}
			if(ce instanceof CTactile && aTactile) {
				CTactile cTactile = (CTactile) ce;
				boolean test = true;
				for(HashMap.Entry<CTactile, Double> e : mCTactile.entrySet())
					if(cTactile.isSameObject(e.getKey())) {
						if(i > e.getValue()) {
							mCTactile.remove(e.getKey());
						} else test = false;
						break;
					}
				if(test) mCTactile.put(cTactile, i);
			}
			this.log(1, 3, 0, (100.0 * i) + "% " + ce);
		}
		mCVisual.entrySet().stream().sorted(
			HashMap.Entry.<CVisual, Double>comparingByValue().reversed()
		).forEachOrdered(i -> lCVisual.add(i.getKey()));
		mCTactile.entrySet().stream().sorted(
			HashMap.Entry.<CTactile, Double>comparingByValue().reversed()
		).forEachOrdered(i -> lCTactile.add(i.getKey()));
		if(!lCVisual.isEmpty())
			for(CVisual cVisual : lCVisual)
				if(cVisual.pleasure > 0)
					Color.get(this.lColor, cVisual.color).hits++;
		if(!lCTactile.isEmpty())
			for(CTactile cTactile : lCTactile)
				if(cTactile.pleasure > 0)
					Texture.get(this.lTexture, cTactile.texture).hits++;
		this.lInterested.clear();
		for(int i = 0; i < Math.max(lCVisual.size(), lCTactile.size()); i++) {
			c = new Context();
			if(i < lCVisual.size()) c.add(lCVisual.get(i));
			if(i < lCTactile.size()) c.add(lCTactile.get(i));
			this.lInterested.add(c);
		}
		c = this.lInterested.isEmpty() ? new Context() : this.lInterested.get(0);
		for(CEmotional ce : new Context(this.cInterested)) {
			if(c.containsObject(ce)) {
				this.cInterested.removeObject(ce);
				this.cInterested.add(c.getObject(ce));
			} else {
				if(ce instanceof CVisual) {
					Context cr = c.removeCVisual();
					c.clear();
					c.addAll(cr);
				}
				if(ce instanceof CTactile) {
					Context cr = c.removeCTactile();
					c.clear();
					c.addAll(cr);
				}
				this.cInterested.removeObject(ce);
				if(ce.pleasure != -1) {
					this.cInterested.add(ce.getLost());
					c.add(ce.getLost());
				}
			}
		}
		this.log(0, 4, 1, "Perception -> Interested in:");
		if(this.cInterested.isEmpty()) this.log(1, 3, 0, "Nothing.");
			else this.log(1, 7, 1, this.cInterested.toString());
		this.log(0, 4, 1, "Perception -> Attended context:");
		this.log(1, 7, 1, c.toString());
		Scheme s =
			new Scheme(0, this.sExperience.expected, this.lAction, c, null);
		s.lAction.addAll(this.agent.lAction);
		this.addExperience(s);
		this.sExperience.success.put(this.sExperience.context, 1);
		this.log(0, 4, 1, "Perception -> Experience short-term memory:");
		this.log(0, 3, 0, this.sExperience.toStringTree2(0));
		this.log(0, 4, 1, "Perception -> Perception short-term memory:");
		this.log(0, 3, 0, this.sPerception.toStringTree2(0));
		c = this.sExperience.context;
		this.curious = this.isCurious();
		this.surprised = this.isSurprised();
		this.log(0, 4, 1, "Perception -> Scheme succeed:");
		boolean succeed = false;
		if(!this.sActual.isBasic() && this.agent.success && this.success)
			if(!this.curious) {
				this.sActual.addSuccess(new Context(c));
				succeed = true;
			} else this.sActual.addFailure(new Context(c));
		this.log(1, 7, 1, succeed ? "Yes." : "No.");
		this.log(0, 4, 1, "Perception -> Emotional:");
		this.log(1, 3, 0, (this.bored ? "B" : "Not b") + "ored.");
		this.log(1, 3, 0, (this.curious ? "C" : "Not c") + "urious.");
		this.log(1, 3, 0, (this.surprised ? "S" : "Not s") + "urprised.");
		this.log(1, 3, 0,
			(this.sExperience.isPleasured() ? "P" : "Not p") + "leasured."
		);
		this.log(1, 3, 0,
			(this.sExperience.isRecovered() ? "R" : "Not r") + "ecovered."
		);
		this.log(1, 3, 0, (
			this.sExperience.expected.removeLost().size() <
			this.sExperience.expected.size() ? "L" : "Not l"
		) + "ost.");
		if(this.sExperience.expected.getPleasure() == 4) this.lose = true;
		if(
			this.sExperience.expected.removeLost().size() <
			this.sExperience.expected.size()
		) this.lose = false;
		this.log(1, 3, 0, (this.lose ? "L" : "Not l") + "ose.");
	}
	private void accommodation() {
		LinkedList<Scheme> lScheme1 =
			new LinkedList<Scheme>(this.sMemory.getSchemes());
		this.differentiation();
		this.generalization();
		LinkedList<Scheme> lScheme2 =
			new LinkedList<Scheme>(this.sMemory.getSchemes());
		if(lScheme1.containsAll(lScheme2) && lScheme2.containsAll(lScheme1))
			this.stable++; else this.stable = 0;
		this.log(0, 4, 1, "Accomodation -> Schemes:");
		this.sMemory.lScheme.sort(null);
		for(Scheme s : this.sMemory.lScheme)
			this.log(0, 3, 0, s.toStringTree2(0));
	}
	private void generalization() {
		String author = "Accomodation -> Generalization";
		int used = JSON.getI("Agent/cognition/schemes/used");
		int success = JSON.getI("Agent/cognition/schemes/success");
		if(this.surprised && this.agent.success && this.success) {
			Scheme sExperience = new Scheme(this.sExperience, null);
			Scheme sPerception = new Scheme(this.sPerception, null);
			Scheme se = sExperience;
			Scheme sp = sPerception;
			do {
				if(!(se.isPleasured() && (se.isRecovered() || !sp.isBasic()))) {
					if(se.sParent != null) se.sParent.lScheme.clear();
					break;
				}
				if(sp.equilibrated < 1) {
					se.context.set(se.context.toEmotional());
					se.expected.set(se.expected.toEmotional());
				} else {
					se.context.set(sp.context);
					se.expected.set(se.expected.toEmotional());
				}
				se = se.lScheme.peek();
				sp = sp.lScheme.peek();
			} while(se != null && sp != null);
			sExperience = new Scheme(sExperience, this.sMemory);
			LinkedList<Scheme> lScheme = sExperience.sParent.lScheme;
			Collections.shuffle(lScheme);
			for(Scheme s : lScheme) {
				if(
					Match.getMatch(
						s.context, this.sExperience.context
					).getTotal() == 100 &&
					s.isSameActions(sExperience.lAction) &&
					s.expected.equals(sExperience.expected)
				) {
					sExperience.context.clear();
					sExperience.context.addAll(new Context(s.context));
					break;
				}
			}
			this.add(sExperience, author);
		}
		for(boolean test = true; test; test = test) {
			test = false;
			for(Scheme s : this.sMemory.getSchemes()) {
				for(Scheme s0 : s.lScheme)
					if(
						!s0.isBasic() && s0.getHits() >= used &&
						s0.getSuccessRate() >= success
					) {
						for(Scheme s1 : s.lScheme)
							if(
								s0 != s1 && !s1.isBasic() &&
								s1.getSuccessRate() >= success &&
								s0.generalize(s1) >= success && (
									(s0.equilibrated == -1 && s1.equilibrated == -1) ||
									(s0.equilibrated != -1 && s1.equilibrated != -1)
								)
							) {
								s0.copyMatchedHits(s1);
								for(Scheme s3 : s1.lScheme)
									this.add(new Scheme(s3, s0), author);
								this.log(0, 4, 1,
									author + " merged " + s1.id + " into " + s0.id
								);
								this.log(0, 3, 0, s0.toString());
								test = this.remove(s1, author);
								if(test) break;
							}
						if(test) break;
					}
				if(test) break;
			}
		}
	}
	private void differentiation() {
		String author = "Accomodation -> Differentiation";
		int used = JSON.getI("Agent/cognition/schemes/used");
		int success = JSON.getI("Agent/cognition/schemes/success");
		for(boolean test = true; test; test = test) {
			test = false;
			for(Scheme s0 : this.sMemory.getSchemes()) {
				if(s0.getHits() >= used && s0.getSuccessRate() < success) {
					Scheme scheme = s0;
					if(s0.equilibrated != -1) {
						scheme = s0.toEmotional(s0.sParent);
						test = this.remove(s0, author) && this.add(scheme, author);
					}
					for(Scheme s1 : s0.differentiate()) {
						if(
							s1.getSuccessRate() >= success &&
							(scheme.equilibrated != -1 || s1.getHits() >= used)
						) {
							s1.equilibrated = 0;
							scheme.removeHits(s1);
							test = this.add(s1, author);
						}
					}
					scheme.equilibrated = -1;
				}
			}
		}
	}
	private boolean engagement() {
		if(!this.bored) {
			if(this.curious || this.sActual.sParent.id == 0) {
				boolean partial = this.lose || this.agent.random.nextDouble() <=
					JSON.getD("Agent/cognition/partialMatching/probability");
				this.log(0, 4, 1,
					"Engagement -> " +
					(partial ? "Partial" : "Full") +
					" assimilation:"
				);
				this.sActual = this.engagement(this.sExperience.expected, partial, 1);
				this.log(0, 4, 1, "Engagement -> Selected:");
				if(this.sActual != null) {
					this.addPerception(this.sActual);
					this.log(0, 7, 1, this.sActual.toString());
					return true;
				} else this.log(1, 3, 0, "Empty.");
			} else {
				this.log(0, 4, 1, "Engagement -> Continue:");
				this.sActual = this.sActual.sParent;
				this.addPerception(this.sActual);
				this.log(0, 7, 1, this.sActual.toString());
				return true;
			}
		}
		return false;
	}
	private Scheme engagement(Context context, boolean partial, int deep) {
		int equilibrium = JSON.getI("Agent/cognition/equilibrium/actual");
		int minimum = JSON.getI("Agent/cognition/partialMatching/minimum");
		LinkedList<Scheme> lScheme = new LinkedList<Scheme>();
		int bestMatch = 0;
		int bestCandidateMatch = 0;
		double bestCandidateRating = 0;
		for(int p = (partial ? 99 : 100); p >= minimum; p--) {
			for(Scheme s0 : this.sMemory.lScheme) if(!s0.isCollector()) {
				for(Scheme s1 : s0.getLeafs()) if(!s1.isCollector()) {
					Match m = Match.getMatch(context, s1.context);
					if(m.getTotal() > bestMatch) bestMatch = m.getTotal();
					if(
						(m.getTotal() == p) && (
							(!s1.isBasic() && (p == 100 || s1.equilibrated > 0)) ||
							(s1.isBasic() && (
								!partial ||
								Match.getMatch(
									m.getMatched(), s1.context
								).getTotal() == 100
							))
						)
					) {
						if(m.getTotal() < 100) {
							int id = s1.id;
							s1 = new Scheme(s1, this.sMemory);
							s1.equilibrated = 0;
							s1.expected.clear();
							s1.clearHits();
							this.log(deep, 3, 0,
								m.getTotal() + "% @ " + m.getMatched() +
								" -> #" + id + " -> #" + s1.id + " " + s1.context
							);
						} else {
							this.log(deep, 3, 0,
								m.getTotal() + "% @ " + m.getMatched() +
								" -> #" + s1.id + " " + s1.context
							);
						}
						if(m.getTotal() < 100 && partial) {
							Context c = new Context(context);
							c.removeAll(m.getMatched());
							if(!c.isEmpty()) {
								Scheme scheme = this.engagement(c, false, deep + 1);
								if(scheme != null) {
									s1 = new Scheme(
										0,
										s1.context.merge(scheme.context),
										s1.lAction,
										new Context(),
										this.sMemory
									);
									s1.addlAction(scheme.lAction);
									m = Match.getMatch(context, s1.context);
									this.log(deep, 3, 1,
										m.getTotal() + "% @ " + m.getMatched() +
										" -> #" + s1.id + " " + s1.context
									);
								} else break;
							}
						}
						double r = s1.getRate();
						if(
							m.getTotal() > bestCandidateMatch || (
								m.getTotal() == bestCandidateMatch &&
								r > bestCandidateRating
							)
						) {
							this.log(deep, 7, 1, "New best candidate.");
							lScheme.clear();
							lScheme.add(s1);
							bestCandidateMatch = m.getTotal();
							bestCandidateRating = r;
						} else if(
							m.getTotal() == bestCandidateMatch &&
							r == bestCandidateRating
						) {
							this.log(deep, 7, 1, "Another best candidate.");
							lScheme.add(s1);
						}
					}
				}
			}
			if(lScheme.isEmpty()) {
				if(!partial && p++ == 100) {
					this.log(deep, 7, 1, "Switching to partial match.");
					partial = true;
				}
				if(p > bestMatch) p = bestMatch + 1;
			} else break;
		}
		return lScheme.isEmpty() ?
			null : lScheme.get(this.agent.random.nextInt(lScheme.size()));
	}
	private void reflection() {
		this.sActual = this.sRandom;
		this.addPerception(this.sActual);
		this.log(0, 4, 1, "Reflection -> Impasse:");
		this.log(0, 3, 0, this.sActual.toString());
	}
	private void equilibrium() {
		String author = "Cognition -> Equilibrium";
		int eActual = JSON.getI("Agent/cognition/equilibrium/actual");
		int sUsed = JSON.getI("Agent/cognition/schemes/used");
		int cStabilization =
			JSON.getI("Agent/cognition/equilibrium/stabilization");
		if(this.stable > this.stableMaximum) this.stableMaximum = this.stable;
		this.log(0, 4, 1, author + " " + eActual + ":");
		this.log(1, 7, 1, this.agent.piaWorld.step + " Steps.");
		this.log(
			1, 7, 1, this.stable + "/" + cStabilization + " <= " +
			this.stableMaximum + " Stable."
		);
		this.log(1, 7, 1, this.sMemory.lScheme.size() + " Structures.");
		this.log(1, 7, 1, (this.sMemory.getSchemes().size() - 1) + " Schemes.");
		if(this.stable == cStabilization) {
			JSON.set("Agent/cognition/equilibrium/actual", String.valueOf(++eActual));
			this.stableMaximum = 0;
			this.stable = 0L;
			LinkedList<Scheme> lScheme =
				new LinkedList<Scheme>(this.sMemory.getSchemes());
			lScheme.pop();
			for(Scheme s : lScheme)
				if(s.equilibrated == 0) {
					if(s.getHits() >= sUsed) s.equilibrated = eActual;
						else if(!s.isBasic()) this.remove(s, author);
				}
			switch(eActual) { case 3: System.exit(0); break; }
		}
	}
	private LinkedList<String> act() {
		LinkedList<String> lAction = new LinkedList<String>();
		int sv = 0;
		int st = 0;
		int cv = 0;
		int ct = 0;
		int r = 0;
		this.lAction.clear();
		for(String s : this.sActual.lAction) {
			//if(s.equals("random")) r++; else
			if(s.equals("showInterestIn V")) sv++; else
			if(s.equals("showInterestIn T")) st++; else
			if(s.equals("changeAttention V")) cv++; else
			if(s.equals("changeAttention T")) ct++; else
				lAction.add(s);
		}
		if(r > 0) {
			if((sv + cv) == 0 && this.agent.random.nextBoolean()) {
				if(this.agent.random.nextBoolean()) sv++; else cv++;
				r = 0;
			} else if((st + ct) == 0 && this.agent.random.nextBoolean()) {
				if(this.agent.random.nextBoolean()) st++; else ct++;
				r = 0;
			} else lAction.add("random");
		}
		this.success = (sv + cv) <= 1 && (st + ct) <= 1;
		if(sv > 0) {
			Context cInterested = this.cInterested.removeCVisual();
			this.cInterested.clear();
			this.cInterested.addAll(cInterested);
			Context c;
			if(
				(c = this.lInterested.peek()) != null &&
				!(c = c.getCVisual().removeLost()).isEmpty()
			) {
				this.cInterested.addAll(c);
				this.lAction.add("showInterestIn V");
			} else this.success = false;
		}
		if(st > 0) {
			Context cInterested = this.cInterested.removeCTactile();
			this.cInterested.clear();
			this.cInterested.addAll(cInterested);
			Context c;
			if(
				(c = this.lInterested.peek()) != null &&
				!(c = c.getCTactile().removeLost()).isEmpty()
			) {
				this.cInterested.addAll(c);
				this.lAction.add("showInterestIn T");
			} else this.success = false;
		}
		if(cv > 0) {
			this.success = false;
			CEmotional ce = this.cInterested.getCVisual().peek();
			if(ce == null) ce = this.sExperience.expected.getCVisual().peek();
			if(ce != null) {
				for(Context c : this.lInterested) {
					c = c.getCVisual();
					if(!c.isEmpty() && !c.containsObject(ce)) {
						this.cInterested.removeObject(ce);
						this.cInterested.addAll(c);
						this.lAction.add("changeAttention V");
						this.success = true;
						break;
					}
				}
			}
		}
		if(ct > 0) {
			this.success = false;
			CEmotional ce = this.cInterested.getCTactile().peek();
			if(ce == null) ce = this.sExperience.expected.getCTactile().peek();
			if(ce != null) {
				for(Context c : this.lInterested) {
					c = c.getCTactile();
					if(!c.isEmpty() && !c.containsObject(ce)) {
						this.cInterested.removeObject(ce);
						this.cInterested.addAll(c);
						this.lAction.add("changeAttention T");
						this.success = true;
						break;
					}
				}
			}
		}
 		return lAction;
	}
	private boolean isBored() {
		int b = JSON.getI("Agent/cognition/attention/bored");
		for(Color c : this.lColor) if(c.hits >= b) return true;
		for(Texture t : this.lTexture) if(t.hits >= b) return true;
		return false;
	}
	private boolean isCurious() {
		return !this.sActual.isBasic() &&
			!this.sActual.isExpectedMet(
				this.sExperience.context, this.sExperience.expected
			);
	}
	private boolean isSurprised() {
		Scheme se = this.sExperience;
		Scheme sp = this.sPerception;
		while(
			se != null && sp != null &&
			sp.equilibrated > 0 &&
			sp.isExpectedMet(se.context, se.expected) &&
			se.isPleasured() && (se.isRecovered() || !se.isBasic())
		) {
			se = se.lScheme.peek();
			sp = sp.lScheme.peek();
		}
		if(se == null && sp == null) return true;
		return !this.sActual.isExpectedMet(
			this.sExperience.context, this.sExperience.expected
		) && (
			this.sExperience.isPleasured() &&
			(this.sExperience.isRecovered() || !this.sActual.isBasic())
		);
	}
	private void addExperience(Scheme scheme) {
		scheme = new Scheme(scheme, null);
		scheme.lScheme.clear();
		scheme.clearHits();
		scheme.lScheme.add(new Scheme(this.sExperience, scheme));
		this.sExperience = scheme;
		LinkedList<Scheme> lScheme = this.sExperience.getSchemes();
		if(lScheme.size() > JSON.getI("Agent/cognition/schemes/deep"))
			lScheme.getLast().sParent.lScheme.clear();
	}
	private void addPerception(Scheme scheme) {
		scheme = new Scheme(scheme, null);
		scheme.lScheme.clear();
		scheme.clearHits();
		scheme.lScheme.add(new Scheme(this.sPerception, scheme));
		this.sPerception = scheme;
		LinkedList<Scheme> lScheme = this.sPerception.getSchemes();
		if(lScheme.size() > JSON.getI("Agent/cognition/schemes/deep"))
			lScheme.getLast().sParent.lScheme.clear();
	}
	private boolean add(Scheme scheme, String author) {
		this.log(0, 4, 1, author + " is trying to add:");
		this.log(0, 3, 0, scheme.toString());
		for(Scheme s : scheme.sParent.lScheme)
			if(s.equals(scheme)) {
				this.log(0, 13, 0, "Same found.");
				this.log(0, 3, 0, s.toString());
				return false;
			}
/*
			if(s.merge(scheme)) {
				this.log(0, 13, 0, "Same found and merged:");
				this.log(0, 3, 0, s.toString());
				return false;
			}
*/
		if(scheme.sParent.lScheme.add(scheme)) {
			this.log(1, 3, 0, "Success.");
			return true;
		} else {
			this.log(1, 1, 0, "Error.");
			System.exit(0);
			return false;
		}
	}
	private boolean remove(Scheme scheme, String author) {
		this.log(0, 4, 1, author + " is trying to remove:");
		this.log(0, 3, 0, scheme.toString());
		if(scheme.sParent.lScheme.remove(scheme)) {
			this.log(1, 3, 0, "Success.");
			return true;
		} else {
			this.log(1, 1, 0, "Error.");
			System.exit(0);
			return false;
		}
	}
	private void log(int deep, int color, int bold, String message) {
		String d = "";
		for(int i = 1; i < deep; i++) System.out.print("   ");
		if(deep > 0) System.out.print(" ~ ");
		System.out.println(
			"\u001B[" + bold + ";3" + color  + "m" + message + "\u001B[0m"
		);
		if(deep > 0) message = " ~ " + message;
		for(int i = 1; i < deep; i++) d += "   ";
		try {
			Files.write(
				Paths.get(this.agent.piaWorld.time + ".log"),
				(d + message + "\n").getBytes(),
				StandardOpenOption.CREATE,
				StandardOpenOption.APPEND
			);
		} catch(Exception exception) {
			exception.printStackTrace(System.err);
		}
	}
	public String toString() {
		StringJoiner sj1 = new StringJoiner(",\n\t", "{\n\t", "\n}");
		StringJoiner sj2 = null;
		StringJoiner sj3 = null;
			sj2 = new StringJoiner(",\n\t\t", "{\n\t\t", "\n\t}");
			sj2.add("\"name\":\"" + JSON.getS("Agent/head/name") + "\"");
				String[] aHv = { "field", "factor", "vertical", "horizontal" };
				sj3 = new StringJoiner(",", "{", "}");
				for(String s : aHv)
					sj3.add("\"" + s + "\":" + JSON.getS("Agent/head/vision/" + s));
			sj2.add("\"vision\":" + sj3);
				String[] aHs = { "y", "yn", "a", "an" };
				sj3 = new StringJoiner(",", "{", "}");
				for(String s : aHs)
					sj3.add("\"" + s + "\":" + JSON.getS("Agent/head/step/" + s));
			sj2.add("\"step\":" + sj3);
		sj1.add("\"head\":" + sj2);
			sj2 = new StringJoiner(",\n\t\t", "{\n\t\t", "\n\t}");
			sj2.add("\"name\":\"" + JSON.getS("Agent/hand/name") + "\"");
				String[] ahs = { "x", "xn", "y", "yn", "z", "zn", "o", "on" };
				sj3 = new StringJoiner(",", "{", "}");
				for(String s : ahs)
					sj3.add("\"" + s + "\":" + JSON.getS("Agent/hand/step/" + s));
			sj2.add("\"step\":" + sj3);
		sj1.add("\"hand\":" + sj2);
			sj2 = new StringJoiner(",\n\t\t", "{\n\t\t", "\n\t}");
				String[] ace = { "actual", "stabilization" };
				sj3 = new StringJoiner(",", "{", "}");
				for(String s : ace)
					sj3.add(
						"\"" + s + "\":" +
						JSON.getS("Agent/cognition/equilibrium/" + s)
					);
			sj2.add("\"equilibrium\":" + sj3);
				String[] aca = {
					"visual", "visualMinimumSize", "tactile", "scalar", "bored"
				};
				sj3 = new StringJoiner(",", "{", "}");
				for(String s : aca)
					sj3.add(
						"\"" + s + "\":" +
						JSON.getS("Agent/cognition/attention/" + s)
					);
			sj2.add("\"attention\":" + sj3);
				String[] acs = { "used", "success", "deep" };
				sj3 = new StringJoiner(",", "{", "}");
				for(String s : acs)
					sj3.add(
						"\"" + s + "\":" +
						JSON.getS("Agent/cognition/schemes/" + s)
					);
			sj2.add("\"schemes\":" + sj3);
				String[] acp = { "probability", "minimum" };
				sj3 = new StringJoiner(",", "{", "}");
				for(String s : acp)
					sj3.add(
						"\"" + s + "\":" +
						JSON.getS("Agent/cognition/partialMatching/" + s)
					);
			sj2.add("\"partialMatching\":" + sj3);
		sj1.add("\"cognition\":" + sj2);
		if(!this.lColor.isEmpty()) {
			sj2 = new StringJoiner(",\n\t\t", "[\n\t\t", "\n\t]");
			for(Color c : this.lColor) sj2.add(c.toString());
			sj1.add("\"lColor\":" + sj2);
		}
		if(!this.lSize.isEmpty()) {
			sj2 = new StringJoiner(",\n\t\t", "[\n\t\t", "\n\t]");
			for(Size s : this.lSize) sj2.add(s.toString());
			sj1.add("\"lSize\":" + sj2);
		}
		if(!this.lTexture.isEmpty()) {
			sj2 = new StringJoiner(",\n\t\t", "[\n\t\t", "\n\t]");
			for(Texture t : this.lTexture) sj2.add(t.toString());
			sj1.add("\"lTexture\":" + sj2);
		}
		this.sMemory.lScheme.sort(null);
		if(!this.sMemory.lScheme.isEmpty()) {
			sj2 = new StringJoiner(",\n\t\t", "[\n\t\t", "\n\t]");
			for(Scheme s : this.sMemory.lScheme)
				sj2.add(s.toString().replaceAll("\n", "\n\t\t"));
			sj1.add("\"lScheme\":" + sj2);
		}
		return sj1.toString();
	}
}
