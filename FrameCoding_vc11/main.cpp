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
#include "mysql_connection.h"
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
#include <ctime>
#include "frame.h"

using namespace cv;
using namespace std;

void video_coding_initialization();

int query_video(int argc, char* argv[]);
int import_new_video(int argc, char* argv[], bool isAAvideo);
int SceneExtraction(string FolderAddress, vector<string> &outputFileNameList, vector<string> & durations, bool AAvideo);
int elasticsearch_commit(json_t * my_source, string &es_id);
int JSON_NewVideo_ObjectExtraction(TSignature &mySig, unsigned int * vwi, json_t * my_source, 
	string &videoPath, string &filePath, string &sigPath,
	int i, vector<string> &duration, int frame_n, string &words_str);
void MYSQL_update___video_meta(vector<string> &scenes);
void MYSQL_commit___video_frame(int frame_n, TSignature &mySig, string &videoPath, string &filePath, string &sigPath, string &words_str, int i, vector<string> &duration, string &es_id);
void imported_videos(vector<string> &video_id, vector<string> &video_url);
void MYSQL_commit__video_meta(double dFPS, double dWidth, double dHeight, double dDuration, int n_frames);
int frame_cropper(bool newVideo, int argc, char* argv[]);
int post_ES_query(json_t * source, vector<string> &ES_result, vector<string> scenes, vector<vector<string>> &ES_results);
void get_json_for_query_frame(TSignature &mySig, unsigned int * vwi, string &words_str, json_t * source);
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
const string sigBasePath = "D:/VideoSearch/Signatures";
const string videosBasePath = "D:/VideoSearch/TestData/mp4";
const string videosBultenPath = "D:/VideoSearch/TestData/ShowTVBulten";
const string framesBasePath = "D:/VideoSearch/Frames";
const string scenesBasePath = "D:/VideoSearch/Scenes";
const string VT_data_path = "D:/VideoSearch/VT/VT_Middle_Tree_100MSig2s_SIFT_HELL_ZSTD_SIFT.dat"; 
//////////////////////////////////////////////////////////////////////////
const string ES_index = "video_search";
const string ES_type	= "frames";
const string ES_url = "http://services.maviucak.com:9200";
//////////////////////////////////////////////////////////////////////////
string AA_ID = "";
string	AA_address = "http://www.anadoluimages.com";
//////////////////////////////////////////////////////////////////////////
string fileName = "";
int video_id = 0;
long long int frame_id = 0;
int algorithm_id = 1;
TVoctreeVLFeat VT;
/************************************************************************/
/*		edit "fileName", "video_id", "AA_ID" than execute	            */
/************************************************************************/
int main(int argc, char* argv[])
{
	Path *myPath = new Path;
	ES_params *myES = new ES_params;
	Video_info *myVideo_info = new Video_info;
	MYSQL_params *myMYSQL = new MYSQL_params;
	Algorithm_info *myAlg = new Algorithm_info;

	myPath->frameBase			= "D:/VideoSearch/Frames";
	myPath->sceneBase			= "D:/VideoSearch/Scenes";
	myPath->signatureBase		= "D:/VideoSearch/Signatures";
	myPath->videoBase			= "D:/VideoSearch/TestData/mp4";
	myPath->videoBulten			= "D:/VideoSearch/TestData/ShowTVBulten";
	myPath->VT_data				= "D:/VideoSearch/VT/VT_Middle_Tree_100MSig2s_SIFT_HELL_ZSTD_SIFT.dat";

	myES->index					= "video_search";
	myES->type					= "frames2";
	myES->url					= "http://services.maviucak.com:9200";;
	myES->userPWD				= "";

	myVideo_info->algorithm_id	= 1;
	myVideo_info->id			= 0;
	myVideo_info->url			= "http://www.anadoluimages.com";
	myVideo_info->fileName		= "";
	
	myMYSQL->hostName			= "tcp://127.0.0.1:3306";
	myMYSQL->userName			= "bitnami";
	myMYSQL->password			= "181420b969";
	myMYSQL->catalog			= "videosearch";
	myMYSQL->table_frame		= "video_frame";
	myMYSQL->table_video		= "video_meta";

	myAlg->feature_type			= "EZ_SIFT";
	myAlg->id					= 1;
	myAlg->name					= "Run1";
	myAlg->tree_name			= "VT_Middle_Tree_100MSig2s_SIFT_HELL_ZSTD_SIFT";
		

	return 0;
}

int main2(int argc, char* argv[])
{
	video_coding_initialization();	
	/*vector<string> v_id,v_url;
	imported_videos(v_id,v_url);
	AA_ID = "p/5445676";
	for (int i = 0;  i<34; i++)
	{
	fileName = v_url[i].substr(v_url[i].rfind("/")+1);
	video_id = atoi(v_id[i].c_str());
	import_new_video(argc,argv, true);
	}*/
	fileName = "AA_concat_news.mp4";
	AA_ID = "search";
	video_id = -1;
	query_video(argc,argv);

	VT.clean();
	return 0;
}


int query_video(int argc, char* argv[])
{
	const clock_t begin_time = clock();
	//frame cropping
	//frame_cropper(false, argc, argv);
	cout << "\rFrameCropper time "<< float( clock () - begin_time ) /  CLOCKS_PER_SEC <<endl;
	clock_t prev_time = clock();
	//////////////////////////////////////////////////////////////////////////
	vector<string> scenes, duration,ES_result;
	vector<vector<string>> ES_results;
	SceneExtraction(framesBasePath + "/" + fileName, scenes, duration, true); //@param argv[1] is directory of frames' folder
	cout << "\rSceneExtraction time "<< float( clock () - prev_time ) /  CLOCKS_PER_SEC <<endl;
	prev_time = clock();
	//////////////////////////////////////////////////////////////////////////
	for(int i = 0; i < scenes.size(); i++)
	{
		// SIFT Feature Extraction
		TSignature mySig;
		unsigned char * MySiftu;
		Mat im = imread(scenesBasePath + "/" + fileName + "/" + scenes[i], IMREAD_GRAYSCALE);
		TByte * IM = im.data;
		//t_feat_init();				// initialization of TFeatureExtractor
		t_feat_init_sig(mySig);		// Signature initialization
		mySig = t_feat_extract(IM, im.cols, im.rows, im.cols, 1, 2000, T_FEAT_EZ_SIFT,T_FEAT_DIST_HELL,T_FEAT_CMPRS_NONE);
		MySiftu = new unsigned char[mySig.numKeypts*128];
		memcpy(MySiftu, mySig.keyPoints, (mySig.numKeypts)*128*sizeof(unsigned char));
		//////////////////////////////////////////////////////////////////////////
		// BoW Quantization
		string words_str = "";
		unsigned int *vwi = new unsigned int[mySig.numKeypts]();
		VT.quantize_multi(vwi,MySiftu,mySig.numKeypts);
		for (unsigned int ii = 0; (ii < mySig.numKeypts) && (ii < 1000); ii++) // 1000 is limit for elasticsearch
			words_str += " " + int2string(vwi[ii]);
		//////////////////////////////////////////////////////////////////////////
		//JSON			
		json_t *source = json_object();
		get_json_for_query_frame(mySig, vwi, words_str, source);
		//////////////////////////////////////////////////////////////////////////
		// ELASTIC SEARCH
		ES_result.push_back(scenes[i]);
		post_ES_query(source, ES_result, scenes, ES_results);
		//////////////////////////////////////////////////////////////////////////
		// releases		
		t_feat_release_signature(&mySig);
		delete[] MySiftu;
		delete[] vwi;
		im.release();
		im.deallocate();
		json_decref(source);
		if(	i % 10 == 0 ) 
			printf("\rScenes Analysis:::::Process Rate: %.2f%%",100.0*i/scenes.size());
	}
	cout << "\rSceneAnalysis total time "<< float( clock () - prev_time ) /  CLOCKS_PER_SEC <<endl;
	prev_time = clock();
	//////////////////////////////////////////////////////////////////////////
	// releases
	ofstream myfile;
	string CSV_Path = fileName + ".csv";
	myfile.open(CSV_Path.c_str());
	for (int i = 0; i < ES_results.size(); i++)
	{
		string lineStr = "";
		for (int ii = 0; ii < 11; ii++)
		{
			lineStr += ES_results[i][ii] + ";";
		}
		lineStr += "\n";
		myfile << lineStr;
	}
	cout << "SceneExtraction time "<< float( clock () - prev_time ) /  CLOCKS_PER_SEC <<endl; 
	myfile.close();
	scenes.clear();
	return 1;
}

int import_new_video(int argc, char* argv[], bool isAAvideo)
{
	const clock_t begin_time = clock();
	// frame cropping
	frame_cropper(true, argc, argv);
	cout << float( clock () - begin_time ) /  CLOCKS_PER_SEC <<endl;
	string framesPath = framesBasePath + "/" + fileName;
	vector<string> scenes, duration;
	SceneExtraction(framesPath, scenes, duration, isAAvideo); //@param argv[1] is directory of frames' folder
	cout <<"\ntotal time:" << float( clock () - begin_time ) /  CLOCKS_PER_SEC << endl;
	//////////////////////////////////////////////////////////////////////////
	//TVoctreeVLFeat VT;
	//VT.init_read(VT_data_path.c_str());
	//t_feat_init();				// initialization of TFeatureExtractor
	//////////////////////////////////////////////////////////////////////////
	// MySQL: update video_meta for scene number
	MYSQL_update___video_meta(scenes);
	cout <<"\ntotal time:" << float( clock () - begin_time ) /  CLOCKS_PER_SEC;
	string videoPath = videosBasePath + "/" + fileName;
	//////////////////////////////////////////////////////////////////////////
	for(int i = 0; i < scenes.size(); i++)
	{
		string filePath = framesBasePath + "/" + fileName + "/" + scenes[i];
		string sigPath = sigBasePath + "/" + fileName + "/" + scenes[i] + ".sig";
		string es_id	= "";
		int frame_n = atoi(scenes[i].substr(0,scenes[i].length()-4).c_str());
		//////////////////////////////////////////////////////////////////////////
		// SIFT Feature Extraction
		TSignature mySig;
		unsigned char * MySiftu;
		Mat im = imread(filePath, IMREAD_GRAYSCALE);
		//TByte *IM = im.data;
		t_feat_init_sig(mySig);		// Signature initialization
		mySig = t_feat_extract((TByte*)im.data, im.cols, im.rows, im.cols, 1, 2000, T_FEAT_EZ_SIFT,T_FEAT_DIST_HELL,T_FEAT_CMPRS_NONE);
		MySiftu = new unsigned char[mySig.numKeypts*128];
		memcpy(MySiftu, mySig.keyPoints, (mySig.numKeypts)*128*sizeof(unsigned char));
		pathControl(sigBasePath+ "/" + fileName);
		int IsSigWritten = t_feat_write_sig_v2(mySig, sigPath.c_str());		
		//////////////////////////////////////////////////////////////////////////
		// BoW Quantization
		string words_str = "";
		unsigned int *vwi = new unsigned int[mySig.numKeypts]();
		VT.quantize_multi(vwi,MySiftu,mySig.numKeypts);
		for (unsigned int ii = 0; ii < mySig.numKeypts; ii++)
			words_str += " " + int2string(vwi[ii]);
		//////////////////////////////////////////////////////////////////////////
		//JSON			
		json_t *source = json_object();
		JSON_NewVideo_ObjectExtraction(mySig, vwi, source, videoPath, filePath, sigPath, i, duration, frame_n, words_str);
		//////////////////////////////////////////////////////////////////////////
		// ELASTIC SEARCH	
		elasticsearch_commit(source, es_id);
		//////////////////////////////////////////////////////////////////////////
		// MySQL//bitnami : 181420b969  /// video_frame ///
		MYSQL_commit___video_frame(frame_n, mySig, videoPath, filePath, sigPath, 
			words_str, i, duration, es_id);
		//////////////////////////////////////////////////////////////////////////
		// releases	
		im.deallocate();
		t_feat_release_signature(&mySig);
		delete[] MySiftu;
		delete[] vwi;
		json_decref(source);
		cout <<"\ntotal time:" << float( clock () - begin_time ) /  CLOCKS_PER_SEC << " scene "<< i;
	}
	//////////////////////////////////////////////////////////////////////////
	// releases
	scenes.clear();
	return 1;
}


int SceneExtraction(string FolderAddress, vector<string> &outputFileNameList, vector<string> & durations, bool AAvideo)
{
	vector<string> fileList;
	vector<int> fileListNumeric;
	get_directory_images(FolderAddress.c_str(),fileList);
	for(int i = 0; i<fileList.size(); i++)
	{
		fileListNumeric.push_back(atoi(fileList[i].substr(0,fileList[i].length()-4).c_str()));	
	}
	sort(fileListNumeric.begin(),fileListNumeric.end());
	vector<MatND> HSV_Hists;
	unsigned int base_frame_number = 0, limit = fileList.size(), querry = 0;
	//vector<Mat> inputImgVectors, outputImgVectors;		// remove inputImgVector after development done!!!
	//MatND prevBaseHistogram;
	for(int i = 0; i < fileList.size(); i++)	
	{
		string filePath = FolderAddress + "\\" +  int2string(fileListNumeric[i]) + ".jpg";
		HSV_histogram hsv_hist(filePath);				//	create HSV object
		hsv_hist.Calculate_Histogram();					//	calculate HSV histogram
		HSV_Hists.push_back(hsv_hist.getHistogram());	//	write histogram to vector
		Mat Im = imread(filePath,IMREAD_GRAYSCALE);	
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
			//if ( correlation_ratio <= 0.95 && hellinger_distance >= 0.4)		//correlation_ratio <= 0.9 && hellinger_distance >= 0.5 very good
			if ( correlation_ratio <= 0.99 && hellinger_distance >= 0.25)		//correlation_ratio <= 0.9 && hellinger_distance >= 0.5 very good
			{			
				//outputImgVectors.push_back(Im);
				outputFileNameList.push_back(int2string(fileListNumeric[i]) + ".jpg");
				durations.push_back(int2string(HSV_Hists.size()-1));
				//prevBaseHistogram = HSV_Hists[0];
				HSV_Hists.clear();				
				HSV_Hists.push_back(hsv_hist.getHistogram());
				querry = 0;
			}
		}
		if(	i % 50 == 0 ) printf("\rScene Extraction:::::Process Rate: %.2f%%",100.0*i/limit);		
		//inputImgVectors.push_back(Im);
		Im.release();
	}
	durations.push_back(int2string(HSV_Hists.size())); //duration of last scene
	//////////////////////////////////////////////////////////////////////////

	if (AAvideo)
	{
		vector<string> temp_outputFileNameList;
		pathControl(scenesBasePath + "/" + fileName);
		for (int i = 10; i<outputFileNameList.size(); i++)
		{
			fileCopy(FolderAddress + "/" + outputFileNameList[i], scenesBasePath + "/" + fileName + "/" + outputFileNameList[i]);
			temp_outputFileNameList.push_back(outputFileNameList[i]);
		}
		outputFileNameList.clear();
		outputFileNameList = temp_outputFileNameList;
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
	printf("\nTotal Frame Number: %5d%", limit);
	printf("\nTotal Scene Number: %5d%", outputFileNameList.size());
	printf("\nScene Extraction is Done.");
	return 1;
}

void MYSQL_update___video_meta(vector<string> &scenes)
{
	try {
		sql::Driver *driver;
		sql::Connection *con;
		sql::Statement *stmt;
		/* Create a connection */
		driver = get_driver_instance();
		con = driver->connect("tcp://127.0.0.1:3306", "bitnami", "181420b969");
		stmt = con->createStatement();
		/* Connect to the MySQL test database */
		con->setSchema("videosearch");
		string stament = "UPDATE video_meta SET n_scenes=\"" + int2string(scenes.size()) + "\" WHERE id=" + int2string(video_id);
		stmt->execute(stament.c_str());
		delete stmt;
		delete con;
		printf("\nMYSQL::::Scene Number is updated (video_meta). ");
	} catch (sql::SQLException &e) {
		cout << "# ERR: SQLException in " << __FILE__;
		cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
		cout << "# ERR: " << e.what();
		cout << " (MySQL error code: " << e.getErrorCode();
		cout << ", SQLState: " << e.getSQLState() << " )" << endl;
	}
}

int JSON_NewVideo_ObjectExtraction(TSignature &mySig, unsigned int * vwi, json_t * my_source, 
						  string &videoPath, string &filePath, string &sigPath,
						  int i, vector<string> &duration, int frame_n, string &words_str)
{
	json_t *array = json_array();	
	try
	{
		for(unsigned int ii = 0; ii<mySig.numKeypts; ii++)
		{
			json_array_append_new(array, json_integer(vwi[ii]));
		}
	}
	catch (Exception e)
	{
		cout << "# ERR: Elasticsearch Exception in " << __FILE__;
		cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
		cout << "# ERR: " << e.what();
		cout << " ES error code: " << e.err;
		cout << ", ES State: " << e.msg << " )" << endl;
	}
	try
	{
		json_object_set_new(my_source, "video_ID", json_string(int2string(video_id).c_str()));
		json_object_set_new(my_source, "algorithm_ID", json_string(int2string(algorithm_id).c_str()));
		json_object_set_new(my_source, "algorithm_name", json_string("Run1"));
		json_object_set_new(my_source, "tree_name", json_string("VT_Middle_Tree_100MSig2s_SIFT_HELL_ZSTD_SIFT"));
		json_object_set_new(my_source, "SIFT_type", json_string("EZ_SIFT"));
		json_object_set_new(my_source, "feature_count", json_integer(mySig.numKeypts));
		json_object_set_new(my_source, "video_disk_path", json_string(videoPath.c_str()));
		json_object_set_new(my_source, "frame_disk_path", json_string(filePath.c_str()));
		json_object_set_new(my_source, "signature_disk_path", json_string(sigPath.c_str()));
		json_object_set_new(my_source, "scene", json_integer(i));
		json_object_set_new(my_source, "scene_duration", json_string(duration[i].c_str()));
		json_object_set_new(my_source, "frame_n", json_string(int2string(frame_n).c_str()));

		json_object_set_new(my_source, "word", array);
		json_object_set_new(my_source, "words_string", json_string(words_str.c_str()));
	}
	catch (Exception e)
	{
		cout << "# ERR: Elasticsearch Exception in " << __FILE__;
		cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
		cout << "# ERR: " << e.what();
		cout << " ES error code: " << e.err;
		cout << ", ES State: " << e.msg << " )" << endl;
	}
	
	if(!json_is_object(my_source))
	{
		fprintf(stderr, "error: commits is not an array\n");
		return -1;
	}
	///releases
	//json_decref(array);
	return 1;
}

int elasticsearch_commit(json_t * my_source, string &es_id)
{
	CURL *curl = curl_easy_init();
	//char *userPWD = "writer:writeme";
	
	string ES_new_object_url = ES_url + "/" + ES_index + "/" + ES_type ;
	struct curl_slist *headers = NULL; 
	size_t json_flags = 0;
	/* set content type */
	headers = curl_slist_append(headers, "Accept: application/json");
	headers = curl_slist_append(headers, "Content-Type: application/json");
	// Response information.
	int httpCode(0);
	std::unique_ptr<std::string> httpData(new std::string());

	if(curl) {
		CURLcode res;
		/* set curl options */
		curl_easy_setopt(curl, CURLOPT_URL, ES_new_object_url.c_str());
		//curl_easy_setopt(curl, CURLOPT_USERPWD, userPWD);
		curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);		
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_dumps(my_source,json_flags));
		// Hook up data handling function.
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, callback);
		// Hook up data container (will be passed as the last parameter to the
		// callback handling function).  Can be any pointer type, since it will
		// internally be passed as a void pointer.
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, httpData.get());
		
		res = curl_easy_perform(curl);
		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
		/* cleanup curl handle */
		curl_easy_cleanup(curl);
		/* free headers */
		curl_slist_free_all(headers);
	}

	if (httpCode == 201)
	{
		json_t *rootRes;
		json_error_t error;
		rootRes = json_loads( httpData->data(), 0, &error );
		if ( !rootRes )
		{
			fprintf( stderr, "error: on line %d: %s\n", error.line, error.text );
			return 1;
		}
		int boolean;
		const char *str;
		json_unpack(rootRes, "{s:s}", "_id", &str);
		es_id = str;
		printf("\nES::::Frames of New Video are committed to video_search\\frames. ");
		httpData->clear();
		httpData->shrink_to_fit();
		json_decref(rootRes);
	}
}

int post_ES_query(json_t * source, vector<string> &ES_result, vector<string> scenes, vector<vector<string>> &ES_results)
{
	CURL *curl = curl_easy_init();
	//char *userPWD = "writer:writeme";

	string ES_new_object_url = ES_url + "/" + ES_index + "/" + "_search" ;
	struct curl_slist *headers = NULL; 
	size_t json_flags = 0;
	/* set content type */
	headers = curl_slist_append(headers, "Accept: application/json");
	headers = curl_slist_append(headers, "Content-Type: application/json");
	// Response information.
	int httpCode(0);
	std::unique_ptr<std::string> httpData(new std::string());

	if(curl) {
		CURLcode res;
		/* set curl options */
		curl_easy_setopt(curl, CURLOPT_URL, ES_new_object_url.c_str());
		//curl_easy_setopt(curl, CURLOPT_USERPWD, userPWD);
		curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "GET");
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);		
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_dumps(source,json_flags));
		// Hook up data handling function.
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, callback);
		// Hook up data container (will be passed as the last parameter to the
		// callback handling function).  Can be any pointer type, since it will
		// internally be passed as a void pointer.
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, httpData.get());

		res = curl_easy_perform(curl);
		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
		/* cleanup curl handle */
		curl_easy_cleanup(curl);
		/* free headers */
		curl_slist_free_all(headers);
	}


	if (httpCode == 200)
	{
		json_t *rootRes, *hits, *hits2;
		json_error_t error;

		rootRes = json_loads( httpData->data(), 0, &error );
		if ( !rootRes )
		{
			fprintf( stderr, "error: on line %d: %s\n", error.line, error.text );
			return 1;
		}
		hits = json_object_get(rootRes, "hits");
		hits2 = json_object_get(hits, "hits");
		if(!json_is_array(hits2))
		{
			fprintf(stderr, "error: hits2 is not an array\n");
			return 1;
		}
		else
		{
			
			for(int i = 0; i < 10 ; i++)
			{
				if (i < json_array_size(hits2)   )
				{
					json_t *data, *sourceRes, *video_id_RES;
					data = json_array_get(hits2, i);
					const char *message_text;				 
					if(!json_is_object(data))
					{
						fprintf(stderr, "error: commit data %d is not an object\n", i + 1);
						return 1;
					}
					sourceRes = json_object_get(data, "_source");
					video_id_RES = json_object_get(sourceRes, "video_ID");
					message_text = json_string_value(video_id_RES);
					ES_result.push_back(message_text);
					//releases
					json_object_clear(data);
					json_object_clear(sourceRes);
					json_object_clear(video_id_RES);
				} 
				else
				{
					ES_result.push_back("-1");
				}
			}
			//releases			
			json_object_clear(rootRes);
			json_object_clear(hits);
			json_object_clear(hits2);
			ES_results.push_back(ES_result);
			ES_result.clear();
		}
	}
}

void MYSQL_commit___video_frame(int frame_n, TSignature &mySig, string &videoPath, string &filePath, string &sigPath, string &words_str, int i, vector<string> &duration, string &es_id)
{
	frame_id = video_id;
	frame_id *= 100000000;		
	frame_id += frame_n;
	/*cout << endl;
	cout << "MYSQL for video_frame" << endl;*/
	try {
		sql::Driver *driver;
		sql::Connection *con;
		sql::PreparedStatement *pstmt;
		/* Create a connection */
		driver = get_driver_instance();
		con = driver->connect("tcp://127.0.0.1:3306", "bitnami", "181420b969");
		/* Connect to the MySQL test database */
		con->setSchema("videosearch");
		pstmt = con->prepareStatement("INSERT INTO video_frame(video_id, frame_id, algorithm_id, " 
			"algorithm_name, tree_name, SIFT_type, feature_count, video_disk_path, frame_disk_path, " 
			"signature_disk_path, words_string, scene_id, scene_duration, frame_n, ES_id) "
			"VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
		pstmt->setInt64(1, video_id);
		pstmt->setInt64(2, frame_id);
		pstmt->setInt(3, 1);
		pstmt->setString(4,"Run1");
		pstmt->setString(5,"Medium_Tree");
		pstmt->setString(6,"Ez_SIFT");
		pstmt->setInt(7, mySig.numKeypts);
		pstmt->setString(8,videoPath.c_str());
		pstmt->setString(9,filePath.c_str());
		pstmt->setString(10,sigPath.c_str());
		pstmt->setString(11, words_str.c_str());
		pstmt->setInt(12,i);
		pstmt->setInt(13,atoi(duration[i].c_str()));
		pstmt->setInt(14,frame_n);
		pstmt->setString(15,es_id.c_str());
		pstmt->executeUpdate();
		delete pstmt;
		delete con;
		printf("\nMYSQL::::Frames of New Video are committed to video_frame.\n ");
	} catch (sql::SQLException &e) {
		cout << "# ERR: SQLException in " << __FILE__;
		cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
		cout << "# ERR: " << e.what();
		cout << " (MySQL error code: " << e.getErrorCode();
		cout << ", SQLState: " << e.getSQLState() << " )" << endl;
	}
}

void imported_videos(vector<string> &video_id, vector<string> &video_url)
{
	string v_id[34] = {"1","2","3","4","5","6","7","8","9","10","11","12","13","14","15","16","17","18","19","20","21","22","23","24","25","26","27","28","29","30","31","32","33","34"};
	string v_url[34] = {
		"D:/VideoSearch/Test Data/mp4/906727dd-77af-4829-af55-34df64a50dd6_copy.mp4",
		"D:/VideoSearch/Test Data/mp4/97eb6651-bbc7-4424-a92f-2649c4eebff3_copy.mp4",
		"D:/VideoSearch/Test Data/mp4/87a227fd-d75a-466b-8c8a-98e8ecc6757e_copy.mp4",
		"D:/VideoSearch/Test Data/mp4/4d4a4950-cbbb-4d8f-972a-b81c27b5bcd0_copy.mp4",
		"D:/VideoSearch/Test Data/mp4/b9f2f9d6-2f3b-4e8a-be70-3298c72fecc7_copy.mp4",
		"D:/VideoSearch/Test Data/mp4/197ec7e9-6a0b-4f2f-8900-3a92c7eee331_copy.mp4",
		"D:/VideoSearch/Test Data/mp4/b69ce2a8-e3a2-4fdb-a0c6-cdebf7489975_copy.mp4",
		"D:/VideoSearch/Test Data/mp4/a91048c5-8e2c-4327-9a51-9cdc08b43a66_copy.mp4",
		"D:/VideoSearch/Test Data/mp4/742fd652-1de8-4008-9449-260a8c1b7716_copy.mp4",
		"D:/VideoSearch/Test Data/mp4/8817c208-5ec2-4e82-95f9-111ad7066f0c_copy.mp4",
		"D:/VideoSearch/Test Data/mp4/a1bca3be-f93e-4932-a646-ddf0895e10d0_copy.mp4",
		"D:/VideoSearch/Test Data/mp4/fb8a922f-2a6a-4a6d-ae75-da4009c55656_copy.mp4",
		"D:/VideoSearch/Test Data/mp4/a0edbcf7-9027-49f9-bca3-9ce828e27353_copy.mp4",
		"D:/VideoSearch/Test Data/mp4/3b898df0-5dd9-4b69-8b08-643f1a412c84_copy.mp4",
		"D:/VideoSearch/Test Data/mp4/e298ceed-3fec-4086-a416-05f909c3ed0d_copy.mp4",
		"D:/VideoSearch/Test Data/mp4/73febf90-01bf-4e63-a9c5-51fc19865680_copy.mp4",
		"D:/VideoSearch/Test Data/mp4/13e85839-d5a6-4462-a13d-0bb7047507c8_copy.mp4",
		"D:/VideoSearch/Test Data/mp4/a7594ee9-ae82-4437-b07b-ff3981235834_copy.mp4",
		"D:/VideoSearch/Test Data/mp4/629617e5-22a3-4b5d-ae5c-5d2001d431c7_copy.mp4",
		"D:/VideoSearch/Test Data/mp4/6038f0a7-dc50-46e0-b570-93ae367d90bf_copy.mp4",
		"D:/VideoSearch/Test Data/mp4/b9d862e6-f293-4dea-848f-5e609daac2f8_copy.mp4",
		"D:/VideoSearch/Test Data/mp4/98aac48e-2f7a-42ea-90db-20c084684512_copy.mp4",
		"D:/VideoSearch/Test Data/mp4/4f694343-81c1-4399-a1df-3e4249dee2fb_copy.mp4",
		"D:/VideoSearch/Test Data/mp4/75eab324-2afd-4165-b2a5-e2f216d4f814_copy.mp4",
		"D:/VideoSearch/Test Data/mp4/dd521502-8fd6-465b-be61-1b39adb7486f_copy.mp4",
		"D:/VideoSearch/Test Data/mp4/805ac6cd-8f5a-40cf-a44c-80fee4c1e5b9_copy.mp4",
		"D:/VideoSearch/Test Data/mp4/60bcdfc1-4838-4c59-adcd-2e59547c466d_copy.mp4",
		"D:/VideoSearch/Test Data/mp4/dedfe767-6766-4085-a385-d033d8905a27_copy.mp4",
		"D:/VideoSearch/Test Data/mp4/a73aa0f5-f2c3-44e9-a612-bab6f4b72db4_copy.mp4",
		"D:/VideoSearch/Test Data/mp4/15dac920-ec34-493b-9d82-466639f0e703_copy.mp4",
		"D:/VideoSearch/Test Data/mp4/48e22f73-be43-4a40-acf5-14f12ff2d226_copy.mp4",
		"D:/VideoSearch/Test Data/mp4/fb682adc-4ea1-4f63-bd82-ef15f6972706_copy.mp4",
		"D:/VideoSearch/Test Data/mp4/6583699e-adc5-4131-ba40-7156ba0a3a4e_copy.mp4",
		"D:/VideoSearch/Test Data/mp4/88f1d6d2-19e3-47ee-8bbc-bdef409d5842_copy.mp4",
	};

	for (int i = 0; i<34; i++)
	{
		video_id.push_back(v_id[i]);
	}
	for (int i = 0; i<34; i++)
	{
		video_url.push_back(v_url[i]);
	}

}

void MYSQL_commit__video_meta(double dFPS, double dWidth, double dHeight, double dDuration, int n_frames)
{
	cout << endl;
	cout << "Let's have MySQL count from 10 to 1..." << endl;
	try {
		sql::Driver *driver;
		sql::Connection *con;
		sql::PreparedStatement *pstmt;
		/* Create a connection */
		driver = get_driver_instance();
		con = driver->connect("tcp://127.0.0.1:3306", "bitnami", "181420b969");
		//////////////////////////////////////////////////////////////////////////
		con->setSchema("videosearch");
		pstmt = con->prepareStatement("INSERT INTO video_meta( id, source_type, source_url, source_encoding, source_fps, source_height, source_width, duration, n_frames, n_scenes, video_disk_path) VALUES ( ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
		pstmt->setInt(1,NULL);
		pstmt->setString(2,"AA");
		string tempStr = AA_address + "/" + AA_ID;
		pstmt->setString(3,tempStr.c_str());
		pstmt->setString(4,"mp4");
		pstmt->setDouble(5,dFPS);
		pstmt->setInt(6,dWidth);
		pstmt->setInt(7,dHeight);
		pstmt->setInt(8,dDuration);
		pstmt->setInt(9,n_frames);
		pstmt->setInt(10,0);
		tempStr = videosBasePath + "/" + fileName;
		pstmt->setString(11,tempStr.c_str());
		pstmt->executeUpdate();
		delete pstmt;
		delete con;
		printf("\nMYSQL::::New Video is committed to video_meta.\n ");
		//////////////////////////////////////////////////////////////////////////
	} catch (sql::SQLException &e) {
		cout << "# ERR: SQLException in " << __FILE__;
		cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
		cout << "# ERR: " << e.what();
		cout << " (MySQL error code: " << e.getErrorCode();
		cout << ", SQLState: " << e.getSQLState() << " )" << endl;
	}
}

int frame_cropper(bool newVideo, int argc, char* argv[])//TODO: do not forget to comment out MYSQL commit video_meta for new videos
{
	string videoPath = "";
	if (newVideo)
	{
		videoPath = videosBasePath + "/" + fileName;
	} 
	else
	{
		videoPath = videosBultenPath + "/" + fileName;
	}
	VideoCapture cap(videoPath);		///@param argv[1] open the video file with this path
	
	if (!cap.isOpened())			///@details if not success, exit program
	{
		cout << "Cannot open the video " << endl;
		return 0;
	}

	string writePath = framesBasePath + "/" + fileName;		///@param argv[2] take folder path for frames to write
	pathControl(writePath);			////@details control the path for existence, if it is not exist then create folder
	int CROP_FRAME_WIDTH, CROP_FRAME_HEIGHT, CROP_FPS;
	if(argc>3) // if params are given
	{
		CROP_FRAME_WIDTH	= atoi(argv[3]);		///@param argv[3] Cropping width 
		CROP_FRAME_HEIGHT	= atoi(argv[4]);		///@param argv[4] Cropping height
		CROP_FPS 			= atoi(argv[5]);		///@param argv[5] Cropping height
	}
	else			// if params are not given
	{
		CROP_FRAME_WIDTH	= __TY_VIDEO_SEARCH___FRAME_WIDTH;
		CROP_FRAME_HEIGHT 	= __TY_VIDEO_SEARCH___FRAME_HEIGHT;
		CROP_FPS			= __TY_VIDEO_SEARCH___FPS_PROCESS;
	}

	double dWidth = cap.get(CAP_PROP_FRAME_WIDTH);			///@desc get the width of frames of the video
	double dHeight = cap.get(CAP_PROP_FRAME_HEIGHT);		///@details the height of frames of the video
	double dFPS = cap.get(CAP_PROP_FPS);					///@details get the frame per seconds of the video 
	double dFrameCount = cap.get(CAP_PROP_FRAME_COUNT);		///@details get the total frame count of the video
	double dDuration = dFrameCount / dFPS;
	cout <<"\n/////////////////////////////////////////////////////" << endl;
	cout <<"//////////		NEW  VIDEO		/////////" << endl;
	cout <<"File Name: " << fileName << endl;
	cout << "Frame size  : " << dWidth << " x " << dHeight << endl;
	cout << "Frame count : " << dFrameCount << endl;
	cout << "FPS         : " << dFPS << endl;

	int FrameNumber = 0;
	float fS = dFPS / (CROP_FPS );
	int frameStep = fS + 0.5;			// rounding
	Mat frame;
	int n_frames = 0;

	float scaleW = dWidth/CROP_FRAME_WIDTH;
	float scaleH = dHeight/CROP_FRAME_HEIGHT;

	while (FrameNumber<(dFrameCount - frameStep-1)) 
	{		
		for(int i = 0 ; i < frameStep; i++)
		{
			bool bSuccess = cap.read(frame); // read a new frame from video
			FrameNumber++;
			if (!bSuccess) // if not success, break loop
			{
				cout << "Cannot read a frame from video stream" << endl;
				break;
			}			
		}
		string Frame2Write = writePath + "/" + int2string(FrameNumber) + ".jpg";  // create the path for frame to write
		Mat frameMid = frame;
		if(!newVideo)
		{
			frameMid = frame(Rect(dWidth*0.1,dHeight*0.05,dWidth*0.8,dHeight*0.7)); // selecting (cropping for process) %80 of image
			scaleW = dWidth*0.8/CROP_FRAME_WIDTH;
			scaleH = dHeight*0.7/CROP_FRAME_HEIGHT;
		}
		/// \brief  A rectangle with sizes  __TY_VIDEO_SEARCH___FRAME_WIDTH x __TY_VIDEO_SEARCH___FRAME_HEIGHT 
		///< is cropped from raw image 
		///< TODO: daha detaylı anlat (%90, rectangle)
		Mat frameDS;
		// frame
		if(scaleH >= scaleW)
		{
			resize(frameMid, frameDS, Size(CROP_FRAME_WIDTH*scaleW/scaleH, CROP_FRAME_HEIGHT), 1.0/(scaleH), 1.0/(scaleH),1); // DOWNSAMPLING		
		}
		else
		{
			resize(frameMid, frameDS, Size(CROP_FRAME_WIDTH, CROP_FRAME_HEIGHT*scaleH/scaleW), 1.0/(scaleW), 1.0/(scaleW),1); // DOWNSAMPLING		
		}
		imwrite(Frame2Write, frameDS);			// A JPG FILE IS BEING SAVED
		n_frames++;
		frameDS.release();
		frameMid.release();
		if(FrameNumber % 200 == 0 )	printf("\rFrame Cropper:::::ProcessRate: %.2f%%",100.0*FrameNumber/dFrameCount);
	}
	//////////////////////////////////////////////////////////////////////////
	// MySQL//bitnami : 181420b969
	//if(newVideo)
	//	MYSQL_commit__video_meta(dFPS, dWidth, dHeight, dDuration, n_frames);
	//////////////////////////////////////////////////////////////////////////
	cap.release();
	frame.release();
	return 1;
}

void get_json_for_query_frame(TSignature &mySig, unsigned int * vwi, string &words_str, json_t * source)
{
	json_t *array = json_array();		
	json_t *query = json_object();
	json_t *match = json_object();
	try
	{
		for(unsigned int ii = 0; ii<mySig.numKeypts; ii++)
		{
			json_array_append_new(array, json_integer(vwi[ii]));
		}		

		//json_object_set_new(match,"word",array);
		json_object_set_new(match,"words_string", json_string(words_str.c_str()));
		json_object_set_new(query, "match", match);
		json_object_set_new(source, "query", query);
	}
	catch (Exception e)
	{
		cout << "# ERR: Elasticsearch Exception in " << __FILE__;
		cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
		cout << "# ERR: " << e.what();
		cout << " ES error code: " << e.err;
		cout << ", ES State: " << e.msg << " )" << endl;
	}		
	if(!json_is_object(source))
	{
		fprintf(stderr, "error: commits is not an array\n");
	}
}

void video_coding_initialization()
{
	VT.init_read(VT_data_path.c_str());
	t_feat_init();			// initialization of TFeatureExtractor
}
