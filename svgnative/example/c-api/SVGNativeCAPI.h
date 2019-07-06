typedef enum data_t_ {
  TYPE_USTRING,

  TYPE_INT32,
  TYPE_UINT32,

  TYPE_DOUBLE,

  TYPE_2_INT32, /* for vector, etc */
  TYPE_2_UINT32,
  TYPE_2_DOUBLE,

  TYPE_4_INT32, /* for rectangle, etc */
  TYPE_4_UINT32,
  TYPE_4_DOUBLE,

  TYPE_6_INT32, /* for matrix, etc */
  TYPE_6_UINT32,
  TYPE_6_DOUBLE

} data_t;

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
int svgnative_hive_install_output( SVGNative_Hive, void* );
int svgnative_hive_install_document_from_buffer( SVGNative_Hive, char*);
int svgnative_hive_render_installed_document( SVGNative_Hive );
long svgnative_hive_get_width_from_installed_document( SVGNative_Hive );
long svgnative_hive_get_height_from_installed_document( SVGNative_Hive );

#ifdef __cplusplus
};
#endif
