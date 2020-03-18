sptbl["bal"] = {

    files = {
        module = "bal.c",
        header = "bal.h",
        example = "ex_bal.c",
    },

    func = {
        create = "sp_bal_create",
        destroy = "sp_bal_destroy",
        init = "sp_bal_init",
        compute = "sp_bal_compute",
    },

    params = {
    },

    modtype = "module",

    description = [[Balance the gain of one signal based on another signal

    This is often used to restore gain lost in the output of a filter.

In the source code, the value "ihp" is set to a static 10hz. This is the default value in Csound, and should not often need to be changed.
]],

    ninputs = 2,
    noutputs = 1,

    inputs = {
        {
            name = "sig",
            description = "Input signal whose gain will be changed."
        },
        {
            name = "comp",
            description = "Secondary signal to compare the input signal to."
        },
    },

    outputs = {
        {
            name = "out",
            description = "The output signal with applied gain."
        },
    }

}
