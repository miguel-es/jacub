import java.io.File;
import java.util.LinkedList;
import java.util.Scanner;

public class JSON {
	private static JSON json;
	private String field;
	protected JSON value;
	protected JSON[] elements;
	static public void load(String s) throws Exception {
		JSON.json = JSON.parseValue(
			new Scanner(new File(s), "UTF-8").useDelimiter("\\Z").next()
		);
	}
	static private String trim(String s, char open, char close) {
		return s.charAt(0) == open && s.charAt(s.length() - 1) == close ?
			s.substring(1, s.length() - 1) : null;
	}
	static private String scapes(String data) {
		return data.replace("\\\"","\"").replace("\\t","\t").replace("\\n","\n");
	}
	static private String[] getBlocks(String s, char separator) {
		s = s.trim();
		LinkedList<String> linkedList = new LinkedList<String>();
		boolean doubleQuotes = false;
		int brackets = 0;
		int braces = 0;
		int i = 0;
		for(int j = 0; j < s.length(); j++) {
			char c = s.charAt(j);
			switch(c) {
				case '\\': j++; break;
				case '"': doubleQuotes^=true; break;
				case '[': brackets++; break;
				case ']': brackets--; break;
				case '{': braces++; break;
				case '}': braces--; break;
				default:
				if(
					separator == c && !doubleQuotes && brackets == 0 && braces == 0
				) {
					linkedList.add(s.substring(i, j));
					i = j + 1;
				}
			}
		}
		if(!doubleQuotes && brackets == 0 && braces == 0)
			linkedList.add(s.substring(i));
		return linkedList.toArray(new String[linkedList.size()]);
	}
	static private JSON parseValue(String s) {
		JSON json = new JSON();
		s = s.trim();
		if(s.length() > 0) switch(s.charAt(0)) {
			case '"': json.field = JSON.scapes(JSON.trim(s, '"', '"')); break;
			case '{': json.elements = JSON.parseObject(s); break;
			case '[': json.elements = JSON.parseArray(s); break;
			case 't': json.field = "true"; break;
			case 'f': json.field = "false"; break;
			case 'n': json.field = "null"; break;
			default: json.field = s;
		} else json.field = "";
		return json;
	}
	static private JSON[] parseObject(String s) {
		LinkedList<JSON> linkedList = new LinkedList<JSON>();
		for(String block : JSON.getBlocks(JSON.trim(s, '{', '}').trim(), ','))
			linkedList.add(JSON.parsePair(block));
		return linkedList.toArray(new JSON[linkedList.size()]);
	}
	static private JSON parsePair(String s) {
		String[] blocks = JSON.getBlocks(s, ':');
		JSON json = new JSON();
		json.field = JSON.scapes(JSON.trim(blocks[0], '"', '"'));
		json.value = JSON.parseValue(blocks[1]);
		return json;
	}
	static private JSON[] parseArray(String s) {
		LinkedList<JSON> linkedList = new LinkedList<JSON>();
		String[] blocks = JSON.getBlocks(JSON.trim(s, '[', ']'), ',');
		if(blocks.length != 1 || !blocks[0].isEmpty())
			for(String block : blocks) linkedList.add(JSON.parseValue(block));
		return linkedList.toArray(new JSON[linkedList.size()]);
	}
	protected JSON get(String field) {
		for(JSON json : this.elements)
			if((json.field != null) && (json.field.equals(field))) return json;
		return null;
	}
	protected boolean getB() {
		return Boolean.parseBoolean(this.getS());
	}
	protected int getI() {
		return Integer.parseInt(this.getS());
	}
	protected long getL() {
		return Long.parseLong(this.getS());
	}
	protected float getF() {
		return Float.parseFloat(this.getS());
	}
	protected double getD() {
		return Double.parseDouble(this.getS());
	}
	protected String getS() {
		return this.field;
	}
	static protected JSON getJ(String path) {
		JSON json = JSON.json;
		try {
			for(String s : path.split("/")) json = json.get(s).value;
			return json;
		} catch(Exception exception) {
			System.err.println("JSON: \"" + path + "\" not found.");
			return null;
		}
	}
	static protected void set(String path, String value) {
		JSON json = JSON.json;
		for(String s : path.split("/")) json = json.get(s).value;
		json.field = value;
	}
	static protected boolean getB(String path) {
		return Boolean.parseBoolean(JSON.getS(path));
	}
	static protected int getI(String path) {
		return Integer.parseInt(JSON.getS(path));
	}
	static protected long getL(String path) {
		return Long.parseLong(JSON.getS(path));
	}
	static protected float getF(String path) {
		return Float.parseFloat(JSON.getS(path));
	}
	static protected double getD(String path) {
		return Double.parseDouble(JSON.getS(path));
	}
	static protected String getS(String path) {
		return JSON.getJ(path).field;
	}
	protected void printAll(String deep) {
		if(this.field == null) {
			System.out.println(deep + "/");
			if(this.elements != null)
				for(JSON json : this.elements) json.printAll(deep + "| ");
			System.out.println(deep + "\\");
		} else if(this.value != null) {
			System.out.println(deep + this.field + ":");
			this.value.printAll(deep + " ");
		} else System.out.println(deep + this.field);
	}
	protected void print() {
		System.out.println("Field: " + this.field + " Value: " + this.value +
			" Elements: " + (this.elements != null ? this.elements.length : null));
	}
}
