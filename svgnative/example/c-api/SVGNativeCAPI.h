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

int appendHive();
int removeHive();

int installRendererToHive(int, render_t);
render_t getRendererType(int);
int installOutputToHive(int, void*);
int installDocumentToHive(int, char*);
void renderDocumentInHive(int);
long getWidthFromDocumentInHive(int);
long getHeightFromDocumentInHive(int);

#ifdef __cplusplus
};
#endif

