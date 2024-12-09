#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;
using namespace std;

void drawBoundingBox(Mat &frame, const Mat &bbox) {
    Point p1(cvRound(bbox.at<float>(0, 0)), cvRound(bbox.at<float>(0, 1)));
    Point p2(cvRound(bbox.at<float>(0, 2)), cvRound(bbox.at<float>(0, 3)));
    Point p3(cvRound(bbox.at<float>(0, 4)), cvRound(bbox.at<float>(0, 5)));
    Point p4(cvRound(bbox.at<float>(0, 6)), cvRound(bbox.at<float>(0, 7)));
    line(frame, p1, p2, Scalar(255, 0, 0), 3); // Draw lines connecting corners
    line(frame, p2, p3, Scalar(0, 255, 0), 3); // Draw lines connecting corners
    line(frame, p3, p4, Scalar(0, 0, 255), 3); // Draw lines connecting corners
    line(frame, p4, p1, Scalar(255, 0, 255), 3); // Draw lines connecting corners
}

QRCodeDetector qrDecoder;

int main() {
    // Open the webcam
    VideoCapture cap(0); // Change the parameter if using a different camera
    if (!cap.isOpened()) {
        cout << "Error: Unable to access the camera" << endl;
        return -1;
    }

    // namedWindow("robot", WINDOW_AUTOSIZE);

    while (true) {
        Mat frame, hsvFrame, mask, result;
        cap >> frame;

        if (frame.empty()) {
            cout << "Error: No frame captured" << endl;
            break;
        }

        // Convert the frame to HSV color space
        cvtColor(frame, hsvFrame, COLOR_BGR2HSV);

        // Define HSV range for green color
        Scalar lowerGreen(30, 50, 50); // Adjust these values for better results
        Scalar upperGreen(85, 255, 255);

        // Create a mask for green color
        inRange(hsvFrame, lowerGreen, upperGreen, mask);

        // Remove noise
        erode(mask, mask, Mat(), Point(-1, -1), 2);
        dilate(mask, mask, Mat(), Point(-1, -1), 2);

        // Find contours
        vector<vector<Point>> contours;
        findContours(mask, contours, RETR_TREE, CHAIN_APPROX_SIMPLE);

        for (size_t i = 0; i < contours.size(); i++) {
            double area = contourArea(contours[i]);

            // Filter by area to ignore small objects
            if (area > 1000) {
                // Get the bounding circle
                Point2f center;
                float radius;
                minEnclosingCircle(contours[i], center, radius);

                // Draw the circle and center
                circle(frame, center, (int)radius, Scalar(0, 255, 0), 3);
                circle(frame, center, 5, Scalar(0, 0, 255), -1);

                putText(frame, "Tennis Ball", center, FONT_HERSHEY_SIMPLEX, 0.5, Scalar(255, 255, 255), 2);
            }
        }
        
        Mat bbox;
        string qrData = qrDecoder.detectAndDecode(frame, bbox);

        if (!qrData.empty()) {
            cout << "QR Code Data: " << qrData << endl;
            drawBoundingBox(frame, bbox);

            // Optional: Display decoded data on the frame
            putText(frame, qrData, Point(50, 50), FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 255, 255), 2);
        }

        // Display the result
        imshow("Robot", frame);

        // Exit if 'q' is pressed
        char c = (char)waitKey(10);
        if (c == 'q' || c == 27) {
            break;
        }
    }

    cap.release();
    destroyAllWindows();
    return 0;
}
