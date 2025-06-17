#include <utility>
#include <iostream>
// #include <opencv2/opencv.hpp>
#include "run_python.hpp"

// int LBP = 3;
int DEPTHMAGMA = 2;
int DINO = 1;
int RANDOM_COLOR = 3;

std::map<int,std::string> featureIndexToString;

int feature(int argc, char**argv, int option, bool conda_successful) {

    if (not conda_successful)
    {
        std::cout << "noconda option used; ignoring command." << std::endl;
        return 0;
    }

    if ((option == DEPTHMAGMA) or (option == RANDOM_COLOR))
        return 1;

    if(option == DINO)
    {
        run_python("./external/src/dino.py");
        return 1;
    }
    
    
    // // Load the image
    // cv::Mat image = cv::imread("temp/depth.png", cv::IMREAD_GRAYSCALE);
    // if (image.empty()) {
    //     std::cout << "Could not open or find the image!" << std::endl;
    //     return 0;
    // }

    
    // cv::Mat dst;
    // if (option == LBP)
    //     dst = lbp(image);
    
    
    // cv::imwrite("./temp/feature.png", dst);
    return 1;
}



