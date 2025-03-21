#version 330

uniform sampler2D c; // The texture sampler
in vec2 u; // The interpolated texture coordinates from the vertex shader
out vec4 f; // The output fragment color

// Vignetting effect parameters
uniform float vignetteIntensity = 0.5; // Intensity of the vignette effect
uniform float vignetteRadius = 0.75; // Radius where the vignette starts

// Chromatic aberration parameters
uniform float chromaAmount = 0.02f; // Amount of chromatic aberration

// Smart denoise parameters
uniform float sigma = 0.5; // Standard deviation of the Gaussian blur
uniform float kSigma = 1.0; // Sigma coefficient
uniform float threshold = 0.1; // Edge sharpening threshold


//Denoiser from https://github.com/BrutPitt/glslSmartDeNoise

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//  Copyright (c) 2018-2019 Michele Morrone
//  All rights reserved.
//
//  https://michelemorrone.eu - https://BrutPitt.com
//
//  me@michelemorrone.eu - brutpitt@gmail.com
//  twitter: @BrutPitt - github: BrutPitt
//  
//  https://github.com/BrutPitt/glslSmartDeNoise/
//
//  This software is distributed under the terms of the BSD 2-Clause license
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#define INV_SQRT_OF_2PI 0.39894228040143267793994605993439  // 1.0/SQRT_OF_2PI
#define INV_PI 0.31830988618379067153776752674503

//  smartDeNoise - parameters
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//  sampler2D tex     - sampler image / texture
//  vec2 uv           - actual fragment coord
//  float sigma  >  0 - sigma Standard Deviation
//  float kSigma >= 0 - sigma coefficient 
//      kSigma * sigma  -->  radius of the circular kernel
//  float threshold   - edge sharpening threshold 

vec4 smartDeNoise(sampler2D tex, vec2 uv, float sigma, float kSigma, float threshold)
{
    float radius = round(kSigma*sigma);
    float radQ = radius * radius;

    float invSigmaQx2 = .5 / (sigma * sigma);      // 1.0 / (sigma^2 * 2.0)
    float invSigmaQx2PI = INV_PI * invSigmaQx2;    // 1/(2 * PI * sigma^2)

    float invThresholdSqx2 = .5 / (threshold * threshold);     // 1.0 / (sigma^2 * 2.0)
    float invThresholdSqrt2PI = INV_SQRT_OF_2PI / threshold;   // 1.0 / (sqrt(2*PI) * sigma^2)

    vec4 centrPx = texture(tex,uv); 

    float zBuff = 0.0;
    vec4 aBuff = vec4(0.0);
    vec2 size = vec2(textureSize(tex, 0));

    vec2 d;
    for (d.x=-radius; d.x <= radius; d.x++) {
        float pt = sqrt(radQ-d.x*d.x);       // pt = yRadius: have circular trend
        for (d.y=-pt; d.y <= pt; d.y++) {
            float blurFactor = exp( -dot(d , d) * invSigmaQx2 ) * invSigmaQx2PI;

            vec4 walkPx =  texture(tex,uv+d/size);
            vec4 dC = walkPx-centrPx;
            float deltaFactor = exp( -dot(dC, dC) * invThresholdSqx2) * invThresholdSqrt2PI * blurFactor;

            zBuff += deltaFactor;
            aBuff += deltaFactor*walkPx;
        }
    }
    return aBuff/zBuff;
}

void main() {
    
    //apply denoiser
    f = smartDeNoise(c, u, sigma, kSigma, threshold);

    // Calculate the distance from the center of the screen
    vec2 center = vec2(0.5, 0.5); // Assuming texture coordinates are normalized [0, 1]
    float distance = length(u - center);

    // Apply vignette effect
    float vignette = smoothstep(vignetteRadius, vignetteRadius - vignetteIntensity, distance);

    // Sample the texture for RGB channels separately with slight offsets for chromatic aberration
    vec2 chromaOffset = chromaAmount * (u - center);
    float r = f.r;
    float g = f.g;
    float b = f.b;

    // Combine the sampled colors and apply the vignette effect
    f = vec4(r, g, b, 1.0) * vignette;

    // Apply the square root transformation from your original shader (if still desired)
    f = sqrt(f);
}