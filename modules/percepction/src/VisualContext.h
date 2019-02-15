//import java.util.LinkedList;
using namespace std;
/*class VisualContext{
	private:
	string color;
	string size[2];;
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

#include <yarp/os/Portable.h>
class VisualContext : public yarp::os::Portable {
public:
	string color;
	string size;
	bool moving;

	virtual bool write(yarp::os::ConnectionWriter& connection){
	connection.appendString(color.c_str());
	connection.appendString(size.c_str());
	//connection.appendBool(moving);
	}

	virtual bool read(yarp::os::ConnectionReader& connection){
	 color = connection.expectText();
	size = connection.expectText();

	}
	
};
