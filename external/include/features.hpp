#include <iostream>
#include <opencv2/opencv.hpp>

cv::Mat lbp(cv::Mat image)
{
    // Compute LBP images
    cv::Mat lbpImage;
    // int radius = 1; // LBP radius
    int radius = 5; // LBP radius
    // int neighbors = 8; // Number of neighbor points
    int neighbors = 80; // Number of neighbor points
    cv::Mat dst = cv::Mat::zeros(image.rows-2*radius, image.cols-2*radius, CV_8UC1);

    for(int i = radius; i < image.rows - radius; i++) {
        for(int j = radius; j < image.cols - radius; j++) {
            unsigned char center = image.at<unsigned char>(i, j);
            unsigned char pattern = 0;
            for(int k = 0; k < neighbors; k++) {
                float angle = 2 * CV_PI * k / neighbors;
                int x = i + radius * std::cos(angle);
                int y = j + radius * std::sin(angle);
                unsigned char neighbor = image.at<unsigned char>(x, y);
                pattern |= (neighbor > center) << k;
            }
            dst.at<unsigned char>(i - radius, j - radius) = pattern;
        }
    }
    return dst;
}

cv::Mat erode(cv::Mat image)
{
    // Define the kernel for morphological erosion
    int kernelSize = 15;
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(kernelSize, kernelSize));

    // Apply morphological erosion
    cv::Mat erodedImage;
    cv::erode(image, erodedImage, kernel, cv::Point(-1,-1), 3);

    // Save the erod
    return erodedImage;
}

// // Create a 10x10 circular kernel (values: 1 inside circle, 0 outside)
// cv::Mat createCircularKernel(int size = 10) {
//     // cv::Mat kernel = cv::Mat::zeros(size, size, CV_32F);
//     cv::Mat kernel = cv::Mat::ones(size, size, CV_32F) * -1;
//     cv::Point center(size / 2, size / 2);
//     float radius = size / 2.0f;

//     for (int y = 0; y < size; ++y)
//         for (int x = 0; x < size; ++x)
//             if (cv::norm(cv::Point(x, y) - center) <= radius)
//                 kernel.at<float>(y, x) = 1.0f;

//     // Normalize so max value doesn't exceed 1 when summing
//     kernel /= cv::sum(kernel)[0];
//     return kernel;
// }

// cv::Mat convolveWithCircularKernel(const cv::Mat& image) {
//     cv::Mat imageFloat;
//     image.convertTo(imageFloat, CV_32F, 1.0 / 255.0);

//     cv::Mat kernel = createCircularKernel(100);
//     cv::Mat result;
//     cv::filter2D(imageFloat, result, -1, kernel);

//     // Convert back to 8-bit for saving
//     cv::Mat output;
//     result.convertTo(output, CV_8U, 255.0);
//     return output;
// }


int feature(int option) {
    // Load the image
    cv::Mat image = cv::imread("temp/depth.png", cv::IMREAD_GRAYSCALE);
    if (image.empty()) {
        std::cout << "Could not open or find the image!" << std::endl;
        return 0;
    }

    cv::Mat dst;
    
    if (option == 2)
        return 0;
    
    if (option == 1)
        dst = erode(image);
    else if (option == 2)
        dst = lbp(image);
    // else if (option == 4)
    //     dst = convolveWithCircularKernel(image);
    // Show the LBP image
    // cv::imshow("LBP Image", dst);
    cv::imwrite("./temp/feature.png", dst);
    // cv::waitKey(0);

    return 1;
}



