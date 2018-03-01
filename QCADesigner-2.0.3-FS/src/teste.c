/*!
* \file grid_test.c
*
* \brief Test program for a grid of Gtk Widgets.
*/
 
#include <stdio.h>
#include <gtk/gtk.h>
 
#define GRID(obj) GTK_CHECK_CAST (obj, grid_get_type (), Grid)
#define IS_GRID(obj) GTK_CHECK_TYPE (obj, grid_get_type ())
 
typedef struct _Grid Grid;
typedef struct _GridClass GridClass;
 
struct _Grid
{
GtkContainer parent;
GtkWidget *widget;
};
 
struct _GridClass
{
GtkContainerClass parent_class;
};
 
G_DEFINE_TYPE (Grid, grid, GTK_TYPE_CONTAINER)
 
GtkType grid_get_type (void);
static void grid_class_init (GridClass *class);
static void grid_init (Grid *grid);
GtkWidget *grid_new (GtkWidget *widget);
static void grid_realize (GtkWidget *widget);
static void grid_size_request (GtkWidget *widget, GtkRequisition *requisition);
static void grid_size_allocate (GtkWidget *widget, GtkAllocation *allocation);
 
static void
grid_class_init (GridClass *class)
{
GtkWidgetClass *widget_class;
widget_class = (GtkWidgetClass*) class;
widget_class->size_request = grid_size_request;
widget_class->size_allocate = grid_size_allocate;
widget_class->realize = grid_realize;
}
 
static void
grid_init (Grid *grid)
{
grid->widget = NULL;
}
 
GtkWidget*
grid_new (GtkWidget *widget)
{
Grid *grid;
 
grid = g_object_new (grid_get_type (), NULL);
grid->widget = widget;
gtk_widget_set_parent (grid->widget, GTK_WIDGET (grid));
return GTK_WIDGET (grid);
}
 
static void
grid_realize (GtkWidget *widget)
{
GdkWindowAttr attributes;
gint attributes_mask;
 
g_return_if_fail (widget != NULL);
g_return_if_fail (IS_GRID (widget));
GTK_WIDGET_SET_FLAGS (widget, GTK_REALIZED);
attributes.x = widget->allocation.x;
attributes.y = widget->allocation.y;
attributes.width = widget->allocation.width;
attributes.height = widget->allocation.height;
attributes.wclass = GDK_INPUT_OUTPUT;
attributes.window_type = GDK_WINDOW_CHILD;
attributes.event_mask = gtk_widget_get_events (widget) | GDK_EXPOSURE_MASK;
attributes.visual = gtk_widget_get_visual (widget);
attributes.colormap = gtk_widget_get_colormap (widget);
attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL | GDK_WA_COLORMAP;
widget->window = gdk_window_new
(
widget->parent->window,
&attributes,
attributes_mask
);
widget->style = gtk_style_attach (widget->style, widget->window);
gdk_window_set_user_data (widget->window, widget);
}
 
static void
grid_size_request (GtkWidget *widget, GtkRequisition *requisition)
{
Grid *grid = GRID (widget);
 
if (grid->widget && GTK_WIDGET_VISIBLE (grid->widget))
{
GtkRequisition child_requisition;
gtk_widget_size_request (grid->widget, &child_requisition);
requisition->width += child_requisition.width;
requisition->height += child_requisition.height;
}
}
 
static void
grid_size_allocate (GtkWidget *widget, GtkAllocation *allocation)
{
Grid *grid = GRID (widget);
 
if (grid->widget && GTK_WIDGET_VISIBLE (grid->widget))
{
gtk_widget_size_allocate (grid->widget, allocation);
}
}
 
int
main (int argc, char *argv[])
{
GtkWidget *window;
GtkWidget *grid;
GtkWidget *button;
 
gtk_init (&argc, &argv);
button = gtk_button_new_with_label ("Button");
grid = grid_new (button);
gtk_widget_show (button);
window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
g_signal_connect
(
G_OBJECT (window),
"destroy",
G_CALLBACK (gtk_main_quit),
NULL
);
gtk_container_add (GTK_CONTAINER (window), grid);
gtk_widget_show (grid);
gtk_widget_show (window);
gtk_main ();
return 0;
}
 
/* EOF */

