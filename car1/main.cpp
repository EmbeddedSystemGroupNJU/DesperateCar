#include <iostream>
#include "GPIOlib.h"
#include <opencv2/opencv.hpp>

using namespace std;
using namespace GPIO;
using namespace cv;

const bool DEBUG = true;
const int MAX_SPEED = 20;
const int TURN_SPEED = 50;
const int MAX_TURN_ANGLE = 30;
const double THRESHOLD = 0.07;

int main() {
    /**
     * 初始化摄像头
     */
    VideoCapture capture(0);
    if (!capture.isOpened()) {
        cerr << "Failed to open camera!";
        return 1;
    }
    capture.set(CV_CAP_PROP_FRAME_WIDTH, 320);
    capture.set(CV_CAP_PROP_FRAME_HEIGHT, 240);
    // capture.set(CV_CAP_PROP_FPS, 25);
    int dWidth = capture.get(CV_CAP_PROP_FRAME_WIDTH);
    int dHeight = capture.get(CV_CAP_PROP_FRAME_HEIGHT);
    if (DEBUG) {
        cout << "Frame Size: " << dWidth << "x" << dHeight << endl;
    }
    
    
    /**
     * 启动小车
     */
     //1为前进，2为把车头从左摆正，3为把车头从右摆正
    int state = 0; 
    
     Mat image, imgLeft, imgRight;
     Rect rectLeft(0, 0, dWidth / 2, dHeight);
     Rect rectRight(dWidth / 2, 0, dWidth, dHeight);
     while (true) {
         capture >> image;
         if (image.empty()) {
             break;
         }
         
         cvtColor(image, image, CV_BGR2GRAY);
         threshold(image, image, 80, 255, THRESH_BINARY);

         imgLeft = image(rectLeft);
         imgRight = image(rectRight);
         double l_PixRate = 1 - countNonZero(imgLeft) * 2.0 / (dWidth * dHeight);
         double r_PixRate = 1 - countNonZero(imgRight) * 2.0 / (dWidth * dHeight);
         if (DEBUG) {
             cout << "Left pixel rate=" << l_PixRate << 
             ",Right pixel rate=" << r_PixRate << 
             ", ts=" << capture.get(CV_CAP_PROP_POS_MSEC) << endl;
         }
         
         if (l_PixRate < THRESHOLD && r_PixRate < THRESHOLD && state != 1) {
             if (DEBUG) {
                 cout << "straight forward" << endl;
             }
             turnTo(0);
             controlLeft(FORWARD, MAX_SPEED);
             controlRight(FORWARD, MAX_SPEED);
             state = 1;
         } else if (l_PixRate >= THRESHOLD && r_PixRate < THRESHOLD && state != 2) {
             if (DEBUG) {
                 cout << "redirect the head from left to centre" << endl;
             }
             init();
             turnTo(MAX_TURN_ANGLE);
             controlRight(BACKWARD, TURN_SPEED);
             state = 2;
         } else if (l_PixRate < THRESHOLD && r_PixRate >= THRESHOLD && state != 3) {
             if (DEBUG) {
                 cout << "redirect the head from right to centre" << endl;
             }
             init();
             turnTo(-MAX_TURN_ANGLE);
             controlLeft(BACKWARD, TURN_SPEED);
             state = 3;
         } else if (l_PixRate >= THRESHOLD && r_PixRate >= THRESHOLD) {
             if (DEBUG) {
                 cout << "sprinting" << endl;
             }
             init();
             turnTo(0);
             controlLeft(FORWARD, MAX_SPEED);
             controlRight(FORWARD, MAX_SPEED);
             delay(500);
             stopLeft();
             stopRight();

             return 0;
         }
     }
}