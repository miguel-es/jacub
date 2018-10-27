// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

#include <string>
#include <cstdio>
#include <cmath>


#include <yarp/os/Network.h>
#include <yarp/dev/ControlBoardInterfaces.h>
#include <yarp/dev/CartesianControl.h>
#include <yarp/dev/PolyDriver.h>
#include <yarp/os/Time.h>
#include <yarp/sig/Vector.h>
#include <yarp/dev/IVelocityControl.h>
#include <yarp/dev/IEncoder.h>


#define MAX_TORSO_PITCH     30.0    // [deg]

using namespace std;
using namespace yarp::dev;
using namespace yarp::sig;
using namespace yarp::os;

class Body{

float refAceleration;
float ref;

IPositionControl *pos_arm;
IEncoders *encs_arm;
ICartesianControl *icart_arm;

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
	    prop.put("device","cartesiancontrollerclient");
	    prop.put("local","/jacub/right_arm");
	    prop.put("remote","/icubSim/cartesianController/left_arm");
	    right_arm = new PolyDriver(prop);
        //this->right_arm = right_arm_d;


    bool ok;
    ok = right_arm->view(icart_arm);
    //ok = ok && right_arm->view(encs_arm);

    if (!ok) {
        printf("Problems acquiring interfaces\n");
        //return 0;
    }

        icart_arm->setTrajTime(1.0);
Vector newDof, curDof;
        icart_arm->getDOF(curDof);
        newDof=curDof;

        // enable the torso yaw and pitch
        // disable the torso roll
        newDof[0]=1;
        newDof[1]=0;
        newDof[2]=1;
        
        // send the request for dofs reconfiguration
        icart_arm->setDOF(newDof,curDof);
        // impose some restriction on the torso pitch
                int axis=0; // pitch joint
        double min, max;

    printf("\nHoli\n");
        // sometimes it may be helpful to reduce
        // the range of variability of the joints;
        // for example here we don't want the torso
        // to lean out more than 30 degrees forward

        // we keep the lower limit
        icart_arm->getLimits(axis,&min,&max);
        icart_arm->setLimits(axis,min,MAX_TORSO_PITCH);

        Bottle info;
        icart_arm->getInfo(info);
        fprintf(stdout,"info _____= %s\n",info.toString().c_str());

        //icart_arm->registerEvent(*this);
    /*


        // print out some info about the controller
        Bottle info;
        icart->getInfo(info);
        fprintf(stdout,"info = %s\n",info.toString().c_str());

        // register the event, attaching the callback
        icart->registerEvent(*this);

        xd.resize(3);
        od.resize(4);

    int nj=0;
    pos_arm->getAxes(&nj);
    Vector encoders;
    Vector tmp;
    encoders.resize(nj);
    tmp.resize(nj);
    
    int i;
    for (i = 0; i < nj; i++) {
         tmp[i] = 20.0;
    }
    pos_arm->setRefAccelerations(tmp.data());

    for (i = 0; i < nj; i++) {
        tmp[i] = 10.0;
        pos_arm->setRefSpeed(i, tmp[i]);
    }
*/
    //pos_arm->setRefSpeeds(tmp.data());

    /*printf("waiting for encoders");
    while(!encs_arm->getEncoders(encoders.data()))
    {
        Time::delay(0.1);
        printf(".");
    }
    printf("\n;");

*/
    printf("\nHoli\n");
        }

void moveCartArm(){
Network yarp;
    if (!yarp.checkNetwork())
    {
        fprintf(stdout,"Error: yarp server does not seem available\n");
        //return 1;
    }

double t;
double t0;double t1;
t=t0=t1=yarp::os::SystemClock::nowSystem();t=yarp::os::SystemClock::nowSystem();
Vector xd(3);
Vector od(4);
        xd[0]=-0.3;
        xd[1]=-0.1+0.1*cos(2.0*M_PI*0.1*(t-t0));
        xd[2]=+0.1+0.1*sin(2.0*M_PI*0.1*(t-t0));

printf("\n:DD\n");
        // we keep the orientation of the left arm constant:
        // we want the middle finger to point forward (end-effector x-axis)
        // with the palm turned down (end-effector y-axis points leftward);
        // to achieve that it is enough to rotate the root frame of pi around z-axis
        od[0]=0.0; od[1]=0.0; od[2]=1.0; od[3]=M_PI;
if(icart_arm!=NULL){
printf("movinggg arm***\n");
    bool ok;
    ok = right_arm->view(icart_arm);
    //ok = ok && right_arm->view(encs_arm);
printf("movinggg arm...***\n");
    if (!ok) {
        printf("Problems acquiring interfaces\n");
        //return 0;
    }
        Bottle info;
        icart_arm->getInfo(info);
        fprintf(stdout,"possssss _____= %s\n",info.toString().c_str());

        icart_arm->goToPose(xd,od);
}else{
printf("\nKKKK-----\n");
}
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

void closeHand(){

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

    if (!right_arm->isValid()) {
        printf("Device not available.  Here are the known devices:\n");
        printf("%s", Drivers::factory().toString().c_str());
    }
    int nj=0;
    pos_arm->getAxes(&nj);
    Vector encoders;
    Vector command;
    Vector tmp;
    encoders.resize(nj);
    tmp.resize(nj);
    command.resize(nj);
    
    //TODO: what's the function of encoders?
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
printf("moving arm\n");
//body.moveCartArm();
printf("moved cart");
body.moveHeadDown(60);
    int dir = -1;
for (int i=0;i<10;i++){

printf("moving head");
body.moveHeadRight(dir*20);

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

    int times=0;
    while(true)
    {
        times++;
        if (times%2)
        {
body.moveRightHand(-50,20,-10,50);
    
        }
        else
        {
        //rest position
body.moveRightHand(-20,40,-10,30);
        }

        //pos_arm->positionMove(command.data());

        int count=5;
        while(count--)
            {
                Time::delay(0.1);
                /*bool ret=encs_arm->getEncoders(encoders.data());
                
                if (!ret)
                {
                    fprintf(stderr, "Error receiving encoders, check connectivity with the robot\n");
                }
                else
                { 
                    printf("%.1lf %.1lf %.1lf %.1lf\n", encoders[0], encoders[1], encoders[2], encoders[3]);
                }*/
            }
    }

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

