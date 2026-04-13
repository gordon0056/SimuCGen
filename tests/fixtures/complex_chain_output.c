#include "nwocg_run.h"
#include <stddef.h>
#include <math.h>

static struct
{
    double sum1;
    double g1Line;
    double g2Line;
    double sum2;
    double delayLine;
    double sum3Line;
} nwocg;

void nwocg_generated_init()
{
    nwocg.delayLine = 0;
}

void nwocg_generated_step()
{
    nwocg.sum1 = nwocg.in1Line + nwocg.in2Line;
    nwocg.g1Line = nwocg.sum1 * 2;
    nwocg.g2Line = nwocg.sum1 * 0.5;
    nwocg.sum2 = nwocg.g1Line - nwocg.g2Line;
    nwocg.sum3Line = nwocg.sum2 + nwocg.delayLine;

    nwocg.delayLine = nwocg.sum2;
}

static const nwocg_ExtPort
    ext_ports[] =
{
    { "out", &nwocg.sum3Line, 0 },
    { "in1", &nwocg.in1Line, 1 },
    { "in2", &nwocg.in2Line, 1 },
    { 0, 0, 0 },
};

const nwocg_ExtPort * const
    nwocg_generated_ext_ports = ext_ports;

const size_t
    nwocg_generated_ext_ports_size = sizeof(ext_ports);
