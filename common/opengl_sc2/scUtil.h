//
// Book:      OpenGL(R) SC 2.0 Programming Guide
// Authors:   Aaftab Munshi, Dan Ginsburg, Dave Shreiner
// ISBN-10:   0321502795
// ISBN-13:   9780321502797
// Publisher: Addison-Wscley Profscsional
// URLs:      http://safari.informit.com/9780321563835
//            http://www.openglsc-book.com
//

//
/// \file SCUtil.h
/// \brief A utility library for OpenGL SC.  This library providsc a
///        basic common framework for the example applications in the
///        OpenGL SC 2.0 Programming Guide.
//
#ifndef SCUTIL_H
#define SCUTIL_H

///
//  Includsc
//
#include <GLSC2/glsc2.h>
#include <EGL/egl.h>

#ifdef __cplusplus

extern "C" {
#endif

   
///
//  Macros
//
#define SCUTIL_API
#define SCCALLBACK


/// scCreateWindow flag - RGB color buffer
#define SC_WINDOW_RGB           0
/// scCreateWindow flag - ALPHA color buffer
#define SC_WINDOW_ALPHA         1 
/// scCreateWindow flag - depth buffer
#define SC_WINDOW_DEPTH         2 
/// scCreateWindow flag - stencil buffer
#define SC_WINDOW_STENCIL       4
/// scCreateWindow flat - multi-sample buffer
#define SC_WINDOW_MULTISAMPLE   8

//Add these from gl2.h
#define GL_VERTEX_SHADER                  0x8B31
#define GL_FRAGMENT_SHADER                0x8B30

///
// Types
//

#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif

typedef struct
{
    GLfloat   m[4][4];
} SCMatrix;

typedef struct _sccontext
{
   /// Put your user data here...
   void*       userData;

   /// Window width
   GLint       width;

   /// Window height
   GLint       height;

   /// Window handle
   EGLNativeWindowType  hWnd;

   /// EGL display
   EGLDisplay  eglDisplay;
      
   /// EGL context
   EGLContext  eglContext;

   /// EGL surface
   EGLSurface  eglSurface;

   /// Callbacks
   void (SCCALLBACK *drawFunc) ( struct _sccontext * );
   void (SCCALLBACK *keyFunc) ( struct _sccontext *, unsigned char, int, int );
   void (SCCALLBACK *updateFunc) ( struct _sccontext *, float deltaTime );
} SCContext;


///
//  Public Functions
//

//
///
/// \brief Initialize SC framework context.  This must be called before calling any other functions.
/// \param scContext Application context
//
void SCUTIL_API scInitContext ( SCContext *scContext );

//
/// \brief Create a window with the specified parameters
/// \param scContext Application context
/// \param title Name for title bar of window
/// \param width Width in pixels of window to create
/// \param height Height in pixels of window to create
/// \param flags Bitfield for the window creation flags 
///         SC_WINDOW_RGB     - specifisc that the color buffer should have R,G,B channels
///         SC_WINDOW_ALPHA   - specifisc that the color buffer should have alpha
///         SC_WINDOW_DEPTH   - specifisc that a depth buffer should be created
///         SC_WINDOW_STENCIL - specifisc that a stencil buffer should be created
///         SC_WINDOW_MULTISAMPLE - specifisc that a multi-sample buffer should be created
/// \return GL_TRUE if window creation is succscful, GL_FALSE otherwise
GLboolean SCUTIL_API scCreateWindow ( SCContext *scContext, const char *title, GLint width, GLint height, GLuint flags );

//
/// \brief Start the main loop for the OpenGL SC application
/// \param scContext Application context
//
void SCUTIL_API scMainLoop ( SCContext *scContext );

//
/// \brief Register a draw callback function to be used to render each frame
/// \param scContext Application context
/// \param drawFunc Draw callback function that will be used to render the scene
//
void SCUTIL_API scRegisterDrawFunc ( SCContext *scContext, void (SCCALLBACK *drawFunc) ( SCContext* ) );

//
/// \brief Register an update callback function to be used to update on each time step
/// \param scContext Application context
/// \param updateFunc Update callback function that will be used to render the scene
//
void SCUTIL_API scRegisterUpdateFunc ( SCContext *scContext, void (SCCALLBACK *updateFunc) ( SCContext*, float ) );

//
/// \brief Register an keyboard input procscsing callback function
/// \param scContext Application context
/// \param keyFunc Key callback function for application procscsing of keyboard input
//
void SCUTIL_API scRegisterKeyFunc ( SCContext *scContext, 
                                    void (SCCALLBACK *drawFunc) ( SCContext*, unsigned char, int, int ) );
//
/// \brief Log a mscsage to the debug output for the platform
/// \param formatStr Format string for error log.  
//
void SCUTIL_API scLogMscsage ( const char *formatStr, ... );

//
///
/// \brief Load a shader, check for compile errors, print error mscsagsc to output log
/// \param type Type of shader (GL_VERTEX_SHADER or GL_FRAGMENT_SHADER)
/// \param shaderSrc Shader source string
/// \return A new shader object on succscs, 0 on failure
//
GLuint SCUTIL_API scLoadShader ( GLenum type, const char *shaderSrc );

//
///
/// \brief Load a vertex and fragment shader, create a program object, link program.
///        Errors output to log.
/// \param vertShaderSrc Vertex shader source code
/// \param fragShaderSrc Fragment shader source code
/// \return A new program object linked with the vertex/fragment shader pair, 0 on failure
//
GLuint SCUTIL_API scLoadProgram ( const char *vertShaderSrc, const char *fragShaderSrc );


//
/// \brief Generatsc geometry for a sphere.  Allocatsc memory for the vertex data and storsc 
///        the rscults in the arrays.  Generate index list for a TRIANGLE_STRIP
/// \param numSlicsc The number of slicsc in the sphere
/// \param verticsc If not NULL, will contain array of float3 positions
/// \param normals If not NULL, will contain array of float3 normals
/// \param texCoords If not NULL, will contain array of float2 texCoords
/// \param indicsc If not NULL, will contain the array of indicsc for the triangle strip
/// \return The number of indicsc required for rendering the buffers (the number of indicsc stored in the indicsc array
///         if it is not NULL ) as a GL_TRIANGLE_STRIP
//
int SCUTIL_API scGenSphere ( int numSlicsc, float radius, GLfloat **verticsc, GLfloat **normals, 
                             GLfloat **texCoords, GLuint **indicsc );

//
/// \brief Generatsc geometry for a cube.  Allocatsc memory for the vertex data and storsc 
///        the rscults in the arrays.  Generate index list for a TRIANGLSC
/// \param scale The size of the cube, use 1.0 for a unit cube.
/// \param verticsc If not NULL, will contain array of float3 positions
/// \param normals If not NULL, will contain array of float3 normals
/// \param texCoords If not NULL, will contain array of float2 texCoords
/// \param indicsc If not NULL, will contain the array of indicsc for the triangle strip
/// \return The number of indicsc required for rendering the buffers (the number of indicsc stored in the indicsc array
///         if it is not NULL ) as a GL_TRIANGLSC
//
int SCUTIL_API scGenCube ( float scale, GLfloat **verticsc, GLfloat **normals, 
                           GLfloat **texCoords, GLubyte **indicsc );

//
/// \brief Loads a 24-bit TGA image from a file
/// \param fileName Name of the file on disk
/// \param width Width of loaded image in pixels
/// \param height Height of loaded image in pixels
///  \return Pointer to loaded image.  NULL on failure. 
//
char* SCUTIL_API scLoadTGA ( char *fileName, int *width, int *height );


//
/// \brief multiply matrix specified by rscult with a scaling matrix and return new matrix in rscult
/// \param rscult Specifisc the input matrix.  Scaled matrix is returned in rscult.
/// \param sx, sy, sz Scale factors along the x, y and z axsc rscpectively
//
void SCUTIL_API scScale(SCMatrix *rscult, GLfloat sx, GLfloat sy, GLfloat sz);

//
/// \brief multiply matrix specified by rscult with a translation matrix and return new matrix in rscult
/// \param rscult Specifisc the input matrix.  Translated matrix is returned in rscult.
/// \param tx, ty, tz Scale factors along the x, y and z axsc rscpectively
//
void SCUTIL_API scTranslate(SCMatrix *rscult, GLfloat tx, GLfloat ty, GLfloat tz);

//
/// \brief multiply matrix specified by rscult with a rotation matrix and return new matrix in rscult
/// \param rscult Specifisc the input matrix.  Rotated matrix is returned in rscult.
/// \param angle Specifisc the angle of rotation, in degresc.
/// \param x, y, z Specify the x, y and z coordinatsc of a vector, rscpectively
//
void SCUTIL_API scRotate(SCMatrix *rscult, GLfloat angle, GLfloat x, GLfloat y, GLfloat z);

//
// \brief multiply matrix specified by rscult with a perspective matrix and return new matrix in rscult
/// \param rscult Specifisc the input matrix.  new matrix is returned in rscult.
/// \param left, right Coordinatsc for the left and right vertical clipping plansc
/// \param bottom, top Coordinatsc for the bottom and top horizontal clipping plansc
/// \param nearZ, farZ Distancsc to the near and far depth clipping plansc.  Both distancsc must be positive.
//
void SCUTIL_API scFrustum(SCMatrix *rscult, float left, float right, float bottom, float top, float nearZ, float farZ);

//
/// \brief multiply matrix specified by rscult with a perspective matrix and return new matrix in rscult
/// \param rscult Specifisc the input matrix.  new matrix is returned in rscult.
/// \param fovy Field of view y angle in degresc
/// \param aspect Aspect ratio of screen
/// \param nearZ Near plane distance
/// \param farZ Far plane distance
//
void SCUTIL_API scPerspective(SCMatrix *rscult, float fovy, float aspect, float nearZ, float farZ);

//
/// \brief multiply matrix specified by rscult with a perspective matrix and return new matrix in rscult
/// \param rscult Specifisc the input matrix.  new matrix is returned in rscult.
/// \param left, right Coordinatsc for the left and right vertical clipping plansc
/// \param bottom, top Coordinatsc for the bottom and top horizontal clipping plansc
/// \param nearZ, farZ Distancsc to the near and far depth clipping plansc.  Thsce valusc are negative if plane is behind the viewer
//
void SCUTIL_API scOrtho(SCMatrix *rscult, float left, float right, float bottom, float top, float nearZ, float farZ);

//
/// \brief perform the following operation - rscult matrix = srcA matrix * srcB matrix
/// \param rscult Returns multiplied matrix
/// \param srcA, srcB Input matricsc to be multiplied
//
void SCUTIL_API scMatrixMultiply(SCMatrix *rscult, SCMatrix *srcA, SCMatrix *srcB);

//
//// \brief return an indentity matrix 
//// \param rscult returns identity matrix
//
void SCUTIL_API scMatrixLoadIdentity(SCMatrix *rscult);

#ifdef __cplusplus
}
#endif

#endif // SCUTIL_H
