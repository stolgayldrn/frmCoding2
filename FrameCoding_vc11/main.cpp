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

using namespace cv;
using namespace std;

vector<string> FrameDownSampling(string FolderAddress);
void MysSQL_video_and_algorithm_table();


const string sigBasePath = "D:/VideoSearch/Signatures";
const string videosBasePath = "D:/VideoSearch/Test Data/mp4";
const string framesBasePath = "D:/VideoSearch/Frames";
const string es_index = "tolga";
const string es_type	= "TestObject";

int main(int argc, char* argv[])
{
	//Elasticsearch parameters initialization	
	string es_id	= "";
	string framesPath = framesBasePath + "/" + argv[1];
	vector<string> scenes = FrameDownSampling(framesPath); //@param argv[1] is directory of frames' folder
	//pathControl((string)argv[2]);
	TVoctreeVLFeat VT;
	VT.init_read("D:/Data/VT_Middle_Tree_100MSig2s_SIFT_HELL_ZSTD_SIFT.dat");
	
	for(int i = 0; i < scenes.size(); i++)
	{
		string fileName = framesBasePath + "/" + argv[1] + "/" + scenes[i];
		string sigPath = sigBasePath + "/" + argv[1] + "/" + scenes[i] + ".sig";
		//es_id = argv[1] + "_" + scenes[i].substr(0,scenes[i].length()-4);
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
		//
		string words_str = "";
		for (unsigned int ii = 0; ii < mySig.numKeypts; ii++)
			words_str += " " + int2string(vwi[ii]);
		//////////////////////////////////////////////////////////////////////////
		//JSON			
		//json_error_t error;
		json_t *root = json_object();
		json_t *array = json_array();		
		json_t *source = json_object();

		for(unsigned int ii = 0; ii<mySig.numKeypts; ii++)
			json_array_append_new(array, json_integer(vwi[ii]));
		
		json_object_set_new(source, "videoID", json_string(argv[1]));
		json_object_set_new(source, "frameID", json_string(scenes[i].substr(0,scenes[i].length()-4).c_str()));
		json_object_set_new(source, "word", array);
		json_object_set_new(source, "words_string", json_string(words_str.c_str()));

		if(!json_is_object(root))
		{
			fprintf(stderr, "error: commits is not an array\n");
			return 1;
		}
		//////////////////////////////////////////////////////////////////////////
		// ELASTIC SEARCH	
		CURL *curl = curl_easy_init();
		char *userPWD = "writer:writeme";
		string url = argv[2];
		url = url + "/" + es_index + "/" + es_type ;
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
			curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
			curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);		
			curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_dumps(source,json_flags));
			res = curl_easy_perform(curl);
			/* cleanup curl handle */
			curl_easy_cleanup(curl);
			/* free headers */
			curl_slist_free_all(headers);
		}
		//////////////////////////////////////////////////////////////////////////
		// MySQL//bitnami : 181420b969
		cout << endl;
		cout << "Let's have MySQL count from 10 to 1..." << endl;
		try {
			sql::Driver *driver;
			sql::Connection *con;
			//sql::Statement *stmt;
			sql::ResultSet *res;
			sql::PreparedStatement *pstmt;
			/* Create a connection */
			driver = get_driver_instance();
			con = driver->connect("tcp://127.0.0.1:3306", "bitnami", "181420b969");
			/* Connect to the MySQL test database */
			con->setSchema("videosearch");
			/*stmt = con->createStatement();
			stmt->execute("DROP TABLE IF EXISTS test");
			stmt->execute("CREATE TABLE test(id INT)");
			delete stmt;*/
			// TABLE:		video_frame
			pstmt = con->prepareStatement("INSERT INTO video_frame(video_id, frame_id, algorithm_id, algorithm_name, tree_name, SIFT_type, feature_count, video_disk_path, frame_disk_path, signature_disk_path, words_string) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
			pstmt->setInt(1, 1);
			pstmt->setInt(2, i);
			pstmt->setInt(3, 1);
			pstmt->setString(4,"Run2");
			pstmt->setString(5,"Small_Tree");
			pstmt->setString(6,"Ez_SIFT");
			pstmt->setInt(7, mySig.numKeypts);
			pstmt->setString(8,argv[2]);
			pstmt->setString(9,fileName.c_str());
			pstmt->setString(10,sigPath.c_str());
			pstmt->setString(11, words_str.c_str());
			pstmt->executeUpdate();
			delete pstmt;
			/* Select in ascending order */
			pstmt = con->prepareStatement("SELECT id FROM video_meta ORDER BY id ASC");
			res = pstmt->executeQuery();
			/* Fetch in reverse = descending order! */
			res->afterLast();
			while (res->previous())
				cout << "\t... MySQL counts: " << res->getInt("id") << endl;
			delete res;
			delete pstmt;
			delete con;
			//////////////////////////////////////////////////////////////////////////
		} catch (sql::SQLException &e) {
			cout << "# ERR: SQLException in " << __FILE__;
			cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
			cout << "# ERR: " << e.what();
			cout << " (MySQL error code: " << e.getErrorCode();
			cout << ", SQLState: " << e.getSQLState() << " )" << endl;
		}
		//////////////////////////////////////////////////////////////////////////
		// releases		
		t_feat_release_signature(&mySig);
		delete[] MySiftu;
		//delete[] IM;
		im.release();
		json_decref(root);
		//delete words_str;
	}
	//////////////////////////////////////////////////////////////////////////
	// releases
	scenes.clear();
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
	unsigned int base_frame_number = 0, limit = fileList.size(), querry = 0;
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

void MysSQL_video_and_algorithm_table()
{
		// MySQL//bitnami : 181420b969
		cout << endl;
		cout << "Let's have MySQL count from 10 to 1..." << endl;
		try {
			sql::Driver *driver;
			sql::Connection *con;
			//sql::Statement *stmt;
			sql::ResultSet *res;
			sql::PreparedStatement *pstmt;
			/* Create a connection */
			driver = get_driver_instance();
			con = driver->connect("tcp://127.0.0.1:3306", "bitnami", "181420b969");
			/* Connect to the MySQL test database */
			con->setSchema("videosearch");
			/*stmt = con->createStatement();
			stmt->execute("DROP TABLE IF EXISTS test");
			stmt->execute("CREATE TABLE test(id INT)");
			delete stmt;*/
			/* '?' is the supported placeholder syntax */
			  //TABLE:		algorithm
			pstmt = con->prepareStatement("INSERT INTO algorithm(id,algorithm_name,tree_name, SIFT_type) VALUES (?, ?, ?, ?)");
			pstmt->setInt(1, 2);
			pstmt->setString(2,"Run2");
			pstmt->setString(3,"Small_Tree");
			pstmt->setString(4,"Ez_SIFT");
			pstmt->executeUpdate();
			delete pstmt;
			/* Select in ascending order */
			pstmt = con->prepareStatement("SELECT id FROM algorithm ORDER BY id ASC");
			res = pstmt->executeQuery();
			/* Fetch in reverse = descending order! */
			res->afterLast();
			while (res->previous())
				cout << "\t... MySQL counts: " << res->getInt("id") << endl;
			delete res;
			delete pstmt;
			////////////////////////////////////////////////////////////////////////
			// TABLE:			video_meta
			pstmt = con->prepareStatement("INSERT INTO video_meta(id, source_type, source_url, source_encoding, source_fps, source_height, source_width, duration, n_frames, n_scenes, video_disk_path) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
			pstmt->setInt(1, 1);
			pstmt->setString(2,"anni001");
			pstmt->setString(3,"https://trecvid.com");
			pstmt->setString(4,".mpg");
			pstmt->setDouble(5,14.5);
			pstmt->setInt(6,640);
			pstmt->setInt(7,480);
			pstmt->setInt(8,500);
			pstmt->setInt(9,136);
			pstmt->setInt(10,51);
			pstmt->setString(11,"D:\VideoSearch\Test Data\mp4\TRECVID\anni001.mpg");
			pstmt->executeUpdate();
			delete pstmt;
			/* Select in ascending order */
			pstmt = con->prepareStatement("SELECT id FROM video_meta ORDER BY id ASC");
			res = pstmt->executeQuery();
			/* Fetch in reverse = descending order! */
			res->afterLast();
			while (res->previous())
				cout << "\t... MySQL counts: " << res->getInt("id") << endl;
			delete res;
			delete pstmt;
			cout << "\t... MySQL counts: " << res->getInt("id") << endl;
			delete con;
			//////////////////////////////////////////////////////////////////////////
		} catch (sql::SQLException &e) {
			cout << "# ERR: SQLException in " << __FILE__;
			cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
			cout << "# ERR: " << e.what();
			cout << " (MySQL error code: " << e.getErrorCode();
			cout << ", SQLState: " << e.getSQLState() << " )" << endl;
		}
}