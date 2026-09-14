#include <video.h>
#include <mouse.h>
#include <stdint.h>
#include <math.h>

#if defined(__riscv)
#include <riscv.h>
#else
#include <idt.h>
#include <utils.h>
#endif

Point3D rotate_y(Point3D p, int angle) {
    Point3D rotated;

    angle = angle % 360;
    if (angle < 0) angle += 360;

    int sin_val = SIN_TABLE[angle];
    int cos_val = SIN_TABLE[(angle + 90) % 360]; 

    rotated.x = (p.x * cos_val - p.z * sin_val) / 1024;
    rotated.z = (p.x * sin_val + p.z * cos_val) / 1024;
    rotated.y = p.y;

    return rotated;
}

Point3D rotate_x(Point3D p, int angle) {
    Point3D rotated;

    angle = angle % 360;
    if (angle < 0) angle += 360;

    int sin_val = SIN_TABLE[angle];
    int cos_val = SIN_TABLE[(angle + 90) % 360]; 

    rotated.y = (p.y * cos_val - p.z * sin_val) / 1024;
    rotated.z = (p.y * sin_val + p.z * cos_val) / 1024;
    rotated.x = p.x;

    return rotated;
}
