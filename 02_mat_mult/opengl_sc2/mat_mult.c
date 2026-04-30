#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <sys/time.h>
#include <math.h>
#include <GLES2/gl2.h>
#include "scUtil.h"

#define BENCHMARKING
#define COMPILE_SHADER

#ifndef SIZE
#define SIZE 200
#endif

#define MATRIX_SIZE SIZE
#define LOOP_SIZE SIZE

#define xstr(s) str(s)
#define str(s) #s

unsigned char *out_data;
unsigned char *texture_d;
GLuint texture[2];
GLuint texture_target;
GLuint FBO;

double t_start, t_setup_end, t_op_end;

typedef struct {
   GLuint programObject;
} UserData;

double get_time() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (double)tv.tv_sec + (double)tv.tv_usec * 1e-6;
}

// from IEEE-754 float to RGBA8 (Identical to ES2 version)
void encode_float(float val, unsigned char* rgba) {
    if (val == 0.0f) { rgba[0]=rgba[1]=rgba[2]=rgba[3]=0; return; }
    int sign = (val < 0);
    val = fabsf(val);
    int exponent = (int)floorf(log2f(val));
    float mantissa = val / powf(2.0f, (float)exponent) - 1.0f;
    rgba[3] = (unsigned char)(exponent + 127);
    rgba[2] = (unsigned char)((sign << 7) | (int)(mantissa * 128.0f));
    rgba[1] = (unsigned char)(fmodf(mantissa * 128.0f * 256.0f, 256.0f));
    rgba[0] = (unsigned char)(fmodf(mantissa * 128.0f * 256.0f * 256.0f, 256.0f));
}

float decode_float(unsigned char* rgba) {
    if (rgba[0]==0 && rgba[1]==0 && rgba[2]==0 && rgba[3]==0) return 0.0f;
    int exponent = rgba[3] - 127;
    int sign = (rgba[2] & 0x80) ? -1 : 1;
    float mantissa = 1.0f + (float)(rgba[2] & 0x7F) / 128.0f + 
                     (float)rgba[1] / (128.0f * 256.0f) + 
                     (float)rgba[0] / (128.0f * 256.0f * 256.0f);
    return (float)sign * mantissa * powf(2.0f, (float)exponent);
}

void verify_results(unsigned char* pixels) {
    printf("Result Matrix (Second row):\n");
    for(int j=0; j<SIZE; j++) {
        printf("%g ", decode_float(&pixels[(1*SIZE+j)*4]));
    }
    printf("\n");

    printf("\nVerifying results against CPU reference...\n");
    int passed = 1;
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            float gpuVal = decode_float(&pixels[(i * SIZE + j) * 4]);
            
            // Reference: C[i][j] = sum_k(A[i][k] * B[k][j]) = sum_k(i * j) = SIZE * i * j
            float cpuRef = (float)i * (float)j * (float)SIZE;
            if (fabsf(gpuVal - cpuRef) > 0.5f || isnan(gpuVal)) {
                printf("Verification FAILED at (%d,%d): Expected %.4f, got %.4f\n", j, i, cpuRef, gpuVal);
                passed = 0;
                goto end_verify;
            }
        }
    }
    end_verify:
    if (passed) printf("VALIDATION: PASSED\n");
    else printf("VALIDATION: FAILED\n");
}

GLuint LoadShader ( GLenum type, const char *shaderSrc )
{
#ifdef COMPILE_SHADER
   GLuint shader = glCreateShader ( type );
   if ( shader == 0 ) return 0;
   glShaderSource ( shader, 1, &shaderSrc, NULL );
   glCompileShader ( shader );
   GLint compiled;
   glGetShaderiv ( shader, GL_COMPILE_STATUS, &compiled );
   if ( !compiled ) {
      GLint infoLen = 0;
      glGetShaderiv ( shader, GL_INFO_LOG_LENGTH, &infoLen );
      if ( infoLen > 1 ) {
         char* infoLog = malloc (infoLen);
         glGetShaderInfoLog ( shader, infoLen, NULL, infoLog );
         scLogMscsage ( "Error compiling shader:\n%s\n", infoLog );            
         free ( infoLog );
      }
      glDeleteShader ( shader );
      return 0;
   }
   return shader;
#else
   return 0;
#endif
}

int Init ( SCContext *scContext )
{
   UserData *userData = (UserData*) scContext->userData;

   const char vShaderStr[] = "attribute vec4 vPosition; void main() { gl_Position = vPosition; }";
   const char fShaderStr[] =  
      "precision highp float;\n"
      "uniform highp sampler2D textureUnit0;\n"
      "uniform highp sampler2D textureUnit1;\n"
      "#define MATRIX_SIZE " xstr(SIZE) ".0\n"
      "#define LOOP_SIZE " xstr(SIZE) "\n"
      "#define decode_exact(reconstructed, textureUnit, texCoord) { \\\n"
      "    highp vec4 u_split = texture2D(textureUnit, texCoord); \\\n"
      "    highp float tmp; \\\n"
      "    highp float sign_value = 1.0; \\\n"
      "    tmp = floor(256.0 * u_split.w - (u_split.w / 255.0)); \\\n"
      "    highp float exponent = tmp - 127.0; \\\n"
      "    tmp = floor(256.0 * u_split.z - (u_split.z / 255.0)); \\\n"
      "    reconstructed = (tmp * 0.0078125); \\\n"
      "    if (exponent >= -126.0 && reconstructed < 1.0) reconstructed += 1.0; \\\n"
      "    if (tmp > 127.0) sign_value = -1.0; \\\n"
      "    tmp = floor(256.0 * u_split.y - (u_split.y / 255.0)); \\\n"
      "    reconstructed += (tmp * 0.000030517578125); \\\n"
      "    tmp = floor(256.0 * u_split.x - (u_split.x / 255.0)); \\\n"
      "    reconstructed += (tmp * 0.00000011920928955078); \\\n"
      "    reconstructed = sign_value * reconstructed * exp2(exponent); \\\n"
      "}\n"
      "#define encode_exact(reconstructed, fragColor) { \\\n"
      "    highp float sign_value = 1.0; \\\n"
      "    highp float exponent; \\\n"
      "    highp float tmp; \\\n"
      "    highp vec4 u_split; \\\n"
      "    exponent = (floor(log2(abs(reconstructed))) + 127.0) * step(exp2(-125.0), abs(reconstructed)); \\\n"
      "    u_split.w = ((exponent - 256.0 * floor(exponent * 0.00390625)) * 0.00392156862745098); \\\n"
      "    tmp = clamp(abs(reconstructed * exp2(-floor(log2(abs(reconstructed))))) - 1.0, 0.0, 1.0); \\\n"
      "    tmp = tmp * exp2(23.0); \\\n"
      "    if (reconstructed < 0.0) sign_value = exp2(23.0); \\\n"
      "    u_split.z = (floor(((tmp + sign_value) - 256.0 * 256.0 * 256.0 * floor((tmp + sign_value) * 0.00000005960464477539)) * 1.52587890625e-05) * 0.00392156862745098); \\\n"
      "    u_split.y = (floor((tmp - 256.0 * 256.0 * floor(tmp * 1.52587890625e-05)) * 0.00390625) * 0.00392156862745098); \\\n"
      "    u_split.x = ((tmp - 256.0 * floor(tmp * 0.00390625)) * 0.00392156862745098); \\\n"
      "    fragColor = u_split; \\\n"
      "}\n"
      "void main() {\n"
      "    highp float row = (floor(gl_FragCoord.y) + 0.5) / MATRIX_SIZE;\n"
      "    highp float col = (floor(gl_FragCoord.x) + 0.5) / MATRIX_SIZE;\n"
      "    highp float sum = 0.0;\n"
      "    for (int k = 0; k < LOOP_SIZE; k++) {\n"
      "        highp float k_coord = (float(k) + 0.5) / MATRIX_SIZE;\n"
      "        highp float a, b;\n"
      "        decode_exact(a, textureUnit0, vec2(k_coord, row));\n"
      "        decode_exact(b, textureUnit1, vec2(col, k_coord));\n"
      "        sum += a * b;\n"
      "    }\n"
      "    encode_exact(sum, gl_FragColor);\n"
      "}\n";

   GLuint vertexShader = LoadShader ( GL_VERTEX_SHADER, vShaderStr );
   GLuint fragmentShader = LoadShader ( GL_FRAGMENT_SHADER, fShaderStr );
   GLuint programObject = glCreateProgram();
   glAttachShader(programObject, vertexShader);
   glAttachShader(programObject, fragmentShader);
   glBindAttribLocation(programObject, 0, "vPosition");
   glLinkProgram(programObject);
   userData->programObject = programObject;

   glGenTextures(2, texture);
   for(int i=0; i<2; i++) {
       for(int r=0; r < SIZE; r++) {
           for(int c=0; c < SIZE; c++) {
               encode_float((i==0 ? (float)r : (float)c), &texture_d[(r * SIZE + c) * 4]);
           }
       }
       glBindTexture(GL_TEXTURE_2D, texture[i]);
       glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
       glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
       glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
       glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
       glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, SIZE, SIZE);
       glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, SIZE, SIZE, GL_RGBA, GL_UNSIGNED_BYTE, texture_d);
   }

   glGenTextures(1, &texture_target);
   glBindTexture(GL_TEXTURE_2D, texture_target);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
   glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, SIZE, SIZE);

   glGenFramebuffers(1, &FBO);
   glBindFramebuffer(GL_FRAMEBUFFER, FBO);
   glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture_target, 0);

   return (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
}

void Draw ( SCContext *scContext )
{
   static int first = 1;
   if (first) { t_setup_end = get_time(); first = 0; }
   UserData *userData = (UserData*) scContext->userData;
   GLfloat vVertices[] = { -1,1,0, -1,-1,0, 1,-1,0, 1,1,0 };
   GLushort indices[] = { 0, 1, 2, 0, 2, 3 };

   glBindFramebuffer(GL_FRAMEBUFFER, FBO);
   glViewport(0, 0, SIZE, SIZE);
   glClearColor(0,0,0,0);
   glClear(GL_COLOR_BUFFER_BIT);
   glUseProgram(userData->programObject);

   glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, texture[0]);
   glUniform1i(glGetUniformLocation(userData->programObject, "textureUnit0"), 0);
   glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D, texture[1]);
   glUniform1i(glGetUniformLocation(userData->programObject, "textureUnit1"), 1);

   glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, vVertices);
   glEnableVertexAttribArray(0);
   glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);
   glFinish();

   glReadPixels(0, 0, SIZE, SIZE, GL_RGBA, GL_UNSIGNED_BYTE, out_data);

   t_op_end = get_time();

   verify_results(out_data);

   printf("Setup time: %f s\n", t_setup_end - t_start);
   printf("Comp. time: %f s\n", t_op_end - t_setup_end);
   printf("Total time: %f s\n", t_op_end - t_start);
   exit(0);
}

int main ( int argc, char *argv[] )
{
   SCContext scContext; UserData userData;
   out_data = malloc(4*SIZE*SIZE); texture_d = malloc(4*SIZE*SIZE);
   t_start = get_time();
   scInitContext(&scContext); scContext.userData = &userData;
   scCreateWindow(&scContext, "GPGPU Matrix Mult SC2", SIZE, SIZE, SC_WINDOW_RGB|SC_WINDOW_ALPHA);
   if (!Init(&scContext)) return 0;
   scRegisterDrawFunc(&scContext, Draw);
   scMainLoop(&scContext);
   return 0;
}
