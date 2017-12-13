#include <yarp/os/all.h>
#include <yarp/dev/PolyDriver.h>
#include <yarp/dev/IRGBDSensor.h>
#include <yarp/dev/IJoypadController.h>
#include <stdio.h>
#include <yarp/os/ResourceFinder.h>
#include <bitset>
#include <yarp/os/RateThread.h>
#include <yarp/os/Mutex.h>
#include <yarp/serversql/Server.h>
#include <yarp/os/Timer.h>

using namespace yarp::os;
using namespace yarp::dev;
using namespace yarp::sig;
using namespace std;

void delay(double s)
{
    volatile int a = 0;
    double currtime = yarp::os::Time::now();
    while (yarp::os::Time::now() - currtime < s)
    {
        a++;
    }
    return;
}

class threadA : public RateThread
{
private:
    int a = 1;

public:
    Mutex m;
    threadA() : RateThread(5) {}
    virtual ~threadA() {}
    virtual bool threadInit()
    {
        return true;
    }

    void f(string p, int &b)
    {
        m.lock();
        b++;
        yDebug() << p << b;
        m.unlock();
    }

    virtual void run()
    {
        m.lock();
        static int n = 0;
        a++;
        yDebug() << "a" << a;
        delay(0.1);
        m.unlock();
    }
};

class threadB : public RateThread
{
public:
    threadA* a;
    int cane = 0;
    std::string l;
    threadB(std::string inL) : RateThread(20), l(inL) {}
    virtual ~threadB() {}
    virtual bool threadInit()
    {
        return a != nullptr;
    }

    virtual void run()
    {
        a->f(l, cane);
    }
};

int testJoypad(int argc, char *argv[])
{
    IJoypadController* controller;
    PolyDriver         client;
    Property           clientCfg, serverCfg;
    bool               allfine;


    controller = 0;

    ResourceFinder r;
    r.configure(argc, argv);

    clientCfg.put("device", "JoypadControlClient");
    clientCfg.put("local",  "/joyclient");
    clientCfg.put("remote", "/dualshock4");
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

        message = "Stick:\n";
        controller->getStickCount(count);
        for(int i = 0; i < count; ++i)
        {
            Vector data;
            controller->getStick(i, data, yarp::dev::IJoypadController::JypCtrlcoord_CARTESIAN);
            message += "  n_" + to_string(i) + ": ";
            for (int j = 0; j < data.size(); ++j)
            {
                message += to_string(data[j]) + " ";
            }
            if (i + 1 < count) {
                message += "\n";
            }
        }
        yInfo() << message;

        message = "trackball:\n";
        controller->getTrackballCount(count);
        for(int i = 0; i < count; ++i)
        {
            Vector data;
            controller->getTrackball(i, data);
            message += "  n_" + to_string(i) + ": ";
            for (int j = 0; j < data.size(); ++j)
            {
                message += to_string(data[j]) + " ";
            }
            if (i + 1 < count) {
                message += "\n";
            }
        }

        message = "touch Surface:\n";
        controller->getTouchSurfaceCount(count);
        for(int i = 0; i < count; ++i)
        {
            Vector data;
            controller->getTouch(i, data);
            message += "  n_" + to_string(i) + ": ";
            for (int j = 0; j < data.size(); ++j)
            {
                message += to_string(data[j]) + " ";
            }
            if (i + 1 < count) {
                message += "\n";
            }
        }
        yInfo() << message;
        yInfo() << "---------------------------------------";
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

int testConnectUDP(int argc, char *argv[])
{
    yarp::os::Network yarp;

    yarp::os::BufferedPort<yarp::sig::Vector> port;

    yarp::os::Contactable* ctbl = dynamic_cast<yarp::os::Contactable*>(&port);

    ctbl->open("/stica");
//    yarp::os::Network::sync(ctbl->getName());
    if (!yarp::os::NetworkBase::connect("/dualshock4/buttons:o", ctbl->getName()), "udp") {
        yError() << "Connection failed";
        return 1;
    }

    while (true) {
        Vector *v = port.read();
        if (v) {
            yDebug() << v->toString();
        }
        yarp::os::Time::delay(1.0);
    }

    ctbl->interrupt();
    ctbl->close();
    return 0;
}

int testThreads()
{
    threadA a;
    threadB b("b");
    threadB c("c");
    threadB d("d");
    b.a = &a;
    c.a = &a;
    d.a = &a;
    a.start();
    b.start();
    c.start();
    d.start();
}

class yarpServerThread : public yarp::os::Thread
{
    typedef yarp::serversql::Server Server;

    Server ys;
    int    argc;
    char** argv;

    virtual void run() override
    {
        ys.run(argc, argv);
    }

    virtual void onStop() override
    {
        ys.stop();
    }
public:
    virtual void configure(int inArgc, char** inArgv)
    {
        argc = inArgc;
        argv = inArgv;
    }
};

class rateport : public yarp::os::RateThread,
                 public yarp::os::Port
{
public:
    rateport() : RateThread(10){}
    virtual void run()
    {
        Bottle b;
        read(b);
        yDebug() << "ssfddfsfdsdf" << b.get(0).asInt();
    }
};

int testPersistent(int argc, char* argv[])
{
#define range(a) int i = 0; i < a; i++
    yarpServerThread ys;
    ys.configure(argc, argv);
    ys.start();
    yarp::os::Port a;
    rateport b;
    a.open("/a");
    b.open("/b");
    b.start();
    yarp::os::ContactStyle style;
    style.persistent = true;
    yarp::os::Network::connect("/a", "/b");
    Bottle bot;
    bot.addInt(42);

    for(range(100))
    {
        a.write(bot);
        yDebug() << "closing a";
        a.close();
        yarp::os::Time::delay(1);
        yDebug() << "opening a";
        a.open("/a");
    }

    return 0;
}

void MiaStaticCallback(const yarp::os::YarpTimerEvent& event)
{
    yDebug() << event.lastDuration + 100;
}

class TimerTest
{
public:

    bool a{true};
    yarp::os::Mutex m;
    TimerTest()
    {
        timer.start();
        timer2.start();
        timer3.start();
        yarp::os::Time::delay(20);
    }

    bool checkTime(const yarp::os::YarpTimerEvent& event)
    {
        static auto ev = event;
        yDebug() << ev.currentExpected - event.lastExpected;
        yDebug() << ev.currentReal - event.lastReal;
        yDebug() << ev.lastDuration;
        yDebug() << event.runCount;
        ev = event;
        return true;
    }

    bool miacallback(const yarp::os::YarpTimerEvent& event)
    {
        if(!a)
        {
            yError() << "no";
        }

        a = false;
        yarp::os::Time::delay(0.8);
        if(a)
        {
            yError() << "noooooo";
        }
        else
        {
            yInfo() << "yes";
        }
        return true;
    }

    bool miacallback2(const yarp::os::YarpTimerEvent& event)
    {
        a = true;
        return true;
    }

    TimerSettings t{std::chrono::milliseconds(1000)};

    yarp::os::Timer timer{t, &TimerTest::miacallback, this, &m};
    yarp::os::Timer timer2{TimerSettings(std::chrono::milliseconds(100)), &TimerTest::miacallback2, this, &m};
    yarp::os::Timer timer3{TimerSettings(std::chrono::milliseconds(1000)), &TimerTest::checkTime, this};
};

int testTimer()
{
    TimerTest();
    return 0;
}

int main(int argc, char* argv[])
{
    yarp::os::Network net;
    //testRGBD(argc, argv);
    //testJoypad(argc, argv);
    //testConnectUDP(argc, argv);
    //testThread();
    //testPersistent(argc, argv);
    testTimer();

}
