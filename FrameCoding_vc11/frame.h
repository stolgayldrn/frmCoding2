#ifndef FRAME_H
#define FRAME_H

#include "frame.h"
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


const int  MAX_features_per_image = 1050;
using namespace std;

typedef struct Path
{
	string signatureBase;
	string videoBase;
	string videoBulten;
	string frameBase;
	string sceneBase;
	string VT_data;
};

typedef struct ES_params
{
	string index;
	string type;
	string url;
	string userPWD;
};

typedef struct Video_info
{
	string url;
	string fileName;
	long long int id;
	int algorithm_id;
};

typedef struct MYSQL_params
{
	string hostName;
	string userName;
	string password;
	string catalog;
	string table_frame;
	string table_video;
};

typedef struct Algorithm_info
{
	string name;
	int id;
	string tree_name;
	string feature_type;
};


class frame
{
public:	
	frame(Path p);
	~frame(void);
	int initialization(TVoctreeVLFeat VT, Path p);
	int feature_extract_and_quantize(TVoctreeVLFeat VT, Path p);
	int get_json4new_video(Video_info* my_VI, Algorithm_info* my_AI, Path* p);
	int ES_commit(ES_params* my_ES);
	int MYSQL_commit(MYSQL_params* my_Mysql, Video_info* my_VI, Algorithm_info* my_AI, Path* p );
private:
	string filePath, sigPath, ES_id, words_str, video_type, fileName,  frameName;
	int frame_n, scene_n, FL_order, duration;
	TSignature signature;
	unsigned int flags;
	json_t* my_source;
};


#endif
