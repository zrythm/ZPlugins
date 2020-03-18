#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "soundpipe.h"

typedef struct {
    sp_noise *ns;
    sp_moogladder *moog;
    int counter;
} UserData;

void write_noise(sp_data *sp, void *udata) {
    UserData *ud = udata;
    SPFLOAT in = 0;
    SPFLOAT out = 0;
    if(ud->counter == 0) {
        ud->moog->res = 0.8;
        ud->moog->freq = 500 + rand() % 4000;
    }
    sp_noise_compute(sp, ud->ns, NULL, &in);
    sp_moogladder_compute(sp, ud->moog, &in, &sp->out[0]); 
    ud->counter = (ud->counter + 1) % 5000;
}
int main() {
    srand(time(NULL));
    UserData ud;
    ud.counter = 0;
    sp_data *sp;
    sp_create(&sp);
    sp_noise_create(&ud.ns);
    sp_moogladder_create(&ud.moog);
    sp_noise_init(sp, ud.ns);
    sp_moogladder_init(sp, ud.moog);
    sp->len = 44100 * 5;
    sp_process(sp, &ud, write_noise);
    sp_noise_destroy(&ud.ns);
    sp_moogladder_destroy(&ud.moog);
    sp_destroy(&sp);
    return 0;
}
