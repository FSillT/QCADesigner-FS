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
// Contents:                                            //
//                                                      //
// The Fault Simulation Setup dialog allows the user to //
// set the parameters for fault simulations.            //
//                                                      //
//DAYANE ALFENAS REIS                                   //
//////////////////////////////////////////////////////////

#include <stdlib.h>
#include <gtk/gtk.h>

#include "support.h"
#include "USE_size_setup_dialog.h"

typedef struct
  {
  GtkWidget *USE_size_setup_dialog;
  GtkWidget *dialog_vbox1;
  GtkWidget *table;
  GtkWidget *label1;
  GtkWidget *USE_size_entry;
  GtkWidget *USE_size_setup_dialog_ok_button;
  GtkWidget *USE_size_setup_dialog_cancel_button;
  } USE_size_setup_dialog_D;

static USE_size_setup_dialog_D USE_size_setup_dialog = {NULL};

static void create_USE_size_setup_dialog (USE_size_setup_dialog_D *dialog) ;

int get_USE_size_from_user (GtkWindow *parent, int USE_size)
  {
  char sz[16] = "" ;
  if (NULL == USE_size_setup_dialog.USE_size_setup_dialog)
    create_USE_size_setup_dialog (&USE_size_setup_dialog) ;

  gtk_window_set_transient_for (GTK_WINDOW (USE_size_setup_dialog.USE_size_setup_dialog), parent) ;

  g_snprintf (sz, 16, "%d", USE_size) ;
  gtk_entry_set_text (GTK_ENTRY (USE_size_setup_dialog.USE_size_entry), sz) ;

  if (GTK_RESPONSE_OK == gtk_dialog_run (GTK_DIALOG (USE_size_setup_dialog.USE_size_setup_dialog)))
    {
    USE_size =         atoi (gtk_entry_get_text (GTK_ENTRY (USE_size_setup_dialog.USE_size_entry))) ;
  }

  gtk_widget_hide (USE_size_setup_dialog.USE_size_setup_dialog) ;

  if (NULL != parent)
    gtk_window_present (parent) ;

  return USE_size;
  }


static void create_USE_size_setup_dialog (USE_size_setup_dialog_D *dialog)
  {
  if (NULL != dialog->USE_size_setup_dialog) return ;

  dialog->USE_size_setup_dialog = gtk_dialog_new ();
  gtk_window_set_title (GTK_WINDOW (dialog->USE_size_setup_dialog), _("Configure USE size"));
  gtk_window_set_resizable (GTK_WINDOW (dialog->USE_size_setup_dialog), FALSE);
  gtk_window_set_modal (GTK_WINDOW (dialog->USE_size_setup_dialog), TRUE) ;

  dialog->dialog_vbox1 = GTK_DIALOG (dialog->USE_size_setup_dialog)->vbox;
  gtk_widget_show (dialog->dialog_vbox1);

  dialog->table = gtk_table_new (3, 2, FALSE);
  gtk_widget_show (dialog->table);
  gtk_container_set_border_width (GTK_CONTAINER (dialog->table), 2);
  gtk_box_pack_start (GTK_BOX (dialog->dialog_vbox1), dialog->table, TRUE, TRUE, 0);

  dialog->label1 = gtk_label_new (_("USE size:"));
  gtk_widget_show (dialog->label1);
  gtk_table_attach (GTK_TABLE (dialog->table), dialog->label1, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);
  gtk_label_set_justify (GTK_LABEL (dialog->label1), GTK_JUSTIFY_RIGHT);
  gtk_misc_set_alignment (GTK_MISC (dialog->label1), 1, 0.5);

  dialog->USE_size_entry = gtk_entry_new ();
  gtk_widget_show (dialog->USE_size_entry);
  gtk_table_attach (GTK_TABLE (dialog->table), dialog->USE_size_entry, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);
  gtk_entry_set_activates_default (GTK_ENTRY (dialog->USE_size_entry), TRUE) ;


  gtk_dialog_add_button (GTK_DIALOG (dialog->USE_size_setup_dialog), GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL) ;
  gtk_dialog_add_button (GTK_DIALOG (dialog->USE_size_setup_dialog), GTK_STOCK_OK, GTK_RESPONSE_OK) ;
  gtk_dialog_set_default_response (GTK_DIALOG (dialog->USE_size_setup_dialog), GTK_RESPONSE_OK) ;
  }
