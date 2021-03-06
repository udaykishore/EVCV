// EVCV.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"


#include <opencv2/opencv.hpp>

#include "Spinnaker.h"
#include "SpinGenApi/SpinnakerGenApi.h"

#include <iostream>
#include <string>

// Libraries windows.h or unistd.h included for Sleep()/usleep().
#if defined WIN32 || defined _WIN32 || defined WIN64 || defined _WIN64
#include "windows.h"
#else
#include "unistd.h"
#endif

using namespace cv;
using namespace std;

using namespace Spinnaker;
using namespace Spinnaker::GenApi;
using namespace Spinnaker::GenICam;

int ConfigureTrigger(INodeMap & nodeMap)
{
	int result = 0;
	cout << endl << endl << "*** CONFIGURING CUSTOM IMAGE SETTINGS ***" << endl << endl;

	try
	{
		//
		// Apply mono 8 pixel format
		//
		// *** NOTES ***
		// Enumeration nodes are slightly more complicated to set than other
		// nodes. This is because setting an enumeration node requires working
		// with two nodes instead of the usual one. 
		//
		// As such, there are a number of steps to setting an enumeration node: 
		// retrieve the enumeration node from the nodemap, retrieve the desired 
		// entry node from the enumeration node, retrieve the integer value from 
		// the entry node, and set the new value of the enumeration node with
		// the integer value from the entry node.
		//
		// Retrieve the enumeration node from the nodemap
		CEnumerationPtr ptrPixelFormat = nodeMap.GetNode("PixelFormat");
		if (IsAvailable(ptrPixelFormat) && IsWritable(ptrPixelFormat))
		{
			// Retrieve the desired entry node from the enumeration node
			CEnumEntryPtr ptrPixelFormatMono8 = ptrPixelFormat->GetEntryByName("Mono8");
			if (IsAvailable(ptrPixelFormatMono8) && IsReadable(ptrPixelFormatMono8))
			{
				// Retrieve the integer value from the entry node
				int64_t pixelFormatMono8 = ptrPixelFormatMono8->GetValue();

				// Set integer as new value for enumeration node
				ptrPixelFormat->SetIntValue(pixelFormatMono8);

				cout << "Pixel format set to " << ptrPixelFormat->GetCurrentEntry()->GetSymbolic() << "..." << endl;
			}
			else
			{
				cout << "Pixel format mono 8 not available..." << endl;
			}
		}
		else
		{
			cout << "Pixel format not available..." << endl;
		}

		// 
		// Apply minimum to offset X
		//
		// *** NOTES ***
		// Numeric nodes have both a minimum and maximum. A minimum is retrieved
		// with the method GetMin(). Sometimes it can be important to check 
		// minimums to ensure that your desired value is within range.
		//
		CIntegerPtr ptrOffsetX = nodeMap.GetNode("OffsetX");
		if (IsAvailable(ptrOffsetX) && IsWritable(ptrOffsetX))
		{
			ptrOffsetX->SetValue(ptrOffsetX->GetMin());
			cout << "Offset X set to " << ptrOffsetX->GetMin() << "..." << endl;
		}
		else
		{
			cout << "Offset X not available..." << endl;
		}

		//
		// Apply minimum to offset Y
		// 
		// *** NOTES ***
		// It is often desirable to check the increment as well. The increment
		// is a number of which a desired value must be a multiple of. Certain
		// nodes, such as those corresponding to offsets X and Y, have an
		// increment of 1, which basically means that any value within range
		// is appropriate. The increment is retrieved with the method GetInc().
		//
		CIntegerPtr ptrOffsetY = nodeMap.GetNode("OffsetY");
		if (IsAvailable(ptrOffsetY) && IsWritable(ptrOffsetY))
		{
			ptrOffsetY->SetValue(ptrOffsetY->GetMin());
			cout << "Offset Y set to " << ptrOffsetY->GetValue() << "..." << endl;
		}
		else
		{
			cout << "Offset Y not available..." << endl;
		}

		//
		// Set maximum width
		//
		// *** NOTES ***
		// Other nodes, such as those corresponding to image width and height, 
		// might have an increment other than 1. In these cases, it can be
		// important to check that the desired value is a multiple of the
		// increment. However, as these values are being set to the maximum,
		// there is no reason to check against the increment.
		//
		CIntegerPtr ptrWidth = nodeMap.GetNode("Width");
		if (IsAvailable(ptrWidth) && IsWritable(ptrWidth))
		{
			int64_t widthToSet = ptrWidth->GetMax();

			ptrWidth->SetValue(widthToSet/4);

			cout << "Width set to " << ptrWidth->GetValue() << "..." << endl;
		}
		else
		{
			cout << "Width not available..." << endl;
		}

		//
		// Set maximum height
		//
		// *** NOTES ***
		// A maximum is retrieved with the method GetMax(). A node's minimum and
		// maximum should always be a multiple of its increment.
		//
		CIntegerPtr ptrHeight = nodeMap.GetNode("Height");
		if (IsAvailable(ptrHeight) && IsWritable(ptrHeight))
		{
			int64_t heightToSet = ptrHeight->GetMax();

			ptrHeight->SetValue(heightToSet/4);

			cout << "Height set to " << ptrHeight->GetValue() << "..." << endl << endl;
		}
		else
		{
			cout << "Height not available..." << endl << endl;
		}
	}
	catch (Spinnaker::Exception &e)
	{
		cout << "Error: " << e.what() << endl;
		result = -1;
	}

	cout << endl << endl << "*** CONFIGURING TRIGGER ***" << endl << endl;
	try
	{
		// Ensure trigger mode off
		//
		// *** NOTES ***
		// The trigger must be disabled in order to configure whether the source
		// is software or hardware.
		//
		CEnumerationPtr ptrTriggerMode = nodeMap.GetNode("TriggerMode");
		if (!IsAvailable(ptrTriggerMode) || !IsReadable(ptrTriggerMode))
		{
			cout << "Unable to disable trigger mode (node retrieval). Aborting..." << endl;
			return -1;
		}

		CEnumEntryPtr ptrTriggerModeOff = ptrTriggerMode->GetEntryByName("Off");
		if (!IsAvailable(ptrTriggerModeOff) || !IsReadable(ptrTriggerModeOff))
		{
			cout << "Unable to disable trigger mode (enum entry retrieval). Aborting..." << endl;
			return -1;
		}

		ptrTriggerMode->SetIntValue(ptrTriggerModeOff->GetValue());

		cout << "Trigger mode disabled..." << endl;

		CEnumerationPtr ptrTriggerSource = nodeMap.GetNode("TriggerSource");
		if (!IsAvailable(ptrTriggerSource) || !IsWritable(ptrTriggerSource))
		{
			cout << "Unable to set trigger mode (node retrieval). Aborting..." << endl;
			return -1;
		}

		CEnumEntryPtr ptrTriggerSourceSoftware = ptrTriggerSource->GetEntryByName("Software");
		if (!IsAvailable(ptrTriggerSourceSoftware) || !IsReadable(ptrTriggerSourceSoftware))
		{
			cout << "Unable to set trigger mode (enum entry retrieval). Aborting..." << endl;
			return -1;
		}

		ptrTriggerSource->SetIntValue(ptrTriggerSourceSoftware->GetValue());

		cout << "Trigger source set to software..." << endl;

		// Turn trigger mode on
		//
		// *** LATER ***
		// Once the appropriate trigger source has been set, turn trigger mode 
		// on in order to retrieve images using the trigger.
		//
		CEnumEntryPtr ptrTriggerModeOn = ptrTriggerMode->GetEntryByName("On");
		if (!IsAvailable(ptrTriggerModeOn) || !IsReadable(ptrTriggerModeOn))
		{
			cout << "Unable to enable trigger mode (enum entry retrieval). Aborting..." << endl;
			return -1;
		}

		ptrTriggerMode->SetIntValue(ptrTriggerModeOn->GetValue());

		cout << "Trigger mode turned back on..." << endl << endl;

		// Set acquisition mode to continuous
		CEnumerationPtr ptrAcquisitionMode = nodeMap.GetNode("AcquisitionMode");
		if (!IsAvailable(ptrAcquisitionMode) || !IsWritable(ptrAcquisitionMode))
		{
			cout << "Unable to set acquisition mode to continuous (node retrieval). Aborting..." << endl << endl;
			return -1;
		}

		CEnumEntryPtr ptrAcquisitionModeContinuous = ptrAcquisitionMode->GetEntryByName("Continuous");
		if (!IsAvailable(ptrAcquisitionModeContinuous) || !IsReadable(ptrAcquisitionModeContinuous))
		{
			cout << "Unable to set acquisition mode to continuous (entry 'continuous' retrieval). Aborting..." << endl << endl;
			return -1;
		}

		int64_t acquisitionModeContinuous = ptrAcquisitionModeContinuous->GetValue();

		ptrAcquisitionMode->SetIntValue(acquisitionModeContinuous);

		cout << "Acquisition mode set to continuous..." << endl;

	}
	catch (Spinnaker::Exception &e)
	{
		cout << "Error: " << e.what() << endl;
		result = -1;
	}
	return result;
}

/*
* This function shows how to convert between Spinnaker ImagePtr container to CVmat container used in OpenCV.
*/
int ConvertToCVmat(ImagePtr pImage)
{
	int result = 0;
	ImagePtr convertedImage = pImage->Convert(PixelFormat_Mono8, NEAREST_NEIGHBOR);

	unsigned int XPadding = convertedImage->GetXPadding();
	unsigned int YPadding = convertedImage->GetYPadding();
	unsigned int rowsize = convertedImage->GetWidth();
	unsigned int colsize = convertedImage->GetHeight();

	//image data contains padding. When allocating Mat container size, you need to account for the X,Y image data padding. 
	Mat cvimg = cv::Mat(colsize + YPadding, rowsize + XPadding, CV_8UC1, convertedImage->GetData(), convertedImage->GetStride());
//	namedWindow("Camera Preview", WINDOW_NORMAL);
//	moveWindow("Camera Preview", 20, 20);
	imshow("Camera Preview", cvimg);
//	resizeWindow("Camera Preview", rowsize / 4, colsize / 4);

	waitKey(1);//otherwise the image will not display...

	return result;
}
int StartPreview(CameraPtr pCam, INodeMap & nodeMap)
{
	int result = 0;

	//
	// Get the current frame rate; acquisition frame rate recorded in hertz
	//
	// *** NOTES ***
	// The video frame rate can be set to anything; however, in order to
	// have videos play in real-time, the acquisition frame rate can be
	// retrieved from the camera.
	//
	CFloatPtr ptrAcquisitionFrameRate = nodeMap.GetNode("AcquisitionFrameRate");
	if (!IsAvailable(ptrAcquisitionFrameRate) || !IsReadable(ptrAcquisitionFrameRate))
	{
		cout << "Unable to retrieve frame rate. Aborting..." << endl << endl;
		return -1;
	}

	float frameRateToSet = static_cast<float>(ptrAcquisitionFrameRate->GetValue());

	cout << "Frame rate set to " << frameRateToSet << "..." << endl;

	// Get user input
	cout << "Press the Enter key to get an image." << endl;
	getchar();

	// Execute software trigger
	//CCommandPtr ptrSoftwareTriggerCommand = nodeMap.GetNode("TriggerSoftware");
	//if (!IsAvailable(ptrSoftwareTriggerCommand) || !IsWritable(ptrSoftwareTriggerCommand))
	//{
	//	cout << "Unable to execute trigger. Aborting..." << endl;
	//	return -1;
	//}

	//ptrSoftwareTriggerCommand->Execute();
//	namedWindow("Camera Preview", WINDOW_KEEPRATIO);
//	moveWindow("Camera Preview", 20, 20);
	ImagePtr convertedImage;
	ImagePtr pResultImage;
	while (true)
	{
		try {
			// Retrieve the next received image
			pResultImage = pCam->GetNextImage();

			if (pResultImage->IsIncomplete())
			{
				cout << "Image incomplete with image status " << pResultImage->GetImageStatus() << "..." << endl << endl;
			}
			else
			{
				// Print image information
				cout << "Grabbed image: width = " << pResultImage->GetWidth() << ", height = " << pResultImage->GetHeight() << endl;
			}
			convertedImage = pResultImage->Convert(PixelFormat_Mono8, NEAREST_NEIGHBOR);

			unsigned int XPadding = convertedImage->GetXPadding();
			unsigned int YPadding = convertedImage->GetYPadding();
			unsigned int rowsize = convertedImage->GetWidth();
			unsigned int colsize = convertedImage->GetHeight();

			//image data contains padding. When allocating Mat container size, you need to account for the X,Y image data padding. 
			Mat cvimg = cv::Mat(colsize + YPadding, rowsize + XPadding, CV_8UC1, convertedImage->GetData(), convertedImage->GetStride());
			imshow("Camera Preview", cvimg);
			pResultImage->Release();
			
			if (waitKey(1) == 27/*ESC*/) {
				cout << "Esc key is pressed by user. Stoppig the video" << endl;
				break;
			}
		}
		catch (Spinnaker::Exception &e)
		{
			cout << "Error: " << e.what() << endl;
			result = -1;
		}
	}
	return result;
}
ImagePtr AcquireImage(CameraPtr pCam, INodeMap & nodeMap)
{
	// Get user input
	cout << "Press the Enter key to get an image." << endl;
	getchar();

	// Execute software trigger
	CCommandPtr ptrSoftwareTriggerCommand = nodeMap.GetNode("TriggerSoftware");
	if (!IsAvailable(ptrSoftwareTriggerCommand) || !IsWritable(ptrSoftwareTriggerCommand))
	{
		cout << "Unable to execute trigger. Aborting..." << endl;
		return -1;
	}

	ptrSoftwareTriggerCommand->Execute();

	// Retrieve the next received image
	ImagePtr pResultImage = pCam->GetNextImage();

	if (pResultImage->IsIncomplete())
	{
		cout << "Image incomplete with image status " << pResultImage->GetImageStatus() << "..." << endl << endl;
	}
	else
	{
		// Print image information
		cout << "Grabbed image: width = " << pResultImage->GetWidth() << ", height = " << pResultImage->GetHeight() << endl;
	}

	return pResultImage;
}

// This function prints the device information of the camera from the transport
// layer; please see NodeMapInfo example for more in-depth comments on printing
// device information from the nodemap.
int PrintDeviceInfo(INodeMap & nodeMap)
{
	int result = 0;

	cout << endl << "*** DEVICE INFORMATION ***" << endl << endl;

	try
	{
		FeatureList_t features;
		CCategoryPtr category = nodeMap.GetNode("DeviceInformation");
		if (IsAvailable(category) && IsReadable(category))
		{
			category->GetFeatures(features);

			FeatureList_t::const_iterator it;
			for (it = features.begin(); it != features.end(); ++it)
			{
				CNodePtr pfeatureNode = *it;
				cout << pfeatureNode->GetName() << " : ";
				CValuePtr pValue = (CValuePtr)pfeatureNode;
				cout << (IsReadable(pValue) ? pValue->ToString() : "Node not readable");
				cout << endl;
			}
		}
		else
		{
			cout << "Device control information not available." << endl;
		}
	}
	catch (Spinnaker::Exception &e)
	{
		cout << "Error: " << e.what() << endl;
		result = -1;
	}

	return result;
}


int RunSingleCamera(CameraPtr pCam)
{
	int result = 0;
	int err = 0;
	try
	{
		// Retrieve TL device nodemap and print device information
		INodeMap & nodeMapTLDevice = pCam->GetTLDeviceNodeMap();

		result = PrintDeviceInfo(nodeMapTLDevice);

		// Initialize camera
		pCam->Init();

		// Retrieve GenICam nodemap
		INodeMap & nodeMap = pCam->GetNodeMap();
		//set trigger to software trigger
//		result = ConfigureTrigger(nodeMap);

		// Begin acquiring images
		pCam->BeginAcquisition();


		//ImagePtr pImage;

		////take an image
		//pImage = AcquireImage(pCam, nodeMap);
		////convert to CVmat format
		//result = ConvertToCVmat(pImage);
		//pImage->Release();

	result = StartPreview(pCam, nodeMap);

		pCam->EndAcquisition();
		// Deinitialize camera
		pCam->DeInit();
	}

	catch (Spinnaker::Exception &e)
	{
		cout << "Error: " << e.what() << endl;
		result = -1;
	}
	return result;
}


int main(int argc, char** argv)
{
	int result = 0;

	// Retrieve singleton reference to system object
	SystemPtr g3System = System::GetInstance();
	// Retrieve list of cameras from the system
	CameraList camList = g3System->GetCameras();
	unsigned int numCameras = camList.GetSize();
	// Finish if there are no cameras
	if (numCameras == 0)
	{
		// Clear camera list before releasing system
		camList.Clear();
		// Release system
		g3System->ReleaseInstance();
		cout << "Not enough cameras!" << endl;
		cout << "Done! Press any to exit..." << endl;
		system("pause"); //wait for any key press

		return -1;
	}

	//// Read the image file
	//Mat image = imread("C:/Users/ukp/Downloads/Edgefield_Proto0_PCB_stacked.jpg");
	//if (image.empty()) // Check for failure
	//{
	//	cout << "Could not open or find the image" << endl;
	//	system("pause"); //wait for any key press
	//	return -1;
	//}

	//String windowName = "East Village Demo"; //Name of the window
	//namedWindow(windowName); // Create a window
	//imshow(windowName, image); // Show our image inside the created window.
	//waitKey(0); // Wait for any keystroke in the window
	//destroyWindow(windowName); //destroy the created window

	for (unsigned int i = 0; i < numCameras; i++)
	{
		cout << endl << "Running example for camera " << i << "..." << endl;

		result = result | RunSingleCamera(camList.GetByIndex(i));

		cout << "Camera " << i << " example complete..." << endl << endl;
	}

	// Clear camera list before releasing system
	camList.Clear();

	// Release system
	g3System->ReleaseInstance();

	cout << "Done! Press any to exit..." << endl;
	system("pause"); //wait for any key press

	return result;
}