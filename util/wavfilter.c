/**
 * Aften: A/52 audio encoder
 * Copyright (c) 2006  Justin Ruggles <jruggle@earthlink.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/**
 * @file wavfilter.c
 * Console WAV File Filter Utility
 */

#include "common.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "wav.h"
#include "filter.h"

static void
wav_filter(FLOAT *samples, int ch, int n, FilterContext *f)
{
    FLOAT samples2[ch][n];
    FLOAT tmp[n];
    int j, i, c;

    for(i=0,j=0; i<n; i++) {
        for(c=0; c<ch; c++,j++) {
            samples2[c][i] = samples[j];
        }
    }

    for(c=0; c<ch; c++) {
        memcpy(tmp, samples2[c], n * sizeof(FLOAT));
        filter_run(&f[c], samples2[c], tmp, n);
    }

    for(i=0,j=0; i<n; i++) {
        for(c=0; c<ch; c++,j++) {
            samples[j] = samples2[c][i];
            if(samples[j] > 1.0) samples[j] = 1.0;
            if(samples[j] < -1.0) samples[j] = -1.0;
        }
    }
}

static void
write2le(uint16_t v, FILE *ofp)
{
    fputc((v     ) & 0xFF, ofp);
    fputc((v >> 8) & 0xFF, ofp);
}

static void
write4le(uint32_t v, FILE *ofp)
{
    fputc((v      ) & 0xFF, ofp);
    fputc((v >>  8) & 0xFF, ofp);
    fputc((v >> 16) & 0xFF, ofp);
    fputc((v >> 24) & 0xFF, ofp);
}

static void
output_wav_header(FILE *ofp, WavFile *wf)
{
    fwrite("RIFF", 1, 4, ofp);
    write4le((wf->data_size + 40), ofp);
    fwrite("WAVE", 1, 4, ofp);
    fwrite("fmt ", 1, 4, ofp);
    write4le(16, ofp);
    write2le(WAVE_FORMAT_PCM, ofp);
    write2le(wf->channels, ofp);
    write4le(wf->sample_rate, ofp);
    write4le(wf->sample_rate * wf->channels * 2, ofp);
    write2le(wf->channels * 2, ofp);
    write2le(16, ofp);
    fwrite("data", 1, 4, ofp);
    write4le(wf->data_size, ofp);
}

static void
output_wav_data(FILE *ofp, FLOAT *samples, int ch, int n)
{
    int16_t s16[1];
    uint16_t *u16 = (uint16_t *)s16;
    int i;

    for(i=0; i<n*ch; i++) {
        s16[0] = (samples[i] * 32767.0);
        write2le(*u16, ofp);
    }
}

static const char *usage = "usage: wavfilter <lp | hp> <cutoff> <in.wav> <out.wav>";

int
main(int argc, char **argv)
{
    FILE *ifp, *ofp;
    WavFile wf;
    FLOAT *buf;
    int frame_size;
    int nr;
    int i;
    FilterContext f[6];
    int ftype=0;

    if(argc != 5) {
        fprintf(stderr, "\n%s\n\n", usage);
        exit(1);
    }

    if(!strncmp(argv[1], "lp", 3)) {
        ftype = FILTER_TYPE_LOWPASS;
    } else if(!strncmp(argv[1], "hp", 3)) {
        ftype = FILTER_TYPE_HIGHPASS;
    } else {
        fprintf(stderr, "\n%s\n\n", usage);
        exit(1);
    }

    ifp = fopen(argv[3], "rb");
    if(!ifp) {
        fprintf(stderr, "cannot open input file\n");
        exit(1);
    }
    ofp = fopen(argv[4], "wb");
    if(!ofp) {
        fprintf(stderr, "cannot open output file\n");
        exit(1);
    }

    if(wavfile_init(&wf, ifp)) {
        fprintf(stderr, "error initializing wav reader\n\n");
        exit(1);
    }
    output_wav_header(ofp, &wf);

#ifdef CONFIG_DOUBLE
    wf.read_format = WAV_SAMPLE_FMT_DBL;
#else
    wf.read_format = WAV_SAMPLE_FMT_FLT;
#endif

    for(i=0; i<wf.channels; i++) {
        int cutoff;
        f[i].type = ftype;
        f[i].cascaded = 1;
        cutoff = atoi(argv[2]);
        f[i].cutoff = (FLOAT)cutoff;
        f[i].samplerate = (FLOAT)wf.sample_rate;
        if(filter_init(&f[i], FILTER_ID_BUTTERWORTH_II)) {
            fprintf(stderr, "error initializing filter\n");
            exit(1);
        }
    }

    frame_size = 512;
    buf = calloc(frame_size * wf.channels, sizeof(FLOAT));

    nr = wavfile_read_samples(&wf, buf, frame_size);
    while(nr > 0) {
        wav_filter(buf, wf.channels, nr, f);
        output_wav_data(ofp, buf, wf.channels, nr);
        nr = wavfile_read_samples(&wf, buf, frame_size);
    }

    for(i=0; i<wf.channels; i++) {
        filter_close(&f[i]);
    }

    free(buf);
    fclose(ifp);
    fclose(ofp);

    return 0;
}
