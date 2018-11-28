#include "stdafx.h"
#include "CompressJPEG2000.h"


CCompressJPEG2000::CCompressJPEG2000()
{

}


CCompressJPEG2000::~CCompressJPEG2000()
{
}

void CCompressJPEG2000::Init()
{
	SetDefaultParameter();

	memset(&m_J2kImgDir, 0, sizeof(J2K_IMAGE_DIR));
}

// BOOL CCompressJPEG2000::DoCompress(J2K_COMPRESS_OPTIONAL_PARAM stJ2kCompParam)
// {
// 	if (m_strInputImageFile.IsEmpty())
// 		return FALSE;
// 
// 	opj_cparameters_t parameters;   /* compression parameters */
// 
// 	opj_stream_t *l_stream = 00;
// 	opj_codec_t* l_codec = 00;
// 	opj_image_t *image = NULL;
// 	raw_cparameters_t raw_cp;
// 	OPJ_SIZE_T num_compressed_files = 0;
// 
// 	char indexfilename[OPJ_PATH_LEN];   /* index file name */
// 
// 	unsigned int i, num_images, imageno;
// 	J2K_IMAGE_DIR img_fol;
// 	dircnt_t *dirptr = NULL;
// 
// 	OPJ_BOOL bSuccess;
// 	OPJ_BOOL bUseTiles = OPJ_FALSE; /* OPJ_TRUE */
// 	OPJ_UINT32 l_nb_tiles = 4;
// 
// 	/* Initialize indexfilename and img_fol */
// 	*indexfilename = 0;
// 	memset(&img_fol, 0, sizeof(J2K_IMAGE_DIR));
// 
// 	/* raw_cp initialization */
// 	raw_cp.rawBitDepth = 0;
// 	raw_cp.rawComp = 0;
// 	raw_cp.rawComps = 0;
// 	raw_cp.rawHeight = 0;
// 	raw_cp.rawSigned = 0;
// 	raw_cp.rawWidth = 0;
// 
// 	/* parse input and get user encoding parameters */
// 	parameters.tcp_mct = (char)255; /* This will be set later according to the input image or the provided option */
// 
// 	ParseCompParameter(&raw_cp);
// 
// 	/* parse input and get user encoding parameters */
// // 	if (parse_cmdline_decoder(argc, argv, &parameters, &img_fol) == 1) {
// // 		failed = 1;
// // 		goto fin;
// // 	}
// 
// 	return 0;
// }

void CCompressJPEG2000::SetDefaultParameter()
{
	memset(&m_J2kCompParam, 0, sizeof(opj_cparameters_t));
	opj_set_default_encoder_parameters(&m_J2kCompParam);
}

// BOOL CCompressJPEG2000::ParseCompParameter(J2K_COMPRESS_OPTIONAL_PARAM stJ2kCompParam, raw_cparameters_t *stRawCp)
// {
// 	m_J2kImgDir.set_out_format = 0;
// 	stRawCp->rawWidth = 0;
// 
// 	if (!stJ2kCompParam.strInputFileName.IsEmpty())
// 	{
// 		char* strInfile = LPSTR(LPCTSTR(stJ2kCompParam.strInputFileName));
// 		m_J2kCompParam.decod_format = GetFileFormat(strInfile);
// 
// 		switch (m_J2kCompParam.decod_format) {
// 		case PGX_DFMT:
// 		case PXM_DFMT:
// 		case BMP_DFMT:
// 		case TIF_DFMT:
// 		case RAW_DFMT:
// 		case RAWL_DFMT:
// 		case TGA_DFMT:
// 		case PNG_DFMT:
// 			break;
// 		default:
// 			AfxMessageBox(_T("Input file's format is not unavailable"));
// 			return FALSE;
// 		}
// 
// 		if (opj_strcpy_s(m_J2kCompParam.infile, sizeof(m_J2kCompParam.infile), strInfile) != 0) {
// 			AfxMessageBox(_T("Input file's name is abnormal"));
// 			return FALSE;
// 		}
// 	}
// 
// 	if (!stJ2kCompParam.strOutputFileName.IsEmpty())
// 	{
// 		char *strOutFile = LPSTR(LPCTSTR(stJ2kCompParam.strOutputFileName));
// 		m_J2kCompParam.cod_format = GetFileFormat(strOutFile);
// 		switch (m_J2kCompParam.cod_format) {
// 		case J2K_CFMT:
// 		case JP2_CFMT:
// 			break;
// 		default:
// 			AfxMessageBox(_T("Output file's format is not unavailable"));
// 			return FALSE;
// 		}
// 		if (opj_strcpy_s(m_J2kCompParam.outfile, sizeof(m_J2kCompParam.outfile), strOutFile) != 0) {
// 			return FALSE;
// 		}
// 	}
// 
// 	return TRUE;
// }

INT_PTR CCompressJPEG2000::GetFileFormat(char * filename)
{
	unsigned int i;
	static const char *extension[] = {
		"pgx", "pnm", "pgm", "ppm", "pbm", "pam", "bmp", "tif", "raw", "rawl", "tga", "png", "j2k", "jp2", "j2c", "jpc"
	};
	static const int format[] = {
		PGX_DFMT, PXM_DFMT, PXM_DFMT, PXM_DFMT, PXM_DFMT, PXM_DFMT, BMP_DFMT, TIF_DFMT, RAW_DFMT, RAWL_DFMT, TGA_DFMT, PNG_DFMT, J2K_CFMT, JP2_CFMT, J2K_CFMT, J2K_CFMT
	};
	char * ext = strrchr(filename, '.');
	if (ext == NULL) {
		return -1;
	}
	ext++;
	for (i = 0; i < sizeof(format) / sizeof(*format); i++) {
		if (strcasecmp(ext, extension[i]) == 0) {
			return format[i];
		}
	}
	return -1;
}

