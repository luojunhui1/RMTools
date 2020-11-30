# SWJTU-HELIOS Test Tools Library(SWHTL)

## 1.Introduction
    This Library is created by the team members of the Robomaster-Helios 2020 in south western jiaotong university, which
    is designed to serve for our debugging work. To use this Library, your machine  should meet these following consitions:
-   OpenCV = 3.4.0    
-   Cmake >= 2.8
    And we don't provide the static library or dynamic library, you can use it only by include the "RMTools.hpp", and you
    can also modify it to meet your requirement.
## 2. functions and demos   
-   **DisplayWave**
    
    this function is used to display the change trend of an variable.     
``` c++
#include "RMTools.hpp"
    using namespace RMTools;
    
    int main()
    {
        srand(0);
    
        int value1,value;
    
        Mat src = Mat(480,640,CV_8UC3,Scalar(255,255,255));
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
```
    