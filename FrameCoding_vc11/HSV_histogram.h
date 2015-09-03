#ifndef HSV_HISTOGRAM_H
#define HSV_HISTOGRAM_H
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <string>

using namespace cv;
using namespace std;

class HSV_histogram
{
	private:
		string fileName;
		MatND hist;// Histograms
	public:
		HSV_histogram(const string FN);
		~HSV_histogram(void);
		void Calculate_Histogram();
		MatND getHistogram();

};
#endif
