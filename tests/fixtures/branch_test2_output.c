#include "nwocg_run.h"
#include <stddef.h>
#include <math.h>

static struct
{
    double Gain1;
} nwocg;

void nwocg_generated_init()
{
}

void nwocg_generated_step()
{
    nwocg.Gain1 = nwocg.inLine * 5;
}

static const nwocg_ExtPort
    ext_ports[] =
{
    { "out1", &nwocg.Gain1, 0 },
    { "out2", &nwocg.Gain1, 0 },
    { "out3", &nwocg.Gain1, 0 },
    { "input", &nwocg.inLine, 1 },
    { 0, 0, 0 },
};

const nwocg_ExtPort * const
    nwocg_generated_ext_ports = ext_ports;

const size_t
    nwocg_generated_ext_ports_size = sizeof(ext_ports);
