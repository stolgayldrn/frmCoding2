#ifndef HELPERS2_H
#define HELPERS2_H

#include <vector>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <sstream>
#include <iostream>
#include <fstream>
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include <Windows.h>
#include <io.h>					// For access().
#include <sys/types.h>			// For stat().
#include <sys/stat.h>			// For stat().
#include "jansson.h"			// JSON
#include "dirent.h"
//#include <opencv2\core.hpp>
//#include "t_common_defs.h"
#define __TY_VIDEO_SEARCH___FPS_PROCESS 4
#define __TY_VIDEO_SEARCH___FRAME_WIDTH 960
#define __TY_VIDEO_SEARCH___FRAME_HEIGHT 720


using namespace std;

// directory
//char* read_file_to_string(const char* input_file_name);
const char *get_filename_ext(const char *filename) ;
int is_sig_file(const char *filename);
int is_image_file(const char *filename);
int file_exist ( const char *filename);
//int get_directory_signatures(const char* Folder, string** FileList);
int get_directory_signatures(const char* Folder, std::vector<string> &fileList);
int get_directory_images(const char* Folder, std::vector<string> &fileList);
int get_folder_list(const char* Folder, std::vector<string> &fileList);
// return negative if it cannot read it. 0 on success
int read_sig(const char* FileName, unsigned int *numdesc, unsigned char** siftDescByte, float** xCoords, float** yCoords, float** orientations, float** scales);
//int cv_get_descs(const char* FileName, cv::Mat &descriptorMat, std::vector<cv::Point2f> &Coords, std::vector<float> &Orientations, std::vector<float> &Scales);
string int2string(int num);
cv::Mat makeCanvas(std::vector<cv::Mat>& vecMat, int windowHeight, int nRows);
int dirExists(const char *path);
void pathControl(string Path);
void fileCopy(string sourePath, string destPath);
std::size_t callback( const char* in, std::size_t size, std::size_t num, std::string* out);
// json
//int decode_query_json(const char* Query, char* filePath, char* id, int* NumMatches);
//int decode_query_json(const char* Query, TRunOptions* runOptions);
//int decode_result_query_json(const char* Query, TRunOptions* runOptions);
//char* get_result_json(const int numMaxMatches, const char* id, const char* name, int* labels, float* scores, vector<string> frameMap);
//char* get_json_empty(const char* id, const char* name);
//char* get_sorted_result_json(const int numMaxMatches, const char* id, const char* name, int* labels, float* scores);
//char* get_run_result_json(TRunOptions &runResult);

//// socket
//void connect_socket(const char* ip, void** context, void** requester);
//void send_socket(void* requester, const char* text);
//int recv_socket(void* requester, char** text);
//void close_socket(void** requester, void** context);
//
//
//// Act like matlab's [Y,I] = SORT(X)
//// Input:
////   unsorted  unsorted vector
//// Output:
////   sorted     sorted vector, allowed to be same as unsorted
////   index_map  an index map such that sorted[i] = unsorted[index_map[i]]
//template <class T>
//void indexed_sort(
//				  std::vector<T> &unsorted,
//				  std::vector<T> &sorted,
//				  std::vector<int> &index_map);
//
//// Act like matlab's Y = X[I]
//// where I contains a vector of indices so that after,
//// Y[j] = X[I[j]] for index j
//// this implies that Y.size() == I.size()
//// X and Y are allowed to be the same reference
//template< class T >
//void reorder(
//			 std::vector<T> & unordered, 
//			 std::vector<size_t> const & index_map, 
//			 std::vector<T> & ordered);
//
//// Comparison struct used by sort
//// http://bytes.com/topic/c/answers/132045-sort-get-index
//template<class T> struct index_cmp 
//{
//	index_cmp(T arr) : arr(arr) {}
//	bool operator()(const size_t a, const size_t b) const
//	{ 
//		return arr[a] < arr[b];
//	}
//	T arr;
//};
//
//
//// Comparison struct used by sort
//// http://bytes.com/topic/c/answers/132045-sort-get-index
//template<class T> struct index_cmp_descend 
//{
//	index_cmp_descend(T arr) : arr(arr) {}
//	bool operator()(const size_t a, const size_t b) const
//	{ 
//		return arr[a] > arr[b];
//	}
//	T arr;
//};
//
//
//// sortType = 0 : Ascend
//// sortType = 1 : Descend
//template <class T>
//void indexed_sort( std::vector<T> & unsorted, std::vector<T> & sorted, std::vector<int> & index_map, int sortType=0)
//{
//	// Original unsorted index map
//	index_map.resize(unsorted.size());
//	for(size_t i=0;i<unsorted.size();i++)
//	{
//		index_map[i] = i;
//	}
//
//	if (sortType==0)
//	{
//	// Sort the index map, using unsorted for comparison
//	sort(
//		index_map.begin(), 
//		index_map.end(), 
//		index_cmp<std::vector<T>& >(unsorted));
//	}
//	else
//	{
//		sort(
//		index_map.begin(), 
//		index_map.end(), 
//		index_cmp_descend<std::vector<T>& >(unsorted));
//	}
//
//	sorted.resize(unsorted.size());
//	reorder(unsorted,index_map,sorted);
//}
//
//// This implementation is O(n), but also uses O(n) extra memory
//template< class T >
//void reorder( std::vector<T> & unordered,  std::vector<int> const & index_map,  std::vector<T> & ordered)
//{
//	// copy for the reorder according to index_map, because unsorted may also be
//	// sorted
//	std::vector<T> copy = unordered;
//	ordered.resize(index_map.size());
//	for(unsigned int i = 0; i<index_map.size();i++)
//	{
//		ordered[i] = copy[index_map[i]];
//	}
//}

#endif