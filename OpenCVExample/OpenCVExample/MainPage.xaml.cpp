//
// MainPage.xaml.cpp
// Implementation of the MainPage class.
//

#include "pch.h"
#include "MainPage.xaml.h"
#include <opencv2\imgproc\types_c.h>
#include <opencv2\imgcodecs\imgcodecs.hpp>
#include <opencv2\core\core.hpp>
#include <opencv2\imgproc\imgproc.hpp>
#include <opencv2\objdetect.hpp>
#include <Robuffer.h>

using namespace OpenCVExample;

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;

using namespace Windows::UI::Xaml::Media::Imaging;
using namespace Windows::Storage::Streams;
using namespace Microsoft::WRL;


// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=402352&clcid=0x409

const cv::String face_cascade_name = "Assets/haarcascade_frontalface_alt.xml";
const cv::String body_cascade_name = "Assets/haarcascade_fullbody.xml";

MainPage::MainPage()
{
	InitializeComponent();
}

void  OpenCVExample::MainPage::UpdateImage(const cv::Mat& image)
{
	// Create the WriteableBitmap
	WriteableBitmap^ bitmap = ref new WriteableBitmap(image.cols, image.rows);

	// Get access to the pixels
	IBuffer^ buffer = bitmap->PixelBuffer;
	unsigned char* dstPixels = nullptr;

	// Obtain IBufferByteAccess
	ComPtr<IBufferByteAccess> pBufferByteAccess;
	ComPtr<IInspectable> pBuffer((IInspectable*)buffer);
	pBuffer.As(&pBufferByteAccess);

	// Get pointer to pixel bytes
	HRESULT get_bytes = pBufferByteAccess->Buffer(&dstPixels);
	if (get_bytes == S_OK) {
		memcpy(dstPixels, image.data, image.step.buf[1] * image.cols*image.rows);

		// Set the bitmap to the Image element
		storedImage->Source = bitmap;
	}
	else {
		printf("Error loading image into buffer\n");
	}
}

// replace the image pane with an image in the "assets" folder, grpPC1.jpg
void OpenCVExample::MainPage::loadImageButton_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	cv::Mat image = cv::imread("Assets/grpPC1.jpg");
	_stored_image = cv::Mat(image.rows, image.cols, CV_8UC4);
	cv::cvtColor(image, _stored_image, CV_BGR2BGRA);
	UpdateImage(_stored_image);

}

// run a Canny filter on the image contained in _stored_image, display the results to the image pane
void OpenCVExample::MainPage::cannyEdgesButton_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	cv::Mat result;
	cv::Mat intermediateMat;
	// Calculates if each pixel is part of an edge based on the gradient at that pixel
	// If a pixel gradient is higher than 90, the pixel is accepted as an edge
	// If a pixel gradient value is below 80, it is rejected.
	// If the pixel gradient is between 80 and 90, it will be accepted only if it is connected to a pixel with a gradient above 90.
	cv::Canny(_stored_image, intermediateMat, 80, 90);
	cv::cvtColor(intermediateMat, result, CV_GRAY2BGRA);
	UpdateImage(result);
}


// takes an image (inputImg), runs face and body classifiers on it, and stores the results in objectVector and objectVectorBodies, respectively
void InternalDetectObjects(cv::Mat& inputImg, std::vector<cv::Rect> & objectVector, std::vector<cv::Rect> & objectVectorBodies, cv::CascadeClassifier& face_cascade, cv::CascadeClassifier& body_cascade)
{
	cv::Mat frame_gray;

	cvtColor(inputImg, frame_gray, CV_BGR2GRAY);
	cv::equalizeHist(frame_gray, frame_gray);

	// Detect faces
	face_cascade.detectMultiScale(frame_gray, objectVector, 1.1, 2, 0 | CV_HAAR_SCALE_IMAGE, cv::Size(30, 30));
	//detect bodies
	body_cascade.detectMultiScale(frame_gray, objectVectorBodies, 1.1, 2, 0 | CV_HAAR_SCALE_IMAGE, cv::Size(30, 300));
}

// run the object detection function on the image and draw rectangles around the results
void OpenCVExample::MainPage::detectFeaturesButton_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	cv::CascadeClassifier face_cascade;
	cv::CascadeClassifier body_cascade;

	if (!face_cascade.load(face_cascade_name)) {
		printf("Couldnt load Face detector '%s'\n", face_cascade_name);
		exit(1);
	}

	if (!body_cascade.load(body_cascade_name)) {
		printf("Couldnt load Body detector '%s'\n", body_cascade_name);
		exit(1);
	}

	cv::Mat frame = cv::imread("Assets/grpPC1.jpg");

	if (frame.empty())
		return;

	std::vector<cv::Rect> faces;
	std::vector<cv::Rect> bodies;
	// run object detection
	InternalDetectObjects(frame, faces, bodies, face_cascade, body_cascade);
	// draw red rectangles around detected faces
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		cv::rectangle(_stored_image, faces[i], cv::Scalar(0, 0, 255, 255), 5);
	}
	// draw black rectangles around detected bodies
	for (unsigned int i = 0; i < bodies.size(); i++)
	{
		cv::rectangle(_stored_image, bodies[i], cv::Scalar(0, 0, 0, 255), 5);
	}
	// draw the image in the pane
	UpdateImage(_stored_image);
}