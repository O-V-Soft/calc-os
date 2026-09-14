#include <video.h>
#include <mouse.h>
#include <stdint.h>

#if defined(__riscv)
#include <riscv.h>
#else
#include <idt.h>
#include <utils.h>
#endif

Point2D project(Point3D point) {
    Point2D result;
    int xs = ((point.x * 256) / point.z) + (SCREEN_WIDTH / 2);
    int ys = ((point.y * 256) / point.z) + (SCREEN_HEIGHT / 2);

    result.x = xs;
    result.y = ys;

    return result;
}
