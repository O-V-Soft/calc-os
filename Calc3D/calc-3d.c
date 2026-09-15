#include <video.h>
#include <mouse.h>
#include <stdint.h>

#if defined(__riscv)
#include <riscv.h>
#else
#include <idt.h>
#include <utils.h>
#endif

void draw_cube(int pos_x, int pos_y, int pos_z, int angle_y, int angle_x) {
    Point2D points2d[8];

    for (int i = 0; i < 8; i++) {
        Point3D node = cube_nodes[i];

        node = rotate_y(node, angle_y);
        node = rotate_x(node, angle_x);

        node.z += pos_z;

        points2d[i] = project(node, pos_x, pos_y);
    }

    for (int i = 0; i < 12; i++) {
        Point2D p1 = points2d[cube_edges[i].a];
        Point2D p2 = points2d[cube_edges[i].b];

        draw_line(p1.x, p1.y, p2.x, p2.y, COLOR_WHITE);
    }
}
