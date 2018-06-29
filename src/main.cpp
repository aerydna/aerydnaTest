#include <yarp/os/all.h>
#include <yarp/dev/PolyDriver.h>
#include <yarp/dev/IRGBDSensor.h>
#include <yarp/dev/IJoypadController.h>
#include <stdio.h>
#include <yarp/os/ResourceFinder.h>
#include <bitset>
#include <yarp/os/RateThread.h>
#include <yarp/os/Mutex.h>
#include <yarp/os/Timer.h>
#include <yarp/rosmsg/std_msgs/String.h>
#include <thread>

using namespace yarp::rosmsg::std_msgs;
using namespace yarp::os;
using namespace yarp::dev;
using namespace yarp::sig;
using namespace std;

void fairoba()
{

}

int main(int argc, char* argv[])
{
    char n = argc > 0 ? *argv[0] : 1;
    Network net;
    vector<thread*> t;
    for (int i = 0; i < n; ++i)
    {
        t.emplace_back(new thread([i]()
        {
            Node n("/node" + to_string(i));
            Publisher<yarp::rosmsg::std_msgs::String> p;
            bool a = p.topic("/asd");
            while (true)
            {
                auto& m = p.prepare();
                m.data = "sono il publisher numero " + to_string(i);
                p.write();
            }
        }));
    }
    while (true)
    {

    }
}
