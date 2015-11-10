#include "frame.h"

using namespace std;


frame::frame(Path *p, string name, Video_info* my_VI, int my_scene_n, int my_duration)
{
	frameName = name;
	fileName = my_VI->fileName;
	filePath = p->frameBase + "/" + fileName + "/" + frameName;
	sigPath = p->signatureBase + "/" + fileName + "/" + frameName + ".sig";
	frame_n = atoi(frameName.substr(0,frameName.length()-4).c_str());
	my_source = json_object();
	scene_n = my_scene_n;
	duration = my_duration;
}

frame::~frame(void)
{
	t_feat_release_signature(&signature);
	json_decref(my_source);
}


int frame::feature_extract_and_quantize(TVoctreeVLFeat* VT, Path* p)
{
	int flags = 1;
	unsigned char* MySiftu;
	Mat im;
	try{// SIFT Feature Extraction		
		im = imread(filePath, IMREAD_GRAYSCALE);
		t_feat_init_sig(signature);		// Signature initialization
		signature = t_feat_extract((TByte*)im.data, im.cols, im.rows, im.cols, 1, 
			MAX_features_per_image, T_FEAT_EZ_SIFT,T_FEAT_DIST_HELL,T_FEAT_CMPRS_NONE);
		MySiftu = new unsigned char[signature.numKeypts*128];
		memcpy(MySiftu, signature.keyPoints, (signature.numKeypts)*128*sizeof(unsigned char));
		flags *= pathControl(p->signatureBase + "/" + fileName);
		flags *= t_feat_write_sig_v2(signature, sigPath.c_str());
	}
	catch (Exception e){
		printf("\nFeature Extraction error: ", e.msg.c_str());
		return 0;
	}			
	// BoW Quantization
	try{
		unsigned int *vwi = new unsigned int[signature.numKeypts];
		VT->quantize_multi(vwi,MySiftu,signature.numKeypts);
		for (unsigned int ii = 0; ii < signature.numKeypts; ii++)
			words_str += " " + int2string(vwi[ii]);
		delete[] vwi;
	}
	catch (Exception e){
		printf("\nQuantization error: ", e.msg.c_str());
		return 0;
	}
	delete[] MySiftu;
	
	im.deallocate();
	return 1;
}

int frame::get_json4new_video(Video_info* my_VI, Algorithm_info* my_AI, Path* p)
{
	try{
		json_object_set_new(my_source, "video_ID", json_string(longlongint2string(my_VI->id).c_str()));
		json_object_set_new(my_source, "algorithm_ID", json_string(int2string(my_AI->id).c_str()));
		json_object_set_new(my_source, "algorithm_name", json_string(my_AI->name.c_str()));
		json_object_set_new(my_source, "tree_name", json_string(my_AI->tree_name.c_str()));
		json_object_set_new(my_source, "SIFT_type", json_string(my_AI->feature_type.c_str()));
		json_object_set_new(my_source, "feature_count", json_integer(signature.numKeypts));
		json_object_set_new(my_source, "video_disk_path", json_string((p->videoBase +  "/" + my_VI->fileName).c_str()));
		json_object_set_new(my_source, "frame_disk_path", json_string(filePath.c_str()));
		json_object_set_new(my_source, "signature_disk_path", json_string(sigPath.c_str()));
		json_object_set_new(my_source, "scene", json_integer(scene_n));
		json_object_set_new(my_source, "scene_duration", json_string(int2string(duration).c_str()));
		json_object_set_new(my_source, "frame_n", json_string(int2string(frame_n).c_str()));
		json_object_set_new(my_source, "words_string", json_string(words_str.c_str()));
	}
	catch (Exception e){
		cout << "# ERR: Elasticsearch Exception in " << __FILE__;
		cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
		cout << "# ERR: " << e.what();
		cout << " ES error code: " << e.err;
		cout << ", ES State: " << e.msg << " )" << endl;
	}
	if(!json_is_object(my_source)){
		fprintf(stderr, "error: commits is not an array\n");
		return 0;
	}
	return 1;
}

int frame::ES_commit(ES_params* my_ES)
{
	CURL *curl = curl_easy_init();
	//char *userPWD = "writer:writeme";

	string ES_new_object_url = my_ES->url + "/" + my_ES->index + "/" + my_ES->type ;
	struct curl_slist *headers = NULL; 
	size_t json_flags = 0;
	/* set content type */
	headers = curl_slist_append(headers, "Accept: application/json");
	headers = curl_slist_append(headers, "Content-Type: application/json");
	// Response information.
	int httpCode(0);
	std::unique_ptr<std::string> httpData(new std::string());

	if(curl) {
		try{
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
		catch (Exception e){
			printf("\nElasticsearch error: ", e.msg.c_str());
			return 0;
		}
	}

	if (httpCode == 201){
		json_t *rootRes;
		json_error_t error;
		try
		{
			rootRes = json_loads( httpData->data(), 0, &error );
			if ( !rootRes ){
				fprintf( stderr, "error: on line %d: %s\n", error.line, error.text );
				return 1;
			}
			int boolean;
			const char *str;
			json_unpack(rootRes, "{s:s}", "_id", &str);
			ES_id = str;
			printf("\nES::::Frames of New Video are committed to video_search\\frames. ");
			httpData->clear();
			httpData->shrink_to_fit();
			json_decref(rootRes);
			return 1;
		}
		catch (Exception e){
			printf("\nElasticsearch error: ", e.msg.c_str());
			return 0;
		}
	}
	else{
		printf("\nElasticsearch error: httpCode, %d", httpCode);
		return 0;
	}
}

int frame::MYSQL_commit(MYSQL_params* my_Mysql, Video_info* my_VI, Algorithm_info* my_AI, Path* p  )
{
	long long int frame_id = my_VI->id;
	frame_id *= 100000000;		
	frame_id += scene_n;
	try {
		sql::Driver *driver;
		sql::Connection *con;
		sql::PreparedStatement *pstmt;
		/* Create a connection */
		driver = get_driver_instance();
		con = driver->connect(my_Mysql->hostName.c_str(),my_Mysql->userName.c_str() , my_Mysql->password.c_str());
		/* Connect to the MySQL test database */
		con->setSchema(my_Mysql->catalog.c_str());
		pstmt = con->prepareStatement(("INSERT INTO " + my_Mysql->table_frame + "(video_id, frame_id, algorithm_id, " 
			"algorithm_name, tree_name, SIFT_type, feature_count, video_disk_path, frame_disk_path, " 
			"signature_disk_path, words_string, scene_id, scene_duration, frame_n, ES_id) "
			"VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ? )").c_str());
		pstmt->setInt64(1, my_VI->id);
		pstmt->setInt64(2, frame_id);
		pstmt->setInt(3, my_AI->id);
		pstmt->setString(4, my_AI->name.c_str());
		pstmt->setString(5, my_AI->tree_name.c_str());
		pstmt->setString(6, my_AI->feature_type.c_str());
		pstmt->setInt(7, signature.numKeypts);
		pstmt->setString(8,(p->videoBase +  "/" + my_VI->fileName).c_str());
		pstmt->setString(9,filePath.c_str());
		pstmt->setString(10,sigPath.c_str());
		pstmt->setString(11, words_str.c_str());
		pstmt->setInt(12,scene_n);
		pstmt->setInt(13,duration);
		pstmt->setInt(14,frame_n);
		pstmt->setString(15,ES_id.c_str());
		pstmt->executeUpdate();
		delete pstmt;
		delete con;
		printf("\nMYSQL::::Frames of New Video are committed to video_frame.\n ");
		return 1;
	} catch (sql::SQLException &e) {
		cout << "# ERR: SQLException in " << __FILE__;
		cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
		cout << "# ERR: " << e.what();
		cout << " (MySQL error code: " << e.getErrorCode();
		cout << ", SQLState: " << e.getSQLState() << " )" << endl;
		return 0;
	}
}

int frame::get_json4query()
{
	json_t *query = json_object();
	json_t *match = json_object();
	try	{
		json_object_set_new(match,"words_string", json_string(words_str.c_str()));
		json_object_set_new(query, "match", match);
		json_object_set_new(my_source, "query", query);
		return 1;
	}
	catch (Exception e){
		cout << "# ERR: Elasticsearch Exception in " << __FILE__;
		cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
		cout << "# ERR: " << e.what();
		cout << " ES error code: " << e.err;
		cout << ", ES State: " << e.msg << " )" << endl;
		return 0;
	}		
}

int frame::ES_post_query(ES_params* my_ES, vector<vector<string>>& ES_results)
{
	vector<string> ES_result;
	ES_result.push_back(frameName.c_str());
	CURL *curl = curl_easy_init();
	//char *userPWD = "writer:writeme";
	string ES_new_object_url = my_ES->url + "/" + my_ES->index + "/" + "_search" ;
	struct curl_slist *headers = NULL; 
	size_t json_flags = 0;
	/* set content type */
	headers = curl_slist_append(headers, "Accept: application/json");
	headers = curl_slist_append(headers, "Content-Type: application/json");
	// Response information.
	int httpCode(0);
	std::unique_ptr<std::string> httpData(new std::string());
	if(curl){
		try{
			CURLcode res;
			/* set curl options */
			curl_easy_setopt(curl, CURLOPT_URL, ES_new_object_url.c_str());
			//curl_easy_setopt(curl, CURLOPT_USERPWD, userPWD);
			curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "GET");
			curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);		
			curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_dumps(my_source,json_flags));
			// Hook up data handling function.
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, callback);
			// Hook up data container (will be passed as the last parameter to the callback handling function).  
			// Can be any pointer type, since it will internally be passed as a void pointer.
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, httpData.get());

			res = curl_easy_perform(curl);
			curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
			/* cleanup curl handle */
			curl_easy_cleanup(curl);
			/* free headers */
			curl_slist_free_all(headers);
		}
		catch (Exception e){
			printf("\nElasticsearch error: ", e.msg.c_str());
			return 0;
		}
	}
	if (httpCode == 200){
		json_t *rootRes, *hits, *hits2;
		json_error_t error;
		rootRes = json_loads( httpData->data(), 0, &error );
		if ( !rootRes ){
			fprintf( stderr, "error: on line %d: %s\n", error.line, error.text );
			return 0;
		}
		hits = json_object_get(rootRes, "hits");
		hits2 = json_object_get(hits, "hits");
		if(!json_is_array(hits2)){
			fprintf(stderr, "error: hits2 is not an array\n");
			return 0;
		}
		else{
			for(int i = 0; i < 10 ; i++){
				if (i < json_array_size(hits2)){
					json_t *data, *sourceRes, *video_id_RES;
					data = json_array_get(hits2, i);
					const char *message_text;				 
					if(!json_is_object(data)){
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
				else{
					ES_result.push_back("-1");
				}
			}
			//releases			
			json_object_clear(rootRes);
			json_object_clear(hits);
			json_object_clear(hits2);
			ES_results.push_back(ES_result);
			ES_result.clear();
			return 1;
		}
	}
	else{
		return 0;
	}
}

int initialization(TVoctreeVLFeat* VT, Path* p)
{
	try{
		VT->init_read(p->VT_data.c_str());
		t_feat_init();			// initialization of TFeatureExtractor
		return 1;
	}
	catch (Exception e){
		printf("\nInitialization error: ", e.msg.c_str());
		return 0;
	}	
}

int frame_extract(Video_info* my_VI, Path* p)
{
	(my_VI->newVideo ? (my_VI->path = p->videoBase + "/" + my_VI->fileName) : (my_VI->path = p->videoBulten + "/" +  my_VI->fileName) );

	VideoCapture cap( my_VI->path );		
	if (!cap.isOpened()){			
		cout << "Cannot open the video " << endl;
		return 0;
	}
	double CROP_FRAME_WIDTH, CROP_FRAME_HEIGHT, CROP_FPS;
	CROP_FRAME_WIDTH	= __TY_VIDEO_SEARCH___FRAME_WIDTH;
	CROP_FRAME_HEIGHT 	= __TY_VIDEO_SEARCH___FRAME_HEIGHT;
	CROP_FPS			= __TY_VIDEO_SEARCH___FPS_PROCESS;

	my_VI->width = cap.get(CAP_PROP_FRAME_WIDTH);			///@desc get the width of frames of the video
	my_VI->height = cap.get(CAP_PROP_FRAME_HEIGHT);		///@details the height of frames of the video
	my_VI->fps = cap.get(CAP_PROP_FPS);					///@details get the frame per seconds of the video 
	double dFrameCount = cap.get(CAP_PROP_FRAME_COUNT);		///@details get the total frame count of the video
	my_VI->duration = dFrameCount / my_VI->fps;
	cout <<"\n/////////////////////////////////////////////////////////" << endl;
	cout <<"//////////		NEW  VIDEO		/////////" << endl;
	cout <<"File Name: " << my_VI->fileName << endl;
	cout << "Frame size  : " << my_VI->width << " x " << my_VI->height << endl;
	cout << "Frame count : " << dFrameCount << endl;
	cout << "FPS         : " << my_VI->fps << endl;

	string writePath = p->frameBase + "/" +  my_VI->fileName;		///@param argv[2] take folder path for frames to write
	pathControl(writePath);			////@details control the path for existence, if it is not exist then create folder

	int FrameNumber = 0;
	float fS = my_VI->fps / (CROP_FPS );
	int frameStep = fS + 0.5;			// rounding
	Mat frame;
	int n_frames = 0;

	float scaleW = my_VI->width / CROP_FRAME_WIDTH;
	float scaleH = my_VI->height / CROP_FRAME_HEIGHT;

	while (FrameNumber<(dFrameCount - frameStep-1)) 
	{		
		for(int i = 0 ; i < frameStep; i++){
			bool bSuccess = cap.read(frame); // read a new frame from video
			FrameNumber++;
			if (!bSuccess){
				cout << "Cannot read a frame from video stream" << endl;
				break;
			}			
		}
		string Frame2Write = writePath + "/" + int2string(FrameNumber) + ".jpg";  // create the path for frame to write
		Mat frameMid = frame;
		if(!my_VI->newVideo){
			frameMid = frame(Rect(my_VI->width*0.1,my_VI->height*0.05,my_VI->width*0.8,my_VI->height*0.7)); // selecting (cropping for process) %80 of image
			scaleW = my_VI->width*0.8/CROP_FRAME_WIDTH;
			scaleH = my_VI->height*0.7/CROP_FRAME_HEIGHT;
		}
		/// \brief  A rectangle with sizes  __TY_VIDEO_SEARCH___FRAME_WIDTH x __TY_VIDEO_SEARCH___FRAME_HEIGHT 
		///< is cropped from raw image 
		Mat frameDS;
		if(scaleH >= scaleW){
			resize(frameMid, frameDS, Size(CROP_FRAME_WIDTH*scaleW/scaleH, CROP_FRAME_HEIGHT), 1.0/(scaleH), 1.0/(scaleH),1); // DOWNSAMPLING		
		}
		else{
			resize(frameMid, frameDS, Size(CROP_FRAME_WIDTH, CROP_FRAME_HEIGHT*scaleH/scaleW), 1.0/(scaleW), 1.0/(scaleW),1); // DOWNSAMPLING		
		}
		imwrite(Frame2Write, frameDS);			// A JPG FILE IS BEING SAVED
		n_frames++;
		frameDS.release();
		frameMid.release();
		if(FrameNumber % 200 == 0 )	printf("\rFrame Cropper:::::ProcessRate: %.2f%%",100.0*FrameNumber/dFrameCount);
	}
	//////////////////////////////////////////////////////////////////////////
	my_VI->n_frames = n_frames;
	//////////////////////////////////////////////////////////////////////////
	cap.release();
	frame.release();
	return 1;
}

int retrieve_data_and_print(sql::ResultSet *rs, int type, int colidx, string colname) {

	/* retrieve the row count in the result set */
	cout << "\nRetrieved " << rs -> rowsCount() << " row(s)." << endl;

	cout << "\nCityName" << endl;
	cout << "--------" << endl;

	/* fetch the data : retrieve all the rows in the result set */
	while (rs->next()) {
		if (type == NUMOFFSET) {
			cout << rs -> getString(colidx) << endl;
		} else if (type == COLNAME) {
			cout << rs -> getString(colname) << endl;
		} // if-else
	} // while

	cout << endl;
	return 1;
} 

int MYSQL_commit_video_meta(MYSQL_params* my_Mysql, Video_info* my_VI)
{
	try {
		sql::Driver *driver;
		sql::Connection *con, *con2;
		sql::Statement *stmt;
		sql::PreparedStatement *pstmt;
		sql::ResultSet *res;
		/* Create a connection */
		driver = get_driver_instance();
		con = driver->connect(my_Mysql->hostName.c_str(),my_Mysql->userName.c_str(),my_Mysql->password.c_str());
		//////////////////////////////////////////////////////////////////////////
		con->setSchema(my_Mysql->catalog.c_str());
		pstmt = con->prepareStatement(("INSERT INTO " + my_Mysql->table_video + "( id, source_type, source_url, source_encoding, source_fps, source_height, source_width, duration, n_frames, n_scenes, video_disk_path) VALUES ( ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)").c_str());
		pstmt->setInt(1,NULL);
		pstmt->setString(2,my_VI->source_type.c_str());
		pstmt->setString(3,my_VI->url.c_str());
		pstmt->setString(4,my_VI->encoding.c_str());
		pstmt->setDouble(5,my_VI->fps);
		pstmt->setInt(6,my_VI->width);
		pstmt->setInt(7,my_VI->height);
		pstmt->setInt(8,my_VI->duration);
		pstmt->setInt(9,my_VI->n_frames);
		pstmt->setInt(10,my_VI->n_scenes);
		pstmt->setString(11,my_VI->path.c_str());
		pstmt->execute();
		
		stmt = con->createStatement();
		res = stmt->executeQuery("select MAX( id ) FROM videosearch.video_meta");
		while (res->next()) {
			my_VI->id = res->getInt(1); // getInt(1) returns the first column
		}
		delete res;
		delete stmt;
		delete pstmt;
		delete con;
		printf("\nMYSQL::::New Video is committed to video_meta. ");
		return 1;
		//////////////////////////////////////////////////////////////////////////
	} catch (sql::SQLException &e) {
		cout << "# ERR: SQLException in " << __FILE__;
		cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
		cout << "# ERR: " << e.what();
		cout << " (MySQL error code: " << e.getErrorCode();
		cout << ", SQLState: " << e.getSQLState() << " )" << endl;
		return 0;
	}
}

int MYSQL_update_video_meta(MYSQL_params* my_Mysql, Video_info* my_VI, int scene_n)
{
	try {
		sql::Driver *driver;
		sql::Connection *con;
		sql::Statement *stmt;
		/* Create a connection */
		driver = get_driver_instance();
		con = driver->connect(my_Mysql->hostName.c_str(), my_Mysql->userName.c_str(), my_Mysql->password.c_str());
		stmt = con->createStatement();
		/* Connect to the MySQL test database */
		con->setSchema(my_Mysql->catalog.c_str());
		string stament = "UPDATE " + my_Mysql->table_video + " SET n_scenes=\"" + int2string(scene_n) + "\" WHERE id=" + int2string(my_VI->id);
		stmt->execute(stament.c_str());
		delete stmt;
		delete con;
		printf("\nMYSQL::::Scene Number is updated (video_meta). ");
		return 1;
	} catch (sql::SQLException &e) {
		cout << "# ERR: SQLException in " << __FILE__;
		cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
		cout << "# ERR: " << e.what();
		cout << " (MySQL error code: " << e.getErrorCode();
		cout << ", SQLState: " << e.getSQLState() << " )" << endl;
		return 0;
	}
}

int scene_extract( vector<string> &outputFileNameList, vector<string> & durations, bool AAvideo, Path* p, Video_info* my_VI)
{
	vector<string> fileList;
	vector<int> fileListNumeric;
	vector<MatND> HSV_Hists;

	get_directory_images((p->frameBase + "/" + my_VI->fileName).c_str(),fileList);
	for(int i = 0; i<fileList.size(); i++){
		fileListNumeric.push_back(atoi(fileList[i].substr(0,fileList[i].length()-4).c_str()));	
	}
	sort(fileListNumeric.begin(),fileListNumeric.end());
	unsigned int base_frame_number = 0, limit = fileList.size(), querry = 0;
	for(int i = 0; i < fileList.size(); i++)	
	{
		string filePath = (p->frameBase + "/" + my_VI->fileName + "/" +  int2string(fileListNumeric[i]) + ".jpg");
		HSV_histogram hsv_hist(filePath);				//	create HSV object
		hsv_hist.Calculate_Histogram();					//	calculate HSV histogram
		HSV_Hists.push_back(hsv_hist.getHistogram());	//	write histogram to vector
		Mat Im = imread(filePath,IMREAD_GRAYSCALE);	
		if(i == 0){
			outputFileNameList.push_back(int2string(fileListNumeric[0]) + ".jpg");
		}
		else{
			querry++;
			double correlation_ratio = compareHist( HSV_Hists[0], HSV_Hists[querry], 0 );
			double hellinger_distance = compareHist( HSV_Hists[0], HSV_Hists[querry], 3 );
			//if ( correlation_ratio <= 0.95 && hellinger_distance >= 0.4)		//correlation_ratio <= 0.9 && hellinger_distance >= 0.5 very good
			if ( correlation_ratio <= 0.993 && hellinger_distance >= 0.25)		//correlation_ratio <= 0.9 && hellinger_distance >= 0.5 very good
			{			
				//outputImgVectors.push_back(Im);
				outputFileNameList.push_back(int2string(fileListNumeric[i]) + ".jpg");
				durations.push_back(int2string(HSV_Hists.size()-1));
				HSV_Hists.clear();				
				HSV_Hists.push_back(hsv_hist.getHistogram());
				querry = 0;
			}
		}
		if(	i % 50 == 0 ) printf("\rScene Extraction:::::Process Rate: %.2f%%",100.0*i/limit);		
		Im.release();
	}
	durations.push_back(int2string(HSV_Hists.size())); //duration of last scene
	
	if (AAvideo)
	{
		vector<string> temp_outputFileNameList, temp_duration;
		
		for (int i = 10; i<outputFileNameList.size(); i++){
			
			temp_outputFileNameList.push_back(outputFileNameList[i]);
			temp_duration.push_back(durations[i]);
		}
		outputFileNameList.clear();
		outputFileNameList = temp_outputFileNameList;
		durations.clear();
		durations = temp_duration;
	}
	pathControl(p->sceneBase + "/" + my_VI->fileName);
	for (int i = 0; i < outputFileNameList.size(); i++)
	{
		fileCopy(p->frameBase + "/" + my_VI->fileName + "/" + outputFileNameList[i], p->sceneBase + "/" + my_VI->fileName + "/" + outputFileNameList[i]);
	}
	// RELEASES
	my_VI->n_scenes = outputFileNameList.size();
	fileList.clear();
	fileListNumeric.clear();
	HSV_Hists.clear();
	printf("\nTotal Frame Number: %5d%", limit);
	printf("\nTotal Scene Number: %5d%", outputFileNameList.size());
	printf("\nScene Extraction is Done.");
	//MYSQL_update_video_meta(my_Mysql, my_VI,outputFileNameList.size());
	return 1;
}

