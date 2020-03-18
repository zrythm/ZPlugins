/*
 * When rendered, this will generate two files: 00_test.wav and 01_test.wav
 *
 * These two files can be interleaved together to a single file using sox:
 *
 * sox -M 00_test.wav 01_test.wav -c 2 out.wav
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "soundpipe.h"

typedef struct {
    sp_pan2 *pan2;
    sp_osc *osc;
    sp_osc *lfo;
    sp_ftbl *ft; 
} UserData;

void process(sp_data *sp, void *udata) {
    UserData *ud = udata;
    SPFLOAT osc = 0, outL = 0, outR = 0, lfo = 0;
    sp_osc_compute(sp, ud->osc, NULL, &osc);
    sp_osc_compute(sp, ud->lfo, NULL, &lfo);
    ud->pan2->pan = lfo;
    sp_pan2_compute(sp, ud->pan2, &osc, &outL, &outR);
    sp_out(sp, 0, outL);
    sp_out(sp, 1, outR);
}

int main() {
    UserData ud;
    sp_data *sp;
    sp_createn(&sp, 2);

    sp_pan2_create(&ud.pan2);
    sp_osc_create(&ud.osc);
    sp_osc_create(&ud.lfo);
    sp_ftbl_create(sp, &ud.ft, 2048);

    sp_pan2_init(sp, ud.pan2);
    ud.pan2->type = 2;
    sp_gen_sine(sp, ud.ft);
    sp_osc_init(sp, ud.osc, ud.ft, 0);
    sp_osc_init(sp, ud.lfo, ud.ft, 0);
    ud.lfo->amp = 1;
    ud.lfo->freq = 1;

    sp->len = 44100 * 5;
    sp_process(sp, &ud, process);

    sp_pan2_destroy(&ud.pan2);
    sp_ftbl_destroy(&ud.ft);
    sp_osc_destroy(&ud.osc);
    sp_osc_destroy(&ud.lfo);

    sp_destroy(&sp);
    return 0;
}
