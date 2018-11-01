#include <iostream>
#include <opencv2/opencv.hpp>
#include "GPIOlib.h"

using namespace std;
using namespace GPIO;
using namespace cv;

const int MAX_SPEED = 30;
const int TURN_SPEED = 10;
const int BACK_DELAY = 500;
const int TURN_DELAY = 500;
const int SPRINT_DELAY = 500;
const double CENTRE_THRESHOLD = 0.5;
const double MARGIN_THRESHOLD = 0.05;

const int TURN_ANGLE = 30;
const double THRESHOLD = 0.05;

const bool DEBUG = true;

int main() {
    /**
     * 初始化摄像头
     */
    VideoCapture capture(0);
    if (!capture.isOpened()) {
        cerr << "Failed to open camera!";
        return 1;
    }
    int dWidth = capture.get(CV_CAP_PROP_FRAME_WIDTH);
    int dHeight = capture.get(CV_CAP_PROP_FRAME_HEIGHT);
    cout << "Frame Size: " << dWidth << "x" << dHeight << endl;


    /**
     * 启动小车
     */
     int state = 0;
     Mat image, imgLeft, imgCentre, imgRight;
     Rect rectLeft(0, 0, dWidth / 4, dHeight);
     Rect rectCentre(dWidth / 4, 0, dWidth / 2, dHeight);
     Rect rectRight(dWidth * 3 / 4, 0, dWidth / 4, dHeight);
     while (true) {
         capture >> image;
         cvtColor(image, image, CV_BGR2GRAY);
         threshold(image, image, 100, 255, THRESH_BINARY);
         imgLeft = image(rectLeft);
         imgCentre = image(rectCentre);
         imgRight = image(rectRight);

         double l_PixRate = 1 - countNonZero(imgLeft) * 8.0 / (dWidth * dHeight);
         double c_PixRate = 1 - countNonZero(imgCentre) * 8.0 / (dWidth * dHeight);
         double r_PixRate = 1 - countNonZero(imgRight) * 8.0 / (dWidth * dHeight);
         cout << "left Pixel Rate=" << l_PixRate <<
         " ,centre Pixel Rate= " << c_PixRate <<
         "right Pixel Rate=" << r_PixRate << endl;

         if (c_PixRate < CENTRE_THRESHOLD) {
             init();
             if (l_PixRate >= MARGIN_THRESHOLD && r_PixRate < MARGIN_THRESHOLD) {
                 if (DEBUG) {
                     cout << "redirect the head from left to centre" << endl;
                 }
                 turnTo(TURN_ANGLE);
                 controlRight(BACKWARD, TURN_SPEED);
                 delay(TURN_DELAY);
             }else if (l_PixRate < MARGIN_THRESHOLD && r_PixRate >= MARGIN_THRESHOLD) {
                 if (DEBUG) {
                     cout << "redirect the head from right to centre" << endl;
                 }
                 turnTo(-TURN_ANGLE);
                 controlLeft(BACKWARD, TURN_SPEED);
                 delay(TURN_DELAY);
             } else {
                 if (DEBUG) {
                     cout << "straight forward" << endl;
                 }
                 turnTo(0);
                 controlLeft(FORWARD, MAX_SPEED);
                 controlRight(FORWARD, MAX_SPEED);
             }
         } else{
             if (DEBUG) {
                 cout << "sprinting" << endl;
             }
             init();
             turnTo(0);
             controlLeft(FORWARD, TURN_SPEED);
             controlRight(FORWARD, TURN_SPEED);
             delay(SPRINT_DELAY);
             stopLeft();
             stopRight();

             return 0;
         }
     }
}

