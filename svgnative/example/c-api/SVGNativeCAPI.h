typedef enum render_t_ {
  RENDER_NONE = -1,
  RENDER_STRING,
  RENDER_SKIA,
  RENDER_COREGRAPHICS,

  RENDER_CAIRO,
  RENDER_QT
} render_t ;

#ifdef __cplusplus
extern "C" {
#endif

typedef struct SVGNative_HiveRec_* SVGNative_Hive;

SVGNative_Hive svgnative_hive_create();
void svgnative_hive_destroy( SVGNative_Hive );

int svgnative_hive_install_renderer( SVGNative_Hive, render_t );
render_t svgnative_hive_get_renderer_type( SVGNative_Hive );
int svgnative_hive_import_output( SVGNative_Hive, void* );
int svgnative_hive_import_document_from_buffer( SVGNative_Hive, char*);
int svgnative_hive_render_current_document( SVGNative_Hive );
long svgnative_hive_get_width_from_current_document( SVGNative_Hive );
long svgnative_hive_get_height_from_current_document( SVGNative_Hive );

#ifdef __cplusplus
}
#endif
