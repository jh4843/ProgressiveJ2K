#pragma once

#include "opj_apps_config.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#include "openjpeg.h"
#include "opj_getopt.h"
#include "convert.h"
#include "index.h"

#include "ImageViewer.h"

#ifdef _WIN32
#include <windows.h>
#define strcasecmp _stricmp
#define strncasecmp _strnicmp
#else
#include <strings.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/times.h>
#endif /* _WIN32 */

#ifdef OPJ_HAVE_LIBLCMS2
#include <lcms2.h>
#endif
#ifdef OPJ_HAVE_LIBLCMS1
#include <lcms.h>
#endif
#include "color.h"

#include "format_defs.h"
#include "opj_string.h"
#include "MyJ2KTypes.h"

#ifdef _WIN32
//#include "windirent.h"
#else
#include <dirent.h>
#endif /* _WIN32 */

/* -------------------------------------------------------------------------- */
#define JP2_RFC3745_MAGIC "\x00\x00\x00\x0c\x6a\x50\x20\x20\x0d\x0a\x87\x0a"
#define JP2_MAGIC "\x0d\x0a\x87\x0a"
/* position 45: "\xff\x52" */
#define J2K_CODESTREAM_MAGIC "\xff\x4f\xff\x51"

class CDecompressJPEG2000
{
public:
	CDecompressJPEG2000();
	~CDecompressJPEG2000();

protected:
	CString m_strInputCompressedFileName;
	CImageViewer* m_pImageView;

public:
	void SetInFileName(CString strInFilename);
	void SetImageViewer(CImageViewer* pProgressiveView);

	BOOL DoDecompress(INT_PTR nStartLayerNum = 0, INT_PTR nEndLayerNum = 0);

protected:
	INT_PTR ParseDecodingOption(opj_decompress_parameters *parameters, J2K_IMAGE_DIR *img_fol);
	BOOL IsValidInputFileType(CString strFileName, opj_decompress_parameters *parameters);
	BOOL SetOutFileType(CString strFileName, opj_decompress_parameters *parameters, J2K_IMAGE_DIR *img_fol);
	void UpdateImageInfo(opj_image_t* pInOpjImage, IMAGE_INFO* pOutImageInfo);

private:
	void set_default_parameters(opj_decompress_parameters* parameters);
	INT_PTR infile_format(const char *fname);
	INT_PTR get_file_format(const char *filename);
	OPJ_FLOAT64 opj_clock(void);

	//INT_PTR get_num_images(char *imgdirpath);
	//INT_PTR load_images(dircnt_t *dirptr, char *imgdirpath);

	// origin is static
	opj_image_t* convert_gray_to_rgb(opj_image_t* original);
	OPJ_BOOL parse_precision(const char* option, opj_decompress_parameters* parameters);
	void destroy_parameters(opj_decompress_parameters* parameters);
	opj_image_t* upsample_image_components(opj_image_t* original);

public:
	static void error_callback(const char *msg, void *client_data);
	static void warning_callback(const char *msg, void *client_data);
	static void info_callback(const char *msg, void *client_data);
	static void quiet_callback(const char *msg, void *client_data);
	
// 	int get_num_images(char *imgdirpath);
// 	int load_images(dircnt_t *dirptr, char *imgdirpath);
// 	
// 	char get_next_file(int imageno, dircnt_t *dirptr, img_fol_t *img_fol, opj_decompress_parameters *parameters);
// 	static int infile_format(const char *fname);
// 	int parse_cmdline_decoder(int argc, char **argv, opj_decompress_parameters *parameters, img_fol_t *img_fol);
// 	int parse_DA_values(char* inArg, unsigned int *DA_x0, unsigned int *DA_y0, unsigned int *DA_x1, unsigned int *DA_y1);
};

