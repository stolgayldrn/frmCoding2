
#ifndef T_VOCTREE
#define T_VOCTREE

#include <math.h>

class TVoctree{
public:
	TVoctree(void);
	virtual ~TVoctree(void);
	
	// Class initialization, memory allocation and voctree read from file
	// Param: fname ... (IN) path to voctree
	virtual int init(const char *fname, int truncate = 0) = 0;

	// Memory deallocation
	virtual void clean(void) = 0;

	// Quantize a 128 dimensional SIFT vector using the voctree. Distances are computed in L2 norm.
	// Param: vwi ... (OUT) visual word for the SIFT vector
	//        sift... (IN) input SIFT vector 128 chars normalized to 0..255
	virtual void quantize(unsigned int *vwi, float *sift) = 0;
	virtual void quantize(unsigned int *vwi, unsigned char *sift) = 0;

	// Quantize multiple 128 dimensional SIFT vectors using the voctree. Distances are computed in L2 norm.
	// Param: vwi ... (OUT) visual word for the SIFT vector
	//        sift... (IN) input SIFT vector 128 chars normalized to 0..255
	virtual void quantize_multi(unsigned int *vwM, float *sift, int numDesc) = 0;
	virtual void quantize_multi(unsigned int *vwM, unsigned char *sift, int numDesc) = 0;

	// Computes the number of treenodes (clustercenters) for given split and level	
	inline int calcnrcenters(int splits, int levels) {
		return (int)(pow((double)splits,(double)levels+1)-1)/(splits-1)-1;
	}

	// Returns number of levels for currently loaded voctree
	inline int nrlevels(void) { return m_levels; };
	// Returns number of splits for currently loaded voctree
	inline int nrsplits(void) { return m_splits; };
	// Returns number of centers for currently loaded voctree
	inline int nrcenters(void) { return m_nrcenters; };
	// Returns number of vw's for current loaded voctree
	inline int numVisualWords(void) { return m_visualwords; };

	///
	inline int isvalid(void) {return m_init;}
protected:
	int m_init;
	unsigned char *m_voc;
	unsigned char *m_cellinfo;
	unsigned int *m_vwtable;
	unsigned int m_levels;
	unsigned int m_splits;
	unsigned int m_nrcenters;
	unsigned int m_visualwords;
	int   m_half; 
	__int64 m_centerpos;
	double m_mindist;
	int m_minindex;
	double m_dist;
	int m_previndex;
	int m_offset;

};

#endif //T_VOCTREE