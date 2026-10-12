#include <stdint.h>
#include <video.h>
#include <fat.h>
#include <utils.h>

#define ICO_FS_NAME  "FILE.ICO"

#define ICO_MAXSIZE  65536

static uint8_t s_ico_buf[ICO_MAXSIZE];

static uint8_t s_folder_pixels[ICON_H][ICON_W];

static uint8_t s_folder_alpha[ICON_H][ICON_W];

static int s_icon_loaded = 0;

static uint8_t s_vga_pal[256 * 3];

static void read_vga_palette() {
    outb(0x3C7, 0);
    for (int i = 0; i < 256 * 3; i++) {
        s_vga_pal[i] = inb(0x3C9);
    }
}

static uint8_t ico_nearest_vga(uint8_t r, uint8_t g, uint8_t b) {
    uint32_t best_dist = 0xFFFFFFFFU;
    uint8_t  best_idx  = 0;

    for (int i = 0; i < 256; i++) {
        int dr = (int)r - (int)(s_vga_pal[i * 3] * 4);
        int dg = (int)g - (int)(s_vga_pal[i * 3 + 1] * 4);
        int db = (int)b - (int)(s_vga_pal[i * 3 + 2] * 4);
        uint32_t dist = (uint32_t)(dr * dr + dg * dg + db * db);
        if (dist < best_dist) {
            best_dist = dist;
            best_idx  = (uint8_t)i;
            if (dist == 0) break;
        }
    }

    return best_idx;
}

static uint16_t ico_u16(const uint8_t* b, int off) {
    return (uint16_t)(b[off] | ((uint16_t)b[off + 1] << 8));
}

static uint32_t ico_u32(const uint8_t* b, int off) {
    return (uint32_t)b[off]
         | ((uint32_t)b[off + 1] <<  8)
         | ((uint32_t)b[off + 2] << 16)
         | ((uint32_t)b[off + 3] << 24);
}

void icon_init() {
    s_icon_loaded = 0;
    char ico_name_11[11];
    format_fat_name(ICO_FS_NAME, ico_name_11);

    read_file(ico_name_11, s_ico_buf);

    if (ico_u16(s_ico_buf, 2) != 1) return;
    int count = (int)ico_u16(s_ico_buf, 4);

    int best = -1;
    int best_w = 0x7FFF;

    for (int i = 0; i < count; i++) {
        int eoff = 6 + i * 16;
        int w    = (int)s_ico_buf[eoff];
        if (w == 0) w = 256;
        int bc   = (int)ico_u16(s_ico_buf, eoff + 6);
        uint32_t img_off = ico_u32(s_ico_buf, eoff + 12);
        uint32_t img_sz  = ico_u32(s_ico_buf, eoff + 8);

        if (bc == 32 && w >= ICON_W && w <= best_w) {
            best_w = w;
            best   = i;
        }
    }

    if (best < 0) {
        for (int i = 0; i < count; i++) {
            int eoff = 6 + i * 16;
            int bc = (int)ico_u16(s_ico_buf, eoff + 6);
            uint32_t img_off = ico_u32(s_ico_buf, eoff + 12);
        }
    }

    if (best < 0) return;

    int eoff   = 6 + best * 16;
    uint32_t img_off = ico_u32(s_ico_buf, eoff + 12);

    const uint8_t* bmp = s_ico_buf + img_off;

    int bmp_w = (int)ico_u32(bmp, 4);
    int bmp_dh = (int)ico_u32(bmp, 8);
    int bmp_bpp = (int)ico_u16(bmp, 14);

    if (bmp_bpp != 32) return;
    if (bmp_w < 1 || bmp_dh < 2) return;

    int bmp_h = bmp_dh / 2;

    int stride  = bmp_w * 4;

    const uint8_t* pixels = bmp + 40;

    int draw_w = bmp_w  < ICON_W ? bmp_w  : ICON_W;
    int draw_h = bmp_h  < ICON_H ? bmp_h  : ICON_H;

    read_vga_palette();

    for (int row = 0; row < ICON_H; row++) {
        if (row < draw_h) {
            int rb         = bmp_h - 1 - row;
            const uint8_t* src = pixels + rb * stride;

            for (int col = 0; col < ICON_W; col++) {
                if (col < draw_w) {
                    uint8_t b_ch = src[col * 4 + 0];
                    uint8_t g_ch = src[col * 4 + 1];
                    uint8_t r_ch = src[col * 4 + 2];
                    uint8_t a_ch = src[col * 4 + 3];

                    if (a_ch >= 128) {
                        int max_c = r_ch > g_ch ? (r_ch > b_ch ? r_ch : b_ch) : (g_ch > b_ch ? g_ch : b_ch);
                        int min_c = r_ch < g_ch ? (r_ch < b_ch ? r_ch : b_ch) : (g_ch < b_ch ? g_ch : b_ch);

                        if ((max_c - min_c < 20) && (max_c > 40) && (max_c < 235)) {
                            r_ch = 255;
                            g_ch = 255;
                            b_ch = 255;
                        }

                        s_folder_pixels[row][col] = ico_nearest_vga(r_ch, g_ch, b_ch);
                        s_folder_alpha [row][col] = 1;
                    } else {
                        s_folder_pixels[row][col] = 0;
                        s_folder_alpha [row][col] = 0;
                    }
                } else {
                    s_folder_pixels[row][col] = 0;
                    s_folder_alpha [row][col] = 0;
                }
            }
        } else {
            for (int col = 0; col < ICON_W; col++) {
                s_folder_pixels[row][col] = 0;
                s_folder_alpha [row][col] = 0;
            }
        }
    }

    s_icon_loaded = 1;
}

void draw_icon(int x, int y) {
    if (!s_icon_loaded) return;

    for (int row = 0; row < ICON_H; row++) {
        for (int col = 0; col < ICON_W; col++) {
            if (s_folder_alpha[row][col]) {
                put_pixel(x + col, y + row, s_folder_pixels[row][col]);
            }
        }
    }
}

