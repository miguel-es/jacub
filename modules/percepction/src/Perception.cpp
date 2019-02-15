// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

/**
* @ingroup icub_tutorials
* \defgroup imageProc imageProc
*/

#include <stdio.h>
#include <yarp/os/Network.h>
#include <yarp/os/BufferedPort.h>
#include <yarp/os/BinPortable.h>
#include <yarp/sig/Image.h>
#include <yarp/os/Time.h>
#include <yarp/os/Property.h>

#include <string>
#include <cmath>
#include "VisualContext.h"

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/prettywriter.h"


using namespace yarp::sig;
using namespace yarp::os;
using namespace rapidjson;

int main(int argc, char *argv[])
{
    Network yarp;
    VisualContext v;
    BufferedPort<ImageOf<PixelRgb> > imagePort;  // make a port for reading images
    BufferedPort<ImageOf<PixelRgb> > outPort;
    BufferedPort<VisualContext> contextOutPort;

    imagePort.open("/imageProc/image/in");  // give the port a name
    outPort.open("/imageProc/image/out");
    contextOutPort.open("/perception/context/out");

    while (true) { // repeat forever
        ImageOf<PixelRgb> *image = imagePort.read();  // read an image
        ImageOf<PixelRgb> &outImage = outPort.prepare(); //get an output image
        VisualContext &outContext = contextOutPort.prepare(); //get an output image
        outContext.color="c1";
outContext.size="s1";
outContext.moving=true;

//		VisualContext v;
//v.size="fuck";
//if(outContext.size=="holis"){printf("igual\n");}
//printf(v.size.c_str());
 printf("size igual a %s\n",outContext.size.c_str());
 //printf("size igual a holis nop %s \n",outContext.size.c_str());

 //if(outContext.content().size=="holis"){printf("igual");}else{printf("diferet2");}
  //  printf("color %i, %s",outContext.content().color,outContext.content().size);
        outImage=*image;

        if (image!=NULL) { // check we actually got something
            printf("We got an image of size %dx%d\n", image->width(), image->height());
            double xMean = 0;
            double yMean = 0;
            double xBoxMax = image->width()-1;
            double yBoxMax = image->height()-1;
            double xBoxMin = -1;
            double yBoxMin = -1;

            int ct = 0;
            for (int x=0; x<image->width(); x++) {
                for (int y=0; y<image->height(); y++) {
                    PixelRgb& pixel = image->pixel(x,y);
                    // very simple test for blueishness
                    // make sure blue level exceeds red and green by a factor of 2
                    if (pixel.b>pixel.r*1.2+10 && pixel.b>pixel.g*1.2+10) {
                        // there's a blueish pixel at (x,y)!
                        // let's find the average location of these pixels
                        if(x<xBoxMax) xBoxMax = x;
                        if(y<yBoxMax) yBoxMax = y;
                        if(x>xBoxMin) xBoxMin = x;
                        if(y>yBoxMin) yBoxMin = y;

                        xMean += x;
                        yMean += y;
                        ct++;

                        outImage(x,y).r=pixel.r;
                        outImage(x,y).g=pixel.g;
                        outImage(x,y).b=pixel.b;
                    }else{
                        double gray = (outImage(x,y).r+outImage(x,y).g+outImage(x,y).b)/3;
                        outImage(x,y).r=gray;
                        outImage(x,y).g=gray;
                        outImage(x,y).b=gray;
                    }
                    if(x==106 or x==212){
                        outImage(x,y).r = 0;
                        outImage(x,y).b = 0;
                        outImage(x,y).g = 255;
                    }
                    if(y==80 or y==160){
                        outImage(x,y).r = 0;
                        outImage(x,y).b = 0;
                        outImage(x,y).g = 255;

                    }
                }
            }
            //printf("Bounding box xmin %g ymin %g xmax %g ymax %g",xBoxMin,yBoxMin,xBoxMax,yBoxMax);
            double centroidx = xBoxMax +floor((xBoxMin-xBoxMax)/2);
            double centroidy = yBoxMax+floor((yBoxMin-yBoxMax)/2);
            //printf("Centroid (%g, %g)",centroidx,centroidy);
            outImage(centroidx,centroidy).r = 255;
            outImage(centroidx,centroidy).b = 0;
            outImage(centroidx,centroidy).g = 0;
            outImage(centroidx+1,centroidy).r = 255;
            outImage(centroidx+1,centroidy).b = 0;
            outImage(centroidx+1,centroidy).g = 0;
            outImage(centroidx-1,centroidy).r = 255;
            outImage(centroidx-1,centroidy).b = 0;
            outImage(centroidx-1,centroidy).g = 0;

            outImage(centroidx,centroidy+1).r = 255;
            outImage(centroidx,centroidy+1).b = 0;
            outImage(centroidx,centroidy+1).g = 0;
            outImage(centroidx,centroidy-1).r = 255;
            outImage(centroidx,centroidy-1).b = 0;
            outImage(centroidx,centroidy-1).g = 0;

            if (ct>0) {
                xMean /= ct;
                yMean /= ct;
            }
            if (ct>(image->width()/20)*(image->height()/20)) {
                printf("Best guess at blue target: %g %g\n", xMean, yMean);
            }

            outPort.write();
            contextOutPort.write();
            int sector = 0;
            int xpos = floor(centroidx / 106);
            int ypos = floor(centroidy / 80);
            //printf("Centroid coordinates: %d, %d\n", xpos, ypos);
            Document context;
            context.SetObject();
            Document::AllocatorType& allocator = context.GetAllocator();
            context.AddMember("color", "blue", allocator);
            context.AddMember("size", "s1", allocator);
            context.AddMember("moving", false, allocator);
            context.AddMember("sector", 3*ypos+xpos+1,allocator);

            StringBuffer sb;
        PrettyWriter<StringBuffer> writer(sb);
        context.Accept(writer);
        auto str = sb.GetString();
        printf("Context %s\n", str);


            //printf(string(context.GetString()));

        }
    }
    return 0;
}



