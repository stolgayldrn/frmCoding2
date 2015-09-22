//#include <zmq.h>
//#include "cmp.h"
#include "helpers2.h"

using namespace cv;
using namespace std;
//#include "cmp.h"

//typedef enum TImageOpMode {T_MODE_SEARCH = 0, T_MODE_INSERT = 1, T_MODE_REMOVE = 2};

//char* read_file_to_string(const char* input_file_name)
//{
//	char *file_contents;
//	long input_file_size;
//	FILE *input_file = fopen(input_file_name, "rb");
//	fseek(input_file, 0, SEEK_END);
//	input_file_size = ftell(input_file);
//	rewind(input_file);
//	file_contents = (char*)calloc(input_file_size+4, sizeof(char));
//	fread(file_contents, sizeof(char), input_file_size, input_file);
//	file_contents[input_file_size] = '\0';
//	fclose(input_file);
//	return file_contents;
//}
//
//
const char *get_filename_ext(const char *filename) 
{
    const char *dot = strrchr(filename, '.');
    if(!dot || dot == filename) return "";
    return dot + 1;
}
//
int is_sig_file(const char *filename)
{
	const char* ext = get_filename_ext(filename);
	size_t lenExt = strlen(ext);

	if (lenExt==3)
	{
		if ( 
			((ext[0]=='s' || ext[0]=='S') && (ext[1]=='i' || ext[1]=='I') && (ext[2]=='g' || ext[2]=='G') ) )
		{
			return 1;
		}
	}	
	return 0;
}

int is_image_file(const char *filename)
{
	const char* ext = get_filename_ext(filename);
	int lenExt = strlen(ext);

	if (lenExt==3)
	{
		if ( 
			((ext[0]=='b' || ext[0]=='B') && (ext[1]=='m' || ext[1]=='M') && (ext[2]=='p' || ext[2]=='P') ) ||
			((ext[0]=='j' || ext[0]=='J') && (ext[1]=='p' || ext[1]=='P') && (ext[2]=='g' || ext[2]=='G') ) ||
			((ext[0]=='p' || ext[0]=='P') && (ext[1]=='n' || ext[1]=='N') && (ext[2]=='g' || ext[2]=='G') )
			)
		{
			return 1;
		}
	}
	else if (lenExt==4)
	{
		if ( 
			((ext[0]=='j' || ext[0]=='J') && (ext[1]=='p' || ext[1]=='P') && (ext[2]=='e' || ext[2]=='E') && (ext[3]=='g' || ext[3]=='G') ) 
			)
		{
			return 1;
		}
	}

	return 0;
}

int file_exist (const char *filename)
{
  struct stat   buffer;   
  return (stat (filename, &buffer) == 0);
}

//
//static bool file_reader(cmp_ctx_t *ctx, void *data, size_t limit){
//memcpy(data, limit, (FILE *)ctx->buf);
//}
//
//static size_t file_writer(cmp_ctx_t *ctx, const void *data, size_t count) 
//{
//return fwrite(data, sizeof(uint8_t), count, (FILE *)ctx->buf);
//}


//
//int decode_query_json(const char* query, trunoptions* runoptions)
//{
//    json_error_t error;
//    json_t* json = json_loadb(query, strlen(query), 0, &error);
//	int numpropertyset = 0;
//
//	runoptions->flags.all = 0;
//
//	// set the unset values to defaults to prevent unexpected behavior
//	runoptions->command = t_mode_unassigned;
//	runoptions->numfeatures = 5000;
//	runoptions->id = "";
//	runoptions->foldername = "";
//	runoptions->filename = "";
//	runoptions->nummatches = 16;
//	runoptions->matchlist.clear();
//
//	/*// decode cmp
//	cmp_init(&cmp, fh, file_reader, file_writer);
//
//	if (!cmp_read_str_size(&cmp, &str_size))
//        error_and_exit(cmp_strerror(&cmp));
//
//    if (str_size > (sizeof(hello) - 1))
//        error_and_exit("packed 'hello' length too long\n");
//
//    if (!read_bytes(hello, str_size, fh))
//        error_and_exit(cmp_strerror(&cmp));*/
//
//	const char *key;
//	json_t *value;
//	void *iter = json_object_iter(json);
//	while(iter)
//	{
//		key = json_object_iter_key(iter);
//		value = json_object_iter_value(iter);
//
//		if (strcmp(key, "filename")==0)
//		{
//			runoptions->filename = json_string_value(value);
//			runoptions->flags.single.filename = 1;		
//		}
//		else if (strcmp(key, "foldername")==0)
//		{
//			runoptions->foldername = json_string_value(value);
//			runoptions->flags.single.foldername = 1;		
//		}
//		else if (strcmp(key, "filelist")==0)
//		{
//			// todo : to be filled
//			runoptions->foldername = json_string_value(value);
//			runoptions->flags.single.foldername = 1;		
//		}
//		else if (strcmp(key, "id")==0)
//		{
//			runoptions->id = json_string_value(value);
//			runoptions->flags.single.id = 1;
//		}
//		else if (strcmp(key, "nummatches")==0)
//		{
//			runoptions->nummatches = (int)json_integer_value(value);
//			runoptions->flags.single.nummatches = 1;
//		}
//		else if (strcmp(key, "imageopmode")==0)
//		{
//			runoptions->command = (timageopmode)json_integer_value(value);
//			runoptions->flags.single.command = 1;
//		}
//		else
//		{
//			printf("unknown option: %s\n",  json_string_value(value));
//			numpropertyset--;
//		}
//
//		//json_decref(value);
//		iter = json_object_iter_next(json, iter);
//		numpropertyset++;
//	}
//
//	json_decref(json);
//	return numpropertyset;
//}
//

//int decode_result_query_json(const char* Query, TRunOptions* runOptions)
//{
//    json_error_t error;
//    json_t* json = json_loads(Query, 0, &error);
//	int numPropertySet = 0;
//
//	runOptions->flags.all = 0;
//
//	// set the unset values to defaults to prevent unexpected behavior
//	runOptions->command = T_MODE_UNASSIGNED;
//	runOptions->numFeatures = 5000;
//	runOptions->id = "";
//	runOptions->folderName = "";
//	runOptions->fileName = "";
//	runOptions->numMatches = 16;
//	runOptions->matchList.clear();
//
//
//	const char *key;
//	json_t *value;
//	void *iter = json_object_iter(json);
//	while(iter)
//	{
//		key = json_object_iter_key(iter);
//		value = json_object_iter_value(iter);
//
//		if (strcmp(key, "FileName")==0)
//		{
//			runOptions->fileName = json_string_value(value);
//			runOptions->flags.single.fileName = 1;		
//		}
//		else if (strcmp(key, "FolderName")==0)
//		{
//			runOptions->folderName = json_string_value(value);
//			runOptions->flags.single.folderName = 1;		
//		}
//		else if (strcmp(key, "Status")==0)
//		{
//			int status = (int)json_integer_value(value);
//			if (status)
//			{
//				printf("Match finding was unsucessfull in previous step\n");
//				return 0;
//			}
//			//runOptions->folderName = json_integer_value(value);
//			//runOptions->flags.single.folderName = 1;		
//		}
//		else if (strcmp(key, "Id")==0)
//		{
//			runOptions->id = json_string_value(value);
//			runOptions->flags.single.id = 1;
//		}
//		else if (strcmp(key, "Matches")==0)
//		{
//			json_t *matches = json_object_get(json, "Matches");
//			if(!json_is_array(matches))
//			{
//				fprintf(stderr, "error: commits is not an array\n");
//				return 1;
//			}
//
//			int numMatches = json_array_size(matches);
//			runOptions->numMatches = numMatches;
//
//			for(int j = 0; j< numMatches; j++)
//			{
//				json_t *match = json_array_get(matches, j);
//				if(!json_is_object(match))
//				{
//					fprintf(stderr, "error: commit %d is not an object\n", j + 1);
//					return 1;
//				}
//
//				json_t* jsLabel = json_object_get(match, "Label");
//				json_t* jsScore = json_object_get(match, "Score");
//				json_t* jsMatch = json_object_get(match, "Match");
//
//				int label = (int)json_integer_value(jsLabel);
//				float score = (float)json_real_value(jsScore);
//				std::string matchStr = json_string_value(jsMatch);
//
//				TMatch newMatch ;
//				newMatch.fileName = matchStr;
//				newMatch.label = label;
//				newMatch.score = score;
//
//				runOptions->matchList.push_back(newMatch);
//			}
//		}
//		else if (strcmp(key, "ImageOpMode")==0)
//		{
//			runOptions->command = (TImageOpMode)json_integer_value(value);
//			runOptions->flags.single.command = 1;
//		}
//		else
//		{
//			printf("Unknown option: %s\n",  json_string_value(value));
//			numPropertySet--;
//		}
//
//		//json_decref(value);
//		iter = json_object_iter_next(json, iter);
//		numPropertySet++;
//	}
//
//	json_decref(json);
//	return numPropertySet;
//}
//
//char* get_json_empty(const char* id, const char* name)
//{
//	json_t *jobj = json_object();
//	json_object_set_new(jobj, "Id", json_string(id));
//	char err[4096];
//	sprintf(err, "Cannot read file: %s", name);
//	json_object_set_new(jobj, "Status", json_integer(-1));
//	json_object_set_new(jobj, "Message", json_string(err));
//	char* result = json_dumps(jobj, 0);
//	printf("%s\r\n", result);
//	json_decref(jobj);
//	return result;
//}
//
//char* get_result_json(const int numMaxMatches, const char* id, const char* name, int* labels, float* scores, vector<string> frameMap)
//{
//	json_t *jobj = json_object(), *jmatches = json_array();
//	//json_t *jscores = json_array(), *jlabels = json_array();
//	json_object_set_new(jobj, "Status", json_integer(0));
//	json_object_set_new(jobj, "Id", json_string(id));
//	json_object_set_new(jobj, "FileName", json_string(name));
//
//	for (int k=0;k<numMaxMatches; k++)
//	{
//		if (labels[k]>=0)
//		{
//			json_t *jmatch = json_object();
//
//			json_object_set_new(jmatch, "Label", json_integer(labels[k]));
//			json_object_set_new(jmatch, "Score", json_real(scores[k]));
//			json_object_set_new(jmatch, "Match", json_string(frameMap[labels[k]].c_str()));
//
//			json_array_append_new(jmatches, jmatch);
//		}
//	}
//
//	json_object_set_new(jobj, "Matches", jmatches);
//
//	char* result = json_dumps(jobj, JSON_PRESERVE_ORDER);
////	printf("%s\r\n", result);
//
//	json_decref(jobj);
//
//	return result;
//}

//char* get_run_result_json(TRunOptions& runResult)
//{
//	json_t *jobj = json_object(), *jmatches = json_array();
//	//json_t *jscores = json_array(), *jlabels = json_array();
//	json_object_set_new(jobj, "Status", json_integer(0));
//	json_object_set_new(jobj, "Id", json_string(runResult.id.c_str()));
//	json_object_set_new(jobj, "FileName", json_string(runResult.fileName.c_str()));
//	
//	int numMaxMatches = runResult.matchList.size();
//
//	for (int k=0;k<numMaxMatches; k++)
//	{
//		TMatch tMatch = runResult.matchList[k];
//		if (tMatch.label>=0)
//		{
//			json_t *jmatch = json_object();
//
//			json_object_set_new(jmatch, "Label", json_integer(tMatch.label));
//			json_object_set_new(jmatch, "Score", json_real(tMatch.score));
//			json_object_set_new(jmatch, "Match", json_string(tMatch.fileName.c_str()));
//
//			json_array_append_new(jmatches, jmatch);
//		}
//	}
//
//	json_object_set_new(jobj, "Matches", jmatches);
//
//	char* result = json_dumps(jobj, JSON_PRESERVE_ORDER);
////	printf("%s\r\n", result);
//
//	json_decref(jobj);
//
//	return result;
//}
//
//
//char* get_sorted_result_json(const int numMaxMatches, const char* id, const char* name, int* labels, float* scores)
//{
//	json_t *jobj = json_object(), *jmatches = json_array();
//	//json_t *jscores = json_array(), *jlabels = json_array();
//	json_object_set_new(jobj, "Status", json_integer(0));
//	json_object_set_new(jobj, "Id", json_string(id));
//	json_object_set_new(jobj, "FileName", json_string(name));
//
//	for (int k=0;k<numMaxMatches; k++)
//	{
//		if (labels[k]>=0)
//		{
//			json_t *jmatch = json_object();
//
//			json_object_set_new(jmatch, "Label", json_integer(labels[k]));
//			json_object_set_new(jmatch, "Score", json_real(scores[k]));
//			//json_object_set_new(jmatch, "Match", json_string(frameMap[labels[k]].c_str()));
//
//			json_array_append_new(jmatches, jmatch);
//		}
//	}
//
//	json_object_set_new(jobj, "Matches", jmatches);
//
//	char* result = json_dumps(jobj, JSON_PRESERVE_ORDER);
////	printf("%s\r\n", result);
//
//	json_decref(jobj);
//
//	return result;
//}

//
//char* get_result_json_list(const int numMaxMatches, const char* id, const char* name, int* labels, float* scores, vector<string> frameMap)
//{
//
//}

//// "tcp://192.168.1.102:5555"
//void connect_socket(const char* ip, void** context, void** requester)
//{
//	*context = zmq_ctx_new ();
//
//	//  Socket to talk to server
//    printf ("Listening...\n");
//    *requester = zmq_socket (*context, ZMQ_REP);
//	zmq_bind(*requester, ip);
//}
//
//int recv_socket(void* requester, char** text)
//{
//	zmq_msg_t reply;
//	zmq_msg_init (&reply);
//	zmq_msg_recv (&reply, requester, 0);
//	char* result = (char*)zmq_msg_data (&reply);
//
//	size_t len = zmq_msg_size(&reply);
//	(*text) = new char[len+4];
//	memcpy(*text, result, len*sizeof(char));
//	(*text)[len]='\0';
//
//	//printf ("Received World %s\n", text);
//	//printf ("Received World %s\n", js);
//
//	zmq_msg_close (&reply);
//
//	return len;
//}
//
//void send_socket(void* requester, const char* text)
//{
//	zmq_msg_t request;
//	zmq_msg_init_size (&request, strlen(text));
//	memcpy (zmq_msg_data (&request), text, strlen(text));
//
//	zmq_msg_send (&request, requester, 0);
//	zmq_msg_close (&request);
//}
//
//void close_socket(void** requester, void** context)
//{
//	zmq_close (*requester);
//    zmq_ctx_destroy (*context);
//
//	*requester = 0;
//	*context = 0;
//}

/////////////////////////////////////// NEW FUNCTIONS GO BEYOND THIS POINT //////////////////////////////


int get_directory_signatures(const char* Folder, std::vector<string> &fileList)
{
	int i=0;
	DIR* dir;	
	struct dirent *ent;
	int numFiles=0;
	dir = opendir(Folder);

	fileList.clear(); //
	if (dir) 
	{
		/* print all the files and directories within directory */
		while ((ent = readdir (dir))) 
		{
			struct dirent *temp=(struct dirent*)malloc(sizeof(struct dirent));

			//memcpy(temp->d_name,ent->d_name,sizeof(TChar)*ent->d_namlen);
			strcpy(temp->d_name,ent->d_name);
			temp->d_namlen=ent->d_namlen;
			temp->d_type=ent->d_type;

			switch (ent->d_type) 
			{
			case DT_REG:
				if (is_sig_file(temp->d_name))
				{
					fileList.push_back(string( temp->d_name ));
					//t_list_push_list(&fileList,temp);
					numFiles++;
					break;
				}
			default:
				break;
			}
		}
		closedir (dir);
	}
	else
	{
		printf("Couldn't open folder!\n");
		return 0;
	}

	return i;
}

int get_directory_images(const char* Folder, std::vector<string> &fileList)
{
	int i=0;
	DIR* dir;	
	struct dirent *ent;
	int numFiles=0;
	dir = opendir(Folder);

	fileList.clear();
	if (dir) 
	{
		/* print all the files and directories within directory */
		while ((ent = readdir (dir))) 
		{
			struct dirent *temp=(struct dirent*)malloc(sizeof(struct dirent));

			//memcpy(temp->d_name,ent->d_name,sizeof(TChar)*ent->d_namlen);
			strcpy(temp->d_name,ent->d_name);
			temp->d_namlen=ent->d_namlen;
			temp->d_type=ent->d_type;

			switch (ent->d_type) 
			{
			case DT_REG:
				if (is_image_file(temp->d_name))
				{
					fileList.push_back(string( temp->d_name ));
					//t_list_push_list(&fileList,temp);
					numFiles++;
					break;
				}
			default:
				break;
			}
		}
		closedir (dir);
	}
	else
	{
		printf("Couldn't open folder!\n");
		return 0;
	}

	return i;
}

int get_folder_list(const char* Folder, std::vector<string> &fileList)
{
	int i=0;
	DIR* dir;	
	struct dirent *ent;
	int numFiles=0;
	dir = opendir(Folder);

	fileList.clear();
	if (dir) 
	{
		/* print all the files and directories within directory */
		while ((ent = readdir (dir))) 
		{
			struct dirent *temp=(struct dirent*)malloc(sizeof(struct dirent));

			//memcpy(temp->d_name,ent->d_name,sizeof(TChar)*ent->d_namlen);
			strcpy(temp->d_name,ent->d_name);
			temp->d_namlen=ent->d_namlen;
			temp->d_type=ent->d_type;
			

			switch (ent->d_type) 
			{
			case DT_DIR:
				
				if(strcmp(ent->d_name,".") && strcmp(ent->d_name , ".."))
				{
					fileList.push_back(string( ent->d_name ));
					//t_list_push_list(&fileList,temp);
					numFiles++;
				}
				
			default:
				break;
			}
		}
		closedir (dir);
	}
	else
	{
		printf("Couldn't open folder!\n");
		return 0;
	}

	return i;
}

int read_sig(const char* FileName, unsigned int *numdesc, unsigned char** siftDescByte, float** xCoords, float** yCoords, float** orientations, float** scales)
{
	FILE *f=fopen(FileName, "rb");

	if (f)
	{
		unsigned long long hash = 0;
		fread(numdesc, sizeof(unsigned int), 1, f);
		unsigned char* siftu = new unsigned char[(*numdesc+4)*128];
		fread(siftu, sizeof(unsigned char), (*numdesc)*128, f);
		fread(&hash, sizeof(unsigned long long), 1, f);

		*xCoords = 0;
		*yCoords = 0;
		*orientations = 0;
		*scales = 0;
		if (!feof(f))
		{
			(*xCoords) = new float[(*numdesc+4)];
			(*yCoords) = new float[(*numdesc+4)];
			(*orientations) = new float[(*numdesc+4)];
			(*scales) = new float[(*numdesc+4)];
			// read the geometry information
			fread((*xCoords), sizeof(float), *numdesc, f);
			fread((*yCoords), sizeof(float), *numdesc, f);
			fread((*orientations), sizeof(float), *numdesc, f);
			fread((*scales), sizeof(float), *numdesc, f);
		}

		fclose(f);

		*siftDescByte = siftu;
	}
	else
	{
		printf("Cannot read file: %s\n", FileName);
		return -1;
	}

	return 0;
}

std::string int2string(int num)
{
	stringstream ss;
	ss << num;
	string str = ss.str();
	return str;
}

Mat makeCanvas(std::vector<Mat>& vecMat, int windowHeight, int nRows) {
	int N = vecMat.size();
	nRows  = nRows > N ? N : nRows; 
	int edgeThickness = 10;
	int imagesPerRow = ceil(double(N) / nRows);
	int resizeHeight = floor(2.0 * ((floor(double(windowHeight - edgeThickness) / nRows)) / 2.0)) - edgeThickness;
	int maxRowLength = 0;

	std::vector<int> resizeWidth;
	for (int i = 0; i < N;) {
		int thisRowLen = 0;
		for (int k = 0; k < imagesPerRow; k++) {
			double aspectRatio = double(vecMat[i].cols) / vecMat[i].rows;
			int temp = int( ceil(resizeHeight * aspectRatio));
			resizeWidth.push_back(temp);
			thisRowLen += temp;
			if (++i == N) break;
		}
		if ((thisRowLen + edgeThickness * (imagesPerRow + 1)) > maxRowLength) {
			maxRowLength = thisRowLen + edgeThickness * (imagesPerRow + 1);
		}
	}
	int windowWidth = maxRowLength;
	Mat canvasImage(windowHeight, windowWidth, CV_8UC3, Scalar(0, 0, 0));

	for (int k = 0, i = 0; i < nRows; i++) {
		int y = i * resizeHeight + (i + 1) * edgeThickness;
		int x_end = edgeThickness;
		for (int j = 0; j < imagesPerRow && k < N; k++, j++) {
			int x = x_end;
			Rect roi(x, y, resizeWidth[k], resizeHeight);
			Size s = canvasImage(roi).size();
			// change the number of channels to three
			Mat target_ROI(s, CV_8UC3);
			if (vecMat[k].channels() != canvasImage.channels()) {
				if (vecMat[k].channels() == 1) {
					cvtColor(vecMat[k], target_ROI,COLOR_GRAY2BGR);
				}
			}
			resize(target_ROI, target_ROI, s);
			if (target_ROI.type() != canvasImage.type()) {
				target_ROI.convertTo(target_ROI, canvasImage.type());
			}
			target_ROI.copyTo(canvasImage(roi));
			x_end += resizeWidth[k] + edgeThickness;
		}
	}
	return canvasImage;
}

int dirExists(const char *path)
{
	if( _access( path, 0 ) == 0 ){

		struct stat status;
		stat( path, &status );

		return (status.st_mode & S_IFDIR) != 0;
	}
	return false;
}
void pathControl(string Path)
{
	if(!dirExists(Path.c_str()))
	{
		wstring stemp = wstring(Path.begin(), Path.end());
		LPCWSTR szDirPath = stemp.c_str();

		// Create a new directory.
		if(!CreateDirectoryW(szDirPath, NULL))
			printf("\nCouldn't create %S directory.\n", szDirPath);
		else
			printf("\n%S directory successfully created.\n", szDirPath);

	} 
}

void fileCopy(string sourePath, string destPath)
{
	ifstream source(sourePath, ios::binary);
	ofstream dest(destPath, ios::binary);

	istreambuf_iterator<char> begin_source(source);
	istreambuf_iterator<char> end_source;
	ostreambuf_iterator<char> begin_dest(dest); 
	copy(begin_source, end_source, begin_dest);

	source.close();
	dest.close();
}

//////////////////////////////////////////////////////////////////////////
// JSON
std::size_t callback(
	const char* in,
	std::size_t size,
	std::size_t num,
	std::string* out)
{
	const std::size_t totalBytes(size * num);
	out->append(in, totalBytes);
	return totalBytes;
}
//////////////////////////////////////////////////////////////////////////
//int cv_get_descs(const char* FileName, cv::Mat &descriptorMat, std::vector<cv::Point2f> &Coords, std::vector<float> &Orientations, std::vector<float> &Scales)
//{
//	int status = -1;
//	unsigned int numdesc = 0;
//	unsigned char* siftDescByte = 0;
//	float* xCoords = 0;
//	float* yCoords = 0;
//	float* orientations = 0;
//	float* scales = 0;
//	
//	if ( read_sig(FileName, &numdesc, &siftDescByte, &xCoords, &yCoords, &orientations, &scales) >=0 )
//	{
//		descriptorMat = cv::Mat(numdesc, 128, CV_32F);
//		for (unsigned int i=0; i<numdesc; i++)
//		{
//			const unsigned char* srcRow = &siftDescByte[i*128];
//			float* descRowCV = (float*)(&descriptorMat.data[descriptorMat.step*i]);
//			for (int j=0; j<128; j++)
//			{
//				descRowCV[j] = (float)srcRow[j];
//			}
//		}
//
//		delete[] siftDescByte;
//		siftDescByte=0;
//
//		if (xCoords)
//		{
//			Coords.reserve(numdesc);
//			Orientations.reserve(numdesc);
//			Scales.reserve(numdesc);
//
//			for (unsigned int i=0; i<numdesc; i++)
//			{
//				Coords.push_back(cv::Point2f(xCoords[i], yCoords[i]));
//				Orientations.push_back(orientations[i]);
//				Scales.push_back(scales[i]);
//			}
//			status = 1;
//		}
//
//		delete[] xCoords; xCoords=0;
//		delete[] yCoords; yCoords=0;
//		delete[] orientations; orientations=0;
//		delete[] scales; scales=0;
//
//		status = 0;
//	}
//
//	return status;
//}


