#include "nwocg_run.h"
#include <stddef.h>
#include <math.h>

static struct
{
    double Ref;
    double Feedback;
    double SumError;
    double Kp;
} nwocg;

void nwocg_generated_init()
{
}

void nwocg_generated_step()
{
    nwocg.SumError = nwocg.Ref - nwocg.Feedback;
    nwocg.Kp = nwocg.SumError * 2.5;
}

static const nwocg_ExtPort
    ext_ports[] =
{
    { "Output", &nwocg.Kp, 0 },
    { "Feedback", &nwocg.Feedback, 1 },
    { "Ref", &nwocg.Ref, 1 },
    { 0, 0, 0 },
};

const nwocg_ExtPort * const
    nwocg_generated_ext_ports = ext_ports;

const size_t
    nwocg_generated_ext_ports_size = sizeof(ext_ports);
