#include "frame.h"

using namespace std;


frame::frame(Path p)
{
	filePath = p.frameBase + "/" + fileName + "/" + frameName;
	sigPath = p.signatureBase + "/" + fileName + "/" + frameName + ".sig";
	frame_n = atoi(frameName.substr(0,frameName.length()-4).c_str());
}


frame::~frame(void)
{
	t_feat_release_signature(&signature);
	json_decref(my_source);
}

int frame::initialization(TVoctreeVLFeat VT, Path p)
{
	try
	{
		VT.init_read(p.VT_data.c_str());
		t_feat_init();			// initialization of TFeatureExtractor
		return 1;
	}
	catch (Exception e)
	{
		printf("\nInitialization error: ", e.msg.c_str());
		return 0;
	}	
}

int frame::feature_extract_and_quantize(TVoctreeVLFeat VT, Path p)
{
	int flags = 1;
	unsigned char * MySiftu;
	unsigned int *vwi;
	Mat im;
	try
	{// SIFT Feature Extraction		
		im = imread(filePath, IMREAD_GRAYSCALE);
		t_feat_init_sig(signature);		// Signature initialization
		signature = t_feat_extract((TByte*)im.data, im.cols, im.rows, im.cols, 1, 
			MAX_features_per_image, T_FEAT_EZ_SIFT,T_FEAT_DIST_HELL,T_FEAT_CMPRS_NONE);
		MySiftu = new unsigned char[signature.numKeypts*128];
		memcpy(MySiftu, signature.keyPoints, (signature.numKeypts)*128*sizeof(unsigned char));
		flags *= pathControl(p.signatureBase + "/" + fileName);
		flags *= t_feat_write_sig_v2(signature, sigPath.c_str());
	}
	catch (Exception e)
	{
		printf("\nFeature Extraction error: ", e.msg.c_str());
		return 0;
	}			
	// BoW Quantization
	try
	{
		vwi = new unsigned int[signature.numKeypts]();
		VT.quantize_multi(vwi,MySiftu,signature.numKeypts);
	}
	catch (Exception e)
	{
		printf("\nQuantization error: ", e.msg.c_str());
		return 0;
	}
	for (unsigned int ii = 0; ii < signature.numKeypts; ii++)
		words_str += " " + int2string(vwi[ii]);
	delete[] MySiftu;
	delete[] vwi;
	im.deallocate();
	return 1;
}

int frame::get_json4new_video(Video_info* my_VI, Algorithm_info* my_AI, Path* p)
{
	try
	{
		json_object_set_new(my_source, "video_ID", json_string(int2string(my_VI->id).c_str()));
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
		try
		{
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
		catch (Exception e)
		{
			printf("\nElasticsearch error: ", e.msg.c_str());
			return 0;
		}
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
		ES_id = str;
		printf("\nES::::Frames of New Video are committed to video_search\\frames. ");
		httpData->clear();
		httpData->shrink_to_fit();
		json_decref(rootRes);
		return 1;
	}
	else
	{
		printf("\nElasticsearch error: httpCode, %d", httpCode);
		return 0;
	}
}

int frame::MYSQL_commit(MYSQL_params* my_Mysql, Video_info* my_VI, Algorithm_info* my_AI, Path* p  )
{
	long long int frame_id = my_VI->id;
	frame_id *= 100000000;		
	frame_id += frame_n;
	try {
		sql::Driver *driver;
		sql::Connection *con;
		sql::PreparedStatement *pstmt;
		/* Create a connection */
		driver = get_driver_instance();
		con = driver->connect(my_Mysql->hostName.c_str(),my_Mysql->userName.c_str() , my_Mysql->password.c_str());
		/* Connect to the MySQL test database */
		con->setSchema(my_Mysql->catalog.c_str());
		pstmt = con->prepareStatement("INSERT INTO " + my_Mysql->table_frame + "(video_id, frame_id, algorithm_id, " 
			"algorithm_name, tree_name, SIFT_type, feature_count, video_disk_path, frame_disk_path, " 
			"signature_disk_path, words_string, scene_id, scene_duration, frame_n, ES_id) "
			"VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
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

