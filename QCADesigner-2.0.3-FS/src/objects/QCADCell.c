//////////////////////////////////////////////////////////
// QCADesigner                                          //
// Copyright 2002 Konrad Walus                          //
// All Rights Reserved                                  //
// Author: Konrad Walus                                 //
// Email: qcadesigner@gmail.com                         //
// WEB: http://qcadesigner.ca/                          //
//////////////////////////////////////////////////////////
//******************************************************//
//*********** PLEASE DO NOT REFORMAT THIS CODE *********//
//******************************************************//
// If your editor wraps long lines disable it or don't  //
// save the core files that way. Any independent files  //
// you generate format as you wish.                     //
//////////////////////////////////////////////////////////
// Please use complete names in variables and fucntions //
// This will reduce ramp up time for new people trying  //
// to contribute to the project.                        //
//////////////////////////////////////////////////////////
// This file was contributed by Gabriel Schulhof        //
// (schulhof@atips.ca).                                 //
//////////////////////////////////////////////////////////
// Contents:                                            //
//                                                      //
// The QCA cell.                                        //
//                                                      //
//////////////////////////////////////////////////////////

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib-object.h>

#ifdef GTK_GUI
  #include <gtk/gtk.h>
#endif

//Dayane
#include "../callbacks.h"
//End of Dayane

#include "../generic_utils.h"
#include "../support.h"
#include "../global_consts.h"
#include "../custom_widgets.h"
#include "QCADCell.h"
#include "QCADDOContainer.h"
#ifdef GTK_GUI
  #include "QCADClockCombo.h"
#endif /* def GTK_GUI */
#include "mouse_handlers.h"
#include "QCADCompoundDO.h"
#include "object_helpers.h"
#include "objects_debug.h"
#include "../fileio_helpers.h"

#define QCAD_CELL_LABEL_DEFAULT_OFFSET_Y 1

#define DBG_VAL(s)

#ifdef GTK_GUI
typedef struct
  {
  GtkWidget *tbl ;
  GtkAdjustment *adjCXCell ;
  GtkAdjustment *adjCYCell ;
  GtkAdjustment *adjDotDiam ;
  GtkWidget *cbClock ;
  } DEFAULT_PROPERTIES ;

typedef struct
  {
  GtkWidget *dlg ;
  GtkAdjustment *adjPolarization ;
  GtkWidget *spnPolarization ;
  GtkWidget *txtName ;
  GtkWidget *rbNormal ;
  GtkWidget *rbFixed ;
  GtkWidget *rbIO ;
  GtkWidget *rbInput ;
  GtkWidget *rbOutput ;
  GtkWidget *fmFixed ;
  GtkWidget *lblName ;
  GtkWidget *fmIO ;
  GtkWidget *cbClock ;
  } PROPERTIES ;
#endif /* def GTK_GUI */

#ifdef DESIGNER
extern DropFunction drop_function ;
#endif /* def DESIGNER */

static void qcad_cell_class_init (GObjectClass *klass, gpointer data) ;
static void qcad_cell_instance_init (GObject *object, gpointer data) ;
static void qcad_cell_instance_finalize (GObject *object) ;
static void qcad_cell_compound_do_interface_init (gpointer iface, gpointer interface_data) ;
static void qcad_cell_do_container_interface_init (gpointer iface, gpointer interface_data) ;
static void qcad_cell_set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec) ;
static void qcad_cell_get_property (GObject *object, guint property_id, GValue *value, GParamSpec *pspec) ;

#ifdef GTK_GUI
static gboolean button_pressed (GtkWidget *widget, GdkEventButton *event, gpointer data) ;
#endif /* def GTK_GUI */

static void copy (QCADDesignObject *src, QCADDesignObject *dst) ;
#ifdef GTK_GUI
static void draw (QCADDesignObject *obj, GdkDrawable *dst, GdkFunction rop) ;
#ifdef UNDO_REDO
static gboolean properties (QCADDesignObject *obj, GtkWidget *parent, QCADUndoEntry **pentry) ;
#else
static gboolean properties (QCADDesignObject *obj, GtkWidget *parent) ;
#endif /* def UNDO_REDO */
static GCallback default_properties_ui (QCADDesignObjectClass *klass, void *default_properties, GtkWidget **pTopContainer, gpointer *pData) ;
#endif /* def GTK_GUI */
#ifdef STDIO_FILEIO
static void serialize (QCADDesignObject *obj, FILE *fp) ;
static gboolean unserialize (QCADDesignObject *obj, FILE *fp) ;
#endif /* def STDIO_FILEIO */
static void move (QCADDesignObject *obj, double dxDelta, double dyDelta) ;
static const char *PostScript_preamble () ;
static char *PostScript_instance (QCADDesignObject *obj, gboolean bColour) ;
static GList *add_unique_types (QCADDesignObject *obj, GList *lst) ;
static gboolean set_selected (QCADDesignObject *obj, gboolean bSelected) ;
static void get_bounds_box (QCADDesignObject *obj, WorldRectangle *rc) ;
static QCADDesignObject *hit_test (QCADDesignObject *obj, int xReal, int yReal) ;
static void transform (QCADDesignObject *obj, double m11, double m12, double m21, double m22) ;
static QCADDesignObject *qcad_cell_compound_do_first (QCADCompoundDO *cdo) ;
static QCADDesignObject *qcad_cell_compound_do_next (QCADCompoundDO *cdo) ;
static gboolean qcad_cell_compound_do_last (QCADCompoundDO *cdo) ;
static gboolean qcad_cell_do_container_add (QCADDOContainer *container, QCADDesignObject *obj) ;
static gboolean qcad_cell_do_container_remove (QCADDOContainer *container, QCADDesignObject *obj) ;

static void qcad_cell_array_next_coord (int idx[2], double coord[2], double length[2], double dDir) ;
#ifdef GTK_GUI
static void create_default_properties_dialog (DEFAULT_PROPERTIES *dialog) ;
static void create_properties_dialog (PROPERTIES *dialog) ;
static void cell_mode_toggled (GtkWidget *widget, gpointer data) ;
static void default_properties_apply (gpointer data) ;
#endif /* def GTK_GUI */
#ifdef STDIO_FILEIO
static gboolean qcad_cell_dot_unserialize (FILE *fp, QCADCellDot *pdots, int idxDot) ;
#endif
static void *default_properties_get (struct QCADDesignObjectClass *klass) ;
static void default_properties_set (struct QCADDesignObjectClass *klass, void *props) ;
static void default_properties_destroy (struct QCADDesignObjectClass *klass, void *props) ;
static void qcad_cell_apply_transformation (QCADCell *cell, double xOld, double yOld) ;

GdkColor clrBlack  = {0, 0x0000, 0x0000, 0x0000} ;
GdkColor clrOrange = {0, 0xFFFF, 0x8000, 0x0000} ;
GdkColor clrYellow = {0, 0xFFFF, 0xFFFF, 0x0000} ;
GdkColor clrBlue   = {0, 0x0000, 0x0000, 0xFFFF} ;

static GdkColor clrClock[4] =
  {
  {0, 0x0000, 0xFFFF, 0x0000},
  {0, 0xFFFF, 0x0000, 0xFFFF},
  {0, 0x0000, 0xFFFF, 0xFFFF},
  {0, 0xFFFF, 0xFFFF, 0xFFFF},
  } ;

enum
  {
  QCAD_CELL_CELL_FUNCTION_CHANGED_SIGNAL,
  QCAD_CELL_LAST_SIGNAL
  } ;

enum
  {
  QCAD_CELL_PROPERTY_FUNCTION = 1,
  QCAD_CELL_PROPERTY_CLOCK,
  QCAD_CELL_PROPERTY_MODE,
  QCAD_CELL_PROPERTY_LABEL,
  QCAD_CELL_PROPERTY_POLARIZATION
  } ;

static guint qcad_cell_signals[QCAD_CELL_LAST_SIGNAL] = {0} ;

GType qcad_cell_get_type ()
  {
  static GType qcad_cell_type = 0 ;

  if (!qcad_cell_type)
    {
    static const GTypeInfo qcad_cell_info =
      {
      sizeof (QCADCellClass),
      (GBaseInitFunc)NULL,
      (GBaseFinalizeFunc)NULL,
      (GClassInitFunc)qcad_cell_class_init,
      (GClassFinalizeFunc)NULL,
      NULL,
      sizeof (QCADCell),
      0,
      (GInstanceInitFunc)qcad_cell_instance_init
      } ;

    static GInterfaceInfo qcad_cell_compound_do_info =
      {
      (GInterfaceInitFunc)qcad_cell_compound_do_interface_init,
      NULL,
      NULL
      } ;

    static GInterfaceInfo qcad_cell_do_container_info =
      {
      (GInterfaceInitFunc)qcad_cell_do_container_interface_init,
      NULL,
      NULL
      } ;

    if ((qcad_cell_type = g_type_register_static (QCAD_TYPE_DESIGN_OBJECT, QCAD_TYPE_STRING_CELL, &qcad_cell_info, 0)))
      {
      g_type_add_interface_static (qcad_cell_type, QCAD_TYPE_COMPOUND_DO, &qcad_cell_compound_do_info) ;
      g_type_add_interface_static (qcad_cell_type, QCAD_TYPE_DO_CONTAINER, &qcad_cell_do_container_info) ;
      g_type_class_ref (qcad_cell_type) ;
      }
    DBG_OO (fprintf (stderr, "Registered QCADCell as %d\n", qcad_cell_type)) ;
    }
  return qcad_cell_type ;
  }

GType qcad_cell_function_get_type ()
  {
  static GType qcad_cell_function_type = 0 ;

  if (!qcad_cell_function_type)
    {
    static const GEnumValue values[] =
      {
      {QCAD_CELL_NORMAL, "QCAD_CELL_NORMAL", "Normal"},
      {QCAD_CELL_INPUT,  "QCAD_CELL_INPUT",  "Input"},
      {QCAD_CELL_OUTPUT, "QCAD_CELL_OUTPUT", "Output"},
      {QCAD_CELL_FIXED,  "QCAD_CELL_FIXED",  "Fixed Polarization"},
      {0, NULL, NULL}
      } ;
    qcad_cell_function_type = g_enum_register_static (QCAD_TYPE_STRING_CELL_FUNCTION, values) ;
    }

  return qcad_cell_function_type ;
  }

GType qcad_cell_mode_get_type ()
  {
  static GType qcad_cell_mode_type = 0 ;

  if (!qcad_cell_mode_type)
    {
    static const GEnumValue values[] =
      {
      {QCAD_CELL_MODE_NORMAL,    "QCAD_CELL_MODE_NORMAL",    "Normal"},
      {QCAD_CELL_MODE_CROSSOVER, "QCAD_CELL_MODE_CROSSOVER", "Crossover"},
      {QCAD_CELL_MODE_VERTICAL,  "QCAD_CELL_MODE_VERTICAL",  "Vertical"},
      {0, NULL, NULL}
      } ;
    qcad_cell_mode_type = g_enum_register_static (QCAD_TYPE_STRING_CELL_MODE, values) ;
    }

  return qcad_cell_mode_type ;
  }

static void qcad_cell_compound_do_interface_init (gpointer iface, gpointer interface_data)
  {
  QCADCompoundDOClass *klass = (QCADCompoundDOClass *)iface ;

  klass->first = qcad_cell_compound_do_first ;
  klass->next = qcad_cell_compound_do_next ;
  klass->last = qcad_cell_compound_do_last ;
  }

static void qcad_cell_do_container_interface_init (gpointer iface, gpointer interface_data)
  {
  QCADDOContainerClass *klass = (QCADDOContainerClass *)iface ;

  klass->add = qcad_cell_do_container_add ;
  klass->remove = qcad_cell_do_container_remove ;
  }

static void qcad_cell_class_init (GObjectClass *klass, gpointer data)
  {
#ifdef GTK_GUI
  GdkColormap *clrmap = gdk_colormap_get_system () ;

  DBG_OO (fprintf (stderr, "QCADCell::class_init:Entering\n")) ;

  if (0 == clrOrange.pixel)
    gdk_colormap_alloc_color (clrmap, &clrOrange, FALSE, TRUE) ;
  if (0 == clrYellow.pixel)
    gdk_colormap_alloc_color (clrmap, &clrYellow, FALSE, TRUE) ;
  if (0 == clrBlue.pixel)
    gdk_colormap_alloc_color (clrmap, &clrBlue, FALSE, TRUE) ;
  if (0 == clrBlack.pixel)
    gdk_colormap_alloc_color (clrmap, &clrBlack, FALSE, TRUE) ;

  gdk_colormap_alloc_color (clrmap, &(clrClock[0]), FALSE, TRUE) ;
  gdk_colormap_alloc_color (clrmap, &(clrClock[1]), FALSE, TRUE) ;
  gdk_colormap_alloc_color (clrmap, &(clrClock[2]), FALSE, TRUE) ;
  gdk_colormap_alloc_color (clrmap, &(clrClock[3]), FALSE, TRUE) ;
#else
  DBG_OO (fprintf (stderr, "QCADCell::class_init:Entering\n")) ;
#endif /* def GTK_GUI */

  memcpy (&(QCAD_DESIGN_OBJECT_CLASS (klass)->clrDefault), &(clrClock[0]), sizeof (GdkColor)) ;
  QCAD_DESIGN_OBJECT_CLASS (klass)->hit_test                   = hit_test ;
  QCAD_DESIGN_OBJECT_CLASS (klass)->get_bounds_box             = get_bounds_box ;
  QCAD_DESIGN_OBJECT_CLASS (klass)->set_selected               = set_selected ;
  QCAD_DESIGN_OBJECT_CLASS (klass)->add_unique_types           = add_unique_types ;
  QCAD_DESIGN_OBJECT_CLASS (klass)->PostScript_preamble        = PostScript_preamble ;
  QCAD_DESIGN_OBJECT_CLASS (klass)->PostScript_instance        = PostScript_instance ;
#ifdef GTK_GUI
  QCAD_DESIGN_OBJECT_CLASS (klass)->draw                       = draw ;
  QCAD_DESIGN_OBJECT_CLASS (klass)->properties                 = properties ;
  QCAD_DESIGN_OBJECT_CLASS (klass)->default_properties_ui      = default_properties_ui ;
  QCAD_DESIGN_OBJECT_CLASS (klass)->mh.button_pressed          = (GCallback)button_pressed ;
#endif /* def GTK_GUI */
#ifdef STDIO_FILEIO
  QCAD_DESIGN_OBJECT_CLASS (klass)->serialize                  = serialize ;
  QCAD_DESIGN_OBJECT_CLASS (klass)->unserialize                = unserialize ;
#endif /* def STDIO_FILEIO */
  QCAD_DESIGN_OBJECT_CLASS (klass)->copy                       = copy ;
  QCAD_DESIGN_OBJECT_CLASS (klass)->move                       = move ;
  QCAD_DESIGN_OBJECT_CLASS (klass)->transform                  = transform ;
  QCAD_DESIGN_OBJECT_CLASS (klass)->default_properties_get     = default_properties_get ;
  QCAD_DESIGN_OBJECT_CLASS (klass)->default_properties_set     = default_properties_set ;
  QCAD_DESIGN_OBJECT_CLASS (klass)->default_properties_destroy = default_properties_destroy ;
  QCAD_CELL_CLASS (klass)->default_cell_options.cxCell         = 18 ;
  QCAD_CELL_CLASS (klass)->default_cell_options.cyCell         = 18 ;
  QCAD_CELL_CLASS (klass)->default_cell_options.dot_diameter   =  5 ;
  QCAD_CELL_CLASS (klass)->default_cell_options.mode           = QCAD_CELL_MODE_NORMAL ;
  G_OBJECT_CLASS (klass)->finalize                             = qcad_cell_instance_finalize ;
  G_OBJECT_CLASS (klass)->set_property                         = qcad_cell_set_property ;
  G_OBJECT_CLASS (klass)->get_property                         = qcad_cell_get_property ;

  g_object_class_install_property (G_OBJECT_CLASS (klass), QCAD_CELL_PROPERTY_FUNCTION,
    g_param_spec_enum ("function", _("Function"), _("Cell Function:Normal/Input/Output/Fixed"),
      QCAD_TYPE_CELL_FUNCTION, QCAD_CELL_NORMAL, G_PARAM_READABLE | G_PARAM_WRITABLE)) ;

  g_object_class_install_property (G_OBJECT_CLASS (klass), QCAD_CELL_PROPERTY_CLOCK,
    g_param_spec_uint ("clock", _("Clock"), _("Cell Clock"),
      0, 3, 0, G_PARAM_READABLE | G_PARAM_WRITABLE)) ;

  g_object_class_install_property (G_OBJECT_CLASS (klass), QCAD_CELL_PROPERTY_MODE,
    g_param_spec_enum ("mode", _("Mode"), _("Cell Drawing Mode"),
      QCAD_TYPE_CELL_MODE, QCAD_CELL_MODE_NORMAL, G_PARAM_READABLE | G_PARAM_WRITABLE)) ;

  g_object_class_install_property (G_OBJECT_CLASS (klass), QCAD_CELL_PROPERTY_LABEL,
    g_param_spec_string ("label", _("Label"), _("Cell Label"),
      _("Untitled"), G_PARAM_READABLE | G_PARAM_WRITABLE)) ;

  g_object_class_install_property (G_OBJECT_CLASS (klass), QCAD_CELL_PROPERTY_POLARIZATION,
    g_param_spec_double ("polarization", _("Polarization"), _("Cell Polarization"),
      -1.0, 1.0, 0.0, G_PARAM_READABLE | G_PARAM_WRITABLE)) ;

  qcad_cell_signals[QCAD_CELL_CELL_FUNCTION_CHANGED_SIGNAL] =
    g_signal_new ("cell-function-changed", G_TYPE_FROM_CLASS (klass), G_SIGNAL_RUN_FIRST,
      G_STRUCT_OFFSET (QCADCellClass, cell_function_changed), NULL, NULL, g_cclosure_marshal_VOID__VOID,
      G_TYPE_NONE, 0) ;

  DBG_OO (fprintf (stderr, "QCADCell::class_init:Leaving\n")) ;
  }

static void qcad_cell_set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
  {
  QCADCell *cell = QCAD_CELL (object) ;

  switch (property_id)
    {
    case QCAD_CELL_PROPERTY_FUNCTION:
      qcad_cell_set_function (cell, g_value_get_enum (value)) ;
      DBG_VAL (fprintf (stderr, "qcad_cell_set_property:Setting cell function to %s\n",
        g_enum_get_value (g_type_class_peek (QCAD_TYPE_CELL_FUNCTION), g_value_get_enum (value))->value_name)) ;
      break ;

    case QCAD_CELL_PROPERTY_CLOCK:
      qcad_cell_set_clock (cell, g_value_get_uint (value)) ;
      DBG_VAL (fprintf (stderr, "qcad_cell_set_property:Setting cell clock to %d\n", g_value_get_uint (value))) ;
      break ;

    case QCAD_CELL_PROPERTY_MODE:
      qcad_cell_set_display_mode (cell, g_value_get_enum (value)) ;
      DBG_VAL (fprintf (stderr, "qcad_cell_set_property:Setting cell mode to %s\n",
        g_enum_get_value (g_type_class_peek (QCAD_TYPE_CELL_MODE), g_value_get_enum (value))->value_name)) ;
      break ;

    case QCAD_CELL_PROPERTY_LABEL:
      qcad_cell_set_label (cell, (char *)g_value_get_string (value)) ;
      DBG_VAL (fprintf (stderr, "qcad_cell_set_property:Setting cell label to \"%s\"\n", (char *)g_value_get_string (value))) ;
      break ;

    case QCAD_CELL_PROPERTY_POLARIZATION:
      qcad_cell_set_polarization (cell, g_value_get_double (value)) ;
      DBG_VAL (fprintf (stderr, "qcad_cell_set_property:Setting cell polarization to %lf\n", g_value_get_double (value))) ;
      break ;
    }
  }

static void qcad_cell_get_property (GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
  {
  QCADCell *cell = QCAD_CELL (object) ;

  switch (property_id)
    {
    case QCAD_CELL_PROPERTY_FUNCTION:
      g_value_set_enum (value, cell->cell_function) ;
      break ;

    case QCAD_CELL_PROPERTY_CLOCK:
      g_value_set_uint (value, cell->cell_options.clock) ;
      break ;

    case QCAD_CELL_PROPERTY_MODE:
      g_value_set_enum (value, cell->cell_options.mode) ;
      break ;

    case QCAD_CELL_PROPERTY_LABEL:
      g_value_set_string (value, qcad_cell_get_label (cell)) ;
      break ;

    case QCAD_CELL_PROPERTY_POLARIZATION:
      g_value_set_double (value, qcad_cell_calculate_polarization (cell)) ;
      break ;
    }
  }

static void qcad_cell_instance_init (GObject *object, gpointer data)
  {
  QCADCellClass *klass = QCAD_CELL_GET_CLASS (object) ;

  DBG_OO (fprintf (stderr, "QCADCell::instance_init:Entering\n")) ;

  memcpy (&(QCAD_CELL (object)->cell_options), &(klass->default_cell_options), sizeof (QCADCellOptions)) ;
  memcpy (&(QCAD_DESIGN_OBJECT (object)->clr), &(clrClock[klass->default_cell_options.clock]), sizeof (GdkColor)) ;
  QCAD_CELL (object)->id = (int)object ;
  QCAD_CELL (object)->host_name = NULL ;
  QCAD_CELL (object)->cell_function = QCAD_CELL_NORMAL ;
  QCAD_CELL (object)->cell_model = NULL ;
  QCAD_CELL (object)->cell_dots = malloc (4 * sizeof (QCADCellDot)) ;
  QCAD_CELL (object)->number_of_dots = 4 ;
  QCAD_CELL (object)->label = NULL ;
  QCAD_CELL (object)->bLabelRemoved = TRUE ;

  DBG_OO (fprintf (stderr, "QCADCell::instance_init:Leaving\n")) ;
  }

static void qcad_cell_instance_finalize (GObject *object)
  {
  DBG_OO (fprintf (stderr, "QCADCell::instance_finalize:Entering\n")) ;
  if (NULL != QCAD_CELL (object)->cell_dots)
    free (QCAD_CELL (object)->cell_dots) ;
  if (NULL != QCAD_CELL (object)->label)
    g_object_unref (QCAD_CELL (object)->label) ;
  if (NULL != QCAD_CELL (object)->cell_model)
    free (QCAD_CELL (object)->cell_model) ;

  G_OBJECT_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_CELL)))->finalize (object) ;
  DBG_OO (fprintf (stderr, "QCADCell::instance_finalize:Leaving\n")) ;
  }

///////////////////////////////////////////////////////////////////////////////

QCADDesignObject *qcad_cell_new_with_function (QCADCellFunction cell_function, char *pszLabel)
  {return g_object_new (QCAD_TYPE_CELL, "function", cell_function, "label", pszLabel, NULL) ;}

QCADDesignObject *qcad_cell_new (double x, double y)
  {
  double dcx, dcy, ddiam ;
  QCADDesignObject *ret = QCAD_DESIGN_OBJECT (g_object_new (QCAD_TYPE_CELL, NULL)) ;
  QCADCellClass *klass = g_type_class_peek (QCAD_TYPE_CELL) ;
  QCADCell *cell = QCAD_CELL (ret) ;

  dcx = klass->default_cell_options.cxCell ;
  dcy = klass->default_cell_options.cyCell ;
  ddiam = klass->default_cell_options.dot_diameter ;

  ret->x = x ;
  ret->y = y ;
  ret->bounding_box.xWorld = x - dcx / 2.0 ;
  ret->bounding_box.yWorld = y - dcy / 2.0 ;
  ret->bounding_box.cxWorld = dcx ;
  ret->bounding_box.cyWorld = dcy ;

  QCAD_CELL (ret)->cell_dots[0].charge =
  QCAD_CELL (ret)->cell_dots[1].charge =
  QCAD_CELL (ret)->cell_dots[2].charge =
  QCAD_CELL (ret)->cell_dots[3].charge = HALF_QCHARGE ;

  QCAD_CELL (ret)->cell_dots[0].diameter =
  QCAD_CELL (ret)->cell_dots[1].diameter =
  QCAD_CELL (ret)->cell_dots[2].diameter =
  QCAD_CELL (ret)->cell_dots[3].diameter = ddiam ;

  cell->cell_dots[3].x = x - (dcx / 2 - cell->cell_dots[3].diameter) / 2 - cell->cell_dots[3].diameter / 2;
  cell->cell_dots[3].y = y - (dcy / 2 - cell->cell_dots[3].diameter) / 2 - cell->cell_dots[3].diameter / 2;
  cell->cell_dots[0].x = x + (dcx / 2 - cell->cell_dots[0].diameter) / 2 + cell->cell_dots[0].diameter / 2;
  cell->cell_dots[0].y = y - (dcy / 2 - cell->cell_dots[0].diameter) / 2 - cell->cell_dots[0].diameter / 2;
  cell->cell_dots[2].x = x - (dcx / 2 - cell->cell_dots[2].diameter) / 2 - cell->cell_dots[2].diameter / 2;
  cell->cell_dots[2].y = y + (dcy / 2 - cell->cell_dots[2].diameter) / 2 + cell->cell_dots[2].diameter / 2;
  cell->cell_dots[1].x = x + (dcx / 2 - cell->cell_dots[1].diameter) / 2 + cell->cell_dots[1].diameter / 2;
  cell->cell_dots[1].y = y + (dcy / 2 - cell->cell_dots[1].diameter) / 2 + cell->cell_dots[1].diameter / 2;

  //Dayane
  int iClock;

  iClock=USE_clock_attribution(x, y, dcx, dcy);

  if (iClock <= 3)
  qcad_cell_set_clock(cell,iClock);
  //End of Dayane

  return ret ;
  }

const char *qcad_cell_get_label (QCADCell *cell)
  {
  if (NULL == cell) return "" ;
  else
  if (cell->bLabelRemoved) return "" ;
  else
  if (NULL == cell->label) return "" ;
  else
  if (NULL == cell->label->psz) return "" ;
  else
    return cell->label->psz ;
  }

void qcad_cell_set_display_mode (QCADCell *cell, int cell_mode)
  {cell->cell_options.mode = cell_mode ;}

#ifdef GTK_GUI
void qcad_cell_drexp_array (GdkDrawable *dst, GdkFunction rop, GtkOrientation orientation, double dRangeBeg, double dRangeEnd, double dOtherCoord)
  {
  int idx[2] = {-1} ;
  GdkGC *gc = gdk_gc_new (dst) ;
  QCADCellClass *klass = NULL ;
  double
    dDir = 0,
    coord[2], length[2] = {-1} ;

  idx[0] = GTK_ORIENTATION_HORIZONTAL == orientation ? 0 : 1 ;
  idx[1] = GTK_ORIENTATION_HORIZONTAL == orientation ? 1 : 0 ;
  klass = g_type_class_peek (QCAD_TYPE_CELL) ;
  dDir = dRangeBeg < dRangeEnd ? 1 : -1 ;
  length[0] = klass->default_cell_options.cxCell ;
  length[1] = klass->default_cell_options.cyCell ;

  gdk_gc_set_function (gc, rop) ;
  gdk_gc_set_foreground (gc, &(clrClock[QCAD_CELL_CLASS (klass)->default_cell_options.clock])) ;

  coord[idx[0]] = dRangeBeg ;
  coord[idx[1]] = dOtherCoord ;

  world_to_grid_pt (&(coord[0]), &(coord[1])) ;

  while (dDir * coord[idx[0]] < dDir * dRangeEnd)
    {
    gdk_draw_rectangle (dst, gc, FALSE,
      world_to_real_x (coord[0] - length[0] / 2.0),
      world_to_real_y (coord[1] - length[1] / 2.0),
      world_to_real_cx (length[0]),
      world_to_real_cy (length[1])) ;
    qcad_cell_array_next_coord (idx, coord, length, dDir) ;
    }
  }
#endif /* def GTK_GUI */

EXP_ARRAY *qcad_cell_create_array (gboolean bHorizontal, double dRangeBeg, double dRangeEnd, double dOtherCoord)
  {
  int idx[2] = {-1} ;
  QCADCellClass *klass = g_type_class_peek (QCAD_TYPE_CELL) ;
  double
    dDir = dRangeBeg < dRangeEnd ? 1 : -1,
    coord[2], length[2] = {-1} ;
  EXP_ARRAY *ret = NULL ;
  QCADDesignObject *cell = NULL ;

  if (NULL == klass) return NULL ;

  idx[0] = bHorizontal ? 0 : 1 ;
  idx[1] = bHorizontal ? 1 : 0 ;
  dDir = dRangeBeg < dRangeEnd ? 1 : -1 ;
  length[0] = klass->default_cell_options.cxCell ;
  length[1] = klass->default_cell_options.cyCell ;

  coord[idx[0]] = dRangeBeg ;
  coord[idx[1]] = dOtherCoord ;

  world_to_grid_pt (&(coord[0]), &(coord[1])) ;

  while (dDir * coord[idx[0]] < dDir * dRangeEnd)
    {
    if (NULL == ret)
      ret = exp_array_new (sizeof (QCADCell *), 1) ;
    cell = qcad_cell_new (coord[0], coord[1]) ;
    exp_array_insert_vals (ret, &cell, 1, 1, -1) ;
    qcad_cell_array_next_coord (idx, coord, length, dDir) ;
    }
  return ret ;
  }

void qcad_cell_rotate_dots (QCADCell *cell, double angle)
  {
  int i;
  double x, cell_x = QCAD_DESIGN_OBJECT (cell)->x ;
  double y, cell_y = QCAD_DESIGN_OBJECT (cell)->y ;

  // -- uses standard rotational transform -- //
  for (i = 0; i < cell->number_of_dots; i++)
    {
    x = cell_x + (cell->cell_dots[i].x -
      cell_x) * (float) cos(angle) -
      (cell->cell_dots[i].y - cell_y) * (float) sin(angle);
    y = cell_y + (cell->cell_dots[i].y -
      cell_y) * (float) cos(angle) +
      (cell->cell_dots[i].x - cell_x) * (float) sin(angle);
    cell->cell_dots[i].x = x;
    cell->cell_dots[i].y = y;
    }
  }

void qcad_cell_set_host_name (QCADCell *cell, char *pszHostName)
  {cell->host_name = g_strdup (pszHostName) ;}

///////////////////////////////////////////////////////////////////////////////

static gboolean qcad_cell_do_container_add (QCADDOContainer *container, QCADDesignObject *obj)
  {
  QCADCell *cell = QCAD_CELL (container) ;

  if (NULL == cell->label) return FALSE ;
  if (QCAD_DESIGN_OBJECT (cell->label) == obj && cell->bLabelRemoved)
    {
    g_object_ref (cell->label) ;
    cell->bLabelRemoved = FALSE ;
    return TRUE ;
    }
  return FALSE ;
  }

static gboolean qcad_cell_do_container_remove (QCADDOContainer *container, QCADDesignObject *obj)
  {
  QCADCell *cell = QCAD_CELL (container) ;

  if (NULL == cell->label) return FALSE ;
  if (QCAD_DESIGN_OBJECT (cell->label) == obj && !(cell->bLabelRemoved))
    {
    cell->bLabelRemoved = TRUE ;
    return TRUE ;
    }
  return FALSE ;
  }

static QCADDesignObject *qcad_cell_compound_do_first (QCADCompoundDO *cdo)
  {return (QCAD_CELL (cdo)->bLabelRemoved ? NULL : (QCADDesignObject *)(QCAD_CELL (cdo)->label)) ;}
static QCADDesignObject *qcad_cell_compound_do_next (QCADCompoundDO *cdo)
  {return NULL ;}
static gboolean qcad_cell_compound_do_last (QCADCompoundDO *cdo)
  {return TRUE ;}

static void copy (QCADDesignObject *src, QCADDesignObject *dst)
  {
  int Nix = -1 ;
  QCADCell *cellSrc = QCAD_CELL (src), *cellDst = QCAD_CELL (dst) ;

  QCAD_DESIGN_OBJECT_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_CELL)))->copy (src, dst) ;

  cellDst->cell_options.cxCell       = cellSrc->cell_options.cxCell ;
  cellDst->cell_options.clock        = cellSrc->cell_options.clock ;
  cellDst->cell_options.cyCell       = cellSrc->cell_options.cyCell ;
  cellDst->cell_options.dot_diameter = cellSrc->cell_options.dot_diameter ;
  cellDst->cell_options.mode         = cellSrc->cell_options.mode ;
  cellDst->cell_function             = cellSrc->cell_function ;
  cellDst->bLabelRemoved             = FALSE ;
  // CAREFUL ! This is a shallow copy of the cell model
  cellDst->cell_model                = cellSrc->cell_model ;
  if (NULL != cellSrc->label)
    {
    cellDst->label = QCAD_LABEL (qcad_design_object_new_from_object (QCAD_DESIGN_OBJECT (cellSrc->label))) ;
    g_object_add_weak_pointer (G_OBJECT (cellDst->label), (gpointer *)&(cellDst->label)) ;
    }
  else
    cellDst->label = NULL ;

  if (NULL != cellDst->cell_dots)
    free (cellDst->cell_dots) ;
  cellDst->cell_dots = NULL ;
  cellDst->number_of_dots = 0 ;

  if (cellSrc->number_of_dots > 0)
    {
    cellDst->cell_dots = malloc (cellSrc->number_of_dots * sizeof (QCADCellDot)) ;
    cellDst->number_of_dots = cellSrc->number_of_dots ;

    for (Nix = 0 ; Nix < cellDst->number_of_dots ; Nix++)
      {
      cellDst->cell_dots[Nix].x         = cellSrc->cell_dots[Nix].x ;
      cellDst->cell_dots[Nix].y         = cellSrc->cell_dots[Nix].y ;
      cellDst->cell_dots[Nix].diameter  = cellSrc->cell_dots[Nix].diameter ;
      cellDst->cell_dots[Nix].charge    = cellSrc->cell_dots[Nix].charge ;
      cellDst->cell_dots[Nix].spin      = cellSrc->cell_dots[Nix].spin ;
      cellDst->cell_dots[Nix].potential = cellSrc->cell_dots[Nix].potential ;
      }
    }
  }

static GList *add_unique_types (QCADDesignObject *obj, GList *lst)
  {
  QCADCell *cell = QCAD_CELL (obj) ;
  GList *lstItr = NULL ;
  gboolean bHaveLabel = FALSE, bHaveCell = FALSE ;

  // If there's no label, then perform default behaviour
  if (NULL == cell->label)
    return QCAD_DESIGN_OBJECT_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_CELL)))->add_unique_types (obj, lst) ;
  else
    {
    //Look for "QCADCell" and "QCADLabel"
    for (lstItr = lst ; lstItr != NULL && !(bHaveCell && bHaveLabel) ; lstItr = lstItr->next)
      if (QCAD_TYPE_CELL == G_TYPE_FROM_INSTANCE (lstItr->data))
        bHaveCell = TRUE ;
      else
      if (QCAD_TYPE_LABEL == G_TYPE_FROM_INSTANCE (lstItr->data))
        bHaveLabel = TRUE ;
    }

  if (bHaveCell && bHaveLabel) return lst ;

  if (!bHaveCell)
    lst = g_list_prepend (lst, cell) ;

  if (!bHaveLabel)
    lst = g_list_prepend (lst, cell->label) ;

  return lst ;
  }

static void *default_properties_get (struct QCADDesignObjectClass *klass)
  {
  QCADCellOptions *pDefaults = g_malloc (sizeof (QCADCellOptions)) ;
  memcpy (pDefaults, &(QCAD_CELL_CLASS (klass)->default_cell_options), sizeof (QCADCellOptions)) ;
  return (void *)pDefaults ;
  }

static void default_properties_set (struct QCADDesignObjectClass *klass, void *props)
  {memcpy (&(QCAD_CELL_CLASS (klass)->default_cell_options), props, sizeof (QCADCellOptions)) ;}

static void default_properties_destroy (struct QCADDesignObjectClass *klass, void *props)
  {g_free (props) ;}

static char *PostScript_instance (QCADDesignObject *obj, gboolean bColour)
  {
  char *pszRet = NULL, *pszLabel = NULL ;
  QCADCell *cell = QCAD_CELL (obj) ;
  GdkColor clrBak = {0}, clr = {0} ;

  float fclr[3] = {0.00, 0.00, 0.00} ;

  // Clock colours and their grayscale counterparts
  float fclrClock[4][2][3] = {
    {{0.00, 0.50, 0.00}, {0.45, 0.45, 0.45}}, // Clock 0: dark green
    {{0.50, 0.00, 0.50}, {0.65, 0.65, 0.65}}, // Clock 1: dark purple
    {{0.00, 0.50, 0.50}, {0.85, 0.85, 0.85}}, // Clock 2: turquoise
    {{0.95, 0.95, 0.95}, {0.95, 0.95, 0.95}}  // Clock 3: white
    } ;

  float fclrCellFunction[QCAD_CELL_LAST_FUNCTION][3] = {
    {0.00, 0.50, 0.00}, // QCAD_CELL_NORMAL: dark green (not used - clock takes precedence)
    {0.21, 0.39, 0.70}, // QCAD_CELL_INPUT:  dark azure blue
    {0.66, 0.66, 0.00}, // QCAD_CELL_OUTPUT: maroonish yellow
    {0.83, 0.44, 0.00}  // QCAD_CELL_FIXED:  dark orange
    } ;

  if (QCAD_CELL_NORMAL == cell->cell_function)
    {
    fclr[0] = fclrClock[cell->cell_options.clock][bColour ? 0 : 1][0] ;
    fclr[1] = fclrClock[cell->cell_options.clock][bColour ? 0 : 1][1] ;
    fclr[2] = fclrClock[cell->cell_options.clock][bColour ? 0 : 1][2] ;
    }
   else
    {
    fclr[0] = bColour ? fclrCellFunction[cell->cell_function][0] : fclrClock[cell->cell_options.clock][1][0] ;
    fclr[1] = bColour ? fclrCellFunction[cell->cell_function][1] : fclrClock[cell->cell_options.clock][1][1] ;
    fclr[2] = bColour ? fclrCellFunction[cell->cell_function][2] : fclrClock[cell->cell_options.clock][1][2] ;
    }

  if (NULL != cell->label)
    {
    // Back up the color of the label, then convert the PostScript colour to a GdkColor
    // and apply it to the label.  Labels don't perform colour lookups so it'll print
    // out the colour exactly as it finds it.  Afterwards, restore label colour from
    // the backup
    memcpy (&clrBak, &(QCAD_DESIGN_OBJECT (cell->label)->clr), sizeof (GdkColor)) ;
    clr.red = fclr[0] * 65535 ;
    clr.green = fclr[1] * 65535 ;
    clr.blue = fclr[2] * 65535 ;
#ifdef GTK_GUI
    // Is this alloc necessary ?
    gdk_colormap_alloc_color (gdk_colormap_get_system (), &clr, FALSE, TRUE) ;
#endif /* def GTK_GUI */
    memcpy (&(QCAD_DESIGN_OBJECT (cell->label)->clr), &clr, sizeof (GdkColor)) ;
    pszLabel = qcad_design_object_get_PostScript_instance (QCAD_DESIGN_OBJECT (cell->label), bColour) ;
    memcpy (&(QCAD_DESIGN_OBJECT (cell->label)->clr), &clrBak, sizeof (GdkColor)) ;
    }

  pszRet =
    g_strdup_printf (
      "%lf nmx %lf nmy %lf nm %lf nm " // x y cx cy
      "%lf nm "
      "%lf nmx %lf nmy %lf nmx %lf nmy %lf nmx %lf nmy %lf nmx %lf nmy " //dot0_x dot0_y dot1_x dot1_y dot2_x dot2_y dot3_x dot3_y
      "%lf nm %lf nm %lf nm %lf nm %lf %lf %lf %d QCADCell%s%s", //charge0_diam charge1_diam charge2_diam charge3_diam r g b mode
      obj->bounding_box.xWorld, obj->bounding_box.yWorld, obj->bounding_box.cxWorld, obj->bounding_box.cyWorld,
      cell->cell_options.dot_diameter,
      cell->cell_dots[0].x, cell->cell_dots[0].y, cell->cell_dots[1].x, cell->cell_dots[1].y,
      cell->cell_dots[2].x, cell->cell_dots[2].y, cell->cell_dots[3].x, cell->cell_dots[3].y,
      (cell->cell_dots[0].diameter * cell->cell_dots[0].charge / QCHARGE),
      (cell->cell_dots[1].diameter * cell->cell_dots[1].charge / QCHARGE),
      (cell->cell_dots[2].diameter * cell->cell_dots[2].charge / QCHARGE),
      (cell->cell_dots[3].diameter * cell->cell_dots[3].charge / QCHARGE),
      fclr[0], fclr[1], fclr[2],
      cell->cell_options.mode,
      NULL == pszLabel ? "" : "\n  ",
      NULL == pszLabel ?  "" : pszLabel) ;

  g_free (pszLabel) ;

  return pszRet ;
  }

static const char *PostScript_preamble ()
  {
  return
    "/point\n"
    "  {\n"
    "  /y exch def\n"
    "  /x exch def\n"
    "\n"
    "  newpath\n"
    "  x 3 sub y 3 sub moveto\n"
    "  x 3 add y 3 add lineto\n"
    "  x 3 sub y 3 add moveto\n"
    "  x 3 add y 3 sub lineto\n"
    "  stroke\n"
    "  }\n"
    "\n"
    "% x y cx cy dot_diam dot0_x dot0_y dot1_x dot1_y dot2_x dot2_y dot3_x dot3_y charge0_diam charge1_diam charge2_diam charge3_diam r g b mode QCADCell\n"
    "/QCADCell\n"
    "  {\n"
    "  gsave\n"
    "  /mode exch def\n"
    "  /b exch def\n"
    "  /g exch def\n"
    "  /r exch def\n"
    "  /charge3_diam exch def\n"
    "  /charge2_diam exch def\n"
    "  /charge1_diam exch def\n"
    "  /charge0_diam exch def\n"
    "  /dot3_y exch def\n"
    "  /dot3_x exch def\n"
    "  /dot2_y exch def\n"
    "  /dot2_x exch def\n"
    "  /dot1_y exch def\n"
    "  /dot1_x exch def\n"
    "  /dot0_y exch def\n"
    "  /dot0_x exch def\n"
    "  /dot_diam exch def\n"
    "  /cy exch def\n"
    "  /cx exch def\n"
    "  /y exch def\n"
    "  /x exch def\n"
    "\n"
    "  % Filler\n"
    "  newpath\n"
    "  x y moveto\n"
    "  x cx add y lineto\n"
    "  x cx add y cy sub lineto\n"
    "  x y cy sub lineto\n"
    "  x y lineto\n"
    "  mode 0 eq dot_diam epsilon gt and\n"
    "    {\n"
    "    dot0_x dot_diam 2 div add dot0_y moveto\n"
    "    dot0_x dot0_y dot_diam 2 div 0 360 arc\n"
    "    dot1_x dot_diam 2 div add dot1_y moveto\n"
    "    dot1_x dot1_y dot_diam 2 div 0 360 arc\n"
    "    dot2_x dot_diam 2 div add dot2_y moveto\n"
    "    dot2_x dot2_y dot_diam 2 div 0 360 arc\n"
    "    dot3_x dot_diam 2 div add dot3_y moveto\n"
    "    dot3_x dot3_y dot_diam 2 div 0 360 arc\n"
    "    }\n"
    "  if\n"
    "  r g b setrgbcolor\n"
    "  fill\n"
    "  grestore\n"
    "\n"
    "  linewidth epsilon gt\n"
    "    {\n"
    "    % Cell outline\n"
    "    newpath\n"
    "    x y moveto\n"
    "    x cx add y lineto\n"
    "    x cx add y cy sub lineto\n"
    "    x y cy sub lineto\n"
    "    closepath\n"
    "    stroke\n"
    "    }\n"
    "  if\n"
    "\n"
    "  gsave\n"
    "\n"
    "  mode 0 eq\n"
    "    {\n"
    "    linewidth epsilon gt\n"
    "      {\n"
    "      %dot0 outline\n"
    "      newpath\n"
    "      dot0_x dot0_y dot_diam 2 div 0 360 arc\n"
    "      closepath stroke\n"
    "\n"
    "      %dot1 outline\n"
    "      newpath\n"
    "      dot1_x dot1_y dot_diam 2 div 0 360 arc\n"
    "      closepath stroke\n"
    "\n"
    "      %dot2 outline\n"
    "      newpath\n"
    "      dot2_x dot2_y dot_diam 2 div 0 360 arc\n"
    "      closepath stroke\n"
    "\n"
    "      %dot3 outline\n"
    "      newpath\n"
    "      dot3_x dot3_y dot_diam 2 div 0 360 arc\n"
    "      closepath stroke\n"
    "      }\n"
    "    if\n"
    "\n"
    "    dot_diam epsilon gt\n"
    "      {\n"
    "      %dot0 charge\n"
    "      newpath\n"
    "      dot0_x dot0_y charge0_diam 2 div 0 360 arc\n"
    "      closepath fill\n"
    "\n"
    "      %dot1 charge\n"
    "      newpath\n"
    "      dot1_x dot1_y charge1_diam 2 div 0 360 arc\n"
    "      closepath fill\n"
    "\n"
    "      %dot2 charge\n"
    "      newpath\n"
    "      dot2_x dot2_y charge2_diam 2 div 0 360 arc\n"
    "      closepath fill\n"
    "\n"
    "      %dot3 charge\n"
    "      newpath\n"
    "      dot3_x dot3_y charge3_diam 2 div 0 360 arc\n"
    "      closepath fill\n"
    "      }\n"
    "    if\n"
    "    }\n"
    "    {\n"
    "    linewidth epsilon gt\n"
    "      {\n"
    "      mode 1 eq\n"
    "        {\n"
    "        % draw an X across the cell\n"
    "        newpath\n"
    "        x y moveto\n"
    "        x cx add y cy sub lineto\n"
    "        stroke\n"
    "\n"
    "        newpath\n"
    "        x cx add y moveto\n"
    "        x y cy sub lineto\n"
    "        stroke\n"
    "        }\n"
    "        {\n"
    "        % draw a circle\n"
    "        newpath\n"
    "        x cx 2 div add y cy 2 div sub cx 2 div 0 360 arc\n"
    "        closepath stroke\n"
    "        }\n"
    "      ifelse\n"
    "      } % linewidth epsilon gt\n"
    "    if\n"
    "    }\n"
    "  ifelse\n"
    "\n"
    "  grestore\n"
    "  } def\n" ;
  }

static void move (QCADDesignObject *obj, double dxDelta, double dyDelta)
  {
  int Nix ;
  QCADCell *cell = QCAD_CELL (obj) ;
  //Dayane
  int iClock;
  double dcx=cell->cell_options.cxCell;
  double dcy=cell->cell_options.cyCell;
  //End of Dayane

  QCAD_DESIGN_OBJECT_CLASS (g_type_class_peek_parent (QCAD_DESIGN_OBJECT_GET_CLASS (obj)))->move (obj, dxDelta, dyDelta) ;

  if (NULL != cell->label)
    qcad_design_object_move (QCAD_DESIGN_OBJECT (cell->label), dxDelta, dyDelta) ;

  for (Nix = 0 ; Nix < cell->number_of_dots ; Nix++)
    {
    cell->cell_dots[Nix].x += dxDelta ;
    cell->cell_dots[Nix].y += dyDelta ;

    //Dayane
    iClock=USE_clock_attribution(cell->cell_dots[Nix].x, cell->cell_dots[Nix].y, dcx, dcy);

    if (iClock <= 3)
    qcad_cell_set_clock(cell,iClock);
    //End of Dayane

    }
  }

#ifdef GTK_GUI
static void draw (QCADDesignObject *obj, GdkDrawable *dst, GdkFunction rop)
  {
  GdkRectangle rc ;
  QCADCell *cell = QCAD_CELL (obj) ;
  int Nix ;
  GdkGC *gc = NULL ;

  world_to_real_rect (&(obj->bounding_box), &rc) ;

  // If this cell is not visible, return
  if (!is_real_rect_visible (&rc)) return ;

  gc = gdk_gc_new (dst) ;

  gdk_gc_set_function (gc, rop) ;
  gdk_gc_set_foreground (gc, obj->bSelected ? &(QCAD_DESIGN_OBJECT_GET_CLASS (obj)->clrSelected) : &(obj->clr)) ;

  // The cell outline
  gdk_draw_rectangle (dst, gc, FALSE, rc.x, rc.y, rc.width, rc.height) ;

  if (QCAD_CELL_MODE_NORMAL == cell->cell_options.mode)
    // The cell dots
    for (Nix = 0 ; Nix < cell->number_of_dots ; Nix++)
      {
      // Blank out the previous contents of the dot
      gdk_gc_set_foreground (gc, &clrBlack) ;

      gdk_gc_set_foreground (gc, obj->bSelected ? &(QCAD_DESIGN_OBJECT_GET_CLASS (obj)->clrSelected) : &(obj->clr)) ;
      // draw the dot in its current state
      gdk_draw_arc (dst, gc, FALSE,
      world_to_real_x (cell->cell_dots[Nix].x - cell->cell_dots[Nix].diameter / 2.0),
      world_to_real_y (cell->cell_dots[Nix].y - cell->cell_dots[Nix].diameter / 2.0),
      world_to_real_cx (cell->cell_dots[Nix].diameter),
      world_to_real_cy (cell->cell_dots[Nix].diameter), 0, 23040) ; // 23040 = 360 * 64
      gdk_draw_arc (dst, gc, TRUE,
      world_to_real_x (cell->cell_dots[Nix].x - cell->cell_dots[Nix].diameter / 2.0 * cell->cell_dots[Nix].charge/QCHARGE),
      world_to_real_y (cell->cell_dots[Nix].y - cell->cell_dots[Nix].diameter / 2.0 * cell->cell_dots[Nix].charge/QCHARGE),
      world_to_real_cx (cell->cell_dots[Nix].diameter * cell->cell_dots[Nix].charge/QCHARGE),
      world_to_real_cy (cell->cell_dots[Nix].diameter * cell->cell_dots[Nix].charge/QCHARGE), 0, 23040) ; // 23040 = 360 * 64
      }
    else
    if (QCAD_CELL_MODE_CROSSOVER == cell->cell_options.mode)
      // draw the diagonals of the cell outline
      {
      gdk_draw_line (dst, gc, rc.x, rc.y, rc.x + rc.width, rc.y + rc.height) ;
      gdk_draw_line (dst, gc, rc.x + rc.width, rc.y, rc.x, rc.y + rc.height) ;
      }
    else
      gdk_draw_arc (dst, gc, FALSE, rc.x, rc.y, rc.width, rc.width, 0, 23040) ;

  g_object_unref (gc) ;

  if (!(cell->bLabelRemoved || NULL == cell->label) && QCAD_DESIGN_OBJECT (cell)->bSelected == QCAD_DESIGN_OBJECT (cell->label)->bSelected)
    qcad_design_object_draw (QCAD_DESIGN_OBJECT (cell->label), dst, rop) ;
  }
#endif /* def GTK_GUI */
#ifdef STDIO_FILEIO
static gboolean unserialize (QCADDesignObject *obj, FILE *fp)
  {
  QCADCell *cell = QCAD_CELL (obj) ;
  char *pszLine = NULL, *pszValue = NULL ;
  gboolean bStopReading = FALSE, bParentInit = FALSE ;
  int current_dot = 0 ;

  if (!SkipPast (fp, '\0', "[TYPE:" QCAD_TYPE_STRING_CELL "]", NULL)) return FALSE ;

  g_free (cell->cell_dots) ;
  cell->cell_dots = NULL ;
  cell->number_of_dots = 0 ;

  while (TRUE)
    {
    if (NULL == (pszLine = ReadLine (fp, '\0', TRUE))) break ;
    if (!strcmp ("[#TYPE:" QCAD_TYPE_STRING_CELL "]", pszLine))
      {
      g_free (pszLine) ;
      break ;
      }

    if (!bStopReading)
      {
      tokenize_line (pszLine, strlen (pszLine), &pszValue, '=') ;

      if (!strncmp (pszLine, "[TYPE:", 6))
        {
        tokenize_line_type (pszLine, strlen (pszLine), &pszValue, ':') ;

        if (!strcmp (pszValue, QCAD_TYPE_STRING_DESIGN_OBJECT))
          {
          if (!(bParentInit = QCAD_DESIGN_OBJECT_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_CELL)))->unserialize (obj, fp)))
            bStopReading = TRUE ;
          }
        else
        if (!strcmp (pszValue, QCAD_TYPE_STRING_LABEL))
          {
          cell->label = QCAD_LABEL (qcad_design_object_new_from_stream (fp)) ;
          qcad_label_shrinkwrap (cell->label) ;
          g_object_add_weak_pointer (G_OBJECT (cell->label), (gpointer *)&(cell->label)) ;
          cell->bLabelRemoved = FALSE ;
          }
        else
        if (!strcmp (pszValue, "CELL_DOT"))
          {
          if (0 == cell->number_of_dots)
            bStopReading = TRUE ;
          else
          if (current_dot < cell->number_of_dots)
            {
            qcad_cell_dot_unserialize (fp, cell->cell_dots, current_dot) ;
            current_dot++ ;
            }
          else
            bStopReading = TRUE ;
          }
        }
      else
      if (!strcmp (pszLine, "cell_options.cxCell"))
        cell->cell_options.cxCell = g_ascii_strtod (pszValue, NULL) ;
      else
      if (!strcmp (pszLine, "cell_options.cyCell"))
        cell->cell_options.cyCell = g_ascii_strtod (pszValue, NULL) ;
      else
      if (!strcmp (pszLine, "cell_options.clock"))
        cell->cell_options.clock = g_ascii_strtod (pszValue, NULL) ;
      else
      if (!strcmp (pszLine, "cell_options.mode"))
        cell->cell_options.mode =
          !strcmp (pszValue, "TRUE")  ? QCAD_CELL_MODE_NORMAL :
          !strcmp (pszValue, "FALSE") ? QCAD_CELL_MODE_CROSSOVER :
            get_enum_value_from_string (QCAD_TYPE_CELL_MODE, pszValue) ;
      else
      if (!strcmp (pszLine, "cell_options.dot_diameter"))
        cell->cell_options.dot_diameter = g_ascii_strtod (pszValue, NULL) ;
      if (!strcmp (pszLine, "cell_function"))
        cell->cell_function = get_enum_value_from_string (QCAD_TYPE_CELL_FUNCTION, pszValue) ;
      else
      if (!strcmp (pszLine, "number_of_dots"))
        {
        cell->number_of_dots = atoi (pszValue) ;
        cell->cell_dots = g_malloc0 (cell->number_of_dots * sizeof (QCADCellDot)) ;
        }
      else
      if (!strcmp (pszLine, "label"))
        if (pszValue[0] != 0)
          qcad_cell_set_label (cell, pszValue) ;
      }
    g_free (pszLine) ;
    g_free (ReadLine (fp, '\0', FALSE)) ;
    }

  return (bParentInit && !bStopReading) ;
  }

static void serialize (QCADDesignObject *obj, FILE *fp)
  {
  int i;
  char *psz = NULL ;
  QCADCell *cell = QCAD_CELL (obj) ;

  // output object type
  fprintf(fp, "[TYPE:%s]\n", QCAD_TYPE_STRING_CELL);

  // call parent serialize function
  QCAD_DESIGN_OBJECT_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_CELL)))->serialize (obj, fp) ;

  // output variables
  fprintf(fp, "cell_options.cxCell=%lf\n", cell->cell_options.cxCell);
  fprintf(fp, "cell_options.cyCell=%lf\n", cell->cell_options.cyCell);
  fprintf(fp, "cell_options.dot_diameter=%lf\n", cell->cell_options.dot_diameter);
  fprintf(fp, "cell_options.clock=%d\n", cell->cell_options.clock);
  fprintf(fp, "cell_options.mode=%s\n", psz = get_enum_string_from_value (QCAD_TYPE_CELL_MODE, cell->cell_options.mode));
  g_free (psz) ;
  fprintf(fp, "cell_function=%s\n", psz = get_enum_string_from_value (QCAD_TYPE_CELL_FUNCTION, cell->cell_function));
  g_free (psz) ;
  fprintf(fp, "number_of_dots=%d\n", cell->number_of_dots);

  for(i = 0; i < cell->number_of_dots; i++)
    {
    fprintf(fp, "[TYPE:CELL_DOT]\n");
    fprintf(fp, "x=%lf\n", cell->cell_dots[i].x);
    fprintf(fp, "y=%lf\n", cell->cell_dots[i].y);
    fprintf(fp, "diameter=%lf\n", cell->cell_dots[i].diameter);
    fprintf(fp, "charge=%e\n", cell->cell_dots[i].charge);
    fprintf(fp, "spin=%f\n", cell->cell_dots[i].spin);

    fprintf(fp, "potential=%f\n", cell->cell_dots[i].potential);
    fprintf(fp, "[#TYPE:CELL_DOT]\n");
    }

  if (NULL != cell->label && !(cell->bLabelRemoved))
    qcad_design_object_serialize (QCAD_DESIGN_OBJECT (cell->label), fp) ;

  // output end of object
  fprintf(fp, "[#TYPE:%s]\n", QCAD_TYPE_STRING_CELL);
  }
#endif /* def STDIO_FILEIO */
#ifdef GTK_GUI
static GCallback default_properties_ui (QCADDesignObjectClass *klass, void *default_properties, GtkWidget **pTopContainer, gpointer *pData)
  {
  QCADCellClass *cklass = QCAD_CELL_CLASS (klass) ;
  static DEFAULT_PROPERTIES dialog = {NULL} ;
  QCADCellOptions *default_cell_options = (NULL == default_properties ? &(cklass->default_cell_options) : default_properties) ;

  if (NULL == dialog.tbl)
    create_default_properties_dialog (&dialog) ;

  dialog.adjCXCell->upper = MAX (dialog.adjCXCell->upper, default_cell_options->cxCell) ;
  dialog.adjCYCell->upper = MAX (dialog.adjCYCell->upper, default_cell_options->cyCell) ;
  dialog.adjDotDiam->upper = MAX (dialog.adjDotDiam->upper, default_cell_options->dot_diameter) ;

  gtk_adjustment_set_value (dialog.adjCXCell, default_cell_options->cxCell) ;
  gtk_adjustment_set_value (dialog.adjCYCell, default_cell_options->cyCell) ;
  gtk_adjustment_set_value (dialog.adjDotDiam, default_cell_options->dot_diameter) ;
  qcad_clock_combo_set_clock (QCAD_CLOCK_COMBO (dialog.cbClock), default_cell_options->clock) ;

  (*pTopContainer) = dialog.tbl ;
  (*pData) = &dialog ;
  return (GCallback)default_properties_apply ;
  }

#ifdef UNDO_REDO
static gboolean properties (QCADDesignObject *obj, GtkWidget *widget, QCADUndoEntry **pentry)
#else
static gboolean properties (QCADDesignObject *obj, GtkWidget *widget)
#endif /* def UNDO_REDO */
  {
  static PROPERTIES dialog = {NULL} ;
  QCADCell *cell = QCAD_CELL (obj) ;
  int iClock = -1 ;
  GtkWidget *tbtn = NULL ;
  gboolean bRet = FALSE ;
#ifdef UNDO_REDO
  EXP_ARRAY *state_before = NULL ;
#endif /* def UNDO_REDO */

  if (NULL == dialog.dlg)
    create_properties_dialog (&dialog) ;

  gtk_window_set_transient_for (GTK_WINDOW (dialog.dlg), GTK_WINDOW (widget)) ;
  g_object_set_data (G_OBJECT (dialog.dlg), "dialog", &dialog) ;
  gtk_adjustment_set_value (GTK_ADJUSTMENT (dialog.adjPolarization), qcad_cell_calculate_polarization (QCAD_CELL (obj))) ;
  gtk_entry_set_text (GTK_ENTRY (dialog.txtName), NULL == cell->label ? "" : NULL == cell->label->psz ? "" : cell->label->psz) ;

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (tbtn =
    QCAD_CELL_INPUT == cell->cell_function ||
    QCAD_CELL_OUTPUT == cell->cell_function ? dialog.rbIO :
    QCAD_CELL_FIXED == cell->cell_function ? dialog.rbFixed : dialog.rbNormal), TRUE) ;

  gtk_toggle_button_toggled (GTK_TOGGLE_BUTTON (tbtn)) ;

  if (QCAD_CELL_INPUT == cell->cell_function || QCAD_CELL_OUTPUT == cell->cell_function)
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (
      QCAD_CELL_INPUT == cell->cell_function ? dialog.rbInput : dialog.rbOutput), TRUE) ;

  qcad_clock_combo_set_clock (QCAD_CLOCK_COMBO (dialog.cbClock), cell->cell_options.clock) ;

  if ((bRet = (GTK_RESPONSE_OK == gtk_dialog_run (GTK_DIALOG (dialog.dlg)))))
    {
#ifdef UNDO_REDO
    if (NULL != pentry)
      state_before = qcad_design_object_get_state_array (obj, "function", "label", "polarization", "clock", NULL) ;
#endif /* def UNDO_REDO */
    iClock = qcad_clock_combo_get_clock (QCAD_CLOCK_COMBO (dialog.cbClock)) ;
    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dialog.rbNormal)))
      g_object_set (G_OBJECT (cell),
        "function",     QCAD_CELL_NORMAL,
        "clock",        iClock, NULL) ;
    else
    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dialog.rbFixed)))
      g_object_set (G_OBJECT (cell),
        "function",     QCAD_CELL_FIXED,
        "polarization", gtk_adjustment_get_value (GTK_ADJUSTMENT (dialog.adjPolarization)),
        "clock",        iClock, NULL) ;
    else // rbIO must be the case
      g_object_set (G_OBJECT (cell),
        "function",     gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dialog.rbInput))
                          ? QCAD_CELL_INPUT
                          : QCAD_CELL_OUTPUT,
        "label",        gtk_entry_get_text (GTK_ENTRY (dialog.txtName)),
        "polarization", 0.0,
        "clock",        iClock, NULL) ;
#ifdef UNDO_REDO
    if (NULL != pentry)
      (*pentry) = qcad_design_object_get_state_undo_entry (obj, state_before,
        qcad_design_object_get_state_array (obj, "function", "label", "polarization", "clock", NULL)) ;
#endif /* UNDO_REDO */
    }
  gtk_widget_hide (dialog.dlg) ;
  return bRet ;
  }
#endif /* def GTK_GUI */

static gboolean set_selected (QCADDesignObject *obj, gboolean bSelected)
  {
  gboolean bRet = QCAD_DESIGN_OBJECT_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_CELL)))->set_selected (obj, bSelected) ;
  if (NULL != QCAD_CELL (obj)->label)
    qcad_design_object_set_selected (QCAD_DESIGN_OBJECT (QCAD_CELL (obj)->label), bSelected) ;
  return bRet ;
  }

static void get_bounds_box (QCADDesignObject *obj, WorldRectangle *rc)
  {
  QCAD_DESIGN_OBJECT_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_CELL)))->get_bounds_box (obj, rc) ;
  if (NULL != QCAD_CELL (obj)->label)
    {
    double x = rc->xWorld, y = rc->yWorld ;
    WorldRectangle rcLabel ;
    qcad_design_object_get_bounds_box (QCAD_DESIGN_OBJECT (QCAD_CELL (obj)->label), &rcLabel) ;

    rc->xWorld = MIN (rc->xWorld, rcLabel.xWorld) ;
    rc->yWorld = MIN (rc->yWorld, rcLabel.yWorld) ;
    rc->cxWorld = MAX (rc->cxWorld + x, rcLabel.cxWorld + rcLabel.xWorld) - rc->xWorld ;
    rc->cyWorld = MAX (rc->cyWorld + y, rcLabel.cyWorld + rcLabel.yWorld) - rc->yWorld ;
    }
  }

static void transform (QCADDesignObject *obj, double m11, double m12, double m21, double m22)
  {
  double xOld = obj->x ;
  double yOld = obj->y ;

  QCAD_DESIGN_OBJECT_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_CELL)))->transform (obj, m11, m12, m21, m22) ;

  // dots are neither rotated nor mirrored when the cell is

  qcad_cell_apply_transformation (QCAD_CELL (obj), xOld, yOld) ;
  }

static void qcad_cell_apply_transformation (QCADCell *cell, double xOld, double yOld)
  {
  QCADDesignObject *obj = QCAD_DESIGN_OBJECT (cell) ;
  double dx = obj->x - xOld, dy = obj->y - yOld ;

  cell->cell_dots[0].x += dx ;
  cell->cell_dots[1].x += dx ;
  cell->cell_dots[2].x += dx ;
  cell->cell_dots[3].x += dx ;

  cell->cell_dots[0].y += dy ;
  cell->cell_dots[1].y += dy ;
  cell->cell_dots[2].y += dy ;
  cell->cell_dots[3].y += dy ;

  if (NULL != cell->label)
    qcad_design_object_move (QCAD_DESIGN_OBJECT (cell->label), dx, dy) ;

  }

static QCADDesignObject *hit_test (QCADDesignObject *obj, int xReal, int yReal)
  {
  QCADDesignObject *obj_from_parent =
    QCAD_DESIGN_OBJECT_CLASS (g_type_class_peek (g_type_parent (QCAD_TYPE_CELL)))->hit_test (obj, xReal, yReal) ;

  return NULL != obj_from_parent ? obj_from_parent :
    NULL == QCAD_CELL (obj)->label ? NULL :
    qcad_design_object_hit_test (QCAD_DESIGN_OBJECT (QCAD_CELL (obj)->label), xReal, yReal) ;
  }

///////////////////////////////////////////////////////////////////////////////

#ifdef GTK_GUI
static gboolean button_pressed (GtkWidget *widget, GdkEventButton *event, gpointer data)
  {
  QCADDesignObject *obj = NULL ;
  double xWorld = real_to_world_x (event->x), yWorld = real_to_world_y (event->y) ;

  if (1 != event->button) return FALSE ;

  world_to_grid_pt (&xWorld, &yWorld) ;

  obj = qcad_cell_new (xWorld, yWorld) ;

#ifdef DESIGNER
  if (NULL != drop_function)
    if ((*drop_function) (obj))
      return FALSE ;
#endif /* def DESIGNER */

  g_object_unref (obj) ;

  return FALSE ;
  }

static void default_properties_apply (gpointer data)
  {
  DEFAULT_PROPERTIES *dialog = (DEFAULT_PROPERTIES *)data ;
  QCADCellClass *cklass = g_type_class_peek (QCAD_TYPE_CELL) ;
  cklass->default_cell_options.cxCell = gtk_adjustment_get_value (dialog->adjCXCell) ;
  cklass->default_cell_options.cyCell = gtk_adjustment_get_value (dialog->adjCYCell) ;
  cklass->default_cell_options.dot_diameter = gtk_adjustment_get_value (dialog->adjDotDiam) ;
  cklass->default_cell_options.clock = qcad_clock_combo_get_clock (QCAD_CLOCK_COMBO (dialog->cbClock)) ;
  }
#endif /* def GTK_GUI */

///////////////////////////////////////////////////////////////////////////////

static void qcad_cell_array_next_coord (int idx[2], double coord[2], double length[2], double dDir)
  {
  double coord_last, coord_old ;

  coord_last = coord[idx[0]] ;
  coord[idx[0]] += length[idx[0]] * dDir ;
  coord_old = coord[idx[0]] ;

  world_to_grid_pt (&(coord[0]), &(coord[1])) ;

  while (coord[idx[0]] * dDir < coord_old * dDir)
    {
    coord[idx[0]] = coord_last + dDir * length[idx[0]] + 2 * (coord_old - coord[idx[0]]) ;
    coord_old = coord[idx[0]] ;
    world_to_grid_pt (&(coord[0]), &(coord[1])) ;
    }
  }

#ifdef GTK_GUI
static void create_default_properties_dialog (DEFAULT_PROPERTIES *dialog)
  {
  GtkWidget *lbl = NULL, *spn = NULL, *tbl = NULL ;

  tbl = dialog->tbl = gtk_table_new (3, 2, FALSE) ;
  gtk_widget_show (dialog->tbl) ;
  gtk_container_set_border_width (GTK_CONTAINER (tbl), 2) ;

  lbl = gtk_label_new (_("Cell Width:")) ;
  gtk_widget_show (lbl) ;
  gtk_table_attach (GTK_TABLE (tbl), lbl, 0, 1, 0, 1,
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), 2, 2) ;
  gtk_label_set_justify (GTK_LABEL (lbl), GTK_JUSTIFY_RIGHT) ;
  gtk_misc_set_alignment (GTK_MISC (lbl), 1.0, 0.5) ;

  dialog->adjCXCell = GTK_ADJUSTMENT (gtk_adjustment_new (1, 1, 100, 1, 10, 10)) ;
  spn = gtk_spin_button_new_infinite (dialog->adjCXCell, 1, 1, ISB_DIR_UP) ;
  gtk_widget_show (spn) ;
  gtk_table_attach (GTK_TABLE (tbl), spn, 1, 2, 0, 1,
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), 2, 2) ;
  gtk_entry_set_activates_default (GTK_ENTRY (spn), TRUE) ;

  lbl = gtk_label_new (_("Cell Height:")) ;
  gtk_widget_show (lbl) ;
  gtk_table_attach (GTK_TABLE (tbl), lbl, 0, 1, 1, 2,
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), 2, 2) ;
  gtk_label_set_justify (GTK_LABEL (lbl), GTK_JUSTIFY_RIGHT) ;
  gtk_misc_set_alignment (GTK_MISC (lbl), 1.0, 0.5) ;

  dialog->adjCYCell = GTK_ADJUSTMENT (gtk_adjustment_new (1, 1, 100, 1, 10, 10)) ;
  spn = gtk_spin_button_new_infinite (dialog->adjCYCell, 1, 1, ISB_DIR_UP) ;
  gtk_widget_show (spn) ;
  gtk_table_attach (GTK_TABLE (tbl), spn, 1, 2, 1, 2,
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), 2, 2) ;
  gtk_entry_set_activates_default (GTK_ENTRY (spn), TRUE) ;

  lbl = gtk_label_new (_("Dot Diameter:")) ;
  gtk_widget_show (lbl) ;
  gtk_table_attach (GTK_TABLE (tbl), lbl, 0, 1, 2, 3,
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), 2, 2) ;
  gtk_label_set_justify (GTK_LABEL (lbl), GTK_JUSTIFY_RIGHT) ;
  gtk_misc_set_alignment (GTK_MISC (lbl), 1.0, 0.5) ;

  dialog->adjDotDiam = GTK_ADJUSTMENT (gtk_adjustment_new (0.0001, 0.0001, 100, 1, 10, 10)) ;
  spn = gtk_spin_button_new_infinite (dialog->adjDotDiam, 1, 4, ISB_DIR_UP) ;
  g_object_set (G_OBJECT (spn), "snap-to-ticks", FALSE, "update-policy", GTK_UPDATE_IF_VALID, NULL) ;
  gtk_widget_show (spn) ;
  gtk_table_attach (GTK_TABLE (tbl), spn, 1, 2, 2, 3,
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), 2, 2) ;
  gtk_entry_set_activates_default (GTK_ENTRY (spn), TRUE) ;

  lbl = gtk_label_new (_("Default Clock:")) ;
  gtk_widget_show (lbl) ;
  gtk_table_attach (GTK_TABLE (tbl), lbl, 0, 1, 3, 4,
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), 2, 2) ;
  gtk_label_set_justify (GTK_LABEL (lbl), GTK_JUSTIFY_RIGHT) ;
  gtk_misc_set_alignment (GTK_MISC (lbl), 1.0, 0.5) ;

  dialog->cbClock = qcad_clock_combo_new () ;
  gtk_widget_show (dialog->cbClock) ;
  gtk_table_attach (GTK_TABLE (tbl), dialog->cbClock, 1, 2, 3, 4,
    (GtkAttachOptions)(GTK_FILL),
    (GtkAttachOptions)(0), 2, 2) ;
  }

static void create_properties_dialog (PROPERTIES *dialog)
  {
  GtkWidget *tbl = NULL, *tblFm = NULL, *tbllbl = NULL, *tblBig = NULL ;

  dialog->dlg = gtk_dialog_new () ;
  gtk_window_set_title (GTK_WINDOW (dialog->dlg), _("Cell Function")) ;
  gtk_window_set_resizable (GTK_WINDOW (dialog->dlg), FALSE) ;

  tblBig = gtk_table_new (2, 2, FALSE) ;
  gtk_widget_show (tblBig) ;
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog->dlg)->vbox), tblBig, TRUE, TRUE, 0) ;
  gtk_container_set_border_width (GTK_CONTAINER (tblBig), 2) ;

  tbl = gtk_table_new (4, 2, FALSE) ;
  gtk_widget_show (tbl) ;
  gtk_table_attach (GTK_TABLE (tblBig), tbl, 0, 2, 0, 1,
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), 2, 2) ;
  gtk_container_set_border_width (GTK_CONTAINER (tbl), 2) ;

  dialog->rbNormal = gtk_radio_button_new_with_label (NULL, _("Normal Cell")) ;
  gtk_widget_show (dialog->rbNormal) ;
  gtk_table_attach (GTK_TABLE (tbl), dialog->rbNormal, 0, 1, 0, 1,
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
    (GtkAttachOptions)(GTK_EXPAND), 2, 2) ;

  dialog->rbFixed = gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (dialog->rbNormal),
    _("Fixed Polarization")) ;
  gtk_widget_show (dialog->rbFixed) ;
  gtk_table_attach (GTK_TABLE (tbl), dialog->rbFixed, 0, 1, 1, 2,
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
    (GtkAttachOptions)(GTK_EXPAND), 2, 2) ;

  dialog->rbIO = gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (dialog->rbFixed),
    _("Input/Output")) ;
  gtk_widget_show (dialog->rbIO) ;
  gtk_table_attach (GTK_TABLE (tbl), dialog->rbIO, 0, 1, 2, 3,
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
    (GtkAttachOptions)(GTK_EXPAND), 2, 2) ;

  dialog->fmFixed = gtk_frame_new (_("Polarization")) ;
  gtk_widget_show (dialog->fmFixed) ;
  gtk_table_attach (GTK_TABLE (tbl), dialog->fmFixed, 1, 2, 1, 2,
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), 2, 2) ;
  gtk_container_set_border_width (GTK_CONTAINER (dialog->fmFixed), 2) ;

  tblFm = gtk_table_new (1, 1, FALSE) ;
  gtk_widget_show (tblFm) ;
  gtk_container_add (GTK_CONTAINER (dialog->fmFixed), tblFm) ;
  gtk_container_set_border_width (GTK_CONTAINER (tblFm), 2) ;

  dialog->adjPolarization = GTK_ADJUSTMENT (gtk_adjustment_new (0.00, -1.00, 1.00, 0.0001, 0.1, 0.1)) ;
  dialog->spnPolarization = gtk_spin_button_new (dialog->adjPolarization, 0.0001, 4) ;
  gtk_widget_show (dialog->spnPolarization) ;
  gtk_table_attach (GTK_TABLE (tblFm), dialog->spnPolarization, 0, 1, 0, 1,
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
    (GtkAttachOptions)(GTK_EXPAND), 2, 2) ;

  dialog->fmIO = gtk_frame_new (_("I/O")) ;
  gtk_widget_show (dialog->fmIO) ;
  gtk_table_attach (GTK_TABLE (tbl), dialog->fmIO, 1, 2, 2, 3,
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
    (GtkAttachOptions)(GTK_EXPAND), 2, 2) ;
  gtk_container_set_border_width (GTK_CONTAINER (dialog->fmIO), 2) ;

  tblFm = gtk_table_new (2, 2, FALSE) ;
  gtk_widget_show (tblFm) ;
  gtk_container_add (GTK_CONTAINER (dialog->fmIO), tblFm) ;
  gtk_container_set_border_width (GTK_CONTAINER (tblFm), 2) ;

  dialog->rbInput = gtk_radio_button_new_with_label (NULL, _("Input")) ;
  gtk_widget_show (dialog->rbInput) ;
  gtk_table_attach (GTK_TABLE (tblFm), dialog->rbInput, 0, 1, 0, 1,
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
    (GtkAttachOptions)(GTK_EXPAND), 2, 2) ;

  dialog->rbOutput = gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (dialog->rbInput), _("Output")) ;
  gtk_widget_show (dialog->rbOutput) ;
  gtk_table_attach (GTK_TABLE (tblFm), dialog->rbOutput, 1, 2, 0, 1,
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
    (GtkAttachOptions)(GTK_EXPAND), 2, 2) ;

  tbllbl = gtk_table_new (1, 2, FALSE) ;
  gtk_widget_show (tbllbl) ;
  gtk_table_attach (GTK_TABLE (tblFm), tbllbl, 0, 2, 1, 2,
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), 2, 2) ;
  gtk_container_set_border_width (GTK_CONTAINER (tbllbl), 2) ;

  dialog->lblName = gtk_label_new (_("Cell Label:")) ;
  gtk_widget_show (dialog->lblName) ;
  gtk_table_attach (GTK_TABLE (tbllbl), dialog->lblName, 0, 1, 0, 1,
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
    (GtkAttachOptions)(GTK_EXPAND), 2, 2) ;
  gtk_label_set_justify (GTK_LABEL (dialog->lblName), GTK_JUSTIFY_RIGHT) ;
  gtk_misc_set_alignment (GTK_MISC (dialog->lblName), 1.0, 0.5) ;

  dialog->txtName = gtk_entry_new () ;
  gtk_widget_show (dialog->txtName) ;
  gtk_table_attach (GTK_TABLE (tbllbl), dialog->txtName, 1, 2, 0, 1,
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
    (GtkAttachOptions)(GTK_EXPAND), 2, 2) ;
  gtk_entry_set_activates_default (GTK_ENTRY (dialog->txtName), TRUE) ;

  tbllbl = gtk_label_new (_("Clock:")) ;
  gtk_widget_show (tbllbl) ;
  gtk_table_attach (GTK_TABLE (tblBig), tbllbl, 0, 1, 1, 2,
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
    (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), 2, 2) ;
  gtk_label_set_justify (GTK_LABEL (tbllbl), GTK_JUSTIFY_RIGHT) ;
  gtk_misc_set_alignment (GTK_MISC (tbllbl), 1.0, 0.5) ;

  dialog->cbClock = qcad_clock_combo_new () ;
  gtk_widget_show (dialog->cbClock) ;
  gtk_table_attach (GTK_TABLE (tblBig), dialog->cbClock, 1, 2, 1, 2,
    (GtkAttachOptions)(GTK_FILL),
    (GtkAttachOptions)(0), 2, 2) ;

  gtk_dialog_add_button (GTK_DIALOG (dialog->dlg), GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL) ;
  gtk_dialog_add_button (GTK_DIALOG (dialog->dlg), GTK_STOCK_OK, GTK_RESPONSE_OK) ;
  gtk_dialog_set_default_response (GTK_DIALOG (dialog->dlg), GTK_RESPONSE_OK) ;

  g_signal_connect (G_OBJECT (dialog->rbNormal), "toggled", (GCallback)cell_mode_toggled, dialog->dlg) ;
  g_signal_connect (G_OBJECT (dialog->rbFixed), "toggled", (GCallback)cell_mode_toggled, dialog->dlg) ;
  g_signal_connect (G_OBJECT (dialog->rbIO), "toggled", (GCallback)cell_mode_toggled, dialog->dlg) ;
  }

static void cell_mode_toggled (GtkWidget *widget, gpointer data)
  {
  PROPERTIES *dialog = (PROPERTIES *)g_object_get_data (G_OBJECT (data), "dialog") ;

  if (!gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget))) return ;

  gtk_widget_set_sensitive (dialog->fmFixed, dialog->rbFixed == widget) ;
  gtk_widget_set_sensitive (dialog->spnPolarization, dialog->rbFixed == widget) ;
  gtk_widget_set_sensitive (dialog->fmIO, dialog->rbIO == widget) ;
  gtk_widget_set_sensitive (dialog->rbInput, dialog->rbIO == widget) ;
  gtk_widget_set_sensitive (dialog->rbOutput, dialog->rbIO == widget) ;
  gtk_widget_set_sensitive (dialog->lblName, dialog->rbIO == widget) ;
  gtk_widget_set_sensitive (dialog->txtName, dialog->rbIO == widget) ;
  }
#endif /* def GTK_GUI */

double qcad_cell_calculate_polarization (QCADCell *cell)
  {
  // I have made an optimization here but it assumes that the cell polarization quality is perfect. //
  return (cell->cell_dots[0].charge - cell->cell_dots[1].charge) * OVER_QCHARGE;
  // The old way of calculating polarization assumes nothing about quality:
  // return ((cell->cell_dots[0].charge + cell->cell_dots[2].charge) - (cell->cell_dots[1].charge + cell->cell_dots[3].charge)) * ONE_OVER_FOUR_HALF_QCHARGE ;
  }

void qcad_cell_set_function (QCADCell *cell, QCADCellFunction function)
  {
  QCADCellFunction old_function = cell->cell_function ;

  cell->cell_function = function ;
  if (QCAD_CELL_NORMAL == function)
    {
    if (NULL != cell->label)
      qcad_cell_set_label (cell, "") ;
    }
  else
  if (QCAD_CELL_FIXED == function)
    {
    memcpy (&(QCAD_DESIGN_OBJECT (cell)->clr), &clrOrange, sizeof (GdkColor)) ;
    if (NULL != cell->label)
      memcpy (&(QCAD_DESIGN_OBJECT (cell->label)->clr), &clrOrange, sizeof (GdkColor)) ;
    }
  else
  if (QCAD_CELL_INPUT == function || QCAD_CELL_OUTPUT == function)
    {
    memcpy (&(QCAD_DESIGN_OBJECT (cell)->clr), (QCAD_CELL_INPUT == function) ? &clrBlue : &clrYellow, sizeof (GdkColor)) ;
    if (NULL != cell->label)
      memcpy (&(QCAD_DESIGN_OBJECT (cell->label)->clr), (QCAD_CELL_INPUT == function) ? &clrBlue : &clrYellow, sizeof (GdkColor)) ;
    }

  if (NULL != cell->label)
    qcad_design_object_set_selected (QCAD_DESIGN_OBJECT (cell->label), QCAD_DESIGN_OBJECT (cell)->bSelected) ;

  if (old_function != function)
    g_signal_emit (cell, qcad_cell_signals[QCAD_CELL_CELL_FUNCTION_CHANGED_SIGNAL], 0) ;
  }

void qcad_cell_scale (QCADCell *cell, double dScale, double dxOrigin, double dyOrigin)
  {
  double xOld = 0.0, yOld = 0.0 ;
  QCADDesignObject *obj = NULL ;

  if (NULL == cell) return ;

  obj = QCAD_DESIGN_OBJECT (cell) ;

  xOld = obj->bounding_box.xWorld ;
  yOld = obj->bounding_box.yWorld ;

  obj->bounding_box.xWorld = dScale * obj->bounding_box.xWorld - dxOrigin ;
  obj->bounding_box.yWorld = dScale * obj->bounding_box.yWorld - dyOrigin ;
  obj->bounding_box.cxWorld *= dScale ;
  obj->bounding_box.cyWorld *= dScale ;
  cell->cell_dots[0].x = dScale * cell->cell_dots[0].x - dxOrigin ;
  cell->cell_dots[0].y = dScale * cell->cell_dots[0].y - dyOrigin ;
  cell->cell_dots[0].diameter *= dScale ;
  cell->cell_dots[1].x = dScale * cell->cell_dots[1].x - dxOrigin ;
  cell->cell_dots[1].y = dScale * cell->cell_dots[1].y - dyOrigin ;
  cell->cell_dots[1].diameter *= dScale ;
  cell->cell_dots[2].x = dScale * cell->cell_dots[2].x - dxOrigin ;
  cell->cell_dots[2].y = dScale * cell->cell_dots[2].y - dyOrigin ;
  cell->cell_dots[2].diameter *= dScale ;
  cell->cell_dots[3].x = dScale * cell->cell_dots[3].x - dxOrigin ;
  cell->cell_dots[3].y = dScale * cell->cell_dots[3].y - dyOrigin ;
  cell->cell_dots[3].diameter *= dScale ;

  obj->x = obj->bounding_box.xWorld + obj->bounding_box.cxWorld / 2.0 ;
  obj->y = obj->bounding_box.yWorld + obj->bounding_box.cyWorld / 2.0 ;

  if (NULL != cell->label)
    qcad_design_object_move (QCAD_DESIGN_OBJECT (cell->label), obj->bounding_box.xWorld - xOld, obj->bounding_box.yWorld - yOld) ;
  }

void qcad_cell_set_clock (QCADCell *cell, int iClock)
  {
  cell->cell_options.clock = iClock ;
  if (QCAD_CELL_NORMAL == cell->cell_function)
    memcpy (&(QCAD_DESIGN_OBJECT (cell)->clr), &(clrClock[iClock]), sizeof (GdkColor)) ;
  }

void qcad_cell_set_label (QCADCell *cell, char *pszLabel)
  {
  gboolean bNewLabel = FALSE ;

  if (NULL == pszLabel || NULL == cell) return ;

  if (0 == pszLabel[0])
    if (NULL != cell->label)
      {
      g_signal_emit_by_name (G_OBJECT (cell), "removed", cell->label) ;
      g_object_unref (cell->label) ;
      cell->bLabelRemoved = TRUE ;
      return ;
      }

  if ((bNewLabel = (NULL == cell->label)))
    {
    cell->label = QCAD_LABEL (g_object_new (QCAD_TYPE_LABEL, NULL)) ;
    g_object_add_weak_pointer (G_OBJECT (cell->label), (gpointer *)&(cell->label)) ;
    g_signal_emit_by_name (G_OBJECT (cell), "added", cell->label) ;
    cell->bLabelRemoved = FALSE ;
    }

  memcpy (&(QCAD_DESIGN_OBJECT (cell->label)->clr), &(QCAD_DESIGN_OBJECT (cell)->clr), sizeof (GdkColor)) ;
  qcad_label_set_text (cell->label, pszLabel) ;

  if (bNewLabel)
    qcad_design_object_move_to (QCAD_DESIGN_OBJECT (cell->label),
      QCAD_DESIGN_OBJECT (cell)->bounding_box.xWorld + QCAD_DESIGN_OBJECT (cell->label)->bounding_box.cxWorld / 2.0,
      QCAD_DESIGN_OBJECT (cell)->bounding_box.yWorld - QCAD_DESIGN_OBJECT (cell->label)->bounding_box.cyWorld / 2.0 - QCAD_CELL_LABEL_DEFAULT_OFFSET_Y) ;

  if (QCAD_DESIGN_OBJECT (cell)->bSelected)
    qcad_design_object_set_selected (QCAD_DESIGN_OBJECT (cell->label), TRUE) ;
  }

void qcad_cell_set_polarization (QCADCell *cell, double new_polarization)
  {
  char *psz = NULL ;
  double half_charge_polarization = HALF_QCHARGE * new_polarization;

  cell->cell_dots[0].charge =
  cell->cell_dots[2].charge = half_charge_polarization + HALF_QCHARGE;
  cell->cell_dots[1].charge =
  cell->cell_dots[3].charge = HALF_QCHARGE - half_charge_polarization;

  if (QCAD_CELL_FIXED == cell->cell_function)
    {
    psz = g_strdup_printf ("%.2lf", new_polarization) ;
    qcad_cell_set_label (cell, psz) ;
    g_free (psz) ;
    }
  }

#ifdef STDIO_FILEIO
static gboolean qcad_cell_dot_unserialize (FILE *fp, QCADCellDot *pdots, int idxDot)
  {
  char *pszLine = NULL, *pszValue = NULL ;
  gboolean bHaveDot = FALSE ;
  QCADCellDot dot = {0} ;

  if (!SkipPast (fp, '\0', "[TYPE:CELL_DOT]", NULL))
    return FALSE ;

  while (TRUE)
    {
    if (NULL == (pszLine = ReadLine (fp, '\0', TRUE))) break ;

    if (!strcmp (pszLine, "[#TYPE:CELL_DOT]"))
      {
      g_free (pszLine) ;
      break ;
      }

    tokenize_line (pszLine, strlen (pszLine), &pszValue, '=') ;

    if ((bHaveDot = !strcmp (pszLine, "x")))
      dot.x = g_ascii_strtod (pszValue, NULL) ;
    else
    if ((bHaveDot = !strcmp (pszLine, "y")))
      dot.y = g_ascii_strtod (pszValue, NULL) ;
    else
    if ((bHaveDot = !strcmp (pszLine, "diameter")))
      dot.diameter = g_ascii_strtod (pszValue, NULL) ;
    else
    if ((bHaveDot = !strcmp (pszLine, "charge")))
      dot.charge = g_ascii_strtod (pszValue, NULL) ;
    else
    if ((bHaveDot = !strcmp (pszLine, "spin")))
      dot.spin = g_ascii_strtod (pszValue, NULL) ;
    else
    if ((bHaveDot = !strcmp (pszLine, "potential")))
      dot.potential = g_ascii_strtod (pszValue, NULL) ;

    g_free (pszLine) ;
    g_free (ReadLine (fp, '\0', FALSE)) ;
    }

  if (!bHaveDot) return FALSE ;
  else
    memcpy (&(pdots[idxDot]), &dot, sizeof (QCADCellDot));

  return TRUE ;
  }

#endif /* STDIO_FILEIO */
