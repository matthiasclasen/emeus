#include <gtk/gtk.h>
#include "emeus.h"

#define EMEUS_TYPE_TEST_APPLICATION_WINDOW (emeus_test_application_window_get_type())

G_DECLARE_FINAL_TYPE (EmeusTestApplicationWindow,
                      emeus_test_application_window,
                      EMEUS, TEST_APPLICATION_WINDOW,
                      GtkApplicationWindow)

#define EMEUS_TYPE_TEST_APPLICATION (emeus_test_application_get_type())

G_DECLARE_FINAL_TYPE (EmeusTestApplication,
                      emeus_test_application,
                      EMEUS, TEST_APPLICATION,
                      GtkApplication)

struct _EmeusTestApplicationWindow
{
  GtkApplicationWindow parent_instance;

  GtkWidget *main_box;
  GtkWidget *layout;
  GtkWidget *quit_button;
};

struct _EmeusTestApplication
{
  GtkApplication parent_instance;

  GtkWidget *window;
};

G_DEFINE_TYPE (EmeusTestApplicationWindow,
               emeus_test_application_window,
               GTK_TYPE_APPLICATION_WINDOW)

static void
emeus_test_application_window_class_init (EmeusTestApplicationWindowClass *klass)
{
}

static gboolean
draw_cb (GtkWidget   *area,
         cairo_t     *cr)
{
  GdkRGBA color;
  const char *c;

  c = (const char *)g_object_get_data (G_OBJECT (area), "color");
  gdk_rgba_parse (&color, c);

  gdk_cairo_set_source_rgba (cr, &color);
  cairo_rectangle (cr, 0, 0,
                   gtk_widget_get_allocated_width (area),
                   gtk_widget_get_allocated_height (area));
  cairo_fill (cr);

  return TRUE;
}

GtkWidget *
new_block (const char *color)
{
  GtkWidget *da;

  da = gtk_drawing_area_new ();
  g_signal_connect (da, "draw", G_CALLBACK (draw_cb), NULL);
  g_object_set_data (G_OBJECT (da), "color", (gpointer)color);

  return da;
}

static void
build_grid (EmeusTestApplicationWindow *self)
{
  EmeusConstraintLayout *layout = (EmeusConstraintLayout *) self->layout;

  GtkWidget *button1 = gtk_button_new_with_label ("Child 1");
  emeus_constraint_layout_pack (layout, button1, "child1", NULL);
  gtk_widget_show (button1);

  GtkWidget *button2 = gtk_button_new_with_label ("Child 2");
  emeus_constraint_layout_pack (layout, button2, "child2", NULL);
  gtk_widget_show (button2);

  GtkWidget *button3 = gtk_button_new_with_label ("Child 3");
  emeus_constraint_layout_pack (layout, button3, "child3", NULL);
  gtk_widget_show (button3);

  GtkWidget *button4 = gtk_button_new_with_label ("Child 4");
  emeus_constraint_layout_pack (layout, button4, "child4", NULL);
  gtk_widget_show (button4);

  GtkWidget *da1 = new_block ("red");
  emeus_constraint_layout_pack (layout, da1, "da1", NULL);
  gtk_widget_show (da1);

  GtkWidget *da2 = new_block ("blue");
  emeus_constraint_layout_pack (layout, da2, "da2", NULL);
  gtk_widget_show (da2);

  GtkWidget *da3 = new_block ("yellow");
  emeus_constraint_layout_pack (layout, da3, "da3", NULL);
  gtk_widget_show (da3);

  EmeusConstraintLayoutGroup *group = emeus_constraint_layout_create_group (layout, 3, 3, 1, 1);
  emeus_constraint_layout_pack (layout, GTK_WIDGET (group),
                                "grid1",
                                emeus_constraint_new (group, EMEUS_CONSTRAINT_ATTRIBUTE_LEFT,
                                                      EMEUS_CONSTRAINT_RELATION_EQ,
                                                      NULL, EMEUS_CONSTRAINT_ATTRIBUTE_LEFT,
                                                      1.0, 0.0, EMEUS_CONSTRAINT_STRENGTH_REQUIRED),
                                emeus_constraint_new (group, EMEUS_CONSTRAINT_ATTRIBUTE_RIGHT,
                                                      EMEUS_CONSTRAINT_RELATION_EQ,
                                                      NULL, EMEUS_CONSTRAINT_ATTRIBUTE_RIGHT,
                                                      1.0, 0.0, EMEUS_CONSTRAINT_STRENGTH_REQUIRED),
                                emeus_constraint_new (group, EMEUS_CONSTRAINT_ATTRIBUTE_TOP,
                                                      EMEUS_CONSTRAINT_RELATION_EQ,
                                                      NULL, EMEUS_CONSTRAINT_ATTRIBUTE_TOP,
                                                      1.0, 0.0, EMEUS_CONSTRAINT_STRENGTH_REQUIRED),
                                emeus_constraint_new (group, EMEUS_CONSTRAINT_ATTRIBUTE_BOTTOM,
                                                      EMEUS_CONSTRAINT_RELATION_EQ,
                                                      NULL, EMEUS_CONSTRAINT_ATTRIBUTE_BOTTOM,
                                                      1.0, 0.0, EMEUS_CONSTRAINT_STRENGTH_REQUIRED),
                                NULL);
  emeus_constraint_layout_add_constraints (layout,
                                           emeus_constraint_new (button1, EMEUS_CONSTRAINT_ATTRIBUTE_WIDTH,
                                                                 EMEUS_CONSTRAINT_RELATION_EQ,
                                                                 button3, EMEUS_CONSTRAINT_ATTRIBUTE_WIDTH,
                                                                 1.0, 0.0, EMEUS_CONSTRAINT_STRENGTH_REQUIRED),
                                           emeus_constraint_new (button2, EMEUS_CONSTRAINT_ATTRIBUTE_HEIGHT,
                                                                 EMEUS_CONSTRAINT_RELATION_EQ,
                                                                 button4, EMEUS_CONSTRAINT_ATTRIBUTE_HEIGHT,
                                                                 1.0, 0.0, EMEUS_CONSTRAINT_STRENGTH_REQUIRED),
                                           NULL);

  EmeusConstraintLayoutGroup *group2 = emeus_constraint_layout_create_group (layout, 3, 3, 1, 1);
  emeus_constraint_layout_pack (layout, GTK_WIDGET (group2), "grid2", NULL);

  emeus_constraint_layout_group_attach (group, button1, 0, 2, 0, 1);
  emeus_constraint_layout_group_attach (group, button2, 0, 1, 1, 3);
  emeus_constraint_layout_group_attach (group, button3, 1, 3, 2, 3);
  emeus_constraint_layout_group_attach (group, button4, 2, 3, 0, 2);
  emeus_constraint_layout_group_attach (group, GTK_WIDGET (group2), 1, 2, 1, 2);

  emeus_constraint_layout_group_attach (group2, da1, 0, 1, 0, 1);
  emeus_constraint_layout_group_attach (group2, da2, 1, 2, 1, 2);
  emeus_constraint_layout_group_attach (group2, da3, 2, 3, 2, 3);
}

static void
build_grid2 (EmeusTestApplicationWindow *self)
{
  EmeusConstraintLayout *layout = (EmeusConstraintLayout *) self->layout;

  GtkWidget *da1 = new_block ("red");
  emeus_constraint_layout_pack (layout, da1, "da1", NULL);
  gtk_widget_show (da1);

  GtkWidget *da2 = new_block ("blue");
  emeus_constraint_layout_pack (layout, da2, "da2", NULL);
  gtk_widget_show (da2);

  GtkWidget *da3 = new_block ("yellow");
  emeus_constraint_layout_pack (layout, da3, "da3", NULL);
  gtk_widget_show (da3);

  GtkWidget *da4 = new_block ("green");
  emeus_constraint_layout_pack (layout, da4, "da4", NULL);
  gtk_widget_show (da4);

  GtkWidget *da5 = new_block ("purple");
  emeus_constraint_layout_pack (layout, da5, "da5", NULL);
  gtk_widget_show (da5);

  emeus_constraint_layout_add_constraints (layout,
                                emeus_constraint_new_constant (da1, EMEUS_CONSTRAINT_ATTRIBUTE_HEIGHT,
                                                               EMEUS_CONSTRAINT_RELATION_GE,
                                                               20.0, EMEUS_CONSTRAINT_STRENGTH_REQUIRED),
                                emeus_constraint_new_constant (da1, EMEUS_CONSTRAINT_ATTRIBUTE_WIDTH,
                                                               EMEUS_CONSTRAINT_RELATION_GE,
                                                               20.0, EMEUS_CONSTRAINT_STRENGTH_REQUIRED),
                                emeus_constraint_new_constant (da2, EMEUS_CONSTRAINT_ATTRIBUTE_HEIGHT,
                                                               EMEUS_CONSTRAINT_RELATION_GE,
                                                               20.0, EMEUS_CONSTRAINT_STRENGTH_REQUIRED),
                                emeus_constraint_new_constant (da3, EMEUS_CONSTRAINT_ATTRIBUTE_HEIGHT,
                                                               EMEUS_CONSTRAINT_RELATION_GE,
                                                               20.0, EMEUS_CONSTRAINT_STRENGTH_REQUIRED),
                                emeus_constraint_new_constant (da4, EMEUS_CONSTRAINT_ATTRIBUTE_HEIGHT,
                                                               EMEUS_CONSTRAINT_RELATION_GE,
                                                               20.0, EMEUS_CONSTRAINT_STRENGTH_REQUIRED),
                                emeus_constraint_new_constant (da5, EMEUS_CONSTRAINT_ATTRIBUTE_HEIGHT,
                                                               EMEUS_CONSTRAINT_RELATION_GE,
                                                               20.0, EMEUS_CONSTRAINT_STRENGTH_REQUIRED),
                                NULL);
  EmeusConstraintLayoutGroup *group1 = emeus_constraint_layout_create_group (layout, 1, 3, 1, 1);
  emeus_constraint_layout_pack (layout, GTK_WIDGET (group1), "grid1",
                                emeus_constraint_new (group1, EMEUS_CONSTRAINT_ATTRIBUTE_LEFT,
                                                      EMEUS_CONSTRAINT_RELATION_EQ,
                                                      NULL, EMEUS_CONSTRAINT_ATTRIBUTE_LEFT,
                                                      1.0, 0.0, EMEUS_CONSTRAINT_STRENGTH_REQUIRED),
                                emeus_constraint_new (group1, EMEUS_CONSTRAINT_ATTRIBUTE_RIGHT,
                                                      EMEUS_CONSTRAINT_RELATION_EQ,
                                                      NULL, EMEUS_CONSTRAINT_ATTRIBUTE_RIGHT,
                                                      1.0, 0.0, EMEUS_CONSTRAINT_STRENGTH_REQUIRED),
                                NULL);

  EmeusConstraintLayoutGroup *group2 = emeus_constraint_layout_create_group (layout, 3, 1, 1, 1);
  emeus_constraint_layout_pack (layout, GTK_WIDGET (group2), "grid2",
                                emeus_constraint_new (group2, EMEUS_CONSTRAINT_ATTRIBUTE_TOP,
                                                      EMEUS_CONSTRAINT_RELATION_EQ,
                                                      NULL, EMEUS_CONSTRAINT_ATTRIBUTE_TOP,
                                                      1.0, 0.0, EMEUS_CONSTRAINT_STRENGTH_REQUIRED),
                                emeus_constraint_new (group2, EMEUS_CONSTRAINT_ATTRIBUTE_BOTTOM,
                                                      EMEUS_CONSTRAINT_RELATION_EQ,
                                                      NULL, EMEUS_CONSTRAINT_ATTRIBUTE_BOTTOM,
                                                      1.0, 0.0, EMEUS_CONSTRAINT_STRENGTH_REQUIRED),
                                NULL);

  emeus_constraint_layout_group_attach (group1, da1, 0, 1, 0, 1);
  emeus_constraint_layout_group_attach (group1, da2, 1, 2, 0, 1);
  emeus_constraint_layout_group_attach (group1, da3, 2, 3, 0, 1);

  emeus_constraint_layout_group_attach (group2, da4, 0, 1, 0, 1);
  emeus_constraint_layout_group_attach (group2, da2, 0, 1, 1, 2);
  emeus_constraint_layout_group_attach (group2, da5, 0, 1, 2, 3);
}

static void
emeus_test_application_window_init (EmeusTestApplicationWindow *self)
{
  GtkWidget *box, *layout, *button;

  gtk_window_set_title (GTK_WINDOW (self), "Grid layout");

  box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 6);
  gtk_container_add (GTK_CONTAINER (self), box);
  gtk_widget_show (box);
  self->main_box = box;

  layout = emeus_constraint_layout_new ();
  gtk_widget_set_hexpand (layout, TRUE);
  gtk_widget_set_vexpand (layout, TRUE);
  gtk_container_add (GTK_CONTAINER (box), layout);
  gtk_widget_show (layout);
  self->layout = layout;

  button = gtk_button_new_with_label ("Quit");
  gtk_widget_set_hexpand (layout, TRUE);
  gtk_container_add (GTK_CONTAINER (box), button);
  gtk_widget_show (button);
  self->quit_button = button;

  g_signal_connect_swapped (self->quit_button,
                            "clicked", G_CALLBACK (gtk_widget_destroy),
                            self);

  build_grid2 (self);
}

static GtkWidget *
emeus_test_application_window_new (EmeusTestApplication *app)
{
  return g_object_new (EMEUS_TYPE_TEST_APPLICATION_WINDOW,
                       "application", app,
                       NULL);
}

G_DEFINE_TYPE (EmeusTestApplication,
               emeus_test_application,
               GTK_TYPE_APPLICATION)

static void
emeus_test_application_activate (GApplication *application)
{
  EmeusTestApplication *self = EMEUS_TEST_APPLICATION (application);

  if (self->window == NULL)
    self->window = emeus_test_application_window_new (self);

  gtk_widget_show (self->window);

  gtk_window_present (GTK_WINDOW (self->window));
}

static void
emeus_test_application_class_init (EmeusTestApplicationClass *klass)
{
  GApplicationClass *application_class = G_APPLICATION_CLASS (klass);

  application_class->activate = emeus_test_application_activate;
}

static void
emeus_test_application_init (EmeusTestApplication *self)
{
}

static GApplication *
emeus_test_application_new (void)
{
  return g_object_new (EMEUS_TYPE_TEST_APPLICATION,
                       "application-id", "io.github.ebassi.EmeusTestApplication",
                       NULL);
}

int
main (int argc, char *argv[])
{
  return g_application_run (emeus_test_application_new (), argc, argv);
}
