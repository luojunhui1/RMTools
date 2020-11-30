#include <unistd.h>
#include <cerrno>
#include <pthread.h>
#include <mutex>
#include <utility>
#include <thread>
#include <string>
#include <cmath>
#include <limits>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>

#define __MYSCALAR__(i) ((i%3==0)?Scalar(i*16.7,i*8,0):((i%3==1)?Scalar(0,i*16.7,255 - i*5):Scalar(255 - i*2,0,i*16.7)))

using namespace cv;
using namespace std;

namespace RMTools
{
/**
 * @brief this class is used to display a waveform figure,if you want to display two waves in one window, you can add one
 * more line in the DisplayWave() function. And if you want to display more infomation in the window, such as sample,
 * value, acceleration, integral...... , just DIY it,bro.
 * @param src_ background image
 * @param value_ the value to display as wave
 * @param wn_ window name
 * @param stand_ the standard line to paint
 * @param lc_ the standard line color
 * @param wc_ the wave line color
 */
    class DisPlayWaveCLASS
    {
    private:
        uint8_t count = 0;
        uint16_t lastValue = 0;
        bool m_isExit;
        std::thread m_thread;

        Mat src;
        int *value;
        string wn;
        int stand = 0;
        int spacing = 5;
        Scalar lc = Scalar(0,0,255);
        Scalar wc = Scalar(255,255,255);
        Mat left,copy;
    public:

        DisPlayWaveCLASS(Mat src_,int* value_): m_isExit(false),src(std::move(src_)),value(value_)
        {
            copy = src.clone();
        };

        DisPlayWaveCLASS(Mat src_,int* value_,string wn_ ,int stand_ = 0,Scalar lc_ = Scalar(0,0,255),\
                    Scalar wc_ = Scalar(255,255,255)): m_isExit(false),src(std::move(src_)),value(value_),wn(std::move(wn_)),stand(stand_),\
                    lc(std::move(lc_)),wc(std::move(wc_))
        {
             copy = src.clone();
        };

        void* DisplayWave();
    };
/**
 * @brief evey time execute this function will update the waveform figure, and you should add this function in your loop.
 * @return none
 */
    void* DisPlayWaveCLASS::DisplayWave()
    {
        if(*value >= src.rows || stand >= src.rows)perror("Value exceeds the src rows");
        if ((src.cols/spacing) > count)
        {
            line(copy,Point2d((count -1)*spacing,lastValue),Point2d(count*spacing,*(value)),wc);
            lastValue = *(value);
            count++;
        }
        else
        {
            copy.colRange(spacing,(count - 1)*spacing + 1).copyTo(left);
            copy.setTo(0);
            left.copyTo(copy.colRange(0,(count - 2)*spacing + 1));
            line(copy,Point2d((count - 2)*spacing,lastValue),Point2d((count - 1)*spacing,*(value)),wc);
            lastValue = *(value);
        }
        line(copy,Point2d(0,stand),Point2d(copy.cols-1,stand),lc);
        flip(copy,src,0);
        imshow(wn,src);
    }
/**
 *  the descriptor of the point in the route, including the color, location, velocity and the situation of point.
 */
    class RoutePoint
    {
    public:
        RoutePoint(const Point& p_,Scalar color_,int vx_,int vy_):p(p_),color(std::move(color_)),vx(vx_),vy(vy_){};
        Point p;
        Scalar color;
        int vx;
        int vy;
        bool used = false;
    };

/**
 * @brief this class is used to track the feature points that user provided and display the route of these feature points
 *        with unique color.
 * @param src the image that the points drawing on
 * @param pSize the size of the points, which reflect the route
 */
    class FeatureRoute
    {
    private:
        int pSize;
        int count;
        int expirectl;
        Mat origin;
        Mat srcs[5];
        vector<RoutePoint> lastPts;
        static int wd;
    public:
        explicit FeatureRoute(const Mat& src,int pSize);
        void DisplayRoute(const vector<Point>& pts);
        void FeatureRouteDrawPoint(const Point& p,const Scalar& color,int expirectl);
    };
    int FeatureRoute::wd = 20;

    FeatureRoute::FeatureRoute(const Mat& src,int pSize = 10)
    {
        this->pSize = pSize;
        count = 1;
        this->origin = src;
        expirectl = 0;
        for(auto & i : srcs)
        {
            src.copyTo(i);
        }
    }
    void FeatureRoute::FeatureRouteDrawPoint(const Point& p_, const Scalar& color_, int expirectl_)
    {
        for(int i = 0;i < 4;i++)
        {
            circle(srcs[expirectl_],p_,pSize,color_,-1);
            expirectl_ = (expirectl_ + 1)%5;
        }
        srcs[expirectl_].setTo(255);
    }
    void FeatureRoute::DisplayRoute(const vector<Point>& pts)
    {
        int i = 0;
        vector<RoutePoint> cur;
        if(lastPts.empty())
        {
            for(const auto& p:pts)
            {
                if(p.x<0||p.x>origin.cols||p.y<0||p.y>origin.rows)
                {
                    printf("Point exceed the limitation of window");
                    continue;
                }
                cur.emplace_back(p,__MYSCALAR__(i),0,0);
                FeatureRouteDrawPoint(p,__MYSCALAR__(i),expirectl);
                i = i + 1;
            }
        }
        else
        {
            vector<Point> lt;
            vector<Point> rb;
            count = pts.size();
            int curError;
            int selectedIndex;
            int error;
            for(const auto& p:pts)
            {
                lt.emplace_back(Point(lastPts[i].p.x - wd,lastPts[i].p.y - wd));
                rb.emplace_back( Point(lastPts[i].p.x + wd,lastPts[i].p.y + wd));
            }
            for(const auto& p:pts)
            {
                error = numeric_limits<int>::max();
                selectedIndex = -1;
                for(i = 0;i<lastPts.size();i++)
                {
                    if((p.x>lt[i].x&&p.y>lt[i].y&&p.x<rb[i].x&&p.y<rb[i].y)
                       &&!lastPts[i].used
                       ||(lastPts[i].vx!=0&&(lastPts[i].p.x - p.x) != 0
                       &&(lastPts[i].vy/lastPts[i].vx > 0) == ((lastPts[i].p.y - p.y)/(lastPts[i].p.x - p.x) > 0)
                       &&(abs(lastPts[i].vy*3)>abs((lastPts[i].p.y - p.y))&&abs(lastPts[i].vx*3)>abs((lastPts[i].p.x - p.x)))
                         )
                       )
                    {
                        curError = abs(lastPts[i].p.y - p.y) + abs(lastPts[i].p.x - p.x);
                        error = (error>curError)?(selectedIndex = i,curError):(error);
                    }
                }
                if(selectedIndex != -1)
                {
                    cur.emplace_back(RoutePoint(p,lastPts[selectedIndex].color,(p.x - lastPts[i].p.x),(p.y - lastPts[i].p.y)));
                    lastPts[selectedIndex].used = true;
                }
                else
                {
                    cur.emplace_back(RoutePoint(p, __MYSCALAR__(count),0,0));
                    count = (count + 1)%18;
                }
            }
            for(const auto& p:cur)
            {
                if(p.p.x<0||p.p.x>origin.cols||p.p.y<0||p.p.y>origin.rows)
                {
                    printf("Point exceed the limitation of window");
                    continue;
                }
                FeatureRouteDrawPoint(p.p,p.color,expirectl);
                //circle(origin,p.p,pSize,p.color,-1);
            }
        }
        imshow("FeatureRoute",srcs[expirectl]);
        expirectl = (expirectl + 1)%5;
        //imwrite("src1.jpg",src);
        lastPts = cur;
    }
}
