#include <yarp/os/all.h>
#include <yarp/dev/PolyDriver.h>
#include <yarp/dev/IRGBDSensor.h>
#include <yarp/dev/IJoypadController.h>
#include <stdio.h>
#include <yarp/os/ResourceFinder.h>
#include <bitset>
using namespace yarp::os;
using namespace yarp::dev;
using namespace yarp::sig;
using namespace std;

int testJoypad(int argc, char *argv[])
{
    IJoypadController* controller;
    PolyDriver         client;
    PolyDriver         server;
    Property           clientCfg, serverCfg;
    bool               allfine;


    controller = 0;

    ResourceFinder r;
    r.configure(argc, argv);

    serverCfg.put("device",             "JoypadControlServer");
    serverCfg.put("subdevice",          "SDLJoypad");
    serverCfg.put("name",               "/joypad");
    serverCfg.put("use_separate_ports", 1);
    allfine = server.open(serverCfg);
    yarp::os::Time::delay(2);
    if(!allfine) yError() << "server not fine";

    clientCfg.put("device", "JoypadControlClient");
    clientCfg.put("local",  "/joyclient");
    clientCfg.put("remote", "/joypad");
    allfine &= client.open(clientCfg);
    if(!allfine) yError() << "client not fine";
    allfine &= client.view(controller);
    if(!allfine) yError() << "view not fine";

    if(!controller)
    {
        yError() << "something very wrong here..";
        return 1;
    }

    while(true)
    {
        string  message;
        unsigned int count;

        message = "Axes: ";
        controller->getAxisCount(count);
        for(int i = 0; i < count; ++i)
        {
            double data;
            controller->getAxis(i, data);
            message += to_string(data) + " ";
        }
        yInfo() << message;

        message = "Hats: ";
        controller->getHatCount(count);
        for(int i = 0; i < count; ++i)
        {
            unsigned char data;
            controller->getHat(i, data);
            message += to_string(data) + " ";
        }
        yInfo() << message;

        message = "Buttons: ";
        controller->getButtonCount(count);
        for(int i = 0; i < count; ++i)
        {
            float data;
            controller->getButton(i, data);
            message += to_string(data) + " ";
        }
        yInfo() << message;

        message = "Stick: ";
        controller->getStickCount(count);
        for(int i = 0; i < count; ++i)
        {
            Vector data;
            controller->getStick(i, data, yarp::dev::IJoypadController::JypCtrlcoord_CARTESIAN);
            message += "n_" + to_string(i) + ": ";
            for (int j = 0; j < data.size(); ++j)
            {
                message += to_string(data[j]) + " ";
            }
            message += "\n";

        }
        yInfo() << message;

        message = "trackball: ";
        controller->getTrackballCount(count);
        for(int i = 0; i < count; ++i)
        {
            Vector data;
            controller->getTrackball(i, data);
            message += "n_" + to_string(i) + ": ";
            for (int j = 0; j < data.size(); ++j)
            {
                message += to_string(data[j]) + " ";
            }
            message += "\n";
        }

        message = "touch Surface: ";
        controller->getTouchSurfaceCount(count);
        for(int i = 0; i < count; ++i)
        {
            Vector data;
            controller->getTouch(i, data);
            message += "n_" + to_string(i) + ": ";
            for (int j = 0; j < data.size(); ++j)
            {
                message += to_string(data[j]) + " ";
            }
            message += "\n";
        }
        yInfo() << message;
        yarp::os::Time::delay(0.1);
    }
}

int testRGBD(int argc, char *argv[])
{
    IRGBDSensor* depthCamera;
    Matrix       m;
    PolyDriver   client;
    PolyDriver   server;
    Property     clientCfg, serverCfg, p;
    double       d1, d2;
    bool         allfine;


    depthCamera = 0;

    ResourceFinder r;
    r.configure(argc, argv);

    serverCfg.put("device",    "RGBDSensorWrapper");
    serverCfg.put("subdevice", "depthCamera");
    serverCfg.put("name",      "/depthCamera");
    //serverCfg.put("from",      "/home/aruzzenenti/RGBDSensorClient.ini ");
    allfine = server.open(r);
    yarp::os::Time::delay(2);
    if(!allfine) yError() << "server not fine";

    clientCfg.put("device",          "RGBDSensorClient");
    clientCfg.put("localImagePort",  "/rgbdClient/rgb:i");
    clientCfg.put("localDepthPort",  "/rgbdClient/dep:i");
    clientCfg.put("localRpcPort",    "/rgbdClient/rpc:o");
    clientCfg.put("remoteImagePort", "/depthCamera/rgbImage:o");
    clientCfg.put("remoteDepthPort", "/depthCamera/depthImage:o");
    clientCfg.put("remoteRpcPort",   "/depthCamera/rpc:i");
    allfine &= client.open(clientCfg);
    if(!allfine) yError() << "client not fine";
    allfine &= client.view(depthCamera);
    if(!allfine) yError() << "view not fine";

    if(!depthCamera)
    {
        yError() << "something very wrong here..";
        return 1;
    }

    yInfo() << "D_accuracy"    << depthCamera->getDepthAccuracy()         << "\n";
    yInfo() << "D_clip planes" << depthCamera->getDepthClipPlanes(d1, d2);
    yInfo()                    << d1 << d2                                << "\n";

    yInfo() << "D_fov"         << depthCamera->getDepthFOV(d1, d2);
    yInfo()                    << d1 << d2                                << "\n";
    yInfo() << "D_height"      << depthCamera->getDepthHeight();
    yInfo() << "D_width"       << depthCamera->getDepthWidth()            << "\n";

    yInfo() << "D_intrinsic"   << depthCamera->getDepthIntrinsicParam(p);
    yInfo()                    << p.toString()                            << "\n";

    yInfo() << "D_extrinsic"   << depthCamera->getExtrinsicParam(m);
    yInfo()                    << m.toString()                            << "\n";

    yInfo() << "D_lastError"   << depthCamera->getLastErrorMsg()          << "\n";

    yInfo() << "C_fov"         << depthCamera->getRgbFOV(d1, d2);
    yInfo()                    << d1 << d2                                << "\n";

    yInfo() << "C_Height"      << depthCamera->getRgbHeight();
    yInfo() << "C_width"       << depthCamera->getRgbWidth()              << "\n";

    yInfo() << "C_intrinsic"   << depthCamera->getRgbIntrinsicParam(p);
    yInfo()                    << p.toString()                            << "\n";

    yInfo() << "status"        << depthCamera->getSensorStatus()          << "\n";

    yInfo() << "D_set_accuracy, expected 0.001-true_0.0001-true_false\n";
    yInfo() << depthCamera->getDepthAccuracy() << depthCamera->setDepthAccuracy(0.001);
    yInfo() << depthCamera->getDepthAccuracy() << depthCamera->setDepthAccuracy(0.0001);
    yInfo() << depthCamera->setDepthAccuracy(0.005) << "\n";

    yInfo() << "depth clip planes";
    yInfo() << depthCamera->setDepthClipPlanes(2, 8);
    depthCamera->getDepthClipPlanes(d1, d2);
    yInfo() << d1 << d2 << "\n";

    bool b;
    b = false;
    yInfo() << "MIRRORING expexted: true, true, true, true, true, false, true, true, true, false, true, true";
    yInfo() << depthCamera->setRgbMirroring(true);
    yInfo() << depthCamera->setDepthMirroring(false);
    yInfo() << depthCamera->getRgbMirroring(b);
    yInfo() << b;
    yInfo() << "get depth mirroring"; yInfo() << depthCamera->getDepthMirroring(b);
    yInfo() << b;
    b = true;
    yInfo() << depthCamera->setRgbMirroring(false);
    yInfo() << depthCamera->setDepthMirroring(true);
    yInfo() << depthCamera->getRgbMirroring(b);
    yInfo() << b;
    yInfo() << depthCamera->getDepthMirroring(b);
    yInfo() << b;

    yInfo() << "D_lastError"   << depthCamera->getLastErrorMsg()          << "\n";

    yInfo() << "depth fov";
    yInfo() << depthCamera->setDepthFOV(35, 50);
    depthCamera->getDepthFOV(d1, d2);
    yInfo() << d1 << d2 << "\n";

    yInfo() << "D_lastError"   << depthCamera->getLastErrorMsg()          << "\n";

    yInfo() << "resolution to 640x480";
    yInfo() << depthCamera->setDepthResolution(640, 480);

    yInfo() << depthCamera->getDepthWidth() << depthCamera->getDepthHeight() << "\n";

    yInfo() << "D_lastError"   << depthCamera->getLastErrorMsg()          << "\n";

    yInfo() << "depth fov";
    yInfo() << depthCamera->setRgbFOV(35, 50);
    depthCamera->getRgbFOV(d1, d2);
    yInfo() << d1 << d2 << "\n";

    yInfo() << "resolution to 640x480";
    yInfo() << depthCamera->setRgbResolution(640, 480);
    yInfo() << depthCamera->getRgbWidth() << depthCamera->getRgbHeight() << "\n";

    BufferedPort<FlexImage>            portColor;
    BufferedPort<ImageOf<PixelFloat> > portDepth;

    portColor.open("/rgbdtest_color");
    portDepth.open("/rgbdtest_depth");


    while (true)
    {
        yarp::os::Time::delay(0.030);
        FlexImage&               color = portColor.prepare();
        ImageOf<PixelFloat>&     depth = portDepth.prepare();
        depthCamera->getImages(color, depth);

        portColor.write();
        portDepth.write();
    }
    return 0;
}

int main(int argc, char* argv[])
{
    //testRGBD(argc, argv);
    testJoypad(argc, argv);
    return 0;
}
