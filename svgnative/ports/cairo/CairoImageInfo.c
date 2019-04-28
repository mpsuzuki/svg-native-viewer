/* -*- Mode: c; tab-width: 8; c-basic-offset: 4; indent-tabs-mode: t; -*- */
/* cairo - a vector graphics library with display and print output
 *
 * Copyright © 2008 Adrian Johnson
 *
 * This library is free software; you can redistribute it and/or
 * modify it either under the terms of the GNU Lesser General Public
 * License version 2.1 as published by the Free Software Foundation
 * (the "LGPL") or, at your option, under the terms of the Mozilla
 * Public License Version 1.1 (the "MPL"). If you do not alter this
 * notice, a recipient may use your version of this file under either
 * the MPL or the LGPL.
 *
 * You should have received a copy of the LGPL along with this library
 * in the file COPYING-LGPL-2.1; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Suite 500, Boston, MA 02110-1335, USA
 * You should have received a copy of the MPL along with this library
 * in the file COPYING-MPL-1.1
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY
 * OF ANY KIND, either express or implied. See the LGPL or the MPL for
 * the specific language governing rights and limitations.
 *
 * The Original Code is the cairo graphics library.
 *
 * The Initial Developer of the Original Code is Adrian Johnson.
 *
 * Contributor(s):
 *	Adrian Johnson <ajohnson@redneon.com>
 */

#include "CairoImageInfo.h"
#include <string.h>

/* JPEG (image/jpeg)
 *
 * http://www.w3.org/Graphics/JPEG/itu-t81.pdf
 */
#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <jpeglib.h>

/* Markers with no parameters. All other markers are followed by a two
 * byte length of the parameters. */
#define TEM       0x01
#define RST_begin 0xd0
#define RST_end   0xd7
#define SOI       0xd8
#define EOI       0xd9

/* Start of frame markers. */
#define SOF0  0xc0
#define SOF1  0xc1
#define SOF2  0xc2
#define SOF3  0xc3
#define SOF5  0xc5
#define SOF6  0xc6
#define SOF7  0xc7
#define SOF9  0xc9
#define SOF10 0xca
#define SOF11 0xcb
#define SOF13 0xcd
#define SOF14 0xce
#define SOF15 0xcf

static const unsigned char *
_jpeg_skip_segment (const unsigned char *p)
{
    int len;

    p++;
    len = (p[0] << 8) | p[1];

    return p + len;
}

static void
_jpeg_extract_info (cairo_image_info_t *info, const unsigned char *p)
{
    info->width = (p[6] << 8) + p[7];
    info->height = (p[4] << 8) + p[5];
    info->num_components = p[8];
    info->bits_per_component = p[3];
}

int
_cairo_image_info_get_jpeg_info (cairo_image_info_t	*info,
				 const unsigned char	*data,
				 long			 length)
{
    const unsigned char *p = data;

    while (p + 1 < data + length) {
	if (*p != 0xff)
	    return -100; /* CAIRO_INT_STATUS_UNSUPPORTED = 100 */
	p++;

	switch (*p) {
	    /* skip fill bytes */
	case 0xff:
	    p++;
	    break;

	case TEM:
	case SOI:
	case EOI:
	    p++;
	    break;

	case SOF0:
	case SOF1:
	case SOF2:
	case SOF3:
	case SOF5:
	case SOF6:
	case SOF7:
	case SOF9:
	case SOF10:
	case SOF11:
	case SOF13:
	case SOF14:
	case SOF15:
	    /* Start of frame found. Extract the image parameters. */
	    if (p + 8 > data + length)
		return -100; /* CAIRO_INT_STATUS_UNSUPPORTED */

	    _jpeg_extract_info (info, p);
	    return 0; /* CAIRO_STATUS_SUCCESS */

	default:
	    if (*p >= RST_begin && *p <= RST_end) {
		p++;
		break;
	    }

	    if (p + 3 > data + length)
		return -100; /* CAIRO_INT_STATUS_UNSUPPORTED */

	    p = _jpeg_skip_segment (p);
	    break;
	}
    }

    return 0; /* CAIRO_STATUS_SUCCESS */
}

struct _cairo_jpeg_error_mgr
{
    struct jpeg_error_mgr pub;
    jmp_buf setjmp_buffer;
};

typedef struct _cairo_jpeg_error_mgr * _cairo_jpeg_error_ptr;

METHODDEF(void)
_cairo_jpeg_error_exit (j_common_ptr cinfo)
{
    _cairo_jpeg_error_ptr _cairo_jpeg_err = (_cairo_jpeg_error_ptr) cinfo->err;
    (*cinfo->err->output_message) (cinfo);
    longjmp(_cairo_jpeg_err->setjmp_buffer, 1);
}

cairo_surface_t *
_cairo_image_surface_create_from_jpeg_stream(const unsigned char* data,
                                             unsigned int length)
{
    cairo_surface_t* _cairo_jpeg_surface = NULL;

    struct jpeg_decompress_struct cinfo;
    struct _cairo_jpeg_error_mgr jerr;
    cinfo.err = jpeg_std_error(&jerr.pub);
    jerr.pub.error_exit = _cairo_jpeg_error_exit;

    unsigned char* outBuff = NULL;
    if (setjmp(jerr.setjmp_buffer)) {
        jpeg_destroy_decompress(&cinfo);
        if (_cairo_jpeg_surface) {
            cairo_surface_destroy (_cairo_jpeg_surface);
        };
        return NULL;
    };

    jpeg_create_decompress(&cinfo);
    jpeg_mem_src(&cinfo, data, length);
    jpeg_read_header(&cinfo, TRUE);

    /* cinfo.image_width, cinfo.image_height, cinfo.num_components are already filled, but
     * cinfo.output_width, cinfo.output_height, cinfo.output_components are not, because
     * they are output parameters
     */
    jpeg_start_decompress(&cinfo);

    size_t outCur, outLimit;
    int jpeg_row_stride = cinfo.output_width * cinfo.output_components;
    int cairo_row_stride = cairo_format_stride_for_width (CAIRO_FORMAT_RGB24, cinfo.output_width);

    outLimit = cairo_row_stride * cinfo.output_height;
    outBuff = (unsigned char*)malloc(outLimit);
    bzero(outBuff, outLimit);
    outCur = 0;

    JSAMPARRAY buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr) &cinfo, JPOOL_IMAGE, jpeg_row_stride, 1);

    while (cinfo.output_scanline < cinfo.output_height) {
        jpeg_read_scanlines(&cinfo, buffer, 1);
        int ipxl, iclr;
        for (ipxl = 0; ipxl < cinfo.output_width; ipxl ++) {
#if 0
            for (iclr = 0; iclr < cinfo.output_components; iclr ++) {
                outBuff[outCur + (ipxl * (cinfo.output_components + 1)) + iclr + 1] = buffer[0][(ipxl * cinfo.output_components) + iclr];
            }
#endif
            /* little endian 32bit case */
            outBuff[outCur+(ipxl * 4)+0] = buffer[0][(ipxl * 3) + 2]; // blue
            outBuff[outCur+(ipxl * 4)+1] = buffer[0][(ipxl * 3) + 1]; // green
            outBuff[outCur+(ipxl * 4)+2] = buffer[0][(ipxl * 3) + 0]; // red
            outBuff[outCur+(ipxl * 4)+3] = 0;
        }
        outCur += cairo_row_stride;
    };
    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);

    /* XXX: this is only RGB case, for grayscale and other colorspaces... */
    _cairo_jpeg_surface = cairo_image_surface_create_for_data(outBuff, CAIRO_FORMAT_RGB24, cinfo.output_width, cinfo.output_height, cairo_row_stride);

    return _cairo_jpeg_surface;
}

/* PNG (image/png)
 *
 * http://www.w3.org/TR/2003/REC-PNG-20031110/
 */

#define PNG_IHDR 0x49484452

static const unsigned char _png_magic[8] = { 137, 80, 78, 71, 13, 10, 26, 10 };

int
_cairo_image_info_get_png_info (cairo_image_info_t     *info,
                               const unsigned char     *data,
                               unsigned long            length)
{
    const unsigned char *p = data;
    const unsigned char *end = data + length;

    if (length < 8 || memcmp (data, _png_magic, 8) != 0)
       return -100; /* CAIRO_INT_STATUS_UNSUPPORTED */

    p += 8;

    /* The first chunk must be IDHR. IDHR has 13 bytes of data plus
     * the 12 bytes of overhead for the chunk. */
    if (p + 13 + 12 > end)
       return -100; /* CAIRO_INT_STATUS_UNSUPPORTED */

    p += 4;
    if (get_unaligned_be32 (p) != PNG_IHDR)
       return -100; /* CAIRO_INT_STATUS_UNSUPPORTED */

    p += 4;
    info->width = get_unaligned_be32 (p);
    p += 4;
    info->height = get_unaligned_be32 (p);

    return 0; /* CAIRO_STATUS_SUCCESS */
}

cairo_status_t
_png_blob_read_func (void           *closure,
                     unsigned char  *data,
                     unsigned int    length)
{
    _png_blob_closure_t  *png_blob_closure = (_png_blob_closure_t*)closure;
    
    if (png_blob_closure->limit <= png_blob_closure->cur_pos)
        return CAIRO_STATUS_READ_ERROR;

    if (png_blob_closure->limit <= png_blob_closure->cur_pos + length) {
        memset(data, 0, length); 
        length = png_blob_closure->limit - png_blob_closure->cur_pos;
    }

    memcpy(data, png_blob_closure->blob + png_blob_closure->cur_pos, length); 
    png_blob_closure->cur_pos += length;
    return CAIRO_STATUS_SUCCESS;
}
