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

    cairo_format_t cairo_color_format;
    switch (cinfo.out_color_space) {
    case JCS_GRAYSCALE:
        cairo_color_format = CAIRO_FORMAT_A8;
        break;
    case JCS_EXT_ARGB:
        cairo_color_format = CAIRO_FORMAT_ARGB32;
        break;
    case JCS_RGB:
    default:
        cairo_color_format = CAIRO_FORMAT_RGB24;
    };

    /* cinfo.image_width, cinfo.image_height, cinfo.num_components are already filled, but
     * cinfo.output_width, cinfo.output_height, cinfo.output_components are not, because
     * they are output parameters
     */
    jpeg_start_decompress(&cinfo);

    size_t outCur, outLimit;
    int jpeg_row_stride = cinfo.output_width * cinfo.output_components;
    int cairo_row_stride = cairo_format_stride_for_width (cairo_color_format, cinfo.output_width);

    outLimit = cairo_row_stride * cinfo.output_height;
    outBuff = (unsigned char*)malloc(outLimit);
    bzero(outBuff, outLimit);
    outCur = 0;

    JSAMPARRAY buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr) &cinfo, JPOOL_IMAGE, jpeg_row_stride, 1);

    while (cinfo.output_scanline < cinfo.output_height) {
        jpeg_read_scanlines(&cinfo, buffer, 1);
        int ipxl, iclr;
        for (ipxl = 0; ipxl < cinfo.output_width; ipxl ++) {
            size_t jpeg_buff_offset = (ipxl * cinfo.output_components); 
            unsigned long rgb = 0;
            for (iclr = 0; iclr < cinfo.output_components; iclr ++) { 
                rgb = (rgb << 8) | buffer[0][jpeg_buff_offset + iclr];
            };
            switch (cairo_color_format) {
            case CAIRO_FORMAT_A8:
                outBuff[ipxl] = rgb;
                break;
            case CAIRO_FORMAT_ARGB32:
            case CAIRO_FORMAT_RGB24:
                ((uint32_t*)(outBuff + outCur))[ipxl] = rgb;
                break;
            }
        }
        outCur += cairo_row_stride;
    };
    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);

    /* XXX: this is only RGB case, for grayscale and other colorspaces... */
    _cairo_jpeg_surface = cairo_image_surface_create_for_data(outBuff, CAIRO_FORMAT_RGB24, cinfo.output_width, cinfo.output_height, cairo_row_stride);

    /* XXX: transfer ownership of pixmap buffer from this caller to the surface,
     * and let the surface free it when it is being destroyed.
     */
    cairo_surface_set_mime_data( _cairo_jpeg_surface, "image/x-pixmap", outBuff, outLimit, free, (void*)outBuff );

    return _cairo_jpeg_surface;
}

/* PNG (image/png)
 *
 * http://www.w3.org/TR/2003/REC-PNG-20031110/
 */

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
