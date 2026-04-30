#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <sys/time.h>
#include <math.h>
#include "esUtil.h"

#ifndef SIZE
#define SIZE 200
#endif
#define BUF_SIZE (SIZE * SIZE * 4)

GLuint texture[2], texture_target, FBO;
unsigned char texture_data[BUF_SIZE];
unsigned char output_data[BUF_SIZE];

typedef struct { GLuint program; } UserData;

double t_start, t_setup_end, t_op_end;

double get_time() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (double)tv.tv_sec + (double)tv.tv_usec * 1e-6;
}

// from IEEE-754 float to RGBA8
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

// from RGBA8 to IEEE-754 float
float decode_float(unsigned char* rgba) {
    if (rgba[0]==0 && rgba[1]==0 && rgba[2]==0 && rgba[3]==0) return 0.0f;
    int exponent = rgba[3] - 127;
    int sign = (rgba[2] & 0x80) ? -1 : 1;
    float mantissa = 1.0f + (float)(rgba[2] & 0x7F) / 128.0f + 
                     (float)rgba[1] / (128.0f * 256.0f) + 
                     (float)rgba[0] / (128.0f * 256.0f * 256.0f);
    return (float)sign * mantissa * powf(2.0f, (float)exponent);
}

char* readShaderFile(const char* path) {
    FILE* f = fopen(path, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    fseek(f, 0, SEEK_SET);
    char* buf = malloc(len + 1);
    fread(buf, 1, len, f);
    buf[len] = '\0';
    fclose(f);
    return buf;
}

int Init(ESContext *esContext) {
    UserData *userData = malloc(sizeof(UserData));
    char* vSrc = readShaderFile("vShader.vert");
    char* fSrcBody = readShaderFile("fShader.frag");
    
    // Inject constants before compiling (needed for coord computation)
    char shaderDefine[128];
    sprintf(shaderDefine, "#define MATRIX_SIZE %.1f\n#define LOOP_SIZE %d\n", (float)SIZE, SIZE);

    char* fSrcFull = malloc(strlen(shaderDefine) + strlen(fSrcBody) + 1);
    strcpy(fSrcFull, shaderDefine);
    strcat(fSrcFull, fSrcBody);

    userData->program = esLoadProgram(vSrc, fSrcFull);
    
    free(vSrc); free(fSrcBody); free(fSrcFull);
    esContext->userData = userData;

    // Load two textures
    glGenTextures(2, texture);
    // Texture0 (row gradient)
    for(int r=0; r<SIZE; r++) {
        for(int c=0; c<SIZE; c++) {
            encode_float((float)r, &texture_data[(r*SIZE+c)*4]);
        }
    }
    glBindTexture(GL_TEXTURE_2D, texture[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SIZE, SIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture_data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Texture1 (column gradient)
    for(int r=0; r<SIZE; r++) {
        for(int c=0; c<SIZE; c++) {
            encode_float((float)c, &texture_data[(r*SIZE+c)*4]);
        }
    }
    glBindTexture(GL_TEXTURE_2D, texture[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SIZE, SIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture_data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Texture to copy output pixels back
    glGenTextures(1, &texture_target);
    glBindTexture(GL_TEXTURE_2D, texture_target);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SIZE, SIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glGenFramebuffers(1, &FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture_target, 0);

    return (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
}

// Verification function adapted from Vulkan
void verify_results(unsigned char* pixels) {
    int errors = 0;
    float max_diff = 0.0f;
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            float gpuVal = decode_float(&pixels[(i * SIZE + j) * 4]);
            
            // CPU Reference: C[i][j] = sum_{k=0}^{SIZE-1} A[i][k] * B[k][j]
            // With A[i][k] = i and B[k][j] = j:
            // C[i][j] = sum_{k=0}^{SIZE-1} i * j = SIZE * i * j
            float cpuRef = (float)SIZE * i * j;
            
            float diff = fabsf(gpuVal - cpuRef);
            if (diff > max_diff) max_diff = diff;
            if (diff > 0.5f) {
                if (errors < 10) printf("Error at [%d][%d]: GPU=%f, CPU=%f\n", i, j, gpuVal, cpuRef);
                errors++;
            }
        }
    }

    if (errors == 0) printf("Verification: PASSED (Max diff: %f)\n", max_diff);
    else printf("Verification: FAILED (%d errors, Max diff: %f)\n", errors, max_diff);
}

void Draw(ESContext *esContext) {
    t_setup_end = get_time(); // Setup ends right before the first draw operation

    UserData *userData = esContext->userData;
    // Full-Screen quad using 2 triangles and norm. coords
    GLfloat vertices[] = { -1,1,0, -1,-1,0, 1,-1,0, -1,1,0, 1,1,0, 1,-1,0 };

    glViewport(0, 0, SIZE, SIZE);
    glUseProgram(userData->program);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, vertices);
    glEnableVertexAttribArray(0);

    // Load textures
    for(int i=0; i<2; i++) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, texture[i]);
        char name[15]; sprintf(name, "textureUnit%d", i);
        glUniform1i(glGetUniformLocation(userData->program, name), i);
    }

    // Draw
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glFinish(); // Ensure operation is complete for timing

    // Read pixels back
    glReadPixels(0, 0, SIZE, SIZE, GL_RGBA, GL_UNSIGNED_BYTE, output_data);

    t_op_end = get_time();

    printf("Result Matrix (Second row):\n");
    for(int j=0; j<SIZE; j++) {
        printf("%g ", decode_float(&output_data[(1*SIZE+j)*4]));
    }
    printf("\n");

    // --- Automated Verification ---
    printf("\nVerifying results against CPU reference...\n");
    int passed = 1;
    for(int i=0; i<SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            float gpuVal = decode_float(&output_data[(i * SIZE + j) * 4]);
            
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

    // Metrics
    double setup_time = t_setup_end - t_start;
    double op_time = t_op_end - t_setup_end;
    double total_time = t_op_end - t_start;

    printf("Setup time: %f s\n", setup_time);
    printf("Comp. time: %f s\n", op_time);
    printf("Total time: %f s\n", total_time);
    exit(0);
}

int main(int argc, char *argv[]) {
    ESContext esContext;
    t_start = get_time();

    esInitContext(&esContext);
    esCreateWindow(&esContext, "GPGPU Matrix Mult", SIZE, SIZE, ES_WINDOW_RGB);
    if (Init(&esContext)) {
        esRegisterDrawFunc(&esContext, Draw);
        esMainLoop(&esContext);
    }
    return 0;
}