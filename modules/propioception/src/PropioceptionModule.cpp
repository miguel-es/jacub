 // -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-
 //
 // A tutorial on how to use the Cartesian Interface to control a limb
 // in the operational space.
 //
 // Author: Ugo Pattacini - <ugo.pattacini@iit.it>

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

 #define CTRL_THREAD_PER     0.02    // [s]
 #define PRINT_STATUS_PER    1.0     // [s]
 #define MAX_TORSO_PITCH     30.0    // [deg]

 using namespace std;
 using namespace yarp::os;
 using namespace yarp::dev;
 using namespace yarp::sig;
 using namespace yarp::math;

 class BodyCtrlThread: public RateThread,
                   public CartesianEvent
 {

     ICartesianControl *left_arm_ctrl;
     IPositionControl *head_ctrl;
 	 PolyDriver head;
     PolyDriver left_arm;
     PolyDriver left_hand;
     IPositionControl *left_hand_ctrl;

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

     // the event callback attached to the "motion-ongoing"
     virtual void cartesianEventCallback()
     {
         fprintf(stdout,"20%% of trajectory attained\n");
     }

 public:
     BodyCtrlThread(const double period) : RateThread(int(period*1000.0))
     {
         // we wanna raise an event each time the arm is at 20%
         // of the trajectory (or 80% far from the target)
         cartesianEventParameters.type="motion-ongoing";
         cartesianEventParameters.motionOngoingCheckPoint=0.2;
     }

     virtual bool threadInit()
     {
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

        Property option("(device cartesiancontrollerclient)");
        option.put("remote","/icubSim/cartesianController/left_arm");
        option.put("local","/jacub/left_arm");
        if (!left_arm.open(option)){
            printf("Couldn't get cartesian left_arm controller. Is the cartesian server running?");
             return false;
        }

	    option.put("device","remote_controlboard");
	    option.put("local","/jacub/left_hand");
	    option.put("remote","/icubSim/left_arm");
        if (!left_hand.open(option)){
            printf("Couldn't get left_arm controller. Is the iCub_SIM running?");
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

	    option.put("device","remote_controlboard");
	    option.put("local", "/jacub/head");
	    option.put("remote", "/icubSim/head");

        if (!head.open(option)){
             printf("Couldn't get head controller. Is the iCub_SIM running?");
             return false;
        }

        head.view(head_ctrl);
        return true;
     }

     virtual void afterStart(bool s)
     {
         if (s)
             printf("Body thread started successfully\n");
         else
             printf("Body thread did not start\n");

         //t=t0=t1=Time::now();
     }

     virtual void run()
     {
     if(!moved){
         //t=Time::now();
        moveHeadDown(60);
        //moveHeadRight(60);
        //moveLeftArm(x,y,z,0.0,0.0,4.0);
        /*
        -x = hacia adelante
        -y = hacia la izquierda
         z = hacia arriba
        */
       //buena moveLeftArm(-0.35,-0.1,-0.1,0.0,0.0,4.0);

      // pra -40 de mulñeca  moveLeftArm(-0.35,-0.1,-0.1,0.0,0.0,4.0);
      moveLeftArm(-0.35,-0.1,-0.034,0.0,0.0,4.0);
        //levantar mano 0.256580715101553 0.657913812963914 0.172362917052873
        fprintf(stdout,"moving wrist...\n");
        moveWrist();
        fprintf(stdout,"wrist movement finished...\n");
        closeHand();

       moveLeftArm(-0.35,-0.1,+0.15,0.0,0.0, 8.0);
       fprintf(stdout,"levantando mano...\n");

       moved = true;
       }
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

    bool moveHead(int angle0, int angle1, int angle2){

	IEncoders *enc;
    IVelocityControl *vel;

    head.view(enc);
    head.view(vel);
    int njoints=0;
    enc->getAxes(&njoints);
    Vector v;
    Vector command;
    command.resize(njoints);
    v.resize(njoints);
    command[0]=angle0;
    command[1]=angle1;
    command[2]=angle2;

    head_ctrl->positionMove(command.data());

    axe0_pos = angle0;
    axe1_pos=angle1;
    axe2_pos=angle2;
    }

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
         od[0]=ox; od[1]=oy; od[2]=oz; od[3]=-M_PI;

         left_arm_ctrl->goToPose(xd,od);
         bool done = left_arm_ctrl->waitMotionDone(0.1,4.0);
         if(!done){
            printf("Left arm is taking to long to complete the movement\n");
        }
        return done;
     }


    bool moveWrist(){
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
         if(!done){
            printf("Left arm is taking to long to complete the movement\n");
        }
        printf("Done wrist %d\n",done);
        return done;

    }

    bool closeHand(){
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
        command[15]=60.0;*/
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
        printf("closing hand...%d\n",done);

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



 class Propioception: public RFModule
 {
 protected:
     BodyCtrlThread *thr;

 public:
     virtual bool configure(ResourceFinder &rf)
     {
         Time::turboBoost();

         thr=new BodyCtrlThread(CTRL_THREAD_PER);
         if (!thr->start())
         {
             delete thr;
             return false;
         }

         return true;
     }

     virtual bool close()
     {
         thr->stop();
         delete thr;

         return true;
     }

     virtual double getPeriod()    { return 1.0;  }
     virtual bool   updateModule() { return true; }
 };



 int main()
 {
     Network yarp;

     if (!yarp.checkNetwork())
     {
         printf("Error: yarp server does not seem available\n");
         return 1;
     }

     Propioception propiomod;

     ResourceFinder rf;

     return propiomod.runModule(rf);
 }
