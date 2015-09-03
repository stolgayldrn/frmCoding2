#include "helpers2.h"
#include "opencv2\highgui\highgui.hpp"
#include "opencv2\imgproc.hpp"
#include <string>
#include <vector>
#include "HSV_histogram.h"
#include "TVoctreeVLFeat.h"
#include "TFeatureExtractor.h"
#include <stdio.h>
#include "curl/curl.h"
#include "jansson.h"
#include <iostream>

using namespace cv;
using namespace std;

vector<string> FrameDownSampling(string FolderAddress);

int main(int argc, char* argv[])
{
	//Elasticsearch parameters initialization
	string es_index = "tolga";
	string es_type	= "TestObject";
	string es_id	= "";
	vector<string> frames = FrameDownSampling(argv[1]); //@param argv[1] is directory of frames' folder
	pathControl((string)argv[2]);
	TVoctreeVLFeat VT;
	VT.init_read("D:/Data/VT_Middle_Tree_100MSig2s_SIFT_HELL_ZSTD_SIFT.dat");

	string video_id = "anni001";
	for(int i = 0; i < frames.size(); i++)
	{
		string fileName = argv[1]; 
		fileName = fileName + "\\" + frames[i];
		es_id = video_id + "_" + frames[i].substr(0,frames[i].length()-4);
		//////////////////////////////////////////////////////////////////////////
		// SIFT Feature Extraction
		Mat im = imread(fileName, IMREAD_GRAYSCALE);
		TByte * IM = im.data;
		t_feat_init();				// initialization of TFeatureExtractor
		TSignature mySig;			
		t_feat_init_sig(mySig);		// Signature initialization
		unsigned char * MySiftu;
		mySig = t_feat_extract(IM, im.cols, im.rows, im.cols, 1, 2000, T_FEAT_EZ_SIFT,T_FEAT_DIST_HELL,T_FEAT_CMPRS_NONE);
		MySiftu = new unsigned char[mySig.numKeypts*128];
		memcpy(MySiftu, mySig.keyPoints, (mySig.numKeypts)*128*sizeof(unsigned char));				
		//////////////////////////////////////////////////////////////////////////
		// BoW QUANTIZATION
		unsigned int *vwi = new unsigned int[mySig.numKeypts]();
		VT.quantize_multi(vwi,MySiftu,mySig.numKeypts);
		
		//////////////////////////////////////////////////////////////////////////
		//JSON			
		json_error_t error;
		json_t *root = json_object();
		json_t *array = json_array();		
		json_t *source = json_object();

		for(int i = 0; i<mySig.numKeypts; i++)
		{
			json_array_append_new(array, json_integer(vwi[i]));
		}
		json_object_set_new(source, "videoID", json_string(video_id.c_str()));
		json_object_set_new(source, "frameID", json_string(frames[i].substr(0,frames[i].length()-4).c_str()));
		json_object_set_new(source, "word", array);

		/*json_object_set_new(root, "_index", json_string(es_index.c_str()));
		json_object_set_new(root, "_type", json_string(es_type.c_str()));
		json_object_set_new(root, "_id", json_string(es_id.c_str()));
		json_object_set_new(root, "_source", source);*/

		//json_object_set_new(root, "videoID", json_string(video_id.c_str()));
		//json_object_set_new(root, "frameID", json_string(frames[i].substr(0,frames[i].length()-4).c_str()));
		//json_object_set_new(root, "word", array);

		if(!json_is_object(root))
		{
			fprintf(stderr, "error: commits is not an array\n");
			return 1;
		}
		//////////////////////////////////////////////////////////////////////////
		// ELASTIC SEARCH
	
		CURL *curl = curl_easy_init();
		char *userPWD = "writer:writeme";
		string url = argv[3];
		url = url + "/" + es_index + "/" + es_type + "/" + es_id;
		struct curl_slist *headers = NULL; 
		size_t json_flags = 0;
		/* set content type */
		headers = curl_slist_append(headers, "Accept: application/json");
		headers = curl_slist_append(headers, "Content-Type: application/json");
		if(curl) {
			CURLcode res;
			curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
			curl_easy_setopt(curl, CURLOPT_USERPWD, userPWD);
			/* set curl options */
			curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
			curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);		
			curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_dumps(source,json_flags));
			res = curl_easy_perform(curl);
			/* cleanup curl handle */
			curl_easy_cleanup(curl);
			/* free headers */
			curl_slist_free_all(headers);
		}
		//////////////////////////////////////////////////////////////////////////
		// releases		
		t_feat_release_signature(&mySig);
		delete[] MySiftu;
		//delete[] IM;
		im.release();
		json_decref(root);
		//////////////////////////////////////////////////////////////////////////
	}
	VT.clean();
	return 0;
}

vector<string> FrameDownSampling(string FolderAddress)
{
	vector<string> fileList, outputFileNameList;
	vector<int> fileListNumeric;
	get_directory_images(FolderAddress.c_str(),fileList);
	for(int i = 0; i<fileList.size(); i++)
	{
		fileListNumeric.push_back(atoi(fileList[i].substr(0,fileList[i].length()-4).c_str()));	
	}
	sort(fileListNumeric.begin(),fileListNumeric.end());
	vector<MatND> HSV_Hists;
	int base_frame_number = 0, limit = fileList.size(), querry = 0;
	//vector<Mat> inputImgVectors, outputImgVectors;		// remove inputImgVector after development done!!!
	
	MatND prevBaseHistogram;
	for(int i = 0; i < fileList.size(); i++)	
	{
		string fileName = FolderAddress + "\\" +  int2string(fileListNumeric[i]) + ".jpg";
		HSV_histogram hsv_hist(fileName);				//	create HSV object
		hsv_hist.Calculate_Histogram();					//	calculate HSV histogram
		HSV_Hists.push_back(hsv_hist.getHistogram());	//	write histogram to vector
		Mat Im = imread(fileName,IMREAD_GRAYSCALE);	
		if(i == 0)
		{
			//outputImgVectors.push_back(Im);
			outputFileNameList.push_back(int2string(fileListNumeric[0]) + ".jpg");
		}
		else
		{
			querry++;
			double correlation_ratio = compareHist( HSV_Hists[0], HSV_Hists[querry], 0 );
			double hellinger_distance = compareHist( HSV_Hists[0], HSV_Hists[querry], 3 );
			if ( correlation_ratio <= 0.95 && hellinger_distance >= 0.4)		//correlation_ratio <= 0.9 && hellinger_distance >= 0.5 very good
			{			
				//outputImgVectors.push_back(Im);
				outputFileNameList.push_back(int2string(fileListNumeric[i]) + ".jpg");
				prevBaseHistogram = HSV_Hists[0];
				HSV_Hists.clear();				
				HSV_Hists.push_back(hsv_hist.getHistogram());
				querry = 0;
			}
		}
		if(	i % 50 == 0 ) printf("\rProcess: %.2f%%",100.0*i/limit);		
		//inputImgVectors.push_back(Im);
		Im.release();
	}
	//////////////////////////////////////////////////////////////////////////
	//  display tool 	
	/*printf("\nTotal frame number: %5d%", limit);
	Mat allImages = makeCanvas(inputImgVectors,900,20);
	namedWindow( "Inputs Window", WINDOW_AUTOSIZE );
	imshow( "Inputs Window", allImages );
	string writeName = FolderAddress + "_inputs.jpg";
	imwrite(writeName, allImages);

	printf("\nTotal reduced frame number: %5d%", outputImgVectors.size());
	Mat OutputImages = makeCanvas(outputImgVectors,800,15);
	namedWindow( "Outputs Window", WINDOW_AUTOSIZE );
	imshow( "Outputs Window", OutputImages );	
	writeName = FolderAddress + "_outputs.jpg";
	imwrite(writeName, OutputImages);*/
	//////////////////////////////////////////////////////////////////////////
	// RELEASES
	fileList.clear();
	fileListNumeric.clear();
	HSV_Hists.clear();
	//inputImgVectors.clear();
	//outputImgVectors.clear();
	printf("\nTotal Processed Frame Number: %5d%", limit);
	printf("\nTotal Reduced Frame Number: %5d%", outputFileNameList.size());
	printf("\nFrame DownSampling is Done");
	return outputFileNameList;
}
