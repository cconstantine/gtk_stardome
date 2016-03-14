#include <gtk/gtk.h>

#include "LedMatrix.h"

GdkDisplay *display;
GdkScreen  *screen;
GdkDevice *pointer;

GtkWidget *image;
GdkPixbuf *gpb = NULL;

gint width;
gint height;
LedMatrix matrix;
GdkPixbuf *mask;

gboolean update_image(gpointer data)
{
  gint x, y;

  gdk_device_get_position(pointer, &screen, &x, &y);


  GdkPixbuf *gpb = gdk_pixbuf_get_from_window(gdk_get_default_root_window(),x-width/2, y-height/2, width, height);

  int channels = gdk_pixbuf_get_n_channels(gpb);
  int rowstride = gdk_pixbuf_get_rowstride(gpb);
  guchar* pixels = gdk_pixbuf_get_pixels (gpb);

  //printf("hi, r: %d, g: %d, b: %d\n", pixels[0], pixels[1], pixels[2]);
  gdk_pixbuf_composite (mask,
                        gpb,
                        0,
                        0,
                        width,
                        height,
                        0,
                        0,
                        1,
                        1,
                        GDK_INTERP_NEAREST,
                        255);
  GdkPixbuf *toDraw = gdk_pixbuf_scale_simple(gpb, width / 4, height / 4, GDK_INTERP_NEAREST );
  g_object_unref (gpb);

  gtk_image_set_from_pixbuf((GtkImage*)image, toDraw);
  gtk_widget_queue_draw(image);

  g_object_unref (toDraw);


  return true;
}

GdkPixbuf* ledmask_new(int width, int height) {
  cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
  cairo_t *cr = cairo_create(surface);
  //gdk_cairo_set_source_pixbuf(cr, buf, 0, 0);


  cairo_set_source_rgba (cr, 0, 0, 0, 0.9);
  cairo_rectangle(cr, 0, 0, width, height);
  cairo_fill(cr);

  int pixel_width = 3;
  cairo_set_operator(cr, CAIRO_OPERATOR_CLEAR);
  for (std::vector<Point>::iterator it = matrix.leds.begin() ; it != matrix.leds.end(); ++it) {
    Point led = *it;

    cairo_set_source_rgba (cr, 0, 0, 0, 0.0);
    cairo_rectangle(cr, led.x-pixel_width/2, led.y-pixel_width/2, pixel_width, pixel_width);
    cairo_fill(cr);
  }
  cairo_destroy(cr);

  return gdk_pixbuf_get_from_surface (surface,0, 0, width, height);

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

  width  = 800;
  height = 480;

  window = gtk_application_window_new (app);
  gtk_window_set_title (GTK_WINDOW (window), "Window");
  gtk_window_set_default_size(GTK_WINDOW(window), width/4, height/4);
  GdkRGBA black = {.0, .0, .0, 1.0};
  gtk_widget_override_background_color(window, GTK_STATE_FLAG_NORMAL, &black);
  
  image = gtk_image_new ();
  gtk_container_add (GTK_CONTAINER (window), image);

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
  
  mask = ledmask_new(width, height);

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
