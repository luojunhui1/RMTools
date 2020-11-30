#include "RMTools.hpp"
using namespace RMTools;

int main()
{
    srand(0);

    int value1,value;

    Mat src = Mat(480,640,CV_8UC3,Scalar(0,0,0));
    auto *a =new DisPlayWaveCLASS(src, &value,"Display Window",100);
    auto *b =new DisPlayWaveCLASS(src, &value1,"Display Window1",100,Scalar(255,0,0),Scalar(255,255,0));

    while(true)
    {
        value = (int)random()%200;
        value1 = (int)random()%200;

        a->DisplayWave();
        b->DisplayWave();

        if(waitKey(20)==27)break;
    }
    return 0;
}