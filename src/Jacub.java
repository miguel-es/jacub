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

class Jacub {
	private PolyDriver head;
	private PolyDriver right_arm;

	public Jacub(){

	System.loadLibrary("yarp_java");
	Network yarp = new Network();

	// set up a description of the device we want 
	// in this case, a remote motor control board
	Property prop = new Property();
	prop.put("device","remote_controlboard");
	prop.put("local", "/jacques/head");
	prop.put("remote", "/icubSim/head");  // this is for the icubSim
	// or fake it with "yarpdev --device test_motor --name /icubSim/head"
	System.out.println("Property is " + prop);

	// create the device
	head = new PolyDriver(prop);
	prop.put("device","remote_controlboard");
	prop.put("local","jacques/right_arm");
	prop.put("remote","/icubSim/right_arm");
	right_arm = new PolyDriver(prop);

	}

	public void moveHead(){
	// get the "IPositionControl" interface
	IPositionControl pos = head.viewIPositionControl();
	IEncoders enc = head.viewIEncoders();

	float at = 0;
	DVector v = new DVector(enc.getAxes());
	for (int i=0; i<10; i++) {
	    pos.positionMove(0,at);
	    System.out.println("Set axis 0 to " + at + " degrees");
	    Time.delay(1);
	    enc.getEncoders(v);
	    System.out.println("Encoder at " + v.get(0));
	    at += 5;
	}

	}
	public void finalize(){
	// shut things down
	head.close();
	right_arm.close();
    }

    public static void main(String args[]){
	Jacub jacub = new Jacub();
	jacub.moveHead();
	jacub.finalize();
    }
}
