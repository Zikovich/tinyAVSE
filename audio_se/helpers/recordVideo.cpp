#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

int main() {
    cv::VideoCapture cap(0); // Open the default camera (index 0)
    if (!cap.isOpened()) {
        std::cerr << "Error: Unable to open the camera." << std::endl;
        return -1;
    }

    int frame_width = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_WIDTH));
    int frame_height = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_HEIGHT));

    //cv::VideoWriter video("output_video.mp4", cv::VideoWriter::fourcc('M','J','P','G'), 30, cv::Size(frame_width, frame_height));
    cv::VideoWriter video("output_video.avi", cv::VideoWriter::fourcc('X','V','I','D'), 30, cv::Size(frame_width, frame_height));

    if (!video.isOpened()) {
        std::cerr << "Error: Unable to create the video writer." << std::endl;
        return -1;
    }

    while (true) {
        cv::Mat frame;
        cap.read(frame);

        if (frame.empty()) {
            std::cerr << "Error: Blank frame grabbed." << std::endl;
            break;
        }

        video.write(frame);        

        // Press 'q' to exit the loop
        if (cv::waitKey(1) == 'q')
            break;
    }

    cap.release();
    video.release();
    cv::destroyAllWindows();

    return 0;
}
