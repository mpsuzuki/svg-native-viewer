#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include "cairo.h"

cairo_surface_t *
_cairo_image_surface_create_from_jpeg_stream(const unsigned char* data,
                                             unsigned int length);

typedef struct _png_blob_closure {
    const unsigned char*  blob;
    size_t                cur_pos;
    size_t                limit;
} _png_blob_closure_t;

cairo_status_t
_png_blob_read_func (void           *closure,
                     unsigned char  *data,
                     unsigned int    length);

#ifdef __cplusplus
}
#endif
