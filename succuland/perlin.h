#pragma once

#ifndef _PERLIN_H
#define _PERLIN_H

#include <stdlib.h>
#include <stdio.h>
#include <math.h>


#define SAMPLE_SIZE 1024

class Perlin
{
public:

    Perlin();
    Perlin(int octaves, float freq, float amp, int seed);


    float Get(float x, float y) const
    {
        float vec[2];
        vec[0] = x;
        vec[1] = y;
        return perlin_noise_2D(vec);
    };

    float operator()(float x, float y) const {
        return Get(x, y);
    }

private:
    void init_perlin(int n, float p);
    float perlin_noise_2D(float vec[2]) const;

    float noise1(float arg);
    float noise2(float vec[2]) const;
    float noise3(float vec[3]);
    void normalize2(float v[2]);
    void normalize3(float v[3]);
    void init(void);

    int   mOctaves;
    float mFrequency;
    float mAmplitude;
    int   mSeed;

    int p[SAMPLE_SIZE + SAMPLE_SIZE + 2];
    float g3[SAMPLE_SIZE + SAMPLE_SIZE + 2][3];
    float g2[SAMPLE_SIZE + SAMPLE_SIZE + 2][2];
    float g1[SAMPLE_SIZE + SAMPLE_SIZE + 2];
    bool  mStart;

};

#endif