#include <video.h>
#include <mouse.h>
#include <stdint.h>

#if defined(__riscv)
#include <riscv.h>
#else
#include <idt.h>
#include <utils.h>
#endif

Point2D project(Point3D point, int screen_offset_x, int screen_offset_y) {
    Point2D p;
    
    if (point.z <= 0) point.z = 1;

    p.x = ((point.x * 256) / point.z) + screen_offset_x;
    p.y = ((point.y * 256) / point.z) + screen_offset_y;
    
    return p;
}
