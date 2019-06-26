// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-
//
// A tutorial on how to use the Cartesian Interface to control a limb
// 5.251 25.727 5.850 83.259 64.800 29.700 0.000 59.000 20.000 20.000 20.000 10.000 10.000 10.000 10.000 10.000
//
// Author: Miguel Estrada - <luism@unam.mx>

#include <cstdio>
#include <cmath>

#include <yarp/os/Network.h>
#include <yarp/os/RFModule.h>
#include <yarp/os/RateThread.h>
#include <yarp/os/Time.h>
#include <yarp/sig/Vector.h>
#include <yarp/math/Math.h>


#include <yarp/dev/ControlBoardInterfaces.h>
#include <yarp/dev/CartesianControl.h>
#include <yarp/dev/PolyDriver.h>
#include <yarp/dev/IVelocityControl.h>

#include <vector>

#define CTRL_THREAD_PER     0.02    // [s]
#define PRINT_STATUS_PER    1.0     // [s]
#define MAX_TORSO_PITCH     30.0    // [deg]

using namespace std;
using namespace yarp::os;
using namespace yarp::dev;
using namespace yarp::sig;
using namespace yarp::math;

class ActionThread: public RateThread,
    public CartesianEvent
{

    string robotName;
    ICartesianControl *left_arm_ctrl;
    IPositionControl *head_ctrl;
    PolyDriver head;
    PolyDriver left_arm;
    PolyDriver left_hand;
    IPositionControl *left_hand_ctrl;

    Port input_port;

    Vector xd;
    Vector od;
    bool moved;

    int startup_context_id;

    double t;
    double t0;
    double t1;
    int axe0_pos;
    int axe1_pos;
    int axe2_pos;
    int headRotAngle;

    // the event callback attached to the "motion-ongoing"
    virtual void cartesianEventCallback()
    {
        fprintf(stdout,"20%% of trajectory attained\n");
    }

public:
    BodyCtrlThread(string robotName, int headRotAnge, const double period) : RateThread(int(period*1000.0))
    {
        // we wanna raise an event each time the arm is at 20%
        // of the trajectory (or 80% far from the target)
        this->robotName = robotName;
        this->headRotAngle = headRotAnge;
        cartesianEventParameters.type="motion-ongoing";
        cartesianEventParameters.motionOngoingCheckPoint=0.2;
    }

    virtual bool threadInit()
    {

        string inPortName = "/"+robotName+"/bodyController:ir";
        if(!input_port.open(inPortName))
        {
            yError("Failed creating input port for body controller module (%s)",inPortName);
            return false;
        }

        moved = false;
        // open a left_arm interface to connect to the cartesian server of the simulator
        // we suppose that:
        //
        // 1 - the iCub simulator is running
        //     (launch: iCub_SIM)
        //
        // 2 - the cartesian server is running
        //     (launch: yarprobotinterface --context simCartesianControl)
        //
        // 3 - the cartesian solver for the left arm is running too
        //     (launch: iKinCartesianSolver --context simCartesianControl --part left_arm)
        //
        axe0_pos =0;
        axe1_pos =0;
        axe2_pos =0;

        string leftArmCtrPortName = "/"+robotName+"/left_arm";
        string remote_port = "/icubSim/cartesianController/left_arm";

        Property options("(device cartesiancontrollerclient)");
        options.put("remote",remote_port);
        options.put("local",leftArmCtrPortName);
        if (!left_arm.open(options))
        {
            yError("Couldn't get cartesian left_arm controller (%s). Is the cartesian server running?\n",remote_port.c_str());
            return false;
        }

        string leftHandCtrPortName = "/"+robotName+"/left_hand";
        options.put("device","remote_controlboard");
        options.put("remote","/icubSim/left_arm");
        options.put("local",leftHandCtrPortName);

        if (!left_hand.open(options))
        {
            yError(" Couldn't get left_arm controller (%s). Is the iCub_SIM running?\n",leftHandCtrPortName);
            return false;
        }

        left_arm.view(left_arm_ctrl);
        left_hand.view(left_hand_ctrl);
        // latch the controller context in order to preserve
        // it after closing the module
        // the context contains the dofs status, the tracking mode,
        // the resting positions, the limits and so on.
        left_arm_ctrl->storeContext(&startup_context_id);
        // set trajectory time
        left_arm_ctrl->setTrajTime(1.0);
        // get the torso dofs
        Vector newDof, curDof;
        left_arm_ctrl->getDOF(curDof);
        newDof=curDof;

        // enable the torso yaw and pitch
        // disable the torso roll
        newDof[0]=1;
        newDof[1]=0;
        newDof[2]=1;

        // send the request for dofs reconfiguration
        left_arm_ctrl->setDOF(newDof,curDof);

        // impose some restriction on the torso pitch
        limitTorsoPitch();

        // print out some info about the controller
        //Bottle info;
        //left_arm_ctrl->getInfo(info);
        //fprintf(stdout,"left arm info = %s\n",info.toString().c_str());

        // register the event, attaching the callback
        left_arm_ctrl->registerEvent(*this);

        xd.resize(3);
        od.resize(4);

        string headCtrPortName = "/"+robotName+"/head";
        options.put("device","remote_controlboard");
        options.put("local", headCtrPortName);
        options.put("remote", "/icubSim/head");

        if (!head.open(options))
        {
            yError("Couldn't get head controller. Is the iCub_SIM running?");
            return false;
        }

        head.view(head_ctrl);
        return true;
    }

    virtual void afterStart(bool s)
    {
        if (s)
            printf("Propioception thread started successfully\n");
        else
            printf("Propioception thread did not start\n");

        //t=t0=t1=Time::now();
    }

    virtual void run()
    {
        //moveHeadDown(60);
        //moveLeftArm(-0.35,-0.1,-0.034,0.0,0.0,4.0);
        printf("Propioception Module: waiting for input \n");
        Bottle input;
        input_port.read(input);
        //if (input!=NULL) {
        printf("Got %s\n",input.toString().c_str());
        string action = input.toString();
        bool done = false;

        vector<std::string> actions {"headUp","headDown","headLeft","headRight",
                                     "headLeftUp","headLeftDown","headRightUp",
                                     "headRightDown","handUp","handDown","handLeft",
                                     "handRight","handBackward","handForward","closeHand","openHand"
                                    };


        if(action=="random")
        {

            int randomi = rand() % actions.size();
            printf("Random action ...%d\n",randomi);
            /*for(int i = 0; i < action.size(); i++)
            {*/
            action = actions[rand() % actions.size()];
            //}
        }

        if(action=="headUp")
        {
            printf("Moving head up ...\n");
            done = moveHead(headRotAngle,0,0);
        }
        else if(action=="headDown")
        {
            printf("Moving head down ...\n");
            done = moveHead(-headRotAngle,0,0);
        }
        else if(action=="headLeft")
        {
            printf("Moving head left ...\n");
            done = moveHead(0,0,headRotAngle);
        }
        else if(action=="headRight")
        {
            printf("Moving head right ...\n");
            done = moveHead(0,0,-headRotAngle);
        }
        else if(action=="headLeftUp")
        {
            printf("Moving head left ...\n");
            done = moveHead(headRotAngle,0,headRotAngle);
            /*if(done)
            {
                printf("Moving head up ...\n");
                done = moveHeadUp(headRotAngle);
            }*/
        }

        else if(action=="headLeftDown")
        {
            printf("Moving head left and down...\n");
            done = moveHead(-headRotAngle,0,headRotAngle);
            /*if(moveHeadLeft(headRotAngle))
            {
                printf("Moving head down ...\n");
                done = moveHeadDown(headRotAngle);
            }*/
        }
        else if(action=="headRightUp")
        {
            printf("Moving head right and up ...\n");
            done = moveHead(headRotAngle,0,-1*headRotAngle);


            /*if(moveHeadRight(headRotAngle))
            {
                printf("Moving head up ...\n");
                done = moveHeadUp(headRotAngle);
            }**/
        }
        else if(action=="headRightDown")
        {
            printf("Moving head right and down ...\n");
            done = moveHead(-headRotAngle,0,-headRotAngle);
        }
        else if(action=="handRight")
        {
            printf("Moving left hand to the right ...\n");
            done  = moveLeftArm(-0.35,-0.05,0.1,0.0,0.0,4.0);
        }
        else if(action=="handLeft")
        {
            printf("Moving left hand to the right ...\n");
            done  =  moveLeftArm(-0.35,0.03,0.1,0.0,0.0,4.0);
        }
        else if(action=="handUp")
        {
            printf("Moving left hand up ...\n");
            done  =  moveLeftArm(-0.35,0.03,0.2,0.0,0.0,4.0);
        }
        else if(action=="handDown")
        {
            printf("Moving left hand down ...\n");
            done  =  moveLeftArm(-0.35,0.03,-0.1,0.0,0.0,4.0);
        }
        else if(action=="handBackward")
        {
        }
        else if(action=="handForward")
        {
        }
        else if(action=="closeHand")
        {
            printf("Closing left hand ...\n");
            done  =  closeHand();
            //closeHand();
        }
        else if(action=="openHand")
        {
            printf("Opening left hand ...\n");
            done  =  openHand();
        }
        else
        {
            yWarning("Unknown action :'%s'\n",action.c_str());
        }





        /*
                if(action=="mv lhnd r")
                {
                    printf("Moving left hand to the right...\n");
                    done  = moveLeftArm(-0.35,-0.05,0.1,0.0,0.0,4.0);
                    printf("Done\n");

                }
                else if(strcmp(action.toString().c_str(),"mv lhnd l")==0)
                {
                    printf("Moving left hand to the right...\n");
                    done  =  moveLeftArm(-0.35,0.03,0.1,0.0,0.0,4.0);
                    printf("Done\n");
                }*/

        //if(done){
        Bottle donecmd;
        string response = done?"true":"false";
        donecmd.addString(response);
        input_port.write(donecmd);

        //}
        /*if(!moved){
            //t=Time::now();

           //moveHeadRight(60);
           //moveLeftArm(x,y,z,0.0,0.0,4.0);
           /*
           -x = hacia adelante
           -y = hacia la izquierda
            z = hacia arriba
           */
        //buena moveLeftArm(-0.35,-0.1,-0.1,0.0,0.0,4.0);

        // pra -40 de mulñeca  moveLeftArm(-0.35,-0.1,-0.1,0.0,0.0,4.0);
        /*moveLeftArm(-0.35,-0.1,-0.034,0.0,0.0,4.0);
          //levantar mano 0.256580715101553 0.657913812963914 0.172362917052873
          fprintf(stdout,"moving wrist...\n");
          moveWrist();
          fprintf(stdout,"wrist movement finished...\n");
          closeHand();

         moveLeftArm(-0.35,-0.1,+0.15,0.0,0.0, 8.0);
         fprintf(stdout,"levantando mano...\n");

         moved = true;
         }*/
    }

    virtual void threadRelease()
    {
        // we require an immediate stop
        // before closing the left_arm for safety reason
        left_arm_ctrl->stopControl();
//head_ctrl->stopControl();

        // it's a good rule to restore the controller
        // context as it was before opening the module
        left_arm_ctrl->restoreContext(startup_context_id);

        left_arm.close();
    }

   /* bool moveHeadUp(int angle)
    {
        return moveHead(angle,axe1_pos,axe2_pos);

    }

    bool moveHeadDown(int angle)
    {
        return moveHead(-1*angle,axe1_pos,axe2_pos);
    }

    bool moveHeadRight(int angle)
    {
        return moveHead(0,0,-1*angle);
    }

    bool moveHeadLeft(int angle)
    {
        return moveHead(axe0_pos,axe1_pos,angle);
    }*/

    bool moveHead(int angle0, int angle1, int angle2)
    {
        // printf("DENTRO %d %d %d\n",angle0, angle1, angle2);
        IEncoders *enc;
        IVelocityControl *vel;

        head.view(enc);
        head.view(vel);
        int njoints=0;
        enc->getAxes(&njoints);
        /*Vector v;
        Vector command;
        command.resize(njoints);
        printf("joints %s",njoints);
        v.resize(njoints);*/

        Vector command;
        command.resize(njoints);
        enc->getEncoders(command.data());

        //printf("\ncommando => %s\n\n\n",command.toString().c_str());

        command[0]=command[0]+angle0;
        command[1]=command[1]+angle1;
        command[2]=command[2]+angle2;

        bool done = head_ctrl->positionMove(command.data());

        /* axe0_pos = 0;
         axe1_pos=0;
         axe2_pos=0;*/
        return done;
    }

    /**
    * Moves the left arm of the robot as indicated by the given coordenates and orientation
    * @param[in] x (x>0,0,0) points behind the robot
    * @param[in] y (0,y>0,0) points to the right side of the robot
    * @param[in] z (0,0,z>0) is the normal vector of the floor pointing upwards
    * @return true y the movement succeded
    */
    bool moveLeftArm(double x, double y, double z, double ox,double oy, double oz)
    {

        // translational target part: a circular trajectory
        // in the yz plane centered in [-0.3,-0.1,0.1] with radius=0.1 m
        // and frequency 0.1 Hz
//-0.55387995 0.35 -0.09
//-0.35 -0.09 0.55387995
        /*xd[0]=-0.35;
                 xd[1]=-0.09;
                 xd[2]=+0.55387995;*/
        /*xd[0]=-0.35;
        xd[1]=-0.1;
        xd[2]=+0.002;*/
        xd[0]=x;
        xd[1]=y;
        xd[2]=z;

        // we keep the orientation of the left arm constant:
        // we want the middle finger to point forward (end-effector x-axis)
        // with the palm turned down (end-effector y-axis points leftward);
        // to achieve that it is enough to rotate the root frame of pi around z-axis
        od[0]=ox;
        od[1]=oy;
        od[2]=oz;
        od[3]=-M_PI;

        left_arm_ctrl->goToPose(xd,od);
        bool done = left_arm_ctrl->waitMotionDone(0.1,4.0);
        if(!done)
        {
            printf("Left arm is taking to long to complete the movement\n");
        }
        return done;
    }


    bool moveWrist()
    {
        IEncoders *enc;
        IVelocityControl *vel;

        left_hand.view(enc);
        left_hand.view(vel);

        int njoints=0;
        enc->getAxes(&njoints);
        //Vector v;
        Vector command;
        command.resize(njoints);
        enc->getEncoders(command.data()); //get the current position

        //command.resize(njoints);

        //v.resize(njoints);
        //New positions for the fingers
        //15.0 80.0   0.0   0.0  70.0   0.0  70.0   0.0  60.0
        //0.0   80.0   10.0   40.0   30.0   60.0   20.0   50.0   150.0
        /*command[7]=0.0;
        command[8]= 80.0;
        command[9]=12.0;
        command[10]=18.0;
        command[11]=0.0;  //command[11]=27.0;
        command[12]=90.0; //command[12]=50.0;
        command[13]=20.0;
        command[14]=50.0;
        command[15]=135.0;*/
        /*command[7]=15.0;
        command[8]= 80.0;
        command[9]=0.0;
        command[10]=0.0;
        command[11]=70.0;  //command[11]=27.0;
        command[12]=0.0; //command[12]=50.0;
        command[13]=70.0;
        command[14]=0.0;
        command[15]=60.0;*/
        /*command[5]=-70.0;
               command[7]=0.0; //juntar dedos - = mas abiertos
        command[8]= 88.0; //pulgar hacia palma
        command[9]=0.0; //pulgar despegar
        command[10]=54.0; //doblar pulgar
        command[11]=56.7;  //command[11]=27.0; //doblar indicie hacia palma
        command[12]=104.0; //command[12]=50.0; //doblar indice +=más doblado
        command[13]=70.0; //dedo medio doblado +=mas doblado
        command[14]=61.0; //dedo medio doblas
        command[15]=151.0; //meñique doblar +=mas doblado*/
        command[5]=0.0;
        command[6]=22.2;
        command[7]=58.8;
        command[8]=20.0;
        command[9]=19.8;
        command[10]=19.8;
        command[11]=9.9;
        command[12]=10.8;
        command[13]=9.9;
        command[14]=10.8;
        command[15]=10.8;
        //command[7]=0.0; //juntar dedos - = mas abiertos
        //command[8]= 88.0; //pulgar hacia palma
        // command[9]=0.0; //pulgar despegar
        //command[10]=54.0; //doblar pulgar
        //command[11]=56.7;  //command[11]=27.0; //doblar indicie hacia palma
        //command[12]=104.0; //command[12]=50.0; //doblar indice +=más doblado
        //command[13]=70.0; //dedo medio doblado +=mas doblado
        //command[14]=61.0; //dedo medio doblas
        //command[15]=151.0; //meñique doblar +=mas doblado*/
        //articulación 5 muñeca arroba abajo
        /* command[7]=58.8; //juntar dedos - = mas abiertos
        command[8]= 80.8; //pulgar hacia palma
        command[9]=0.0; //pulgar despegar
        command[10]=104.4; //doblar pulgar
        command[11]=46.8;  //command[11]=27.0; //doblar indicie hacia palma
        command[12]=86.4; //command[12]=50.0; //doblar indice +=más doblado
        command[13]=50.0; //dedo medio doblado +=mas doblado
        command[14]=81.0; //dedo medio doblas
        command[15]=137.0; //meñique doblar +=mas doblado*/
        printf("\ncommando => %s\n\n\n",command.toString().c_str());


        left_hand_ctrl->positionMove(command.data());
        printf("wrist moving upwards...\n");

        bool done = left_arm_ctrl->waitMotionDone(0.1,4.0);
        if(!done)
        {
            printf("Left arm is taking to long to complete the movement\n");
        }
        printf("Done wrist %d\n",done);
        return done;

    }

    bool closeHand()
    {
        IEncoders *enc;
        IVelocityControl *vel;

        left_hand.view(enc);
        head.view(vel);

        int njoints=0;
        enc->getAxes(&njoints);
        Vector v;
        Vector command;
        command.resize(njoints);
        enc->getEncoders(command.data()); //get the current position

        command.resize(njoints);

        v.resize(njoints);
        //New positions for the fingers
        //15.0 80.0   0.0   0.0  70.0   0.0  70.0   0.0  60.0
        //0.0   80.0   10.0   40.0   30.0   60.0   20.0   50.0   150.0
        /*command[7]=0.0;
        command[8]= 80.0;
        command[9]=12.0;
        command[10]=18.0;
        command[11]=0.0;  //command[11]=27.0;
        command[12]=90.0; //command[12]=50.0;
        command[13]=20.0;
        command[14]=50.0;
        command[15]=135.0;*/
        /*command[7]=15.0;
        command[8]= 80.0;
        command[9]=0.0;
        command[10]=0.0;
        command[11]=70.0;  //command[11]=27.0;
        command[12]=0.0; //command[12]=50.0;
        command[13]=70.0;
        command[14]=0.0;
        command[15]=60.0;i/
        */
        printf("\ncommandobe> %s\n\n\n",command.toString().c_str());
        command[5]=-37.0;
        command[7]=0.0; //juntar dedos - = mas abiertos
        command[8]= 88.0; //pulgar hacia palma
        command[9]=0.0; //pulgar despegar
        command[10]=54.0; //doblar pulgar
        command[11]=56.7;  //command[11]=27.0; //doblar indicie hacia palma
        command[12]=104.0; //command[12]=50.0; //doblar indice +=más doblado
        command[13]=70.0; //dedo medio doblado +=mas doblado
        command[14]=61.0; //dedo medio doblas
        command[15]=151.0; //meñique doblar +=mas doblado*/
        //articulación 5 muñeca arroba abajo
        /* command[7]=58.8; //juntar dedos - = mas abiertos
        command[8]= 80.8; //pulgar hacia palma
        command[9]=0.0; //pulgar despegar
        command[10]=104.4; //doblar pulgar
        command[11]=46.8;  //command[11]=27.0; //doblar indicie hacia palma
        command[12]=86.4; //command[12]=50.0; //doblar indice +=más doblado
        command[13]=50.0; //dedo medio doblado +=mas doblado
        command[14]=81.0; //dedo medio doblas
        command[15]=137.0; //meñique doblar +=mas doblado*/

        bool done = left_hand_ctrl->positionMove(command.data());
        //printf("closing hand...%d\n",done);

    }

    bool openHand()
    {
        IEncoders *enc;
        IVelocityControl *vel;

        left_hand.view(enc);
        head.view(vel);

        int njoints=0;
        enc->getAxes(&njoints);
        Vector v;
        Vector command;
        command.resize(njoints);
        enc->getEncoders(command.data()); //get the current position
        command.resize(njoints);
        v.resize(njoints);

        //0.000723	 79.997541	 0.000561	 49.999764	-0.000103	 30.000009	 0.000003	 58.999975	 19.999992	 19.999991	 19.999992	 9.999996	 9.999996	 9.999996	 9.999996	 9.999997
        command[0]= 0.000723;
        command[1]= 79.997541;
        command[2]= 0.000561;
        command[3]= 49.999764;
        command[4]= -0.000103;
        command[5]= 30.000009;
        command[6]=0.000003; //juntar dedos - = mas abiertos
        command[7]= 58.999975; //pulgar hacia palma
        command[8]=19.999992; //pulgar despegar
        command[9]=19.999991; //doblar pulgar
        command[10]=19.999992;//command[11]=27.0; //doblar indicie hacia palma
        command[11]=9.999996; //command[12]=50.0; //doblar indice +=más doblado
        command[12]=9.999996; //dedo medio doblado +=mas doblado
        command[13]=9.999996; //dedo medio doblas
        command[14]=9.999996; //meñique doblar +=mas doblado*/
        command[15]=9.999997;

        printf("COMMAND");
        bool done = left_hand_ctrl->positionMove(command.data());

    }


    void limitTorsoPitch()
    {
        int axis=0; // pitch joint
        double min, max;

        // sometimes it may be helpful to reduce
        // the range of variability of the joints;
        // for example here we don't want the torso
        // to lean out more than 30 degrees forward

        // we keep the lower limit
        left_arm_ctrl->getLimits(axis,&min,&max);
        left_arm_ctrl->setLimits(axis,min,MAX_TORSO_PITCH);
    }

    void printStatus()
    {
        if (t-t1>=PRINT_STATUS_PER)
        {
            Vector x,o,xdhat,odhat,qdhat;

            // we get the current arm pose in the
            // operational space
            left_arm_ctrl->getPose(x,o);

            // we get the final destination of the arm
            // as found by the solver: it differs a bit
            // from the desired pose according to the tolerances
            left_arm_ctrl->getDesired(xdhat,odhat,qdhat);

            double e_x=norm(xdhat-x);
            double e_o=norm(odhat-o);

            fprintf(stdout,"+++++++++\n");
            fprintf(stdout,"xd          [m] = %s\n",xd.toString().c_str());
            fprintf(stdout,"xdhat       [m] = %s\n",xdhat.toString().c_str());
            fprintf(stdout,"x           [m] = %s\n",x.toString().c_str());
            fprintf(stdout,"od        [rad] = %s\n",od.toString().c_str());
            fprintf(stdout,"odhat     [rad] = %s\n",odhat.toString().c_str());
            fprintf(stdout,"o         [rad] = %s\n",o.toString().c_str());
            fprintf(stdout,"norm(e_x)   [m] = %g\n",e_x);
            fprintf(stdout,"norm(e_o) [rad] = %g\n",e_o);
            fprintf(stdout,"---------\n\n");

            t1=t;
        }
    }
};



class Action: public RFModule
{
protected:
    ActionThread *actionThr;

public:
    virtual bool configure(ResourceFinder &rf)
    {

        //Time::turboBoost();

        string robotName = rf.check("robot",Value("jacub")).asString();

        int headRotAngle = rf.check("head_rot_angle",Value("10")).asInt();

        actionThr=new BodyCtrlThread(robotName,headRotAngle, CTRL_THREAD_PER);
        if (!actionThr->start())
        {
            delete actionThr;
            return false;
        }

        return true;
    }

    virtual bool close()
    {
    	actionThr->stop();
        delete actionThr;

        return true;
    }

    virtual double getPeriod()
    {
        return 1.0;
    }
    virtual bool   updateModule()
    {
        return true;
    }
};



int main(int argc, char *argv[])
{
    Network yarp;

    if (!yarp.checkNetwork())
    {
        yError(" yarp server does not seem available\n");
        return 1;
    }

    ResourceFinder rf;
    rf.setVerbose();
    rf.setDefaultConfigFile("config.ini");
    rf.configure(argc,argv);

    string robotName = rf.check("robot",Value("jacub")).asString();
    Action action;

    return action.runModule(rf);
}
