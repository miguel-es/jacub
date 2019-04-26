//import java.util.LinkedList;
using namespace std;
/*class VisualContext{
	private:
	string color;
	string size[2];
	public:
	g
	//bool moving = false;
	//protected final Integer area

VisualContext:: VisualContext(char* color,char* size, bool moving){
	this.color = color;
	this.size = size;
	this.moving = moving;
}
};*/

struct VisualContext{
	string color;
	string size;

	virtual bool write(yarp::os::ConnectionWriter& connection) {
	string = "holis";
    connection.appendString();
    return true;
  }
};
