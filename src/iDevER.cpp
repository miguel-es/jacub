#include <yarp/os/all.h>
#include <yarp/os/BinPortable.h>
#include <iostream>

#include "VisualContext.h"
using namespace std;
using namespace yarp::os;
int main(int argc, char *argv[]) {
    Network yarp;
    BufferedPort<VisualContext> port;
    port.open("/iDevER/context/in");
    while (true) {
        cout << "waiting for input" << endl;
        VisualContext *input = port.read();
        if (input!=NULL) {
            //cout << "got context" << input->color << endl;
printf("recibido = %s",input->size.c_str());
printf("recibido moving = %s",input->moving);
//		VisualContext v;
/*if(input->size.c_str()=="holis"){
 //printf("size igual a holis fuck %s",v.size);
 printf("size igual a holis");
}*/

//if(input->size=="holis"){printf("igual\n");}

//}
//cout << "got sis" << input->content().size << endl;
            double total = 0;
       }
    }
    return 0;
}
