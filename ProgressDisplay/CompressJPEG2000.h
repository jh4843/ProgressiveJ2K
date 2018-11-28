#pragma once

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

// #ifdef _WIN32
// #include "windirent.h"
// #else
// #include <dirent.h>
// #endif /* _WIN32 */

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

#include "opj_apps_config.h"
#include "openjpeg.h"
#include "opj_getopt.h"
#include "convert.h"
#include "index.h"

#include "format_defs.h"
#include "opj_string.h"
#include "opj_includes.h"

#include "MyJ2KTypes.h"

class CCompressJPEG2000
{
public:
	CCompressJPEG2000();
	~CCompressJPEG2000();

protected:
	CString m_strInputImageFile;
	opj_cparameters_t m_J2kCompParam;
	J2K_IMAGE_DIR m_J2kImgDir;

public:
	void Init();
//	BOOL DoCompress(MY_J2K_COMPRESS_OPTIONAL_PARAM stJ2kCompParam);

private:
	void SetDefaultParameter();
//	BOOL ParseCompParameter(MY_J2K_COMPRESS_OPTIONAL_PARAM stJ2kCompParam, raw_cparameters_t *stRawCp);
	INT_PTR GetFileFormat(char *filename);

};

