#include "nwocg_run.h"
#include <stddef.h>
#include <math.h>

static struct
{
    double Add1;
    double pGainLine;
    double iGainLine;
    double tsLine;
    double unitDelayLine;
    double Add2;
    double add3Line;
} nwocg;

void nwocg_generated_init()
{
    nwocg.unitDelayLine = 0;
}

void nwocg_generated_step()
{
    nwocg.Add1 = nwocg.setpointLine - nwocg.feedbackLine;
    nwocg.pGainLine = nwocg.Add1 * 3;
    nwocg.iGainLine = nwocg.Add1 * 2;
    nwocg.tsLine = nwocg.iGainLine * 0.01;
    nwocg.Add2 = nwocg.tsLine + nwocg.unitDelayLine;
    nwocg.add3Line = nwocg.pGainLine + nwocg.Add2;

    nwocg.unitDelayLine = nwocg.Add2;
}

static const nwocg_ExtPort
    ext_ports[] =
{
    { "command", &nwocg.add3Line, 0 },
    { "feedback", &nwocg.feedbackLine, 1 },
    { "setpoint", &nwocg.setpointLine, 1 },
    { 0, 0, 0 },
};

const nwocg_ExtPort * const
    nwocg_generated_ext_ports = ext_ports;

const size_t
    nwocg_generated_ext_ports_size = sizeof(ext_ports);
