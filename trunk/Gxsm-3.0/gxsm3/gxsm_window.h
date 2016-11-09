#ifndef __GXSM3APPWIN_H
#define __GXSM3APPWIN_H

#include <gtk/gtk.h>
#include "gxsm_app.h"


#define GXSM3_APP_WINDOW_TYPE (gxsm3_app_window_get_type ())
#define GXSM3_APP_WINDOW(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), GXSM3_APP_WINDOW_TYPE, Gxsm3appWindow))


typedef struct _Gxsm3appWindow         Gxsm3appWindow;
typedef struct _Gxsm3appWindowClass    Gxsm3appWindowClass;


GType                 gxsm3_app_window_get_type     (void);
Gxsm3appWindow       *gxsm3_app_window_new          (Gxsm3app *app);
void                  gxsm3_app_window_open         (Gxsm3appWindow *win,
						     GFile            *file);


#endif /* __GXSM3APPWIN_H */
