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
draw (GtkWidget *area,
      cairo_t   *cr,
      gpointer   data)
{
  cairo_set_source_rgb (cr, 1, 0, 1);
  cairo_rectangle (cr, 0, 0,
                   gtk_widget_get_allocated_width (area),
                   gtk_widget_get_allocated_height (area));
  cairo_fill (cr);

  return TRUE;
}

/* Layout:
 *
 *   +-----------------------------+
 *   | +-----------+ +-----------+ |
 *   | |  Child 1  | |  Child 2  | |
 *   | +-----------+ +-----------+ |
 *   | +-------------------------+ |
 *   | |         Child 3         | |
 *   | +-------------------------+ |
 *   +-----------------------------+
 *
 * Visual format:
 *
 *   H:|-8-[view1(==view2)]-12-[view2]-8-|
 *   H:|-8-[view3]-8-|
 *   V:|-8-[view1,view2]-12-[view3(==view1,view2)]-8-|
 *
 * Constraints:
 *
 *   super.start = child1.start - 8
 *   child1.width = child2.width
 *   child1.end = child2.start - 12
 *   child2.end = super.end - 8
 *   super.start = child3.start - 8
 *   child3.end = super.end - 8
 *   super.top = child1.top - 8
 *   super.top = child2.top - 8
 *   child1.bottom = child3.top - 12
 *   child2.bottom = child3.top - 12
 *   child3.height = child1.height
 *   child3.height = child2.height
 *   child3.bottom = super.bottom - 8
 *
 */
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
  emeus_constraint_layout_pack (layout, button4, "child3", NULL);
  gtk_widget_show (button4);

  GtkWidget *button5 = gtk_button_new_with_label ("Child 5");
  emeus_constraint_layout_pack (layout, button5, "child3", NULL);
  gtk_widget_show (button5);

  GtkWidget *child6 = gtk_drawing_area_new ();
  g_signal_connect (child6, "draw", G_CALLBACK (draw), NULL);
  emeus_constraint_layout_pack (layout, child6, "child6", NULL);
  gtk_widget_show (child6);

  GtkWidget *area1 = emeus_constraint_layout_create_stack (layout,
                                                GTK_ORIENTATION_HORIZONTAL,
                                                -8.0,
                                                "area1",
                                                button1, button2, button3, NULL);

  GtkWidget *area2 = emeus_constraint_layout_create_stack (layout,
                                                GTK_ORIENTATION_HORIZONTAL,
                                                -8.0,
                                                "area2",
                                                button4, button5, NULL);
  GtkWidget *area3 = emeus_constraint_layout_create_stack (layout,
                                                GTK_ORIENTATION_VERTICAL,
                                                -8.0,
                                                "area3",
                                                area1, area2, child6, NULL);

  emeus_constraint_layout_add_constraints (layout,
                                /* center area3, 2/3s of the width */
                                emeus_constraint_new (area3,
                                                      EMEUS_CONSTRAINT_ATTRIBUTE_CENTER_X,
                                                      EMEUS_CONSTRAINT_RELATION_EQ,
                                                      NULL,
                                                      EMEUS_CONSTRAINT_ATTRIBUTE_CENTER_X,
                                                      1.0,
                                                      0.0,
                                                      EMEUS_CONSTRAINT_STRENGTH_REQUIRED),
                                emeus_constraint_new (area3,
                                                      EMEUS_CONSTRAINT_ATTRIBUTE_CENTER_Y,
                                                      EMEUS_CONSTRAINT_RELATION_EQ,
                                                      NULL,
                                                      EMEUS_CONSTRAINT_ATTRIBUTE_CENTER_Y,
                                                      1.0,
                                                      -12.0,
                                                      EMEUS_CONSTRAINT_STRENGTH_REQUIRED),
                                emeus_constraint_new (area3,
                                                      EMEUS_CONSTRAINT_ATTRIBUTE_WIDTH,
                                                      EMEUS_CONSTRAINT_RELATION_EQ,
                                                      NULL,
                                                      EMEUS_CONSTRAINT_ATTRIBUTE_WIDTH,
                                                      0.66,
                                                      0.0,
                                                      EMEUS_CONSTRAINT_STRENGTH_REQUIRED),
                                emeus_constraint_new (area2,
                                                      EMEUS_CONSTRAINT_ATTRIBUTE_HEIGHT,
                                                      EMEUS_CONSTRAINT_RELATION_EQ,
                                                      area1,
                                                      EMEUS_CONSTRAINT_ATTRIBUTE_HEIGHT,
                                                      2.0,
                                                      0.0,
                                                      EMEUS_CONSTRAINT_STRENGTH_REQUIRED),
                                emeus_constraint_new (child6,
                                                      EMEUS_CONSTRAINT_ATTRIBUTE_HEIGHT,
                                                      EMEUS_CONSTRAINT_RELATION_EQ,
                                                      button3,
                                                      EMEUS_CONSTRAINT_ATTRIBUTE_WIDTH,
                                                      1.0,
                                                      0.0,
                                                      EMEUS_CONSTRAINT_STRENGTH_REQUIRED),

                                emeus_constraint_new (button2,
                                                      EMEUS_CONSTRAINT_ATTRIBUTE_WIDTH,
                                                      EMEUS_CONSTRAINT_RELATION_EQ,
                                                      button1,
                                                      EMEUS_CONSTRAINT_ATTRIBUTE_WIDTH,
                                                      1.0,
                                                      0.0,
                                                      EMEUS_CONSTRAINT_STRENGTH_REQUIRED),
                                emeus_constraint_new (button5,
                                                      EMEUS_CONSTRAINT_ATTRIBUTE_WIDTH,
                                                      EMEUS_CONSTRAINT_RELATION_EQ,
                                                      button4,
                                                      EMEUS_CONSTRAINT_ATTRIBUTE_WIDTH,
                                                      1.0,
                                                      0.0,
                                                      EMEUS_CONSTRAINT_STRENGTH_REQUIRED),
                                NULL);
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

  build_grid (self);
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
