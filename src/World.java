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
import yarp.RpcClient;
import yarp.RpcClient;
import yarp.Bottle;

class World {
	private RpcClient world_port;

	public World(){
	   System.loadLibrary("yarp_java");
	   world_port = new RpcClient();
	   world_port.open("/jacques/world");
	   world_port.addOutput("/icubSim/world");
	}

	public void addBox(float length,float width,float height,float x,float y,float z,int r,int g,int b){
		addObj("box",length,width,height,x,y,z,r,g,b);
	}

	public void import3DModel(String xmodel,String texture,float x,float y, float z){
	Bottle cmd = new Bottle();
	cmd.addString("world");
	cmd.addString("mk");
	cmd.addString("smodel");
	cmd.addString(xmodel);
	cmd.addString(texture);
	cmd.addDouble(x);
	cmd.addDouble(y);
	cmd.addDouble(z);
	Bottle response = new Bottle();
	world_port.write(cmd,response);
	System.out.println("Response: "+response.toString());
	}


	private void addObj(String obj,float size1,float size2,float size3,float x,float y,float z, int r, int g, int b){
	  Bottle cmd = new Bottle();
	  cmd.addString("world");
	  cmd.addString("mk");
	  cmd.addString(obj);
	  if(size1>=0) cmd.addDouble(size1);
	  if(size2>=0) cmd.addDouble(size2);
	  if(size3>=0) cmd.addDouble(size3);
	  cmd.addDouble(x);
	  cmd.addDouble(y);
	  cmd.addDouble(z);
	  cmd.addInt(r);
	  cmd.addInt(g);
	  cmd.addInt(b);
	System.out.println("adding "+obj);
	//Network yarp = new Network();
	Bottle response = new Bottle();
	world_port.write(cmd,response);
	System.out.println("Response:"+response.toString());
	// set up a description of the device we want 
	// in this case, a remote motor control board
    }

    public static void main(String[] args){
	World world = new World();
	world.addBox(0.05f,0.05f,0.05f,0.0f,0.6f,0.4f,0,0,1);
	world.import3DModel("pia/mesaes.x","woodred.bmp",0.0f,0.0f,-0.2f);
    }
}
