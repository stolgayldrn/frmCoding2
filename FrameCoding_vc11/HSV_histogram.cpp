#include "HSV_histogram.h"

using namespace cv;
using namespace std;
HSV_histogram::HSV_histogram(const string FN)
{
	fileName = FN;	
}


HSV_histogram::~HSV_histogram(void)
{
	hist.release();
}

void HSV_histogram::Calculate_Histogram()
{
	Mat image;	
	Mat hsv;	
	image = imread(fileName,1);
	try
	{
		cvtColor(image, hsv, COLOR_BGR2HSV);	
	}
	catch(Exception e) 
	{
		printf(fileName.c_str());
	}
	
	static int h_bins = 50; 
	static int s_bins = 60;// Using 50 bins for hue and 60 for saturation
	int histSize[] = { h_bins, s_bins };	
	float h_ranges[] = { 0, 180 };// hue varies from 0 to 179, saturation from 0 to 255
	float s_ranges[] = { 0, 256 };
	const float* ranges[] = { h_ranges, s_ranges };
	int channels[] = { 0, 1 };// Use the o-th and 1-st channels
	
	/// Calculate the histograms for the HSV images
	calcHist( &hsv, 1, channels, Mat(), hist, 2, histSize, ranges, true, false );
	normalize( hist, hist, 0, 1, NORM_MINMAX, -1, Mat() );

	//////////////////////////////////////////////////////////////////////////
	// releases
	image.release();
	hsv.release();
	//delete ranges;	
}

cv::MatND HSV_histogram::getHistogram()
{
	return hist;
}

