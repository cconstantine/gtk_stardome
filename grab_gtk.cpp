#include <gtk/gtk.h>

#include "LedMatrix.h"
#include "opc_client.h"

#include "bcm_host.h"

#ifndef ALIGN_TO_16
#define ALIGN_TO_16(x)  ((x + 15) & ~15)
#endif

GdkDisplay *display;
GdkScreen  *screen;
GdkDevice *pointer;

GtkWidget *image;
GdkPixbuf *gpb = NULL;


#define DEFAULT_DELAY 0
#define DEFAULT_DISPLAY_NUMBER 0

uint32_t displayNumber = DEFAULT_DISPLAY_NUMBER;
VC_IMAGE_TYPE_T imageType = VC_IMAGE_RGBA32;
int8_t dmxBytesPerPixel  = 4;
DISPMANX_DISPLAY_HANDLE_T displayHandle;
DISPMANX_MODEINFO_T modeInfo;
DISPMANX_RESOURCE_HANDLE_T resourceHandle;
void *dmxImagePtr;
int32_t dmxPitch;

gint width;
gint height;
gint scale = 4;

LedMatrix matrix;
OPCClient opc_client;
void send_leds();
gboolean update_image(gpointer data)
{
  //fprintf(stderr, "update_image\n");
  gint x, y;

  gdk_device_get_position(pointer, &screen, &x, &y);

   int  result = vc_dispmanx_snapshot(displayHandle,
                                  resourceHandle,
                                  DISPMANX_NO_ROTATE);
   if (result != 0)
    {
        vc_dispmanx_resource_delete(resourceHandle);
        vc_dispmanx_display_close(displayHandle);

        fprintf(stderr,
                "vc_dispmanx_snapshot() failed\n");
        exit(EXIT_FAILURE);
    }

    VC_RECT_T rect;
    result = vc_dispmanx_rect_set(&rect, 0, 0, width, height);

    if (result != 0)
    {
        vc_dispmanx_resource_delete(resourceHandle);
        vc_dispmanx_display_close(displayHandle);

        fprintf(stderr,
                "vc_dispmanx_resource_read_data() failed\n");
        exit(EXIT_FAILURE);
    }

    result = vc_dispmanx_resource_read_data(resourceHandle,
                                            &rect,
                                            dmxImagePtr,
                                            dmxPitch);

  
    if (result != 0)
    {
        vc_dispmanx_resource_delete(resourceHandle);
        vc_dispmanx_display_close(displayHandle);

        fprintf(stderr,
                "vc_dispmanx_resource_read_data() failed\n");

        exit(EXIT_FAILURE);
    }
  send_leds();
  gtk_widget_queue_draw(image);

  return true;
}

void send_leds() {
  //fprintf(stderr, "send_leds\n");
  std::vector<uint8_t> data(matrix.leds.size() * 3, 0);
  for (std::vector<Point>::iterator it = matrix.leds.begin() ; it != matrix.leds.end(); ++it) {
    Point led = *it;
    guchar* pixel = (guchar*)dmxImagePtr + led.y * width * dmxBytesPerPixel + led.x * dmxBytesPerPixel;
    
    data.push_back(pixel[0]);
    data.push_back(pixel[1]);
    data.push_back(pixel[2]);
  }
  opc_client.write(data);
}

gboolean
draw_leds(GtkWidget *widget, cairo_t *cr, gpointer data)
{
  //fprintf(stderr, "draw_leds\n");
  int pixel_width = 4/scale;

  cairo_set_operator(cr, CAIRO_OPERATOR_LIGHTEN);
  for (std::vector<Point>::iterator it = matrix.leds.begin() ; it != matrix.leds.end(); ++it) {
    Point led = *it;
    guchar* pixel = (guchar*)dmxImagePtr + led.y * width * dmxBytesPerPixel + led.x * dmxBytesPerPixel;

    cairo_set_source_rgb (cr, float(pixel[0]) / 255, float(pixel[1]) / 255, float(pixel[2]) / 255);
    cairo_rectangle(cr, led.x/scale-pixel_width/2, led.y/scale-pixel_width/2, pixel_width, pixel_width);
    cairo_fill(cr);
  }

  return false;
}

static void
activate (GtkApplication* app,
          gpointer        user_data)
{
  GtkWidget *window;
  GtkWidget *button_box;

  display = gdk_display_get_default();
  screen  = gdk_display_get_default_screen(display);
  pointer = gdk_device_manager_get_client_pointer(gdk_display_get_device_manager(display) );

  width  = gdk_screen_get_width(screen);
  height = gdk_screen_get_height(screen);

  window = gtk_application_window_new (app);
  gtk_window_set_title (GTK_WINDOW (window), "Window");
  gtk_window_set_default_size(GTK_WINDOW(window), width/scale, height/scale);
  GdkRGBA black = {.0, .0, .0, 1.0};
  gtk_widget_override_background_color(window, GTK_STATE_FLAG_NORMAL, &black);
  //gtk_widget_override_background_color(window, GTK_STATE_FLAG_NORMAL, GdkRGBA(0.0,0.0,0.0,0.0));


  image = gtk_drawing_area_new ();
  g_signal_connect (image, "draw", G_CALLBACK (draw_leds), NULL);
  gtk_container_add (GTK_CONTAINER (window), image);

  bcm_host_init();
  displayHandle = vc_dispmanx_display_open(displayNumber);
   if (displayHandle == 0)
    {
        fprintf(stderr,
                "unable to open display %d\n",
                displayNumber);

        exit(EXIT_FAILURE);
    }
    int result = vc_dispmanx_display_get_info(displayHandle, &modeInfo);

    if (result != 0)
    {
        fprintf(stderr, "unable to get display information\n");

        exit(EXIT_FAILURE);
    }
    printf("modeInfo.width: %d, modeInfo.height: %d\n", modeInfo.width, modeInfo.height);
    dmxPitch = dmxBytesPerPixel * ALIGN_TO_16(width);

    dmxImagePtr = malloc(dmxPitch * height);
    if (dmxImagePtr == NULL)
    {
        exit(EXIT_FAILURE);
    }

    //-------------------------------------------------------------------
    uint32_t vcImagePtr = 0;

    resourceHandle = vc_dispmanx_resource_create(imageType,
                                                 width,
                                                 height,
                                                 &vcImagePtr);

  int x = width / 2;
  int y = height / 2;

  float scale = y / 7.8;
  Point pos_0 = Point(x      , y      );

  Point pos_1 = Point(x + scale *  3.917, y + scale *  1.272);
  Point pos_2 = Point(x + scale *  0    , y + scale *  4.118);
  Point pos_3 = Point(x + scale * -3.917, y + scale *  1.272);
  Point pos_4 = Point(x + scale * -2.421, y + scale * -3.332);
  Point pos_5 = Point(x + scale *  2.421, y + scale * -3.332);

  Point pos_6 = Point(x + scale *  4.423, y + scale *  6.088);
  Point pos_7 = Point(x + scale * -4.423, y + scale *  6.088);
  Point pos_8 = Point(x + scale * -7.157, y + scale * -2.325);
  Point pos_9 = Point(x + scale *  0    , y + scale * -7.526);
  Point pos_A = Point(x + scale *  7.157, y + scale * -2.325);

  matrix.add_strip(pos_1, pos_0, 70);
  matrix.add_strip(pos_1, pos_2, 82);
  matrix.add_strip(pos_1, pos_A, 84);
  matrix.add_strip(pos_1, pos_6, 84);

  matrix.add_strip(pos_2, pos_0, 70);
  matrix.add_strip(pos_2, pos_3, 82);
  matrix.add_strip(pos_2, pos_7, 84);
  matrix.add_strip(pos_2, pos_6, 84);


  matrix.add_strip(pos_3, pos_0, 70);
  matrix.add_strip(pos_3, pos_4, 82);
  matrix.add_strip(pos_3, pos_8, 84);
  matrix.add_strip(pos_3, pos_7, 84);

  matrix.add_strip(pos_4, pos_0, 70);
  matrix.add_strip(pos_4, pos_5, 82);
  matrix.add_strip(pos_4, pos_9, 84);
  matrix.add_strip(pos_4, pos_8, 84);

  matrix.add_strip(pos_5, pos_0, 70);
  matrix.add_strip(pos_5, pos_1, 82);
  matrix.add_strip(pos_5, pos_9, 84);
  matrix.add_strip(pos_5, pos_A, 84);

  opc_client.resolve("stardome.local");
  gtk_widget_show_all (window);
}

int
main (int    argc,
      char **argv)
{
  GtkApplication *app;
  int status;

  app = gtk_application_new ("org.gtk.example", G_APPLICATION_FLAGS_NONE);
  g_signal_connect (app, "activate", G_CALLBACK (activate), NULL);
  g_timeout_add (1000/30, update_image, NULL);

  status = g_application_run (G_APPLICATION (app), argc, argv);
  g_object_unref (app);

  return status;
}
