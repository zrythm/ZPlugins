#include "soundpipe.h"
#include "md5.h"
#include "tap.h"
#include "test.h"

typedef struct {
    sp_smoothdelay *smoothdelay;
    sp_osc *osc;
    sp_metro *met;
    sp_tenv *tenv;
    sp_ftbl *ft;
} UserData;


int t_smoothdelay(sp_test *tst, sp_data *sp, const char *hash) 
{
    sp_srand(sp, 0);
    uint32_t n;
    int fail = 0;
    SPFLOAT osc = 0, smoothdelay = 0, met = 0, tenv = 0;
    UserData ud;

    sp_smoothdelay_create(&ud.smoothdelay);
    sp_osc_create(&ud.osc);
    sp_ftbl_create(sp, &ud.ft, 2048);
    sp_metro_create(&ud.met);
    sp_tenv_create(&ud.tenv);

    sp_smoothdelay_init(sp, ud.smoothdelay, 0.75 * 0.5, 256);
    ud.smoothdelay->del = 0.11;
    ud.smoothdelay->feedback = 0.66;
    sp_gen_sine(sp, ud.ft);
    sp_osc_init(sp, ud.osc, ud.ft, 0);
    ud.osc->amp = 0.5;
    sp_metro_init(sp, ud.met);
    ud.met->freq = 1;
    sp_tenv_init(sp, ud.tenv);
    ud.tenv->atk = 0.005;
    ud.tenv->hold = 0.1;
    ud.tenv->rel =  0.1;

    for(n = 0; n < tst->size; n++) {
        osc = 0, smoothdelay = 0, met = 0, tenv = 0;
        sp_metro_compute(sp, ud.met, NULL, &met);
        sp_tenv_compute(sp, ud.tenv, &met, &tenv);
        if(met) {
            ud.osc->freq = 100 + sp_rand(sp) % 500;
        }
        sp_osc_compute(sp, ud.osc, NULL, &osc);
        osc *= tenv;
        sp_smoothdelay_compute(sp, ud.smoothdelay, &osc, &smoothdelay);

        sp_test_add_sample(tst, osc + smoothdelay);
    }

    fail = sp_test_verify(tst, hash);
    
    sp_metro_destroy(&ud.met);
    sp_smoothdelay_destroy(&ud.smoothdelay);
    sp_osc_destroy(&ud.osc);
    sp_ftbl_destroy(&ud.ft);
    sp_tenv_destroy(&ud.tenv);

    if(fail) return SP_NOT_OK;
    else return SP_OK;
}
