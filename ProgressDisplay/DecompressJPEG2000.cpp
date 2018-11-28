#include "stdafx.h"
#include "DecompressJPEG2000.h"

#include "ImageInfo.h"


CDecompressJPEG2000::CDecompressJPEG2000()
{
	m_strInputCompressedFileName.Empty();
	m_pImageView = NULL;
}

CDecompressJPEG2000::~CDecompressJPEG2000()
{
}

void CDecompressJPEG2000::SetInFileName(CString strInFilename)
{
	m_strInputCompressedFileName = strInFilename;
}

void CDecompressJPEG2000::SetImageViewer(CImageViewer* pImageView)
{
	if (pImageView == NULL)
		return;

	m_pImageView = pImageView;
}

BOOL CDecompressJPEG2000::DoDecompress(INT_PTR nStartLayerNum, INT_PTR nEndLayerNum)
{
	if (m_strInputCompressedFileName.IsEmpty())
		return FALSE;

	opj_decompress_parameters parameters;           /* decompression parameters */
	opj_image_t* image = NULL;
	opj_stream_t *l_stream = NULL;              /* Stream */
	opj_codec_t* l_codec = NULL;                /* Handle to a decompressor */
	opj_codestream_index_t* cstr_index = NULL;
	opj_codestream_info_v2_t* cstr_info = NULL;

	J2K_IMAGE_DIR img_fol;
	dircnt_t *dirptr = NULL;
	int failed = 0;
	OPJ_UINT32 numDecompressedImages = 0;
	OPJ_UINT32 cp_reduce;

	/* set decoding parameters to default values */
	set_default_parameters(&parameters);

	/* Initialize img_fol */
	memset(&img_fol, 0, sizeof(J2K_IMAGE_DIR));

	if (ParseDecodingOption(&parameters, &img_fol) != 1) {
		failed = 1;
		goto fin;
	}

	parameters.decod_format = infile_format((CStringA)m_strInputCompressedFileName);

	if (IsValidInputFileType(m_strInputCompressedFileName, &parameters) == FALSE)
		return FALSE;

	cp_reduce = parameters.core.cp_reduce;

	if (getenv("USE_OPJ_SET_DECODED_RESOLUTION_FACTOR") != NULL) {
		/* For debugging/testing purposes, do not set the cp_reduce member */
		/* if USE_OPJ_SET_DECODED_RESOLUTION_FACTOR is defined, but used */
		/* the opj_set_decoded_resolution_factor() API instead */
		parameters.core.cp_reduce = 0;
	}

	if (nStartLayerNum <= 0)
	{
		nStartLayerNum = 1;
	}

	BOOL bSetEndLayer = FALSE;

	if (nEndLayerNum > 0)
	{
		bSetEndLayer = TRUE;
	}
	else
	{
		nEndLayerNum = 10;
	}

	for (INT_PTR iLayer = nStartLayerNum; iLayer < nEndLayerNum; iLayer++)
	{
		parameters.core.cp_layer = iLayer;

		if (!parameters.quiet) {
			fprintf(stderr, "\n");
		}

		l_stream = opj_stream_create_default_file_stream(parameters.infile, 1);
		if (!l_stream) {
			fprintf(stderr, "ERROR -> failed to create the stream from the file %s\n",
				parameters.infile);
			failed = 1;
			goto fin;
		}

		/* decode the JPEG2000 stream */
		/* ---------------------- */

		switch (parameters.decod_format) {
		case J2K_CFMT: { /* JPEG-2000 codestream */
						 /* Get a decoder handle */
			l_codec = opj_create_decompress(OPJ_CODEC_J2K);
			break;
		}
		case JP2_CFMT: { /* JPEG 2000 compressed image data */
						 /* Get a decoder handle */
			l_codec = opj_create_decompress(OPJ_CODEC_JP2);
			break;
		}
		case JPT_CFMT: { /* JPEG 2000, JPIP */
						 /* Get a decoder handle */
			l_codec = opj_create_decompress(OPJ_CODEC_JPT);
			break;
		}
		default:
			fprintf(stderr, "skipping file..\n");
			destroy_parameters(&parameters);
			opj_stream_destroy(l_stream);
			goto fin;
		}

		if (parameters.quiet) {
			/* Set all callbacks to quiet */
			opj_set_info_handler(l_codec, quiet_callback, 00);
			opj_set_warning_handler(l_codec, quiet_callback, 00);
			opj_set_error_handler(l_codec, quiet_callback, 00);
		}
		else {
			/* catch events using our callbacks and give a local context */
			opj_set_info_handler(l_codec, info_callback, 00);
			opj_set_warning_handler(l_codec, warning_callback, 00);
			opj_set_error_handler(l_codec, error_callback, 00);
		}

		/* read the input file and put it in memory */
		/* ---------------------------------------- */

		/* Setup the decoder decoding parameters using user parameters */
		if (!opj_setup_decoder(l_codec, &(parameters.core))) {
			fprintf(stderr, "ERROR -> opj_decompress: failed to setup the decoder\n");
			opj_stream_destroy(l_stream);
			opj_destroy_codec(l_codec);
			failed = 1;
			goto fin;
		}

		parameters.num_threads = (int)opj_get_num_cpus()/2;

		if (parameters.num_threads >= 1 &&
			!opj_codec_set_threads(l_codec, parameters.num_threads)) {
			fprintf(stderr, "ERROR -> opj_decompress: failed to set number of threads\n");
			opj_stream_destroy(l_stream);
			opj_destroy_codec(l_codec);
			failed = 1;
			goto fin;
		}

		/* Read the main header of the codestream and if necessary the JP2 boxes*/
		if (!opj_read_header(l_stream, l_codec, &image)) {
			fprintf(stderr, "ERROR -> opj_decompress: failed to read the header\n");
			opj_stream_destroy(l_stream);
			opj_destroy_codec(l_codec);
			opj_image_destroy(image);
			failed = 1;
			goto fin;
		}

		if (cstr_info == NULL)
		{
			cstr_info = opj_get_cstr_info(l_codec);

			if (bSetEndLayer == FALSE)
			{
				nEndLayerNum = cstr_info->m_default_tile_info.numlayers + 1;
			}
		}

		m_pImageView->SetLayerNum(iLayer, cstr_info->m_default_tile_info.numlayers);

		if (parameters.numcomps) {
			if (!opj_set_decoded_components(l_codec,
				parameters.numcomps,
				parameters.comps_indices,
				OPJ_FALSE)) {
				fprintf(stderr,
					"ERROR -> opj_decompress: failed to set the component indices!\n");
				opj_destroy_codec(l_codec);
				opj_stream_destroy(l_stream);
				opj_image_destroy(image);
				failed = 1;
				goto fin;
			}
		}

		if (getenv("USE_OPJ_SET_DECODED_RESOLUTION_FACTOR") != NULL) {
			/* For debugging/testing purposes, and also an illustration on how to */
			/* use the alternative API opj_set_decoded_resolution_factor() instead */
			/* of setting parameters.cp_reduce */
			if (!opj_set_decoded_resolution_factor(l_codec, cp_reduce)) {
				fprintf(stderr,
					"ERROR -> opj_decompress: failed to set the resolution factor tile!\n");
				opj_destroy_codec(l_codec);
				opj_stream_destroy(l_stream);
				opj_image_destroy(image);
				failed = 1;
				goto fin;
			}
		}

		if (!parameters.nb_tile_to_decode) {
			if (getenv("SKIP_OPJ_SET_DECODE_AREA") != NULL &&
				parameters.DA_x0 == 0 &&
				parameters.DA_y0 == 0 &&
				parameters.DA_x1 == 0 &&
				parameters.DA_y1 == 0) {
				/* For debugging/testing purposes, */
				/* do nothing if SKIP_OPJ_SET_DECODE_AREA env variable */
				/* is defined and no decoded area has been set */
			}
			/* Optional if you want decode the entire image */
			else if (!opj_set_decode_area(l_codec, image, (OPJ_INT32)parameters.DA_x0,
				(OPJ_INT32)parameters.DA_y0, (OPJ_INT32)parameters.DA_x1,
				(OPJ_INT32)parameters.DA_y1)) {
				fprintf(stderr, "ERROR -> opj_decompress: failed to set the decoded area\n");
				opj_stream_destroy(l_stream);
				opj_destroy_codec(l_codec);
				opj_image_destroy(image);
				failed = 1;
				goto fin;
			}

			/* Get the decoded image */
			if (!(opj_decode(l_codec, l_stream, image) &&
				opj_end_decompress(l_codec, l_stream))) {
				fprintf(stderr, "ERROR -> opj_decompress: failed to decode image!\n");
				opj_destroy_codec(l_codec);
				opj_stream_destroy(l_stream);
				opj_image_destroy(image);
				failed = 1;
				goto fin;
			}

		}
		else {
			if (!(parameters.DA_x0 == 0 &&
				parameters.DA_y0 == 0 &&
				parameters.DA_x1 == 0 &&
				parameters.DA_y1 == 0)) {
				if (!(parameters.quiet)) {
					fprintf(stderr, "WARNING: -d option ignored when used together with -t\n");
				}
			}

			if (!opj_get_decoded_tile(l_codec, l_stream, image, parameters.tile_index)) {
				fprintf(stderr, "ERROR -> opj_decompress: failed to decode tile!\n");
				opj_destroy_codec(l_codec);
				opj_stream_destroy(l_stream);
				opj_image_destroy(image);
				failed = 1;
				goto fin;
			}
			if (!(parameters.quiet)) {
				fprintf(stdout, "tile %d is decoded!\n\n", parameters.tile_index);
			}
		}

		/* FIXME? Shouldn't that situation be considered as an error of */
		/* opj_decode() / opj_get_decoded_tile() ? */
		if (image->comps[0].data == NULL) {
			fprintf(stderr, "ERROR -> opj_decompress: no image data!\n");
			opj_destroy_codec(l_codec);
			opj_stream_destroy(l_stream);
			opj_image_destroy(image);
			failed = 1;
			goto fin;
		}

		numDecompressedImages++;

		/* Close the byte stream */
		opj_stream_destroy(l_stream);

		if (image->color_space != OPJ_CLRSPC_SYCC
			&& image->numcomps == 3 && image->comps[0].dx == image->comps[0].dy
			&& image->comps[1].dx != 1) {
			image->color_space = OPJ_CLRSPC_SYCC;
		}
		else if (image->numcomps <= 2) {
			image->color_space = OPJ_CLRSPC_GRAY;
		}

		if (image->color_space == OPJ_CLRSPC_SYCC) {
			color_sycc_to_rgb(image);
		}
		else if ((image->color_space == OPJ_CLRSPC_CMYK) &&
			(parameters.cod_format != TIF_DFMT)) {
			color_cmyk_to_rgb(image);
		}
		else if (image->color_space == OPJ_CLRSPC_EYCC) {
			color_esycc_to_rgb(image);
		}

		if (image->icc_profile_buf) {
#if defined(OPJ_HAVE_LIBLCMS1) || defined(OPJ_HAVE_LIBLCMS2)
			if (image->icc_profile_len) {
				color_apply_icc_profile(image);
			}
			else {
				color_cielab_to_rgb(image);
			}
#endif
			free(image->icc_profile_buf);
			image->icc_profile_buf = NULL;
			image->icc_profile_len = 0;
		}

		/* Force output precision */
		/* ---------------------- */
		if (parameters.precision != NULL) {
			OPJ_UINT32 compno;
			for (compno = 0; compno < image->numcomps; ++compno) {
				OPJ_UINT32 precno = compno;
				OPJ_UINT32 prec;

				if (precno >= parameters.nb_precision) {
					precno = parameters.nb_precision - 1U;
				}

				prec = parameters.precision[precno].prec;
				if (prec == 0) {
					prec = image->comps[compno].prec;
				}

				switch (parameters.precision[precno].mode) {
				case OPJ_PREC_MODE_CLIP:
					clip_component(&(image->comps[compno]), prec);
					break;
				case OPJ_PREC_MODE_SCALE:
					scale_component(&(image->comps[compno]), prec);
					break;
				default:
					break;
				}

			}
		}

		/* Upsample components */
		/* ------------------- */
		if (parameters.upsample) {
			image = upsample_image_components(image);
			if (image == NULL) {
				fprintf(stderr,
					"ERROR -> opj_decompress: failed to upsample image components!\n");
				opj_destroy_codec(l_codec);
				failed = 1;
				goto fin;
			}
		}

			

		/* Force RGB output */
		/* ---------------- */
		if (parameters.force_rgb) {
			switch (image->color_space) {
			case OPJ_CLRSPC_SRGB:
				break;
			case OPJ_CLRSPC_GRAY:
				image = convert_gray_to_rgb(image);
				break;
			default:
				fprintf(stderr,
					"ERROR -> opj_decompress: don't know how to convert image to RGB colorspace!\n");
				opj_image_destroy(image);
				image = NULL;
				break;
			}
			if (image == NULL) {
				fprintf(stderr, "ERROR -> opj_decompress: failed to convert to RGB image!\n");
				opj_destroy_codec(l_codec);
				failed = 1;
				goto fin;
			}
		}

		IMAGE_INFO stInImageInfo;
		UpdateImageInfo(image, &stInImageInfo);
		m_pImageView->SetInImageInfo(stInImageInfo);

		// As Stream
		if (!m_pImageView->AllocInImage())
			goto fin;

		void* pInImageStream;
		pInImageStream = m_pImageView->GetInputImageStream();

		if (!pInImageStream)
		{
			m_pImageView->FreeInImage();
			goto fin;
		}

		if (imagetostream(image, pInImageStream)) {
			failed = 1;
		}
		else if (!(parameters.quiet)) {
		}

		// Update info for windowing
		UpdateImageInfo(image, &stInImageInfo);
		m_pImageView->SetInImageInfo(stInImageInfo);

		m_pImageView->LoadImageFromStream();
		m_pImageView->UpdateViewer();

		/* free remaining structures */
		if (l_codec) {
			opj_destroy_codec(l_codec);
		}

		/* free image data structure */
		opj_image_destroy(image);

		/* destroy the codestream index */
		opj_destroy_cstr_index(&cstr_index);

		// 여기까지...

		if (failed) {
			(void)remove(parameters.outfile);    /* ignore return value */
		}

	}


fin:

	return TRUE;
}

INT_PTR CDecompressJPEG2000::ParseDecodingOption(opj_decompress_parameters *parameters, J2K_IMAGE_DIR *img_fol)
{
	/* parse the command line */
	int totlen, c;
	opj_option_t long_option[] = {
		{ "ImgDir",    REQ_ARG, NULL, 'y' },
		{ "OutFor",    REQ_ARG, NULL, 'O' },
		{ "force-rgb", NO_ARG,  NULL, 1 },		//[2]
		{ "upsample",  NO_ARG,  NULL, 1 },		//[3]
		{ "split-pnm", NO_ARG,  NULL, 1 },		//[4]
		{ "threads",   REQ_ARG, NULL, 'T' },
		{ "quiet", NO_ARG,  NULL, 1 },			//[6]
	};

	const char optlist[] = "i:o:r:l:x:d:t:p:c:"
		/* UniPG>> */
#ifdef USE_JPWL
		"W:"
#endif /* USE_JPWL */
		/* <<UniPG */
		"h";

	long_option[2].flag = &(parameters->force_rgb);
	long_option[3].flag = &(parameters->upsample);
	long_option[4].flag = &(parameters->split_pnm);
	long_option[6].flag = &(parameters->quiet);

	totlen = sizeof(long_option);
	opj_reset_options_reading();
	img_fol->set_out_format = 0;

	return 1;
}

BOOL CDecompressJPEG2000::IsValidInputFileType(CString strFileName, opj_decompress_parameters *parameters)
{
	CString errMes;

	switch (parameters->decod_format) {
	case J2K_CFMT:
		break;
	case JP2_CFMT:
		break;
	case JPT_CFMT:
		break;
	case -2:
		errMes.Format(_T("!! infile cannot be read: %s !!\n\n"), strFileName);
		AfxMessageBox(errMes);
		return FALSE;
	default:
		errMes.Format(_T("[ERROR] Unknown input file format: %s \n Known file formats are *.j2k, *.jp2, *.jpc or *.jpt\n"), strFileName);
		AfxMessageBox(errMes);
		return FALSE;
	}

	if (opj_strcpy_s(parameters->infile, sizeof(parameters->infile), (CStringA)strFileName) != 0) {
		errMes.Format(_T("[ERROR] Path is too long\n"));
		AfxMessageBox(errMes);
		return FALSE;
	}

	return TRUE;
}

BOOL CDecompressJPEG2000::SetOutFileType(CString strFileName, opj_decompress_parameters *parameters, J2K_IMAGE_DIR *img_fol)
{
	parameters->cod_format = get_file_format((CStringA)strFileName);

	switch (parameters->cod_format) {
	case PGX_DFMT:
		img_fol->out_format = "pgx";
		break;
	case PXM_DFMT:
		img_fol->out_format = "ppm";
		break;
	case BMP_DFMT:
		img_fol->out_format = "bmp";
		break;
	case TIF_DFMT:
		img_fol->out_format = "tif";
		break;
	case RAW_DFMT:
		img_fol->out_format = "raw";
		break;
	case RAWL_DFMT:
		img_fol->out_format = "rawl";
		break;
	case TGA_DFMT:
		img_fol->out_format = "raw";
		break;
	case PNG_DFMT:
		img_fol->out_format = "png";
		break;
	default:
		CString errMes;
		errMes.Format(_T("Unknown output format image %d [only *.png, *.pnm, *.pgm, *.ppm, *.pgx, *.bmp, *.tif, *.raw or *.tga]!!\n"), parameters->cod_format);
		AfxMessageBox(errMes);
		return FALSE;
	}

	if (opj_strcpy_s(parameters->outfile, sizeof(parameters->outfile), (CStringA)strFileName) != 0) {
		return FALSE;
	}

	return TRUE;
}

void CDecompressJPEG2000::UpdateImageInfo(opj_image_t* pInImageInfo, IMAGE_INFO* pOutImageInfo)
{
	pOutImageInfo->init();

	pOutImageInfo->nWidth = pInImageInfo->x1 - pInImageInfo->x0;
	pOutImageInfo->nHeight = pInImageInfo->y1 - pInImageInfo->y0;
	pOutImageInfo->nBitsPerPixel = pInImageInfo->comps->prec;

	if (pOutImageInfo->nBitsPerPixel <= 8)
	{
		pOutImageInfo->nBytesPerPixel = 1;
		pOutImageInfo->nSamplesPerPixel = pInImageInfo->numcomps;
	}
	else if (pOutImageInfo->nBitsPerPixel <= 16)
	{
		pOutImageInfo->nBytesPerPixel = 2;
		pOutImageInfo->nSamplesPerPixel = pInImageInfo->numcomps;
	}
	else if (pOutImageInfo->nBitsPerPixel <= 24)
	{
		pOutImageInfo->nBytesPerPixel = 3;
		pOutImageInfo->nSamplesPerPixel = pInImageInfo->numcomps;
	}

	pOutImageInfo->nTotalAllocBytesPerPixel = pOutImageInfo->nSamplesPerPixel * pOutImageInfo->nBytesPerPixel;
	pOutImageInfo->nBytesPerLine = pOutImageInfo->nWidth * pOutImageInfo->nTotalAllocBytesPerPixel;

	pOutImageInfo->nW1 = pInImageInfo->nMinWindow;
	pOutImageInfo->nW2 = pInImageInfo->nMaxWindow;
}

void CDecompressJPEG2000::set_default_parameters(opj_decompress_parameters* parameters)
{
	if (parameters) {
		memset(parameters, 0, sizeof(opj_decompress_parameters));

		/* default decoding parameters (command line specific) */
		parameters->decod_format = -1;
		parameters->cod_format = -1;

		/* default decoding parameters (core) */
		opj_set_default_decoder_parameters(&(parameters->core));
	}
}

INT_PTR CDecompressJPEG2000::infile_format(const char *fname)
{
	FILE *reader;
	const char *s, *magic_s;
	INT_PTR ext_format, magic_format;
	unsigned char buf[12];
	OPJ_SIZE_T l_nb_read;

	reader = fopen(fname, "rb");

	if (reader == NULL) {
		return -2;
	}

	memset(buf, 0, 12);
	l_nb_read = fread(buf, 1, 12, reader);
	fclose(reader);
	if (l_nb_read != 12) {
		return -1;
	}

	ext_format = get_file_format(fname);

	if (ext_format == JPT_CFMT) {
		return JPT_CFMT;
	}

	if (memcmp(buf, JP2_RFC3745_MAGIC, 12) == 0 || memcmp(buf, JP2_MAGIC, 4) == 0) {
		magic_format = JP2_CFMT;
		magic_s = ".jp2";
	}
	else if (memcmp(buf, J2K_CODESTREAM_MAGIC, 4) == 0) {
		magic_format = J2K_CFMT;
		magic_s = ".j2k or .jpc or .j2c";
	}
	else {
		return -1;
	}

	if (magic_format == ext_format) {
		return ext_format;
	}

	s = fname + strlen(fname) - 4;

	fputs("\n===========================================\n", stderr);
	fprintf(stderr, "The extension of this file is incorrect.\n"
		"FOUND %s. SHOULD BE %s\n", s, magic_s);
	fputs("===========================================\n", stderr);

	return magic_format;
}

INT_PTR CDecompressJPEG2000::get_file_format(const char *filename)
{
	unsigned int i;
	static const char *extension[] = { "pgx", "pnm", "pgm", "ppm", "bmp", "tif", "raw", "rawl", "tga", "png", "j2k", "jp2", "jpt", "j2c", "jpc" };
	static const int format[] = { PGX_DFMT, PXM_DFMT, PXM_DFMT, PXM_DFMT, BMP_DFMT, TIF_DFMT, RAW_DFMT, RAWL_DFMT, TGA_DFMT, PNG_DFMT, J2K_CFMT, JP2_CFMT, JPT_CFMT, J2K_CFMT, J2K_CFMT };
	const char * ext = strrchr(filename, '.');
	if (ext == NULL) {
		return -1;
	}
	ext++;
	if (*ext) {
		for (i = 0; i < sizeof(format) / sizeof(*format); i++) {
			if (strcasecmp(ext, extension[i]) == 0) {
				return format[i];
			}
		}
	}

	return -1;
}

OPJ_FLOAT64 CDecompressJPEG2000::opj_clock(void)
{
#ifdef _WIN32
	/* _WIN32: use QueryPerformance (very accurate) */
	LARGE_INTEGER freq, t;
	/* freq is the clock speed of the CPU */
	QueryPerformanceFrequency(&freq);
	/* cout << "freq = " << ((double) freq.QuadPart) << endl; */
	/* t is the high resolution performance counter (see MSDN) */
	QueryPerformanceCounter(&t);
	return freq.QuadPart ? ((OPJ_FLOAT64)t.QuadPart / (OPJ_FLOAT64)freq.QuadPart) :
		0;
#elif defined(__linux)
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	return ((OPJ_FLOAT64)ts.tv_sec + (OPJ_FLOAT64)ts.tv_nsec * 1e-9);
#else
	/* Unix : use resource usage */
	/* FIXME: this counts the total CPU time, instead of the user perceived time */
	struct rusage t;
	OPJ_FLOAT64 procTime;
	/* (1) Get the rusage data structure at this moment (man getrusage) */
	getrusage(0, &t);
	/* (2) What is the elapsed time ? - CPU time = User time + System time */
	/* (2a) Get the seconds */
	procTime = (OPJ_FLOAT64)(t.ru_utime.tv_sec + t.ru_stime.tv_sec);
	/* (2b) More precisely! Get the microseconds part ! */
	return (procTime + (OPJ_FLOAT64)(t.ru_utime.tv_usec + t.ru_stime.tv_usec) *
		1e-6);
#endif
}

opj_image_t* CDecompressJPEG2000::convert_gray_to_rgb(opj_image_t* original)
{
	OPJ_UINT32 compno;
	opj_image_t* l_new_image = NULL;
	opj_image_cmptparm_t* l_new_components = NULL;

	l_new_components = (opj_image_cmptparm_t*)malloc((original->numcomps + 2U) *
		sizeof(opj_image_cmptparm_t));
	if (l_new_components == NULL) {
		fprintf(stderr,
			"ERROR -> opj_decompress: failed to allocate memory for RGB image!\n");
		opj_image_destroy(original);
		return NULL;
	}

	l_new_components[0].bpp = l_new_components[1].bpp = l_new_components[2].bpp =
		original->comps[0].bpp;
	l_new_components[0].dx = l_new_components[1].dx = l_new_components[2].dx =
		original->comps[0].dx;
	l_new_components[0].dy = l_new_components[1].dy = l_new_components[2].dy =
		original->comps[0].dy;
	l_new_components[0].h = l_new_components[1].h = l_new_components[2].h =
		original->comps[0].h;
	l_new_components[0].w = l_new_components[1].w = l_new_components[2].w =
		original->comps[0].w;
	l_new_components[0].prec = l_new_components[1].prec = l_new_components[2].prec =
		original->comps[0].prec;
	l_new_components[0].sgnd = l_new_components[1].sgnd = l_new_components[2].sgnd =
		original->comps[0].sgnd;
	l_new_components[0].x0 = l_new_components[1].x0 = l_new_components[2].x0 =
		original->comps[0].x0;
	l_new_components[0].y0 = l_new_components[1].y0 = l_new_components[2].y0 =
		original->comps[0].y0;

	for (compno = 1U; compno < original->numcomps; ++compno) {
		l_new_components[compno + 2U].bpp = original->comps[compno].bpp;
		l_new_components[compno + 2U].dx = original->comps[compno].dx;
		l_new_components[compno + 2U].dy = original->comps[compno].dy;
		l_new_components[compno + 2U].h = original->comps[compno].h;
		l_new_components[compno + 2U].w = original->comps[compno].w;
		l_new_components[compno + 2U].prec = original->comps[compno].prec;
		l_new_components[compno + 2U].sgnd = original->comps[compno].sgnd;
		l_new_components[compno + 2U].x0 = original->comps[compno].x0;
		l_new_components[compno + 2U].y0 = original->comps[compno].y0;
	}

	l_new_image = opj_image_create(original->numcomps + 2U, l_new_components,
		OPJ_CLRSPC_SRGB);
	free(l_new_components);
	if (l_new_image == NULL) {
		fprintf(stderr,
			"ERROR -> opj_decompress: failed to allocate memory for RGB image!\n");
		opj_image_destroy(original);
		return NULL;
	}

	l_new_image->x0 = original->x0;
	l_new_image->x1 = original->x1;
	l_new_image->y0 = original->y0;
	l_new_image->y1 = original->y1;

	l_new_image->comps[0].factor = l_new_image->comps[1].factor =
		l_new_image->comps[2].factor = original->comps[0].factor;
	l_new_image->comps[0].alpha = l_new_image->comps[1].alpha =
		l_new_image->comps[2].alpha = original->comps[0].alpha;
	l_new_image->comps[0].resno_decoded = l_new_image->comps[1].resno_decoded =
		l_new_image->comps[2].resno_decoded = original->comps[0].resno_decoded;

	memcpy(l_new_image->comps[0].data, original->comps[0].data,
		original->comps[0].w * original->comps[0].h * sizeof(OPJ_INT32));
	memcpy(l_new_image->comps[1].data, original->comps[0].data,
		original->comps[0].w * original->comps[0].h * sizeof(OPJ_INT32));
	memcpy(l_new_image->comps[2].data, original->comps[0].data,
		original->comps[0].w * original->comps[0].h * sizeof(OPJ_INT32));

	for (compno = 1U; compno < original->numcomps; ++compno) {
		l_new_image->comps[compno + 2U].factor = original->comps[compno].factor;
		l_new_image->comps[compno + 2U].alpha = original->comps[compno].alpha;
		l_new_image->comps[compno + 2U].resno_decoded =
			original->comps[compno].resno_decoded;
		memcpy(l_new_image->comps[compno + 2U].data, original->comps[compno].data,
			original->comps[compno].w * original->comps[compno].h * sizeof(OPJ_INT32));
	}
	opj_image_destroy(original);
	return l_new_image;
}

OPJ_BOOL CDecompressJPEG2000::parse_precision(const char* option, opj_decompress_parameters* parameters)
{
	const char* l_remaining = option;
	OPJ_BOOL l_result = OPJ_TRUE;

	/* reset */
	if (parameters->precision) {
		free(parameters->precision);
		parameters->precision = NULL;
	}
	parameters->nb_precision = 0U;

	for (;;) {
		int prec;
		char mode;
		char comma;
		int count;

		count = sscanf(l_remaining, "%d%c%c", &prec, &mode, &comma);
		if (count == 1) {
			mode = 'C';
			count++;
		}
		if ((count == 2) || (mode == ',')) {
			if (mode == ',') {
				mode = 'C';
			}
			comma = ',';
			count = 3;
		}
		if (count == 3) {
			if ((prec < 1) || (prec > 32)) {
				fprintf(stderr, "Invalid precision %d in precision option %s\n", prec, option);
				l_result = OPJ_FALSE;
				break;
			}
			if ((mode != 'C') && (mode != 'S')) {
				fprintf(stderr, "Invalid precision mode %c in precision option %s\n", mode,
					option);
				l_result = OPJ_FALSE;
				break;
			}
			if (comma != ',') {
				fprintf(stderr, "Invalid character %c in precision option %s\n", comma, option);
				l_result = OPJ_FALSE;
				break;
			}

			if (parameters->precision == NULL) {
				/* first one */
				parameters->precision = (opj_precision *)malloc(sizeof(opj_precision));
				if (parameters->precision == NULL) {
					fprintf(stderr, "Could not allocate memory for precision option\n");
					l_result = OPJ_FALSE;
					break;
				}
			}
			else {
				OPJ_UINT32 l_new_size = parameters->nb_precision + 1U;
				opj_precision* l_new;

				if (l_new_size == 0U) {
					fprintf(stderr, "Could not allocate memory for precision option\n");
					l_result = OPJ_FALSE;
					break;
				}

				l_new = (opj_precision *)realloc(parameters->precision,
					l_new_size * sizeof(opj_precision));
				if (l_new == NULL) {
					fprintf(stderr, "Could not allocate memory for precision option\n");
					l_result = OPJ_FALSE;
					break;
				}
				parameters->precision = l_new;
			}

			parameters->precision[parameters->nb_precision].prec = (OPJ_UINT32)prec;
			switch (mode) {
			case 'C':
				parameters->precision[parameters->nb_precision].mode = OPJ_PREC_MODE_CLIP;
				break;
			case 'S':
				parameters->precision[parameters->nb_precision].mode = OPJ_PREC_MODE_SCALE;
				break;
			default:
				break;
			}
			parameters->nb_precision++;

			l_remaining = strchr(l_remaining, ',');
			if (l_remaining == NULL) {
				break;
			}
			l_remaining += 1;
		}
		else {
			fprintf(stderr, "Could not parse precision option %s\n", option);
			l_result = OPJ_FALSE;
			break;
		}
	}

	return l_result;
}

void CDecompressJPEG2000::destroy_parameters(opj_decompress_parameters* parameters)
{
	if (parameters) {
		if (parameters->precision) {
			free(parameters->precision);
			parameters->precision = NULL;
		}

		free(parameters->comps_indices);
		parameters->comps_indices = NULL;
	}
}

opj_image_t* CDecompressJPEG2000::upsample_image_components(opj_image_t* original)
{
	opj_image_t* l_new_image = NULL;
	opj_image_cmptparm_t* l_new_components = NULL;
	OPJ_BOOL l_upsample_need = OPJ_FALSE;
	OPJ_UINT32 compno;

	for (compno = 0U; compno < original->numcomps; ++compno) {
		if (original->comps[compno].factor > 0U) {
			fprintf(stderr,
				"ERROR -> opj_decompress: -upsample not supported with reduction\n");
			opj_image_destroy(original);
			return NULL;
		}
		if ((original->comps[compno].dx > 1U) || (original->comps[compno].dy > 1U)) {
			l_upsample_need = OPJ_TRUE;
			break;
		}
	}
	if (!l_upsample_need) {
		return original;
	}
	/* Upsample is needed */
	l_new_components = (opj_image_cmptparm_t*)malloc(original->numcomps * sizeof(
		opj_image_cmptparm_t));
	if (l_new_components == NULL) {
		fprintf(stderr,
			"ERROR -> opj_decompress: failed to allocate memory for upsampled components!\n");
		opj_image_destroy(original);
		return NULL;
	}

	for (compno = 0U; compno < original->numcomps; ++compno) {
		opj_image_cmptparm_t* l_new_cmp = &(l_new_components[compno]);
		opj_image_comp_t*     l_org_cmp = &(original->comps[compno]);

		l_new_cmp->bpp = l_org_cmp->bpp;
		l_new_cmp->prec = l_org_cmp->prec;
		l_new_cmp->sgnd = l_org_cmp->sgnd;
		l_new_cmp->x0 = original->x0;
		l_new_cmp->y0 = original->y0;
		l_new_cmp->dx = 1;
		l_new_cmp->dy = 1;
		l_new_cmp->w =
			l_org_cmp->w; /* should be original->x1 - original->x0 for dx==1 */
		l_new_cmp->h =
			l_org_cmp->h; /* should be original->y1 - original->y0 for dy==0 */

		if (l_org_cmp->dx > 1U) {
			l_new_cmp->w = original->x1 - original->x0;
		}

		if (l_org_cmp->dy > 1U) {
			l_new_cmp->h = original->y1 - original->y0;
		}
	}

	l_new_image = opj_image_create(original->numcomps, l_new_components,
		original->color_space);
	free(l_new_components);
	if (l_new_image == NULL) {
		fprintf(stderr,
			"ERROR -> opj_decompress: failed to allocate memory for upsampled components!\n");
		opj_image_destroy(original);
		return NULL;
	}

	l_new_image->x0 = original->x0;
	l_new_image->x1 = original->x1;
	l_new_image->y0 = original->y0;
	l_new_image->y1 = original->y1;

	for (compno = 0U; compno < original->numcomps; ++compno) {
		opj_image_comp_t* l_new_cmp = &(l_new_image->comps[compno]);
		opj_image_comp_t* l_org_cmp = &(original->comps[compno]);

		l_new_cmp->factor = l_org_cmp->factor;
		l_new_cmp->alpha = l_org_cmp->alpha;
		l_new_cmp->resno_decoded = l_org_cmp->resno_decoded;

		if ((l_org_cmp->dx > 1U) || (l_org_cmp->dy > 1U)) {
			const OPJ_INT32* l_src = l_org_cmp->data;
			OPJ_INT32*       l_dst = l_new_cmp->data;
			OPJ_UINT32 y;
			OPJ_UINT32 xoff, yoff;

			/* need to take into account dx & dy */
			xoff = l_org_cmp->dx * l_org_cmp->x0 - original->x0;
			yoff = l_org_cmp->dy * l_org_cmp->y0 - original->y0;
			if ((xoff >= l_org_cmp->dx) || (yoff >= l_org_cmp->dy)) {
				fprintf(stderr,
					"ERROR -> opj_decompress: Invalid image/component parameters found when upsampling\n");
				opj_image_destroy(original);
				opj_image_destroy(l_new_image);
				return NULL;
			}

			for (y = 0U; y < yoff; ++y) {
				memset(l_dst, 0U, l_new_cmp->w * sizeof(OPJ_INT32));
				l_dst += l_new_cmp->w;
			}

			if (l_new_cmp->h > (l_org_cmp->dy -
				1U)) { /* check subtraction overflow for really small images */
				for (; y < l_new_cmp->h - (l_org_cmp->dy - 1U); y += l_org_cmp->dy) {
					OPJ_UINT32 x, dy;
					OPJ_UINT32 xorg;

					xorg = 0U;
					for (x = 0U; x < xoff; ++x) {
						l_dst[x] = 0;
					}
					if (l_new_cmp->w > (l_org_cmp->dx -
						1U)) { /* check subtraction overflow for really small images */
						for (; x < l_new_cmp->w - (l_org_cmp->dx - 1U); x += l_org_cmp->dx, ++xorg) {
							OPJ_UINT32 dx;
							for (dx = 0U; dx < l_org_cmp->dx; ++dx) {
								l_dst[x + dx] = l_src[xorg];
							}
						}
					}
					for (; x < l_new_cmp->w; ++x) {
						l_dst[x] = l_src[xorg];
					}
					l_dst += l_new_cmp->w;

					for (dy = 1U; dy < l_org_cmp->dy; ++dy) {
						memcpy(l_dst, l_dst - l_new_cmp->w, l_new_cmp->w * sizeof(OPJ_INT32));
						l_dst += l_new_cmp->w;
					}
					l_src += l_org_cmp->w;
				}
			}
			if (y < l_new_cmp->h) {
				OPJ_UINT32 x;
				OPJ_UINT32 xorg;

				xorg = 0U;
				for (x = 0U; x < xoff; ++x) {
					l_dst[x] = 0;
				}
				if (l_new_cmp->w > (l_org_cmp->dx -
					1U)) { /* check subtraction overflow for really small images */
					for (; x < l_new_cmp->w - (l_org_cmp->dx - 1U); x += l_org_cmp->dx, ++xorg) {
						OPJ_UINT32 dx;
						for (dx = 0U; dx < l_org_cmp->dx; ++dx) {
							l_dst[x + dx] = l_src[xorg];
						}
					}
				}
				for (; x < l_new_cmp->w; ++x) {
					l_dst[x] = l_src[xorg];
				}
				l_dst += l_new_cmp->w;
				++y;
				for (; y < l_new_cmp->h; ++y) {
					memcpy(l_dst, l_dst - l_new_cmp->w, l_new_cmp->w * sizeof(OPJ_INT32));
					l_dst += l_new_cmp->w;
				}
			}
		}
		else {
			memcpy(l_new_cmp->data, l_org_cmp->data,
				l_org_cmp->w * l_org_cmp->h * sizeof(OPJ_INT32));
		}
	}
	opj_image_destroy(original);
	return l_new_image;
}

/**
sample error callback expecting a FILE* client object
*/
void CDecompressJPEG2000::error_callback(const char *msg, void *client_data)
{
	(void)client_data;
	fprintf(stdout, "[ERROR] %s", msg);
}
/**
sample warning callback expecting a FILE* client object
*/
void CDecompressJPEG2000::warning_callback(const char *msg, void *client_data)
{
	(void)client_data;
	fprintf(stdout, "[WARNING] %s", msg);
}
/**
sample debug callback expecting no client object
*/
void CDecompressJPEG2000::info_callback(const char *msg, void *client_data)
{
	(void)client_data;
	fprintf(stdout, "[INFO] %s", msg);
}
/**
sample quiet callback expecting no client object
*/
void CDecompressJPEG2000::quiet_callback(const char *msg, void *client_data)
{
	(void)msg;
	(void)client_data;
}