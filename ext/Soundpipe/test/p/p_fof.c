#include <stdlib.h>
#include <stdio.h>
#include "soundpipe.h"
#include "config.h"

int main() {
    sp_data *sp;
    sp_create(&sp);
    sp_srand(sp, 12345);
    sp->sr = SR;
    sp->len = sp->sr * LEN;
    uint32_t t, u;
    SPFLOAT in = 0, out = 0;

    sp_fof *unit[NUM];

    sp_ftbl *sine;
    sp_ftbl *win;

    sp_ftbl_create(sp, &sine, 2048);
    sp_ftbl_create(sp, &win, 1024);

    for(u = 0; u < NUM; u++) { 
        sp_fof_create(&unit[u]);
        sp_fof_init(sp, unit[u], sine, win, 100, 0);
    }
    

    for(t = 0; t < sp->len; t++) {
        for(u = 0; u < NUM; u++) sp_fof_compute(sp, unit[u], &in, &out);
    }

    for(u = 0; u < NUM; u++) sp_fof_destroy(&unit[u]);
    sp_ftbl_destroy(&sine);
    sp_ftbl_destroy(&win);

    sp_destroy(&sp);
    return 0;
}

