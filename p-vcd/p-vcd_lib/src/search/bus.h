/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */

#ifndef BUS_H
#define BUS_H

#include "../pvcd.h"

#define getUIDSearchSegment(ssegment)  ( (ssegment)->sfile->id_seq_segment_first + (ssegment)->segment->num_segment )
#define getSecondsSearchFile(sfile) (sfile)->fdb->lengthSec

struct SearchSegment {
	void *descriptor;
	struct VideoSegment *segment;
	struct SearchFile *sfile;
};
struct SearchFile {
	char *name;
	//sequential id in struct SearchCollection
	int64_t id_seq, id_seq_segment_first;
	FileDB *fdb;
	const struct Segmentation *seg;
	int64_t numSegments;
	struct SearchSegment **ssegments;
	struct SearchCollection* col;
};
struct SearchCollection {
	char *collectionName;
	int64_t numModalities, numFiles, totalSegments;
	DescriptorType *modalities;
	struct SearchFile **sfiles;
	struct SearchSegment **allSegments;
	//more data, see method
	void *internalData;
};
struct SearchProfile {
	char *path_profile;
	struct SearchCollection *colQuery;
	struct SearchCollection *colReference;
	char *id_dist, *id_dist_custom;
};

////////////////////////////////////

//bus_collection.c
struct LoadSearchCollectionParams {
	MyVectorString *dbsNames;
	MyVectorString *descriptorsAlias;
};

struct SearchCollection *loadSearchCollection(
		struct LoadSearchCollectionParams *params);
void releaseCollection(struct SearchCollection* col);

void loadDescriptorsInCollection(struct SearchCollection *col);
void unloadDescriptorsInCollection(struct SearchCollection *col);
void loadDescriptorsInSearchFile(struct SearchFile *sfile);
void unloadDescriptorsInSearchFile(struct SearchFile *sfile);

void reorderSearchFiles(struct SearchCollection* col, char *criterion);
void reorderSearchObjects(struct SearchCollection* col, char *criterion);
void createSortedFileArray(struct SearchCollection* col);
void createAllKeyframesArray(struct SearchCollection* col);
char *toString_SearchFile(struct SearchFile *sfile);
char *toString_SearchSegmentNoFile(struct SearchSegment *sseg);
char *toString_SearchSegmentAndFile(struct SearchSegment *sseg);
struct SearchFile *findSearchFileInCollection(struct SearchCollection *col,
		const char *ssfile_string, bool fail);
struct SearchSegment *findSearchSegmentInFile(struct SearchFile* ssfile,
		const char *sseg_string, bool fail);
struct SearchSegment *findSearchSegmentInCollection(
		struct SearchCollection *col, const char *sseg_string, bool fail);

//bus_profile.c
struct LoadSearchProfileParams {
	bool is_Q_equal_R;
	struct LoadSearchCollectionParams paramR;
	struct LoadSearchCollectionParams paramQ;
	const char *id_dist;
	const char *id_dist_custom;
	const char *path_profile;
};
struct SearchProfile *loadProfileParams(struct LoadSearchProfileParams *params);
struct SearchProfile *loadProfileFromFile(const char *filename_in_profile);
struct SearchProfile *loadProfile(const char *profile_name);
void releaseProfile(struct SearchProfile *profile);
void saveProfileParams(const char *filename,
		struct LoadSearchProfileParams *params);
void loadDescriptorsInProfile(struct SearchProfile *profile);
void unloadDescriptorsInProfile(struct SearchProfile *profile);

MknnDomain *getMknnDomainDescriptorArray(DescriptorType td);

int64_t getNumLocalVectors(struct SearchSegment **segments,
		int64_t num_segments);

MknnDataset *get_dataset_global_descriptors(struct SearchCollection *col,
		struct SearchSegment **segments, int64_t num_segments);
MknnDataset *get_dataset_local_descriptors(struct SearchCollection *col,
		struct SearchSegment **segments, int64_t num_segments);

#if 0
struct LocalVectorsDataset;
MknnDataset *get_dataset_local_descriptors(struct SearchCollection *col,
		struct SearchSegment **segments, int64_t num_segments,
		struct LocalVectorsDataset **out_local_ds);
void get_vector_at_pos(struct LocalVectorsDataset *local_ds, int64_t pos,
		struct SearchSegment **out_ssegNN, int64_t *out_numVecNN);
#endif

MknnDataset *profile_get_reference_globalDescriptors(
		struct SearchProfile *profile);
MknnDataset *profile_get_query_globalDescriptors(struct SearchProfile *profile);

MknnDistance *profile_get_mdistance(struct SearchProfile *profile);

//////////////
struct D_Match {
	struct SearchSegment *ssegmentRef;
	double distance;
	//starting from 1 to k.
	int64_t rank;
};
struct D_Frame {
	struct SearchSegment *ssegmentQuery;
	int64_t numNNs;
	struct D_Match **nns;
	struct D_Query *query;
};
struct D_Query {
	struct SearchFile *sfileQuery;
	int64_t numFrames;
	double searchTime;
	struct D_Frame **frames;
	struct ArchivoFrames *af;
};
struct ArchivoFrames {
	MyVectorObj *allQueries;
	int64_t max_nns;
	double max_dist;
};
struct ArchivoFrames *loadFileSS(const char *filename, int64_t maxNNload,
		double maxDistLoad, struct SearchProfile *profile);
void releaseFileSS(struct ArchivoFrames *af);

//SearchSpace.c
typedef struct SearchSpace SearchSpace;
struct SearchSpace {
	void (*init)(SearchSpace *searchSpace, struct SearchProfile *profile);
	void (*startSearch)(SearchSpace *searchSpace,
			struct SearchSegment *ssegmentQuery, int64_t *out_numReferenceKfs,
			struct SearchSegment ***out_referenceKfs);
	void (*endSearch)(SearchSpace *searchSpace,
			struct SearchSegment *ssegmentQuery, int64_t numReferenceKfs,
			struct SearchSegment **referenceKfs);
	void (*release)(SearchSpace *searchSpace);
	void *state;
};
char *getSearchSpaceHelp();
SearchSpace *newSearchSpace(char *parameters);
SearchSpace *newDefaultsSearchSpace();

//Search.c

struct CorrectoGt {
	struct SearchSegment *kf_correcto;
	double dist_al_query, dist_lb;
	int64_t posicion, posicionEnLBs;
};

struct Dataset {
	int64_t idDescriptor;
	DescriptorType tdCollection, tdDataset;
	bool useLocal;
	void *dataset;
	int64_t numRows, numCols, numBytesCell;
	int64_t *local_idsKf, *local_idsVec;
};

struct NNTxt {
	char *name_nn;
	double distance;
};
struct QueryTxt {
	char *name_file;
	int64_t num_queries;
	double search_time;
	//name for each query (column-1)
	char **name_query;
	//list of struct NNTxt* (column-2,3,...)
	MyVectorObj **nns;
};
MyVectorObj *loadSsFileTxt(const char *filename, int64_t maxNNLoad,
		double maxDistLoad);
void loadSsFileTxt_inlineProcessByQueryFile(const char *filename,
		int64_t maxNNLoad, double maxDistLoad,
		void (*func_processQuery)(struct QueryTxt *query, void *state_func),
		void *state_func);
MyVectorObj *loadVectorsFileTxt(const char *filename, int64_t maxNNLoad);
void releaseSsFile(MyVectorObj *ssFile);

#endif
