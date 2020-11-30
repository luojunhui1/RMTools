#include <iostream>
#include <vector>
#include <string>
#include <sstream>

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "RMTools.hpp"
using namespace std;
using namespace cv;
using namespace RMTools;
int main()
{
    Mat src,dst,binary;
    Mat bg = Mat(480,640,CV_8UC3,Scalar(255,255,255));
    VideoCapture cap(2);
    cap.read(src);
    FeatureRoute a = FeatureRoute(bg,5);
    vector<Point> pts;
    vector<Mat> channels;
    Mat kernel = getStructuringElement(MORPH_ELLIPSE,Size(11,11));
    while(!src.empty())
    {
        split(src,channels);
        dst = channels[2] - channels[1];
        blur(dst,dst,Size(5,5));
        threshold(dst,binary,100,255,THRESH_BINARY);
        morphologyEx(binary,binary,MORPH_CLOSE,kernel);
        vector<vector<Point>> contours_light;
        findContours(binary, contours_light, RETR_EXTERNAL, CHAIN_APPROX_NONE);
        for (auto & i : contours_light)
        {
            if (i.size() < 5)
                continue;
            RotatedRect rr = fitEllipse(i);
            pts.emplace_back(rr.center);
        }
        imshow("binary",binary);
        a.DisplayRoute(pts);
        pts.clear();
        cap.read(src);
        waitKey(30);
    }
}