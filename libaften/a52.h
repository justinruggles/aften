/**
 * Aften: A/52 audio encoder
 * Copyright (c) 2006  Justin Ruggles <jruggle@earthlink.net>
 *
 * Based on "The simplest AC3 encoder" from FFmpeg
 * Copyright (c) 2000 Fabrice Bellard.
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
 * @file a52.h
 * A/52 encoder header
 */

#ifndef A52_H
#define A52_H

#include "common.h"
#include "bitio.h"
#include "aften.h"
#include "filter.h"
#include "mdct.h"

#define A52_MAX_CHANNELS 6

#define A52_MAX_COEFS 253

/* exponent encoding strategy */
#define EXP_REUSE 0
#define EXP_NEW   1
#define EXP_D15   1
#define EXP_D25   2
#define EXP_D45   3

#define SNROFFST(csnr, fsnr) (((((csnr)-15) << 4) + (fsnr)) << 2)
#define QUALITY(csnr, fsnr) ((SNROFFST(csnr, fsnr)+960)/4)

/* possible frequencies */
extern const uint16_t a52_freqs[3];

/* possible bitrates */
extern const uint16_t a52_bitratetab[19];

typedef struct A52Block {
    FLOAT *input_samples[A52_MAX_CHANNELS]; /* 512 per ch */
    FLOAT *mdct_coef[A52_MAX_CHANNELS]; /* 256 per ch */
    FLOAT transient_samples[A52_MAX_CHANNELS][512];
    int block_num;
    int blksw[A52_MAX_CHANNELS];
    int dithflag[A52_MAX_CHANNELS];
    int dynrng;
    uint8_t exp[A52_MAX_CHANNELS][256];
    int16_t psd[A52_MAX_CHANNELS][256];
    int16_t mask[A52_MAX_CHANNELS][50];
    uint8_t exp_strategy[A52_MAX_CHANNELS];
    uint8_t nexpgrps[A52_MAX_CHANNELS];
    uint8_t grp_exp[A52_MAX_CHANNELS][85];
    uint8_t bap[A52_MAX_CHANNELS][256];
    uint16_t qmant[A52_MAX_CHANNELS][256];
    uint8_t rematstr;
    uint8_t rematflg[4];
} A52Block;

typedef struct A52BitAllocParams {
    int fscod;
    int halfratecod;
    int fgain, sgain, sdecay, fdecay, dbknee, floor;
    int cplfleak, cplsleak;
} A52BitAllocParams;

typedef struct A52Frame {
    int frame_num;
    int quality;
    int bit_rate;
    int bwcode;

    FLOAT input_audio[A52_MAX_CHANNELS][A52_FRAME_SIZE];
    A52Block blocks[A52_NUM_BLOCKS];
    int frame_bits;
    int exp_bits;
    int mant_bits;
    unsigned int frame_size_min; // minimum frame size
    unsigned int frame_size;     // current frame size in words
    unsigned int frmsizecod;

    // bitrate allocation control
    int sgaincod, sdecaycod, fdecaycod, dbkneecod, floorcod;
    A52BitAllocParams bit_alloc;
    int csnroffst;
    int fgaincod;
    int fsnroffst;
    int ncoefs[A52_MAX_CHANNELS];
} A52Frame;

typedef struct A52Context {
    A52Frame frame;
    BitWriter bw;
    AftenEncParams params;
    AftenMetadata meta;
    void (*fmt_convert_from_src)(FLOAT dest[A52_MAX_CHANNELS][A52_FRAME_SIZE],
         void *vsrc, int nch, int n);

    int n_channels;
    int n_all_channels;
    int acmod;
    int lfe;
    int lfe_channel;
    int sample_rate;
    int halfratecod;
    int bsid;
    int fscod;
    int bsmod;
    int target_bitrate;
    int frmsizecod;
    int fixed_bwcode;

    uint32_t bit_cnt;
    uint32_t sample_cnt;
    uint32_t frame_cnt;

    FilterContext bs_filter[A52_MAX_CHANNELS];
    FilterContext dc_filter[A52_MAX_CHANNELS];
    FilterContext bw_filter[A52_MAX_CHANNELS];
    FilterContext lfe_filter;

    FLOAT last_samples[A52_MAX_CHANNELS][256];
    FLOAT last_transient_samples[A52_MAX_CHANNELS][256];
    int last_csnroffst;
    int last_quality;

    MDCTContext mdct_ctx_512;
    MDCTContext mdct_ctx_256;
} A52Context;

#endif /* A52_H */
