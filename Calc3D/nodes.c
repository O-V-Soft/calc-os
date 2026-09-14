#include <video.h>
#include <mouse.h>
#include <stdint.h>

#if defined(__riscv)
#include <riscv.h>
#else
#include <idt.h>
#include <utils.h>
#endif

Point3D cube_nodes[8] = {
    {-10, -10, 100},
    { 10, -10, 100},
    { 10,  10, 100},
    {-10,  10, 100},

    {-10, -10, 120},
    { 10, -10, 120},
    { 10,  10, 120},
    {-10,  10, 120}
};
