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

#define __TY_VIDEO_SEARCH___FPS_PROCESS 4
#define __TY_VIDEO_SEARCH___FRAME_WIDTH 500
#define __TY_VIDEO_SEARCH___FRAME_HEIGHT 500
#define  MAX_features_per_image  1050
#define NUMOFFSET 100
#define COLNAME 200

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
	string url;			// address to download
	string fileName;	// filename @database
	long long int id;	// unique id of file @MYSQL
	int algorithm_id;	// unique id algorithm @MYSQL
	string source_type;	// "internet", "AA", ...
	string encoding;	// "mp4", "mpg", ...
	double fps;			// belong to source video
	double height;		// belong to source video
	double width;		// belong to source video
	int duration;		// belong to source video
	int n_frames;		// number of frames (real frame numbers, not related with frameList or frameBase)
	int n_scenes;		// amount of of s
	string path;		// disk path
	bool newVideo;		// true: to import new video, false: for query video
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
	frame(Path *p, string name, Video_info* my_VI, int my_scene_n, int my_duration);
	~frame(void);
	int feature_extract_and_quantize(TVoctreeVLFeat* VT, Path* p);
	int get_json4new_video(Video_info* my_VI, Algorithm_info* my_AI, Path* p);
	int ES_commit(ES_params* my_ES);
	int MYSQL_commit(MYSQL_params* my_Mysql, Video_info* my_VI, Algorithm_info* my_AI, Path* p );
	int get_json4query();
	int ES_post_query(ES_params* my_ES, vector<vector<string>>& ES_results);
private:
	string filePath, sigPath, ES_id, words_str, video_type, fileName,  frameName;
	int frame_n, scene_n, FL_order, duration;
	TSignature signature;
	unsigned int flags;
	json_t* my_source;
};

int initialization(TVoctreeVLFeat* VT, Path* p);

int MYSQL_commit_video_meta(MYSQL_params* my_Mysql, Video_info* my_VI);
int MYSQL_update_video_meta(MYSQL_params* my_Mysql, Video_info* my_VI, int scene_n);

int frame_extract(Video_info* my_VI, Path* p);

int retrieve_data_and_print(sql::ResultSet *rs, int type, int colidx, string colname);
int scene_extract(vector<string> &outputFileNameList, vector<string> & durations, bool AAvideo, Path* p, Video_info* my_VI);

#endif
