#ifndef VLFEAT_VOCTREE
#define VLFEAT_VOCTREE



#include <stdio.h>
#include <vl/generic.h>
#include <vl/hikmeans.h>
#include <vl/kmeans.h>
#include <vl/random.h>
#include "math.h"
#include "helpers2.h"
#include "TVoctree.h"
#include <omp.h>

#ifndef __IPL_H__
   typedef unsigned char uchar;
   typedef unsigned short ushort;
#endif


enum VLFeat_Init_Mode { Small_Tree=1, Middle_Tree, Large_Tree, TEST_Tree};
//const char * VL_FeatTree_Names [] = {"Small_Tree", "Middle_Tree", "Large_Tree", "TEST_Tree"};

int read_SIG_V2s_From_OneFolder(string ReadFolderAddr, unsigned char* MySiftu, vl_size &numTotalDesc, vl_size maxTotalDesc, bool RootSIFT = false);
int read_SIGs_From_OneFolder(string ReadFolderAddr, unsigned char* MySiftu, vl_size &numTotalDesc, vl_size maxTotalDesc, bool RootSIFT = false);
int read_SIGs_From_YearFolder(string ReadFolderAddr,unsigned char* MySiftu, vl_size &numTotalDesc, vl_size maxTotalDesc, bool RootSIFT);
// sen o inheritance'larla ugrasma. cok fazla sey var. ben yapiyorum.
// o zman bunu eski haliyle çalıştırıp hataya bakalım
// aynen
class TVoctreeVLFeat {
public:
	TVoctreeVLFeat(void);
	~TVoctreeVLFeat(void);

	// Class initialization, memory allocation and voctree read from file
	// Param: fname ... (IN) path to voctree
	int init_read(const char *fname, int truncate = 0);
	
	int init(const uchar *data , int DataSize, int TreeMode);
	// Memory deallocation
	void clean(void);


	int ReturnMaxNodeNumber(vl_size depth , vl_size K );
	int ReturnMaxNodeNumber();
	void dump_node(VlHIKMNode* node, int *c, FILE* f);
	void dump_tree(VlHIKMNode* root, int *c, FILE* f);
	void load_node(VlHIKMNode* node, int *c, FILE* f);
	void load_tree(VlHIKMNode** root, int c, FILE* f);
	int write_hikm(const char* FileName);
	int read_hikm(const char* FileName);
	
	// Quantize a 128 dimensional SIFT vector using the voctree. Distances are computed in L2 norm.
	// Param: vwi ... (OUT) visual word for the SIFT vector
	//        sift... (IN) input SIFT vector 128 chars normalized to 0..255
	void quantize(unsigned int *vwi, unsigned char *sift);

	void quantize_multi(unsigned int *vwM, unsigned char *sift, int numDesc); 


	// Computes the number of treenodes (clustercenters) for given split and level
	//int calcnrcenters(int splits, int levels);

	// Returns number of levels for currently loaded voctree
	inline int nrlevels(void) { return m_levels; };
	// Returns number of splits for currently loaded voctree
	inline int nrsplits(void) { return m_splits; };
	// Returns number of centers for currently loaded voctree
	inline int nrcenters(void) { return m_nrcenters; };
	// Returns number of vw's for current loaded voctree
	inline int numVisualWords(void) { return m_visualwords; };
		///
	inline int isvalid(void) {return m_init;};
	int m_stupidCount;
	VlHIKMTree vocabTree;

private:
	int m_init;
	unsigned int m_levels;
	unsigned int m_splits;
	unsigned int m_nrcenters;
	unsigned int m_visualwords;
	////
	
	vl_size m_depth;/**< Depth of the tree */
	vl_size m_K;/**< IKM: K */
	vl_size m_M;/**< IKM: data dimensionality */
	vl_size m_max_niters;/**< IKM: maximum # of iterations */
	int m_method;/**< IKM: method */
	VlHIKMNode m_root;/**< Tree root node */
	int m_verb;/**< Verbosity level */

	//vl_ikmacc_t* centers, vl_ikmacc_t* inter_dist
	VlHIKMNode *create_tree_node(FILE* f);
	vl_ikmacc_t* m_centerTemp, *m_interDist;

};
#endif//VLFEAT_VOCTREE