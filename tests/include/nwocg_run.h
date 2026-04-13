#ifndef NWOCG_RUN_H
#define NWOCG_RUN_H

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct
{
    const char * name;
    double *     value;
    int          is_input;
} nwocg_ExtPort;

extern void nwocg_generated_init(void);
extern void nwocg_generated_step(void);

extern const nwocg_ExtPort * const nwocg_generated_ext_ports;
extern const size_t nwocg_generated_ext_ports_size;

#ifdef __cplusplus
}
#endif

#endif /* NWOCG_RUN_H */
