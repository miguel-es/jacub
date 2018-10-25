/*
 * Copyright: (C) 2010 RobotCub Consortium
 * Author: Paul Fitzpatrick
 * CopyPolicy: Released under the terms of the LGPLv2.1 or later, see LGPL.TXT
 */

import yarp.Network;
import yarp.Time;
import yarp.Property;
import yarp.PolyDriver;
import yarp.IPositionControl;
import yarp.IEncoders;
import yarp.DVector;
import yarp.IFrameGrabberImage;

class Perception {
	private PolyDriver igrabber;
	private PolyDriver right_arm;

	public Perception(){

	System.loadLibrary("yarp_java");
	Network yarp = new Network();

	// set up a description of the device we want 
	// in this case, a remote motor control board
	Property prop = new Property();
	prop.put("device","remote_grabber");
	prop.put("local", "/jacques/right_eye");
	prop.put("remote", "/icubSim/cam/right");  // this is for the icubSim
	// or fake it with "yarpdev --device test_motor --name /icubSim/head"
	System.out.println("Property is " + prop);

	// create the device
	igrabber = new PolyDriver(prop);
	//prop.put("device","remote_controlboard");
	//prop.put("local","jacques/right_arm");
	//prop.put("remote","/icubSim/right_arm");
	//right_arm = new PolyDriver(prop);

	}

	public void see(){
	// get the "IPositionControl" interface
	IFrameGrabberImage grabberInterface;
	igrabber.view(grabberInterface);
	//float at = 0;
	//DVector v = new DVector(enc.getAxes());
	//for (int i=0; i<10; i++) {
	 /*   pos.positionMove(0,at);
	    System.out.println("Set axis 0 to " + at + " degrees");
	    Time.delay(1);
	    enc.getEncoders(v);
	    System.out.println("Encoder at " + v.get(0));
	    at += 5;
	}*/

}
	public void finalize(){
	// shut things down
	//head.close();
	//right_arm.close();
    }

    public static void main(String args[]){
	Perception p = new Perception();
	p.see();
	//jacub.finalize();
    }
}