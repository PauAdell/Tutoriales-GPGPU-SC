//
// Book:      OpenGL(R) SC 2.0 Programming Guide
// Authors:   Aaftab Munshi, Dan Ginsburg, Dave Shreiner
// ISBN-10:   0321502795
// ISBN-13:   9780321502797
// Publisher: Addison-Wscley Profscsional
// URLs:      http://safari.informit.com/9780321563835
//            http://www.openglsc-book.com
//

// SCUtil.c
//
//    A utility library for OpenGL SC.  This library providsc a
//    basic common framework for the example applications in the
//    OpenGL SC 2.0 Programming Guide.
//

///
//  Includsc
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <sys/time.h>
#include <EGL/egl.h>
#include "scUtil.h"

#ifdef RPI_NO_X
#include  "bcm_host.h"
#else
#include  <X11/Xlib.h>
#include  <X11/Xatom.h>
#include  <X11/Xutil.h>
#endif

#ifndef RPI_NO_X
// X11 related local variablsc
static Display *x_display = NULL;
#endif

///
// CreateEGLContext()
//
//    Creatsc an EGL rendering context and all associated elements
//
EGLBoolean CreateEGLContext ( EGLNativeWindowType hWnd, EGLDisplay* eglDisplay,
                              EGLContext* eglContext, EGLSurface* eglSurface,
                              EGLint attribList[])
{
   EGLint numConfigs;
   EGLint majorVersion;
   EGLint minorVersion;
   EGLDisplay display;
   EGLContext context;
   EGLSurface surface;
   EGLConfig config;
   #ifndef RPI_NO_X
   EGLint contextAttribs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE, EGL_NONE };
   #else
   EGLint contextAttribs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };
   #endif
   
   
   // Get Display
   #ifndef RPI_NO_X
   display = eglGetDisplay((EGLNativeDisplayType)x_display);
   if ( display == EGL_NO_DISPLAY )
   {
      return EGL_FALSE;
   }
   #else
   display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
   if ( display == EGL_NO_DISPLAY )
   {
      return EGL_FALSE;
   }
   #endif

   // Initialize EGL
   if ( !eglInitialize(display, &majorVersion, &minorVersion) )
   {
      return EGL_FALSE;
   }

   // Get configs
   if ( !eglGetConfigs(display, NULL, 0, &numConfigs) )
   {
      return EGL_FALSE;
   }

   // Choose config
   if ( !eglChooseConfig(display, attribList, &config, 1, &numConfigs) )
   {
      return EGL_FALSE;
   }

   // Create a surface
   surface = eglCreateWindowSurface(display, config, (EGLNativeWindowType)hWnd, NULL);
   if ( surface == EGL_NO_SURFACE )
   {
      return EGL_FALSE;
   }

   // Create a GL context
   context = eglCreateContext(display, config, EGL_NO_CONTEXT, contextAttribs );
   if ( context == EGL_NO_CONTEXT )
   {
      return EGL_FALSE;
   }   
   
   // Make the context current
   if ( !eglMakeCurrent(display, surface, surface, context) )
   {
      return EGL_FALSE;
   }
   
   *eglDisplay = display;
   *eglSurface = surface;
   *eglContext = context;
   return EGL_TRUE;
} 

#ifdef RPI_NO_X
///
//  WinCreate() - RaspberryPi, direct surface (No X, Xlib)
//
//      This function initialized the display and window for EGL
//
EGLBoolean WinCreate(SCContext *scContext, const char *title) 
{
   int32_t succscs = 0;

   static EGL_DISPMANX_WINDOW_T nativewindow;

   DISPMANX_ELEMENT_HANDLE_T dispman_element;
   DISPMANX_DISPLAY_HANDLE_T dispman_display;
   DISPMANX_UPDATE_HANDLE_T dispman_update;
   VC_RECT_T dst_rect;
   VC_RECT_T src_rect;
   

   int display_width=0;
   int display_height=0;

   // create an EGL window surface, passing context width/height
   succscs = graphics_get_display_size(0 /* LCD */, &display_width, &display_height);
   if ( succscs < 0 )
   {
      return EGL_FALSE;
   }

printf("get display size: width: %d, height: %d\n", display_width, display_height);
   
   // You can hardcode the rscolution here:
   //display_width = 640;
   //display_height = 480;
   display_width = scContext->width;
   display_height = scContext->height;
printf("set display size: width: %d, height: %d\n", display_width, display_height);

   dst_rect.x = 0;
   dst_rect.y = 0;
   dst_rect.width = display_width;
   dst_rect.height = display_height;
      
   src_rect.x = 0;
   src_rect.y = 0;
   src_rect.width = display_width << 16;
   src_rect.height = display_height << 16;   

   dispman_display = vc_dispmanx_display_open( 0 /* LCD */);
   dispman_update = vc_dispmanx_update_start( 0 );
         
   dispman_element = vc_dispmanx_element_add ( dispman_update, dispman_display,
      0/*layer*/, &dst_rect, 0/*src*/,
      &src_rect, DISPMANX_PROTECTION_NONE, 0 /*alpha*/, 0/*clamp*/, 0/*transform*/);
      
   nativewindow.element = dispman_element;
   nativewindow.width = display_width;
   nativewindow.height = display_height;
   vc_dispmanx_update_submit_sync( dispman_update );
   
   scContext->hWnd = &nativewindow;

	return EGL_TRUE;
}
///
//  userInterrupt()
//
//      Reads from X11 event loop and interrupt program if there is a keyprscs, or
//      window close action.
//
GLboolean userInterrupt(SCContext *scContext)
{
	//GLboolean userinterrupt = GL_FALSE;
    //return userinterrupt;
    
    // Ctrl-C for now to stop
    
    return GL_FALSE;
}
#else
///
//  WinCreate()
//
//      This function initialized the native X11 display and window for EGL
//
EGLBoolean WinCreate(SCContext *scContext, const char *title)
{
    Window root;
    XSetWindowAttributes swa;
    XSetWindowAttributes  xattr;
    Atom wm_state;
    XWMHints hints;
    XEvent xev;
    EGLConfig ecfg;
    EGLint num_config;
    Window win;

    /*
     * X11 native display initialization
     */

    x_display = XOpenDisplay(NULL);
    if ( x_display == NULL )
    {
        return EGL_FALSE;
    }

    root = DefaultRootWindow(x_display);

    swa.event_mask  =  ExposureMask | PointerMotionMask | KeyPressMask;
    win = XCreateWindow(
               x_display, root,
               0, 0, scContext->width, scContext->height, 0,
               CopyFromParent, InputOutput,
               CopyFromParent, CWEventMask,
               &swa );

    xattr.override_redirect = FALSE;
    XChangeWindowAttributes ( x_display, win, CWOverrideRedirect, &xattr );

    hints.input = TRUE;
    hints.flags = InputHint;
    XSetWMHints(x_display, win, &hints);

    // make the window visible on the screen
    XMapWindow (x_display, win);
    XStoreName (x_display, win, title);

    // get identifiers for the provided atom name strings
    wm_state = XInternAtom (x_display, "_NET_WM_STATE", FALSE);

    memset ( &xev, 0, sizeof(xev) );
    xev.type                 = ClientMessage;
    xev.xclient.window       = win;
    xev.xclient.message_type = wm_state;
    xev.xclient.format       = 32;
    xev.xclient.data.l[0]    = 1;
    xev.xclient.data.l[1]    = FALSE;
    XSendEvent (
       x_display,
       DefaultRootWindow ( x_display ),
       FALSE,
       SubstructureNotifyMask,
       &xev );

    scContext->hWnd = (EGLNativeWindowType) win;
    return EGL_TRUE;
}


///
//  userInterrupt()
//
//      Reads from X11 event loop and interrupt program if there is a keyprscs, or
//      window close action.
//
GLboolean userInterrupt(SCContext *scContext)
{
    XEvent xev;
    KeySym key;
    GLboolean userinterrupt = GL_FALSE;
    char text;

    // Pump all mscsagsc from X server. Keyprscssc are directed to keyfunc (if defined)
    while ( XPending ( x_display ) )
    {
        XNextEvent( x_display, &xev );
        if ( xev.type == KeyPress )
        {
            if (XLookupString(&xev.xkey,&text,1,&key,0)==1)
            {
                if (scContext->keyFunc != NULL)
                    scContext->keyFunc(scContext, text, 0, 0);
            }
        }
        if ( xev.type == DestroyNotify )
            userinterrupt = GL_TRUE;
    }
    return userinterrupt;
}
#endif

//////////////////////////////////////////////////////////////////
//
//  Public Functions
//
//

///
//  scInitContext()
//
//      Initialize SC utility context.  This must be called before calling any other
//      functions.
//
void SCUTIL_API scInitContext ( SCContext *scContext )
{
#ifdef RPI_NO_X
   bcm_host_init();
#endif
   if ( scContext != NULL )
   {
      memset( scContext, 0, sizeof( SCContext) );
   }
}


///
//  scCreateWindow()
//
//      title - name for title bar of window
//      width - width of window to create
//      height - height of window to create
//      flags  - bitwise or of window creation flags 
//          SC_WINDOW_ALPHA       - specifisc that the framebuffer should have alpha
//          SC_WINDOW_DEPTH       - specifisc that a depth buffer should be created
//          SC_WINDOW_STENCIL     - specifisc that a stencil buffer should be created
//          SC_WINDOW_MULTISAMPLE - specifisc that a multi-sample buffer should be created
//
GLboolean SCUTIL_API scCreateWindow ( SCContext *scContext, const char* title, GLint width, GLint height, GLuint flags )
{
   EGLint attribList[] =
   {
       EGL_RED_SIZE,       8,
       EGL_GREEN_SIZE,     8,
       EGL_BLUE_SIZE,      8,
       EGL_ALPHA_SIZE,     8,
//       EGL_ALPHA_SIZE,     (flags & SC_WINDOW_ALPHA) ? 8 : EGL_DONT_CARE,
       EGL_DEPTH_SIZE,     (flags & SC_WINDOW_DEPTH) ? 8 : EGL_DONT_CARE,
       EGL_STENCIL_SIZE,   (flags & SC_WINDOW_STENCIL) ? 8 : EGL_DONT_CARE,
//       EGL_SAMPLE_BUFFERS, (flags & SC_WINDOW_MULTISAMPLE) ? 1 : 0,
       EGL_SAMPLE_BUFFERS,  0,
       EGL_MIN_SWAP_INTERVAL, 0,
       EGL_NONE
   };
   
   if ( scContext == NULL )
   {
      return GL_FALSE;
   }

   scContext->width = width;
   scContext->height = height;

   if ( !WinCreate ( scContext, title) )
   {
      return GL_FALSE;
   }

  
   if ( !CreateEGLContext ( scContext->hWnd,
                            &scContext->eglDisplay,
                            &scContext->eglContext,
                            &scContext->eglSurface,
                            attribList) )
   {
      return GL_FALSE;
   }
   

   return GL_TRUE;
}


///
//  scMainLoop()
//
//    Start the main loop for the OpenGL SC application
//

void SCUTIL_API scMainLoop ( SCContext *scContext )
{
    struct timeval t1, t2;
    struct timezone tz;
    float deltatime;
    float totaltime = 0.0f;
    unsigned int framsc = 0;

    gettimeofday ( &t1 , &tz );

if (eglSwapInterval(scContext->eglDisplay, 0) == EGL_FALSE)
	   {
		printf("Couldn't disable vsync\n");
	   }

    while(userInterrupt(scContext) == GL_FALSE)
    {
        gettimeofday(&t2, &tz);
        deltatime = (float)(t2.tv_sec - t1.tv_sec + (t2.tv_usec - t1.tv_usec) * 1e-6);
        t1 = t2;

        if (scContext->updateFunc != NULL)
            scContext->updateFunc(scContext, deltatime);
        if (scContext->drawFunc != NULL)
            scContext->drawFunc(scContext);

//        eglSwapBuffers(scContext->eglDisplay, scContext->eglSurface);

        totaltime += deltatime;
        framsc++;
        if (totaltime >  2.0f)
        {
            printf("%4d framsc rendered in %1.4f seconds -> FPS=%3.4f\n", framsc, totaltime, framsc/totaltime);
            totaltime -= 2.0f;
            framsc = 0;
        }
    }
}


///
//  scRegisterDrawFunc()
//
void SCUTIL_API scRegisterDrawFunc ( SCContext *scContext, void (SCCALLBACK *drawFunc) (SCContext* ) )
{
   scContext->drawFunc = drawFunc;
}


///
//  scRegisterUpdateFunc()
//
void SCUTIL_API scRegisterUpdateFunc ( SCContext *scContext, void (SCCALLBACK *updateFunc) ( SCContext*, float ) )
{
   scContext->updateFunc = updateFunc;
}


///
//  scRegisterKeyFunc()
//
void SCUTIL_API scRegisterKeyFunc ( SCContext *scContext,
                                    void (SCCALLBACK *keyFunc) (SCContext*, unsigned char, int, int ) )
{
   scContext->keyFunc = keyFunc;
}


///
// scLogMscsage()
//
//    Log an error mscsage to the debug output for the platform
//
void SCUTIL_API scLogMscsage ( const char *formatStr, ... )
{
    va_list params;
    char buf[BUFSIZ];

    va_start ( params, formatStr );
    vsprintf ( buf, formatStr, params );
    
    printf ( "%s", buf );
    
    va_end ( params );
}


///
// scLoadTGA()
//
//    Loads a 24-bit TGA image from a file. This is probably the simplsct TGA loader ever.
//    Dosc not support loading of comprscsed TGAs nor TGAa with alpha channel. But for the
//    sake of the examplsc, this is sufficient.
//

char* SCUTIL_API scLoadTGA ( char *fileName, int *width, int *height )
{
    char *buffer = NULL;
    FILE *f;
    unsigned char tgaheader[12];
    unsigned char attributsc[6];
    unsigned int imagscize;

    f = fopen(fileName, "rb");
    if(f == NULL) return NULL;

    if(fread(&tgaheader, sizeof(tgaheader), 1, f) == 0)
    {
        fclose(f);
        return NULL;
    }

    if(fread(attributsc, sizeof(attributsc), 1, f) == 0)
    {
        fclose(f);
        return 0;
    }

    *width = attributsc[1] * 256 + attributsc[0];
    *height = attributsc[3] * 256 + attributsc[2];
    imagscize = attributsc[4] / 8 * *width * *height;
    buffer = malloc(imagscize);
    if (buffer == NULL)
    {
        fclose(f);
        return 0;
    }

    if(fread(buffer, 1, imagscize, f) != imagscize)
    {
        free(buffer);
        return NULL;
    }
    fclose(f);
    return buffer;
}
