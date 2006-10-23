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
 * @file aften.c
 * Commandline encoder frontend
 */

#include "common.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include <io.h>
#include <fcntl.h>
#endif

#include "aften.h"
#include "wav.h"

static void
print_intro(FILE *out)
{
    fprintf(out, "\nAften: A/52 audio encoder\n(c) 2006 Justin Ruggles\n\n");
}

static void
print_usage(FILE *out)
{
    fprintf(out, "usage: aften [options] <input.wav> <output.ac3>\n"
                 "type 'aften -h' for more details.\n\n");
}

static void
print_help(FILE *out)
{
    fprintf(out, "usage: aften [options] <input.wav> <output.ac3>\n"
                 "options:\n"
                 "    [-h]           Print out list of commandline options\n"
                 "    [-v #]         Verbosity (controls output to console)\n"
                 "                       0 = quiet mode\n"
                 "                       1 = show average stats (default)\n"
                 "                       2 = show each frame's stats\n"
                 "    [-b #]         CBR bitrate in kbps (default: about 96kbps per channel)\n"
                 "    [-q #]         VBR quality [0 - 1023] (default: 220)\n"
                 "    [-fba #]       Fast bit allocation (default: 0)\n"
                 "                       0 = more accurate encoding\n"
                 "                       1 = faster encoding\n"
                 "    [-w #]         Bandwidth\n"
                 "                       0 to 60 = fixed bandwidth (28%%-99%% of full bandwidth)\n"
                 "                      -1 = fixed adaptive bandwidth (default)\n"
                 "                      -2 = variable adaptive bandwidth\n"
                 "    [-m #]         Stereo rematrixing\n"
                 "                       0 = independent L+R channels\n"
                 "                       1 = mid/side rematrixing (default)\n"
                 "    [-s #]         Block switching\n"
                 "                       0 = use only 512-point MDCT (default)\n"
                 "                       1 = selectively use 256-point MDCT\n"
                 "    [-cmix #]      Center mix level\n"
                 "                       0 = -3.0 dB (default)\n"
                 "                       1 = -4.5 dB\n"
                 "                       2 = -6.0 dB\n"
                 "    [-smix #]      Surround mix level\n"
                 "                       0 = -3 dB (default)\n"
                 "                       1 = -6 dB\n"
                 "                       2 = 0\n"
                 "    [-dsur #]      Dolby Surround mode\n"
                 "                       0 = not indicated (default)\n"
                 "                       1 = not Dolby surround encoded\n"
                 "                       2 = Dolby surround encoded\n"
                 "    [-dnorm #]     Dialog normalization [0 - 31] (default: 31)\n"
                 "    [-dynrng #]    Dynamic Range Compression profile\n"
                 "                       0 = Film Standard\n"
                 "                       1 = Film Light\n"
                 "                       2 = Music Standard\n"
                 "                       3 = Music Light\n"
                 "                       4 = Speech\n"
                 "                       5 = None (default)\n"
                 "    [-acmod #]     Audio coding mode (overrides wav header)\n"
                 "                       0 = 1+1 (Ch1,Ch2)\n"
                 "                       1 = 1/0 (C)\n"
                 "                       2 = 2/0 (L,R)\n"
                 "                       3 = 3/0 (L,R,C)\n"
                 "                       4 = 2/1 (L,R,S)\n"
                 "                       5 = 3/1 (L,R,C,S)\n"
                 "                       6 = 2/2 (L,R,SL,SR)\n"
                 "                       7 = 3/2 (L,R,C,SL,SR)\n"
                 "    [-lfe #]       Specify use of LFE channel (overrides wav header)\n"
                 "                       0 = LFE channel is not present\n"
                 "                       1 = LFE channel is present\n"
                 "    [-bwfilter #]  Specify use of the bandwidth low-pass filter\n"
                 "                       0 = do not apply filter (default)\n"
                 "                       1 = apply filter\n"
                 "    [-dcfilter #]  Specify use of the DC high-pass filter\n"
                 "                       0 = do not apply filter (default)\n"
                 "                       1 = apply filter\n"
                 "    [-lfefilter #] Specify use of the LFE low-pass filter\n"
                 "                       0 = do not apply filter (default)\n"
                 "                       1 = apply filter\n"
                 "    [-xbsi1 #]     Specify use of extended bitstream info 1\n"
                 "                       0 = do not write xbsi1\n"
                 "                       1 = write xbsi1\n"
                 "    [-dmixmod #]   Preferred stereo downmix mode\n"
                 "                       0 = not indicated (default)\n"
                 "                       1 = Lt/Rt downmix preferred\n"
                 "                       2 = Lo/Ro downmix preferred\n"
                 "    [-ltrtcmix #]  Lt/Rt center mix level\n"
                 "    [-ltrtsmix #]  Lt/Rt surround mix level\n"
                 "    [-lorocmix #]  Lo/Ro center mix level\n"
                 "    [-lorosmix #]  Lo/Ro surround mix level\n"
                 "                       0 = +3.0 dB\n"
                 "                       1 = +1.5 dB\n"
                 "                       2 =  0.0 dB\n"
                 "                       3 = -1.5 dB\n"
                 "                       4 = -3.0 dB (default)\n"
                 "                       5 = -4.5 dB\n"
                 "                       6 = -6.0 dB\n"
                 "                       7 = -inf dB\n"
                 "    [-xbsi2 #]     Specify use of extended bitstream info 2\n"
                 "                       0 = do not write xbsi2\n"
                 "                       1 = write xbsi2\n"
                 "    [-dsurexmod #] Dolby Surround EX mode\n"
                 "                       0 = not indicated (default)\n"
                 "                       1 = Not Dolby Surround EX encoded\n"
                 "                       2 = Dolby Surround EX encoded\n"
                 "    [-dheadphon #] Dolby Headphone mode\n"
                 "                       0 = not indicated (default)\n"
                 "                       1 = Not Dolby Headphone encoded\n"
                 "                       2 = Dolby Headphone encoded\n"
                 "    [-adconvtyp #] A/D converter type\n"
                 "                       0 = Standard (default)\n"
                 "                       1 = HDCD\n"
                 "\n");
}

typedef struct CommandOptions {
    char *infile;
    char *outfile;
    AftenContext *s;
} CommandOptions;

static int
parse_commandline(int argc, char **argv, CommandOptions *opts)
{
    int i;
    int found_input = 0;
    int found_output = 0;

    if(argc < 2) {
        return 1;
    }

    opts->infile = argv[1];
    opts->outfile = argv[2];

    for(i=1; i<argc; i++) {
        if(argv[i][0] == '-' && argv[i][1] != '\0') {
            if(argv[i][2] != '\0') {
                // multi-character arguments
                if(!strncmp(&argv[i][1], "cmix", 5)) {
                    i++;
                    if(i >= argc) return 1;
                    opts->s->meta.cmixlev = atoi(argv[i]);
                    if(opts->s->meta.cmixlev < 0 || opts->s->meta.cmixlev > 2) {
                        fprintf(stderr, "invalid cmix: %d. must be 0 to 2.\n", opts->s->meta.cmixlev);
                        return 1;
                    }
                } else if(!strncmp(&argv[i][1], "smix", 5)) {
                    i++;
                    if(i >= argc) return 1;
                    opts->s->meta.surmixlev = atoi(argv[i]);
                    if(opts->s->meta.surmixlev < 0 || opts->s->meta.surmixlev > 2) {
                        fprintf(stderr, "invalid smix: %d. must be 0 to 2.\n", opts->s->meta.surmixlev);
                        return 1;
                    }
                } else if(!strncmp(&argv[i][1], "dsur", 5)) {
                    i++;
                    if(i >= argc) return 1;
                    opts->s->meta.dsurmod = atoi(argv[i]);
                    if(opts->s->meta.dsurmod < 0 || opts->s->meta.dsurmod > 2) {
                        fprintf(stderr, "invalid dsur: %d. must be 0 to 2.\n", opts->s->meta.dsurmod);
                        return 1;
                    }
                } else if(!strncmp(&argv[i][1], "dnorm", 6)) {
                    i++;
                    if(i >= argc) return 1;
                    opts->s->meta.dialnorm = atoi(argv[i]);
                    if(opts->s->meta.dialnorm < 0 || opts->s->meta.dialnorm > 31) {
                        fprintf(stderr, "invalid dnorm: %d. must be 0 to 31.\n", opts->s->meta.dialnorm);
                        return 1;
                    }
                } else if(!strncmp(&argv[i][1], "dynrng", 7)) {
                    int profile;
                    i++;
                    if(i >= argc) return 1;
                    profile = atoi(argv[i]);
                    if(profile < 0 || profile > 5) {
                        fprintf(stderr, "invalid dynrng: %d. must be 0 to 5.\n", profile);
                        return 1;
                    }
                    opts->s->params.dynrng_profile = profile;
                } else if(!strncmp(&argv[i][1], "acmod", 6)) {
                    i++;
                    if(i >= argc) return 1;
                    opts->s->acmod = atoi(argv[i]);
                    if(opts->s->acmod < 0 || opts->s->acmod > 7) {
                        fprintf(stderr, "invalid acmod: %d. must be 0 to 7.\n", opts->s->acmod);
                        return 1;
                    }
                } else if(!strncmp(&argv[i][1], "lfe", 4)) {
                    i++;
                    if(i >= argc) return 1;
                    opts->s->lfe = atoi(argv[i]);
                    if(opts->s->lfe < 0 || opts->s->lfe > 1) {
                        fprintf(stderr, "invalid lfe: %d. must be 0 or 1.\n", opts->s->lfe);
                        return 1;
                    }
                } else if(!strncmp(&argv[i][1], "bwfilter", 9)) {
                    i++;
                    if(i >= argc) return 1;
                    opts->s->params.use_bw_filter = atoi(argv[i]);
                    if(opts->s->params.use_bw_filter < 0 || opts->s->params.use_bw_filter > 1) {
                        fprintf(stderr, "invalid bwfilter: %d. must be 0 or 1.\n", opts->s->params.use_bw_filter);
                        return 1;
                    }
                } else if(!strncmp(&argv[i][1], "dcfilter", 9)) {
                    i++;
                    if(i >= argc) return 1;
                    opts->s->params.use_dc_filter = atoi(argv[i]);
                    if(opts->s->params.use_dc_filter < 0 || opts->s->params.use_dc_filter > 1) {
                        fprintf(stderr, "invalid dcfilter: %d. must be 0 or 1.\n", opts->s->params.use_dc_filter);
                        return 1;
                    }
                } else if(!strncmp(&argv[i][1], "lfefilter", 10)) {
                    i++;
                    if(i >= argc) return 1;
                    opts->s->params.use_lfe_filter = atoi(argv[i]);
                    if(opts->s->params.use_lfe_filter < 0 || opts->s->params.use_lfe_filter > 1) {
                        fprintf(stderr, "invalid lfefilter: %d. must be 0 or 1.\n", opts->s->params.use_lfe_filter);
                        return 1;
                    }
                } else if(!strncmp(&argv[i][1], "xbsi1", 6)) {
                    i++;
                    if(i >= argc) return 1;
                    opts->s->meta.xbsi1e = atoi(argv[i]);
                    if(opts->s->meta.xbsi1e < 0 || opts->s->meta.xbsi1e > 1) {
                        fprintf(stderr, "invalid xbsi1: %d. must be 0 or 1.\n", opts->s->meta.xbsi1e);
                        return 1;
                    }
                } else if(!strncmp(&argv[i][1], "dmixmod", 8)) {
                    i++;
                    if(i >= argc) return 1;
                    opts->s->meta.dmixmod = atoi(argv[i]);
                    if(opts->s->meta.dmixmod < 0 || opts->s->meta.dmixmod > 2) {
                        fprintf(stderr, "invalid dmixmod: %d. must be 0 to 2.\n", opts->s->meta.dmixmod);
                        return 1;
                    }
                } else if(!strncmp(&argv[i][1], "ltrtcmix", 9)) {
                    i++;
                    if(i >= argc) return 1;
                    opts->s->meta.ltrtcmixlev = atoi(argv[i]);
                    if(opts->s->meta.ltrtcmixlev < 0 || opts->s->meta.ltrtcmixlev > 7) {
                        fprintf(stderr, "invalid ltrtcmix: %d. must be 0 to 7.\n", opts->s->meta.ltrtcmixlev);
                        return 1;
                    }
                } else if(!strncmp(&argv[i][1], "ltrtsmix", 9)) {
                    i++;
                    if(i >= argc) return 1;
                    opts->s->meta.ltrtsmixlev = atoi(argv[i]);
                    if(opts->s->meta.ltrtsmixlev < 0 || opts->s->meta.ltrtsmixlev > 7) {
                        fprintf(stderr, "invalid ltrtsmix: %d. must be 0 to 7.\n", opts->s->meta.ltrtsmixlev);
                        return 1;
                    }
                } else if(!strncmp(&argv[i][1], "lorocmix", 9)) {
                    i++;
                    if(i >= argc) return 1;
                    opts->s->meta.lorocmixlev = atoi(argv[i]);
                    if(opts->s->meta.lorocmixlev < 0 || opts->s->meta.lorocmixlev > 7) {
                        fprintf(stderr, "invalid lorocmix: %d. must be 0 to 7.\n", opts->s->meta.lorocmixlev);
                        return 1;
                    }
                } else if(!strncmp(&argv[i][1], "lorosmix", 9)) {
                    i++;
                    if(i >= argc) return 1;
                    opts->s->meta.lorosmixlev = atoi(argv[i]);
                    if(opts->s->meta.lorosmixlev < 0 || opts->s->meta.lorosmixlev > 7) {
                        fprintf(stderr, "invalid lorosmix: %d. must be 0 to 7.\n", opts->s->meta.lorosmixlev);
                        return 1;
                    }
                    opts->s->meta.xbsi1e = 1;
                } else if(!strncmp(&argv[i][1], "xbsi2", 6)) {
                    i++;
                    if(i >= argc) return 1;
                    opts->s->meta.xbsi2e = atoi(argv[i]);
                    if(opts->s->meta.xbsi2e < 0 || opts->s->meta.xbsi2e > 1) {
                        fprintf(stderr, "invalid xbsi2: %d. must be 0 or 1.\n", opts->s->meta.xbsi2e);
                        return 1;
                    }
                } else if(!strncmp(&argv[i][1], "dsurexmod", 10)) {
                    i++;
                    if(i >= argc) return 1;
                    opts->s->meta.dsurexmod = atoi(argv[i]);
                    if(opts->s->meta.dsurexmod < 0 || opts->s->meta.dsurexmod > 2) {
                        fprintf(stderr, "invalid dsurexmod: %d. must be 0 to 2.\n", opts->s->meta.dsurexmod);
                        return 1;
                    }
                } else if(!strncmp(&argv[i][1], "dheadphon", 10)) {
                    i++;
                    if(i >= argc) return 1;
                    opts->s->meta.dheadphonmod = atoi(argv[i]);
                    if(opts->s->meta.dheadphonmod < 0 || opts->s->meta.dheadphonmod > 2) {
                        fprintf(stderr, "invalid dheadphon: %d. must be 0 to 2.\n", opts->s->meta.dheadphonmod);
                        return 1;
                    }
                } else if(!strncmp(&argv[i][1], "adconvtyp", 10)) {
                    i++;
                    if(i >= argc) return 1;
                    opts->s->meta.adconvtyp = atoi(argv[i]);
                    if(opts->s->meta.adconvtyp < 0 || opts->s->meta.adconvtyp > 1) {
                        fprintf(stderr, "invalid adconvtyp: %d. must be 0 or 1.\n", opts->s->meta.adconvtyp);
                        return 1;
                    }
                } else if(!strncmp(&argv[i][1], "fba", 4)) {
                    i++;
                    if(i >= argc) return 1;
                    opts->s->params.bitalloc_fast = atoi(argv[i]);
                    if(opts->s->params.bitalloc_fast < 0 || opts->s->params.bitalloc_fast > 1) {
                        fprintf(stderr, "invalid fba: %d. must be 0 or 1.\n", opts->s->params.bitalloc_fast);
                        return 1;
                    }
                }
            } else {
                // single-character arguments
                if(argv[i][1] == 'h') {
                    return 2;
                } else if(argv[i][1] == 'v') {
                    i++;
                    if(i >= argc) return 1;
                    opts->s->params.verbose = atoi(argv[i]);
                    if(opts->s->params.verbose < 0 || opts->s->params.verbose > 2) {
                        fprintf(stderr, "invalid verbosity level: %d. must be 0 to 2.\n", opts->s->params.verbose);
                        return 1;
                    }
                } else if(argv[i][1] == 'b') {
                    i++;
                    if(i >= argc) return 1;
                    opts->s->params.bitrate = atoi(argv[i]);
                    if(opts->s->params.bitrate < 0 || opts->s->params.bitrate > 640) {
                        fprintf(stderr, "invalid bitrate: %d. must be 0 to 640.\n", opts->s->params.bitrate);
                        return 1;
                    }
                } else if(argv[i][1] == 'q') {
                    i++;
                    if(i >= argc) return 1;
                    opts->s->params.quality = atoi(argv[i]);
                    opts->s->params.encoding_mode = AFTEN_ENC_MODE_VBR;
                    if(opts->s->params.quality < 0 || opts->s->params.quality > 1023) {
                        fprintf(stderr, "invalid quality: %d. must be 0 to 1023.\n", opts->s->params.quality);
                        return 1;
                    }
                } else if(argv[i][1] == 'w') {
                    i++;
                    if(i >= argc) return 1;
                    opts->s->params.bwcode = atoi(argv[i]);
                    if(opts->s->params.bwcode < -2 || opts->s->params.bwcode > 60) {
                        fprintf(stderr, "invalid bandwidth code: %d. must be 0 to 60.\n", opts->s->params.bwcode);
                        return 1;
                    }
                } else if(argv[i][1] == 'm') {
                    i++;
                    if(i >= argc) return 1;
                    opts->s->params.use_rematrixing = atoi(argv[i]);
                    if(opts->s->params.use_rematrixing < 0 || opts->s->params.use_rematrixing > 1) {
                        fprintf(stderr, "invalid stereo rematrixing: %d. must be 0 or 1.\n", opts->s->params.use_rematrixing);
                        return 1;
                    }
                } else if(argv[i][1] == 's') {
                    i++;
                    if(i >= argc) return 1;
                    opts->s->params.use_block_switching = atoi(argv[i]);
                    if(opts->s->params.use_block_switching < 0 || opts->s->params.use_block_switching > 1) {
                        fprintf(stderr, "invalid block switching: %d. must be 0 or 1.\n", opts->s->params.use_block_switching);
                        return 1;
                    }
                }
            }
        } else {
            if(!found_input) {
                opts->infile = argv[i];
                found_input = 1;
            } else {
                if(found_output) return 1;
                opts->outfile = argv[i];
                found_output = 1;
            }
        }
    }
    if(!found_input || !found_output) {
        return 1;
    }
    // disallow infile & outfile with same name except with piping
    if(strncmp(opts->infile, "-", 2) && strncmp(opts->outfile, "-", 2)) {
        if(!strcmp(opts->infile, opts->outfile)) {
            fprintf(stderr, "output filename cannot match input filename\n");
            return 1;
        }
    }
    return 0;
}

int
main(int argc, char **argv)
{
    uint8_t *frame;
    FLOAT *fwav;
    int nr, fs, err;
    FILE *ifp;
    FILE *ofp;
    WavFile wf;
    CommandOptions opts;
    AftenContext s;
    uint32_t samplecount, bytecount, t0, t1, percent;
    FLOAT kbps, qual, bw;

    opts.s = &s;
    aften_set_defaults(&s);
    err = parse_commandline(argc, argv, &opts);
    if(err) {
        if(err == 2) {
            print_intro(stdout);
            print_help(stdout);
            return 0;
        } else {
            print_intro(stderr);
            print_usage(stderr);
            return 1;
        }
    }

    if(s.params.verbose > 0) {
        print_intro(stderr);
    }

    if(!strncmp(opts.infile, "-", 2)) {
#ifdef _WIN32
        _setmode(_fileno(stdin), _O_BINARY);
#endif
        ifp = stdin;
    } else {
        ifp = fopen(opts.infile, "rb");
        if(!ifp) {
            fprintf(stderr, "error opening input file: %s\n", opts.infile);
            return 1;
        }
    }
    if(!strncmp(opts.outfile, "-", 2)) {
#ifdef _WIN32
        _setmode(_fileno(stdout), _O_BINARY);
#endif
        ofp = stdout;
    } else {
        ofp = fopen(opts.outfile, "wb");
        if(!ofp) {
            fprintf(stderr, "error opening output file: %s\n", opts.outfile);
            return 1;
        }
    }

    if(wavfile_init(&wf, ifp)) {
        fprintf(stderr, "invalid wav file: %s\n", argv[1]);
        return 1;
    }

    if(s.params.verbose > 0) {
        wavfile_print(stderr, &wf);
    }

    s.channels = wf.channels;
    aften_wav_chmask_to_acmod(wf.channels, wf.ch_mask, &s.acmod, &s.lfe);
    s.samplerate = wf.sample_rate;
#ifdef CONFIG_DOUBLE
    wf.read_format = WAV_SAMPLE_FMT_DBL;
    s.sample_format = A52_SAMPLE_FMT_DBL;
#else
    wf.read_format = WAV_SAMPLE_FMT_FLT;
    s.sample_format = A52_SAMPLE_FMT_FLT;
#endif

    if(aften_encode_init(&s)) {
        fprintf(stderr, "error initializing encoder\n");
        aften_encode_close(&s);
        return 1;
    }

    frame = calloc(A52_MAX_CODED_FRAME_SIZE, 1);
    fwav = calloc(A52_FRAME_SIZE * wf.channels, sizeof(FLOAT));
    if(frame == NULL || fwav == NULL) {
        aften_encode_close(&s);
        exit(1);
    }

    samplecount = bytecount = t0 = t1 = percent = 0;
    qual = bw = 0.0;

    nr = wavfile_read_samples(&wf, fwav, A52_FRAME_SIZE);
    while(nr > 0) {
        aften_remap_wav_to_a52(fwav, nr, wf.channels, s.sample_format,
                               s.acmod);

        fs = aften_encode_frame(&s, frame, fwav);
        if(fs <= 0) {
            fprintf(stderr, "Error encoding frame %d\n", s.status.frame_num);
        } else {
            if(s.params.verbose > 0) {
                samplecount += A52_FRAME_SIZE;
                bytecount += fs;
                qual += s.status.quality;
                bw += s.status.bwcode;
                if(s.params.verbose == 1) {
                    t1 = samplecount / wf.sample_rate;
                    if(t1 > t0 || samplecount == wf.samples) {
                        if(samplecount > 0) {
                            kbps = (bytecount * FCONST(8.0) * wf.sample_rate) /
                                (FCONST(1000.0) * samplecount);
                        } else {
                            kbps = 0;
                        }
                        if(wf.samples > 0) {
                            percent = (uint32_t)((samplecount * FCONST(100.5)) / wf.samples);
                        }
                        fprintf(stderr, "\rprogress: %3u%% | q: %4.1f | "
                                        "bw: %2.1f | bitrate: %4.1f kbps ",
                                percent, (qual / (s.status.frame_num+1)),
                                (bw / (s.status.frame_num+1)), kbps);
                    }
                    t0 = t1;
                } else if(s.params.verbose == 2) {
                    fprintf(stderr, "frame: %7d | q: %4d | bw: %2d | bitrate: %3d kbps\n",
                            s.status.frame_num, s.status.quality, s.status.bwcode,
                            s.status.bit_rate);
                }
            }
            fwrite(frame, 1, fs, ofp);
        }
        nr = wavfile_read_samples(&wf, fwav, A52_FRAME_SIZE);
    }
    if(s.params.verbose == 1) {
        fprintf(stderr, "\n");
    } else if(s.params.verbose == 2) {
        if(samplecount > 0) {
            kbps = (bytecount * FCONST(8.0) * wf.sample_rate) / (FCONST(1000.0) * samplecount);
        } else {
            kbps = 0;
        }
        fprintf(stderr, "\n");
        fprintf(stderr, "average quality:   %4.1f\n", (qual / (s.status.frame_num+1)));
        fprintf(stderr, "average bandwidth: %2.1f\n", (bw / (s.status.frame_num+1)));
        fprintf(stderr, "average bitrate:   %4.1f kbps\n\n", kbps);
    }

    free(fwav);
    free(frame);

    fclose(ifp);
    fclose(ofp);

    aften_encode_close(&s);

    return 0;
}
