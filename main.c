#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <time.h>
#include <signal.h>

#include <sgl.h>
#include <sgl_font.h>

static void *fb_mem;
static struct fb_var_screeninfo fb_info;
static sgl_color_t *panel_buffer;
static unsigned long frames;

static void
fbdev_log(const char *str)
{
    printf("%s", str);
}

static void
fbdev_flush_area(int16_t x, int16_t y, int16_t w, int16_t h, sgl_color_t *src)
{
    sgl_color_t *dest = fb_mem;
    dest += x + y * fb_info.xres;

    for (int i = 0; i < h; i ++) {
        memcpy(dest, src, w * sizeof(sgl_color_t));
        dest += fb_info.xres;
        src += w;
    }
}

static void
timer_handler(int unused)
{
    static unsigned long last;
    printf("render: %lufps\n", frames - last);
    last = frames;
    alarm(1);
}

int
main(int argc, char *argv[])
{
    size_t screen_size, buffer_size;
    int fbdev;

    fbdev = open("/dev/fb0", O_RDWR);
    if (fbdev < 0)
        return fbdev;

    if (ioctl(fbdev, FBIOGET_VSCREENINFO, &fb_info) < 0)
        return -1;

    printf("fb_info.xres: %u\n", fb_info.xres);
    printf("fb_info.yres: %u\n", fb_info.yres);
    printf("fb_info.bpp: %u\n", fb_info.yres);

    if (fb_info.bits_per_pixel != 32)
        return -1;

    screen_size = fb_info.xres * fb_info.yres * fb_info.bits_per_pixel / 8;
    fb_mem = mmap(NULL, screen_size, PROT_READ | PROT_WRITE, MAP_SHARED, fbdev, 0);
    if (fb_mem == (unsigned char*)MAP_FAILED)
        return -1;

    buffer_size = fb_info.xres * 4;
    panel_buffer = malloc(buffer_size * sizeof(sgl_color_t));
    if (!panel_buffer)
        return -1;

    sgl_device_fb_t sgl_fb = {
        .xres = fb_info.xres,
        .yres = fb_info.yres,
        .xres_virtual = fb_info.xres,
        .yres_virtual = fb_info.yres,
        .framebuffer = panel_buffer,
        .framebuffer_size = buffer_size,
        .flush_area = fbdev_flush_area,
    };

    sgl_device_log_register(fbdev_log);
    sgl_device_fb_register(&sgl_fb);
    sgl_init();

    sgl_obj_t *msgbox = sgl_msgbox_create(NULL);
    sgl_obj_set_pos(msgbox, 20, 20);
    sgl_obj_set_size(msgbox, 600, 400);
    sgl_obj_set_style(msgbox, SGL_STYLE_FONT, SGL_FONT(song23));
    sgl_obj_set_style(msgbox, SGL_STYLE_MSGBOX_TITLE, SGL_TEXT("Message Box"));
    sgl_obj_set_style(msgbox, SGL_STYLE_MSGBOX_TEXT, SGL_TEXT("SGL (Small Graphics Library) is a lightweight and fast graphics library"));
    sgl_obj_set_style(msgbox, SGL_STYLE_MSGBOX_APPLY_TEXT, SGL_TEXT("OK"));
    sgl_obj_set_style(msgbox, SGL_STYLE_MSGBOX_CLOSE_TEXT, SGL_TEXT("NO"));
    sgl_obj_set_style(msgbox, SGL_STYLE_BORDER_WIDTH, 2);
    sgl_obj_set_style(msgbox, SGL_STYLE_BORDER_COLOR, SGL_COLOR(SGL_COLOR_LIGHT_GRAY));
    sgl_obj_set_alpha(msgbox, 255);

    sgl_obj_t *button = sgl_button_create(NULL);
    sgl_obj_set_pos(button, 620, 80);
    sgl_obj_set_size(button, 200, 100);
    sgl_obj_set_style(button, SGL_STYLE_RADIUS, 50);
    sgl_obj_set_style(button, SGL_STYLE_BORDER_WIDTH, 2);
    sgl_obj_set_style(button, SGL_STYLE_BORDER_COLOR, SGL_COLOR(SGL_COLOR_BLACK));
    sgl_obj_set_font(button, &song23);
    sgl_obj_set_style(button, SGL_STYLE_TEXT, SGL_TEXT("click me"));

    sgl_obj_t *rect1 = sgl_rect_create(NULL);
    sgl_obj_set_pos(rect1, 20, 20);
    sgl_obj_set_size(rect1, 50, 100);
    sgl_obj_set_alpha(rect1, 255);
    sgl_obj_set_color(rect1, SGL_COLOR_GRAY);
    sgl_obj_set_border_color(rect1, SGL_COLOR_GREEN);
    sgl_obj_set_border_width(rect1, 3);
    sgl_obj_set_radius(rect1, 10);
    sgl_obj_set_alpha(rect1, 150);

    sgl_obj_t *rect2 = sgl_rect_create(NULL);
    sgl_obj_set_pos(rect2, 20, 20);
    sgl_obj_set_size(rect2, 50, 100);
    sgl_obj_set_alpha(rect2, 255);
    sgl_obj_set_color(rect2, SGL_COLOR_BRIGHT_PURPLE);
    sgl_obj_set_border_color(rect2, SGL_COLOR_GREEN);
    sgl_obj_set_border_width(rect2, 3);
    sgl_obj_set_radius(rect2, 10);
    sgl_obj_set_alpha(rect2, 150);

    signal(SIGALRM, timer_handler);
    alarm(1);

    int x = 0;
    int direct = 1;

    for (;;) {
        sgl_task_handle();
        frames++;

        sgl_obj_set_pos(rect1, x, x);
        sgl_obj_set_pos(rect2, x + 60, x + 60);

        if (direct == 1)
            x += 2;
        else
            x -= 2;

        if (x == fb_info.yres)
            direct = 0;
        else if (x == 0)
            direct = 1;
    }

    return 0;
}
