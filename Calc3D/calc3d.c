#include <video.h>
#include <mouse.h>
#include <stdint.h>

#if defined(__riscv)
#include <riscv.h>
#else
#include <idt.h>
#include <utils.h>
#endif

int is_face_visible(Point2D p0, Point2D p1, Point2D p2) {
    return ((p1.x - p0.x) * (p2.y - p0.y) - (p1.y - p0.y) * (p2.x - p0.x)) > 0;
}

void draw_cube(int pos_x, int pos_y, int pos_z, int angle_y, int angle_x) {
    Point2D points2d[8];

    for (int i = 0; i < 8; i++) {
        Point3D node = cube_nodes[i];
        node = rotate_y(node, angle_y);
        node = rotate_x(node, angle_x);
        node.z += pos_z;
        points2d[i] = project(node, pos_x, pos_y);
    }

    struct Face { 
        int v[4]; 
        uint8_t color; 
    } faces[6] = {
        {{0, 1, 2, 3}, COLOR_DARK},   
        {{4, 5, 1, 0}, COLOR_DARK_GRAY},    
        {{4, 0, 3, 7}, COLOR_DARK_GRAY}, 
        {{1, 5, 6, 2}, COLOR_DARK_GRAY}, 
        {{3, 2, 6, 7}, COLOR_DARK_GRAY}, 
        {{5, 4, 7, 6}, COLOR_DARK_GRAY}       
    };

    for (int i = 0; i < 6; i++) {
        Point2D p0 = points2d[faces[i].v[0]];
        Point2D p1 = points2d[faces[i].v[1]];
        Point2D p2 = points2d[faces[i].v[2]];
        Point2D p3 = points2d[faces[i].v[3]];

        if (is_face_visible(p0, p1, p2)) {
            draw_filled_triangle(p0, p1, p2, faces[i].color);
            draw_filled_triangle(p0, p2, p3, faces[i].color);
        }
    }
}

void draw_textured_cube(int pos_x, int pos_y, int pos_z, int angle_x) {
    Point2D points2d[8];

    for (int i = 0; i < 8; i++) {
        Point3D node = cube_nodes[i];
        node = rotate_x(node, angle_x);
        node.z += pos_z;
        points2d[i] = project(node, pos_x, pos_y);
    }

    struct Face { 
        int v[4]; 
    } faces[6] = {
        {0, 1, 2, 3},  
        {4, 5, 1, 0}, 
        {4, 0, 3, 7}, 
        {1, 5, 6, 2},
        {3, 2, 6, 7}, 
        {5, 4, 7, 6}     
    };

    for (int i = 0; i < 6; i++) {
        Point2D p0 = points2d[faces[i].v[0]];
        Point2D p1 = points2d[faces[i].v[1]];
        Point2D p2 = points2d[faces[i].v[2]];
        Point2D p3 = points2d[faces[i].v[3]];

        if (is_face_visible(p0, p1, p2)) {
            draw_textured_triangle(p0, p1, p2);
            draw_textured_triangle(p0, p2, p3);
        }
    }
}
