// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

#include <string>
#include <cstdio>

#include <yarp/os/Network.h>
#include <yarp/dev/ControlBoardInterfaces.h>
#include <yarp/dev/PolyDriver.h>
#include <yarp/os/Time.h>
#include <yarp/sig/Vector.h>

using namespace std;
using namespace yarp::dev;
using namespace yarp::sig;
using namespace yarp::os;

class Body{

float refAceleration;
float ref;

IPositionControl *pos_arm;
IEncoders *encs_arm;


public:
	PolyDriver* head;
	PolyDriver* right_arm;

    int axe0_pos;
    int axe1_pos;
    int axe2_pos;
    Body(){
        //System.loadLibrary("yarp_java");
    axe0_pos =0;
    axe1_pos =0;
    axe2_pos =0;
    //pos_arm;
    //encs_arm;



	    Network yarp;

	    // set up a description of the device we want 
	    // in this case, a remote motor control board
	    Property prop;
	    prop.put("device","remote_controlboard");
	    prop.put("local", "/jacub/head");
	    prop.put("remote", "/icubSim/head");  // this is for the icubSim
	    // or fake it with "yarpdev --device test_motor --name /icubSim/head"
	    printf("Property is %s",prop.toString());

    	// create the device
	    head = new PolyDriver(prop);
printf("poli creado");
	    prop.put("device","remote_controlboard");
	    prop.put("local","/jacub/right_arm");
	    prop.put("remote","/icubSim/right_arm");
	    right_arm = new PolyDriver(prop);
        //this->right_arm = right_arm_d;


    bool ok;
    ok = right_arm->view(pos_arm);
    ok = ok && right_arm->view(encs_arm);

    if (!ok) {
        printf("Problems acquiring interfaces\n");
        //return 0;
    }
    int nj=0;
    pos_arm->getAxes(&nj);
    Vector encoders;
    Vector tmp;
    encoders.resize(nj);
    tmp.resize(nj);
    
    int i;
    for (i = 0; i < nj; i++) {
         tmp[i] = 10.0;
    }
    pos_arm->setRefAccelerations(tmp.data());

    for (i = 0; i < nj; i++) {
        tmp[i] = 5.0;
        pos_arm->setRefSpeed(i, tmp[i]);
    }

    pos_arm->setRefSpeeds(tmp.data());

    /*printf("waiting for encoders");
    while(!encs_arm->getEncoders(encoders.data()))
    {
        Time::delay(0.1);
        printf(".");
    }
    printf("\n;");
*/
        }

void moveArm(){}

void moveHeadUp(int angle){
    moveHead(angle,axe1_pos,axe2_pos);

}

void moveHeadDown(int angle){
    moveHead(-1*angle,axe1_pos,axe2_pos);
}

void moveHeadRight(int angle){
    moveHead(axe0_pos,axe1_pos,-1*angle);
}

void moveHeadLeft(int angle){
    moveHead(axe0_pos,axe1_pos,angle);
}

void moveHead(int angle0, int angle1, int angle2){

    IPositionControl *pos;
	IEncoders *enc;
    IVelocityControl *vel;

    head->view(pos);
    head->view(enc);
    head->view(vel);

    int nj=0;
    enc->getAxes(&nj);
    Vector v;
    Vector command;

    command.resize(nj);
    v.resize(nj);
    command[0]=angle0;
    command[1]=angle1;
    command[2]=angle2;

    pos ->positionMove(command.data());

    axe0_pos = angle0;
    axe1_pos=angle1;
    axe2_pos=angle2;
}

void moveRightHand(int pos0,int pos1,int pos2, int pos3){
/*Network yarp;

    Property params;
    params.fromCommand(argc, argv);

    if (!params.check("robot"))
    {
        fprintf(stderr, "Please specify the name of the robot\n");
        fprintf(stderr, "--robot name (e.g. icub)\n");
        return 1;
    }
    std::string robotName=params.find("robot").asString();
    std::string remotePorts="/";
    remotePorts+=robotName;
    remotePorts+="/right_arm";

    std::string localPorts="/test/client";

    Property options;
    options.put("device", "remote_controlboard");
    options.put("local", localPorts);   //local port names
    options.put("remote", remotePorts); //where we connect to

    // create a device
    PolyDriver right_arm(options);*/
    if (!right_arm->isValid()) {
        printf("Device not available.  Here are the known devices:\n");
        printf("%s", Drivers::factory().toString().c_str());
        //return 0;
    }

    /*IPositionControl *pos_arm;
    IEncoders *encs_arm;

    bool ok;
    ok = right_arm->view(pos_arm);
    ok = ok && right_arm->view(encs_arm);

    if (!ok) {
        printf("Problems acquiring interfaces\n");
        //return 0;
    }*/

    int nj=0;
    pos_arm->getAxes(&nj);
   Vector encoders;
    Vector command;
    Vector tmp;
    encoders.resize(nj);
    tmp.resize(nj);
    command.resize(nj);
    
    /*int i;
    for (i = 0; i < nj; i++) {
         tmp[i] = 10.0;
    }
    pos_arm->setRefAccelerations(tmp.data());

    for (i = 0; i < nj; i++) {
        tmp[i] = 5.0;
        pos_arm->setRefSpeed(i, tmp[i]);
    }

    pos_arm->setRefSpeeds(tmp.data());*/
    
    //fisrst read all encoders
    //

    printf("waiting for encoders");
    while(!encs_arm->getEncoders(encoders.data()))
    {
        Time::delay(0.1);
        printf(".");
    }
    printf("\n;");

    command=encoders;
    //now set the shoulder to some value
    command[0]=pos0;
    command[1]=pos1;
    command[2]=pos2;
    command[3]=pos3;
    pos_arm->positionMove(command.data());
    
    bool done=false;

    while(!done)
    {
        pos_arm->checkMotionDone(&done);
        Time::delay(0.1);
    }
/*
    int times=0;
    while(true)
    {
        times++;
        if (times%2)
        {
             command[0]=-50;
             command[1]=20;
             command[2]=-10;
             command[3]=50;
        }
        else
        {
             command[0]=-20;
             command[1]=40;
             command[2]=-10;
             command[3]=30;
        }

        pos_arm->positionMove(command.data());

        int count=50;
        while(count--)
            {
                Time::delay(0.1);
                bool ret=encs_arm->getEncoders(encoders.data());
                
                if (!ret)
                {
                    fprintf(stderr, "Error receiving encoders, check connectivity with the robot\n");
                }
                else
                { 
                    printf("%.1lf %.1lf %.1lf %.1lf\n", encoders[0], encoders[1], encoders[2], encoders[3]);
                }
            }
    }*/

    //right_arm.close();

}
};

int main(int argc, char *argv[]) 
{

Body body;
//printf("\nholis :D");
//public:
//PolyDriver p;
//p = body.head;p//rintf("\nd %s",p);
//printf("\nholis2 :D");

    //float at = 0;
    int dir = -1;
for (int i=10;i<10;i++){

//body.moveHeadRight(dir*20);

    //pos->positionMove(2,dir*45);
    //pos->positionMove(command.data());
    dir = (-1)*dir;
    //printf("Set axis 2 to %f degrees",at);
    Time::delay(2);
    //enc->getEncoders(v.data());
   // enc.getEncoders(v);
    //printf("Encoder at %f",v[0]);
    //at = +5;
    /*command[0]=dir*angle0;
    command[1]=dir*angle1;
    command[2]=dir*angle2;*/
    
}
body.moveHeadDown(60);
body.moveHeadRight(20);
body.moveRightHand(-50,20,-10,50);
    /*
    Network yarp;

    Property params;
    params.fromCommand(argc, argv)
    if (!params.check("robot"))
    {
        fprintf(stderr, "Please specify the name of the robot\n");
        fprintf(stderr, "--robot name (e.g. icub)\n");
        return 1;
    }
    std::string robotName=params.find("robot").asString();
    std::string remotePorts="/";
    remotePorts+=robotName;
    remotePorts+="/right_arm";

    std::string localPorts="/test/client";

    Property options;
    options.put("device", "remote_controlboard");
    options.put("local", localPorts);   //local port names
    options.put("remote", remotePorts); //where we connect to

    // create a device
    PolyDriver right_arm(options);
    if (!right_arm.isValid()) {
        printf("Device not available.  Here are the known devices:\n");
        printf("%s", Drivers::factory().toString().c_str());
        return 0;
    }

    IPositionControl *pos;
    IEncoders *encs;

    bool ok;
    ok = right_arm.view(pos);
    ok = ok && right_arm.view(encs);

    if (!ok) {
        printf("Problems acquiring interfaces\n");
        return 0;
    }

    int nj=0;
    pos->getAxes(&nj);
    Vector encoders;
    Vector command;
    Vector tmp;
    encoders.resize(nj);
    tmp.resize(nj);
    command.resize(nj);
    
    int i;
    for (i = 0; i < nj; i++) {
         tmp[i] = 50.0;
    }
    pos->setRefAccelerations(tmp.data());

    for (i = 0; i < nj; i++) {
        tmp[i] = 10.0;
        pos->setRefSpeed(i, tmp[i]);
    }

    //pos->setRefSpeeds(tmp.data()))
    
    //fisrst read all encoders
    //
    printf("waiting for encoders");
    while(!encs->getEncoders(encoders.data()))
    {
        Time::delay(0.1);
        printf(".");
    }
    printf("\n;");

    command=encoders;
    //now set the shoulder to some value
    command[0]=-50;
    command[1]=20;
    command[2]=-10;
    command[3]=50;
    pos->positionMove(command.data());
    
    bool done=false;

    while(!done)
    {
        pos->checkMotionDone(&done);
        Time::delay(0.1);
    }

    int times=0;
    while(true)
    {
        times++;
        if (times%2)
        {
             command[0]=-50;
             command[1]=20;
             command[2]=-10;
             command[3]=50;
        }
        else
        {
             command[0]=-20;
             command[1]=40;
             command[2]=-10;
             command[3]=30;
        }

        pos->positionMove(command.data());

        int count=50;
        while(count--)
            {
                Time::delay(0.1);
                bool ret=encs->getEncoders(encoders.data());
                
                if (!ret)
                {
                    fprintf(stderr, "Error receiving encoders, check connectivity with the robot\n");
                }
                else
                { 
                    printf("%.1lf %.1lf %.1lf %.1lf\n", encoders[0], encoders[1], encoders[2], encoders[3]);
                }
            }
    }

    right_arm.close();
    */
    return 0;
}

