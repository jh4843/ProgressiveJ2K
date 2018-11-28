#pragma once

typedef struct _J2K_COMPRESS_OPTIONAL_PARAM_
{
	CString strInputFileName;
	CString strOutputFileName;
	raw_cparameters_t stRawImgParam;

	_J2K_COMPRESS_OPTIONAL_PARAM_()
	{
		Init();
	}

	void Init()
	{
		strInputFileName.Empty();
		strOutputFileName.Empty();

		/* raw_cp initialization */
		stRawImgParam.rawBitDepth = 0;
		stRawImgParam.rawComp = 0;
		stRawImgParam.rawComps = 0;
		stRawImgParam.rawHeight = 0;
		stRawImgParam.rawSigned = 0;
		stRawImgParam.rawWidth = 0;
	}

}J2K_COMPRESS_OPTIONAL_PARAM;

typedef struct dircnt {
	/** Buffer for holding images read from Directory*/
	char *filename_buf;
	/** Pointer to the buffer*/
	char **filename;
} dircnt_t;
// 
typedef struct _J2K_IMAGE_DIR_ {
	/** The directory path of the folder containing input images*/
	char *imgdirpath;
	/** Output format*/
	char *out_format;
	/** Enable option*/
	char set_imgdir;
	/** Enable Cod Format for output*/
	char set_out_format;
} J2K_IMAGE_DIR;

typedef enum opj_prec_mode {
	OPJ_PREC_MODE_CLIP,
	OPJ_PREC_MODE_SCALE
} opj_precision_mode;

typedef struct opj_prec {
	OPJ_UINT32         prec;
	opj_precision_mode mode;
} opj_precision;

typedef struct opj_decompress_params {
	/** core library parameters */
	opj_dparameters_t core;

	/** input file name */
	char infile[OPJ_PATH_LEN];
	/** output file name */
	char outfile[OPJ_PATH_LEN];
	/** input file format 0: J2K, 1: JP2, 2: JPT */
	int decod_format;
	/** output file format 0: PGX, 1: PxM, 2: BMP */
	int cod_format;
	/** index file name */
	char indexfilename[OPJ_PATH_LEN];

	/** Decoding area left boundary */
	OPJ_UINT32 DA_x0;
	/** Decoding area right boundary */
	OPJ_UINT32 DA_x1;
	/** Decoding area up boundary */
	OPJ_UINT32 DA_y0;
	/** Decoding area bottom boundary */
	OPJ_UINT32 DA_y1;
	/** Verbose mode */
	OPJ_BOOL m_verbose;

	/** tile number ot the decoded tile*/
	OPJ_UINT32 tile_index;
	/** Nb of tile to decode */
	OPJ_UINT32 nb_tile_to_decode;

	opj_precision* precision;
	OPJ_UINT32     nb_precision;

	/* force output colorspace to RGB */
	int force_rgb;
	/* upsample components according to their dx/dy values */
	int upsample;
	/* split output components to different files */
	int split_pnm;
	/** number of threads */
	int num_threads;
	/* Quiet */
	int quiet;
	/** number of components to decode */
	OPJ_UINT32 numcomps;
	/** indices of components to decode */
	OPJ_UINT32* comps_indices;
} opj_decompress_parameters;