/**  Hot Dog Classifer
/   by James Bell
/   Modified from original project by Microsoft ELL team
**/
#include <chrono>
#include <fstream>
#include <opencv2/opencv.hpp>
#include <sstream>
#include <stdio.h>
#include <stdexcept>

// Include the model interface file for the compiled ELL model
// Note: the "<modelname>_MAIN" preprocessor symbol must be defined in exactly one source file
//       that includes the model interface file. This is typically the file that defines "main()"
#define model_MAIN
#include "model.h"

// Include helper functions
#include "tutorialHelpers.h"

// Read an image from the camera
static cv::Mat GetImageFromCamera(cv::VideoCapture& camera)
{
    cv::Mat frame;
    camera >> frame;
    return frame;
}

// Read an image from a file
static cv::Mat GetImageFromFile(const std::string& filename)
{
    cv::Mat img = cv::imread(filename, cv::IMREAD_COLOR);
    if (img.empty())
    {
        std::cout << "Could not read the image: " << filename << std::endl;
        throw std::invalid_argument("invalid image file");
        exit(EXIT_FAILURE);
    }
    return img;
}

// Read a file of strings
static std::vector<std::string> ReadLinesFromFile(const std::string& filename)
{
    std::vector<std::string> lines;
    std::string line;

    std::ifstream file(filename);

    while (std::getline(file, line))
    {
        if (line.length() > 0) lines.emplace_back(line);
    }

    return lines;
}

int main(int argc, char** argv)
{
    ModelWrapper wrapper;

    // Open the video camera. To use a different camera, change the camera index.
    //cv::VideoCapture camera(0);

    // Read the category names
    auto categories = ReadLinesFromFile("../categories.txt");

    // Get the model's input shape. We will use this information later to resize images appropriately.
    TensorShape inputShape = wrapper.GetInputShape();

    // Get the model's preprocessing metadata. We will use this information later to preprocess images
    tutorialHelpers::ImagePreprocessingMetadata metadata = tutorialHelpers::GetImagePreprocessingMetadata(wrapper);

    // Create a vector to hold the model's output predictions
    std::vector<float> predictions(wrapper.GetOutputSize());

    // Declare a variable to hold the prediction times
    std::vector<double> predictionTimes;
    double meanTimeToPredict = 0.0;

    while ((cv::waitKey(1) & 0xFF) != 27)
    {
        // Get an image from the camera. (Alternatively, call GetImageFromFile to read from file)
        cv::Mat image = GetImageFromFile("../cat.jpg");

        // Prepare an image for processing
        // - Resize and center-crop to the required width and height while preserving aspect ratio.
        // - OpenCV gives the image in BGR order. If needed, re-order the channels to RGB.
        // - Convert the OpenCV result to a std::vector<float>
        auto input = tutorialHelpers::PrepareImageForModel(image, inputShape.columns, inputShape.rows, &metadata);

        // Send the image to the compiled model and fill the predictions vector with scores, measure how long it takes
        auto start = std::chrono::steady_clock::now();
        predictions = wrapper.Predict(input);
        auto end = std::chrono::steady_clock::now();

        // Get the value of the top 5 predictions
        auto top1 = tutorialHelpers::GetTopN(predictions, 1);

        // Generate header text that represents the top5 predictions
        std::stringstream headerText;
        cv::Scalar color;
        for (auto element : top1)
        {
            if (element.first == 934)
            {
                headerText << "(" << std::floor(element.second * 100.0) << "%) " << categories[element.first] << "  ";
                color = cv::Scalar(50, 200, 50); // green
            }
            else 
            {
                headerText << "not hot dog " << "(" << std::floor(element.second * 100) << "%) "<< categories[element.first] << "  ";
                color = cv::Scalar(50, 50, 200); // red
            }
            
        }
        tutorialHelpers::DrawHeader(image, headerText.str(), color);

        // Generate footer text that represents the mean evaluation time
        /*std::stringstream footerText;
        meanTimeToPredict = std::floor(tutorialHelpers::GetMeanDuration(predictionTimes, std::chrono::duration<double>(end - start).count()) * 1000);
        footerText << meanTimeToPredict << "ms/frame";
        tutorialHelpers::DrawFooter(image, footerText.str());*/

        // Display the image
        cv::imshow("ELL model", image);
    }

    std::cout << "Mean prediction time: " << meanTimeToPredict << "ms/frame" << std::endl;

    return 0;
}