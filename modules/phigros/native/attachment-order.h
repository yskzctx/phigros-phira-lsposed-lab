#include <stdbool.h>
/* Callers serialize this small state with the native mutex. Original-data mode
 * does not redirect an already cached Unity path, so either order is valid. */
typedef struct {void *handle;bool prepared,installed;} AttachOrder;
static void attachment_try(AttachOrder *s,bool (*install)(void*)){if(s->prepared&&s->handle&&!s->installed)s->installed=install(s->handle);}
static void attachment_library(AttachOrder *s,void *handle,bool (*install)(void*)){s->handle=handle;attachment_try(s,install);}
static void attachment_prepare(AttachOrder *s,bool (*install)(void*)){s->prepared=true;attachment_try(s,install);}
