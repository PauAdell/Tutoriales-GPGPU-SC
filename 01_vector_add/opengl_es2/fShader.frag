precision highp float;

uniform highp sampler2D textureUnit0;
uniform highp sampler2D textureUnit1;

// #define FRAMEBUFFER_SIZE ... (injected from C)

// Optimized bit-exact reconstruction macro
#define decode_exact(reconstructed, textureUnit, texCoord) { \
    highp vec4 u_split = texture2D(textureUnit, texCoord); \
    highp float tmp; \
    highp float sign_value = 1.0; \
    tmp = floor(256.0 * u_split.w - (u_split.w / 255.0)); \
    highp float exponent = tmp - 127.0; \
    tmp = floor(256.0 * u_split.z - (u_split.z / 255.0)); \
    reconstructed = (tmp * 0.0078125); \
    if (exponent >= -126.0 && reconstructed < 1.0) reconstructed += 1.0; \
    if (tmp > 127.0) sign_value = -1.0; \
    tmp = floor(256.0 * u_split.y - (u_split.y / 255.0)); \
    reconstructed += (tmp * 0.000030517578125); \
    tmp = floor(256.0 * u_split.x - (u_split.x / 255.0)); \
    reconstructed += (tmp * 0.00000011920928955078); \
    reconstructed = sign_value * reconstructed * exp2(exponent); \
}

// Optimized bit-exact encoding macro
#define encode_exact(reconstructed, fragColor) { \
    highp float sign_value = 1.0; \
    highp float exponent; \
    highp float tmp; \
    highp vec4 u_split; \
    exponent = (floor(log2(abs(reconstructed))) + 127.0) * step(exp2(-125.0), abs(reconstructed)); \
    u_split.w = ((exponent - 256.0 * floor(exponent * 0.00390625)) * 0.00392156862745098); \
    tmp = clamp(abs(reconstructed * exp2(-floor(log2(abs(reconstructed))))) - 1.0, 0.0, 1.0); \
    tmp = tmp * exp2(23.0); \
    if (reconstructed < 0.0) sign_value = exp2(23.0); \
    u_split.z = (floor(((tmp + sign_value) - 256.0 * 256.0 * 256.0 * floor((tmp + sign_value) * 0.00000005960464477539)) * 1.52587890625e-05) * 0.00392156862745098); \
    u_split.y = (floor((tmp - 256.0 * 256.0 * floor(tmp * 1.52587890625e-05)) * 0.00390625) * 0.00392156862745098); \
    u_split.x = ((tmp - 256.0 * floor(tmp * 0.00390625)) * 0.00392156862745098); \
    fragColor = u_split; \
}

void main() {
    highp vec2 coord = vec2((floor(gl_FragCoord.x) + 0.5) / FRAMEBUFFER_SIZE, 
                            (floor(gl_FragCoord.y) + 0.5) / FRAMEBUFFER_SIZE);
    
    highp float a;
    highp float b;
    
    decode_exact(a, textureUnit0, coord);
    decode_exact(b, textureUnit1, coord);
    
    highp float result = a + b;
    
    encode_exact(result, gl_FragColor);
}
