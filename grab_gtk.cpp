#include <gtk/gtk.h>

GdkDisplay *display;
GdkScreen  *screen;
GdkDevice *pointer;

GtkWidget *image;

gint width;
gint height;

gboolean update_image(gpointer data)
{
  gint x, y;

  gdk_device_get_position(pointer, &screen, &x, &y);


  GdkPixbuf *gpb = gdk_pixbuf_get_from_window(gdk_get_default_root_window(),x-width/2, y-height/2, width, height);

  int channels = gdk_pixbuf_get_n_channels(gpb);
  int rowstride = gdk_pixbuf_get_rowstride(gpb);
  guchar* pixels = gdk_pixbuf_get_pixels (gpb);

  pixels = pixels + height/2 * rowstride + width/2 * channels;

  //printf("hi, r: %d, g: %d, b: %d\n", pixels[0], pixels[1], pixels[2]);

  gtk_image_set_from_pixbuf((GtkImage*)image, gpb);
  gtk_widget_queue_draw(image);

  g_object_unref (gpb);

  return true;
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

  width  = gdk_screen_get_width(screen) / 8 ;
  height = gdk_screen_get_height(screen) / 8;

  window = gtk_application_window_new (app);
  gtk_window_set_title (GTK_WINDOW (window), "Window");

  button_box = gtk_button_box_new (GTK_ORIENTATION_HORIZONTAL);
  gtk_container_add (GTK_CONTAINER (window), button_box);

  image = gtk_image_new ();
  gtk_container_add (GTK_CONTAINER (button_box), image);

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
  g_timeout_add (1000/60, update_image, NULL);

  status = g_application_run (G_APPLICATION (app), argc, argv);
  g_object_unref (app);

  return status;
}
