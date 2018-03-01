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
#include "sim_fault_read_dialog.h"
#include "file_selection_window.h"
#include "fault_simulation.h"


typedef struct
  {
  GtkWidget *sim_fault_read_dialog;
  GtkWidget *dialog_vbox1;
  GtkWidget *table;
  GtkWidget *label1;
  GtkWidget *label2;
  GtkWidget *label3;
  GtkWidget *label4;
  GtkWidget *path1_entry;
  GtkWidget *path2_entry;
  GtkWidget *action_area;
  GtkWidget *input_path1_button;
  GtkWidget *input_path2_button;
  GtkWidget *sim_fault_read_dialog_ok_button;
  GtkWidget *sim_fault_read_dialog_cancel_button;
  } sim_fault_read_dialog_D;


static sim_fault_read_dialog_D sim_fault_read_dialog = {NULL};

static void create_sim_fault_read_dialog (sim_fault_read_dialog_D *dialog, analysis_result_fault_simulation_OP *pbo) ;
static void input1_path_button_clicked(GtkButton *button, gpointer user_data);
static void input2_path_button_clicked(GtkButton *button, gpointer user_data);

void get_sim_fault_read_from_user (GtkWindow *parent, analysis_result_fault_simulation_OP *pbo)
  {

  char sz1[64] = "" ;
  char sz2[64] = "" ;
  GtkWidget *msg = NULL ;
  gchar *path1 = NULL, *path2 = NULL;

  if (NULL == sim_fault_read_dialog.sim_fault_read_dialog)
    create_sim_fault_read_dialog (&sim_fault_read_dialog, pbo) ;

  gtk_window_set_transient_for (GTK_WINDOW (sim_fault_read_dialog.sim_fault_read_dialog), parent) ;

  g_snprintf (sz1, 64, "%s", pbo->path_results_file) ;
  gtk_entry_set_text (GTK_ENTRY (sim_fault_read_dialog.path1_entry), sz1) ;

  g_snprintf (sz2, 64, "%s", pbo->path_design_file) ;
  gtk_entry_set_text (GTK_ENTRY (sim_fault_read_dialog.path2_entry), sz2) ;

 if (GTK_RESPONSE_OK == gtk_dialog_run (GTK_DIALOG (sim_fault_read_dialog.sim_fault_read_dialog)))
    {

          path1 =    gtk_entry_get_text (GTK_ENTRY (sim_fault_read_dialog.path1_entry)) ;
          path2 =    gtk_entry_get_text (GTK_ENTRY (sim_fault_read_dialog.path2_entry)) ;


          if ((g_file_test (path1, G_FILE_TEST_EXISTS) == FALSE ) || (g_file_test (path2, G_FILE_TEST_EXISTS) == FALSE ))
	    {
      	     gtk_dialog_run (GTK_DIALOG (msg = gtk_message_dialog_new (GTK_DIALOG(sim_fault_read_dialog.sim_fault_read_dialog), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "Please check if the files indicated are valids and then try again. ")));
      	     gtk_widget_hide (msg) ;
      	     return;
	    }
          //else
              //synthetize_phase_shifts_sim_results(path1, path2);
	      //voltar com isso depois, organizar
              //synthetize_and_create_hm(path1, path2);

    }
  gtk_widget_hide (sim_fault_read_dialog.sim_fault_read_dialog) ;

   if (NULL != parent)
    gtk_window_present (parent) ;
  }

static void create_sim_fault_read_dialog (sim_fault_read_dialog_D *dialog, analysis_result_fault_simulation_OP *pbo)
  {
  GtkWidget *hbox1 = NULL, *hbox2 = NULL, *lbl = NULL ;

  if (NULL != dialog->sim_fault_read_dialog) return ;

  dialog->sim_fault_read_dialog = gtk_dialog_new ();
  gtk_window_set_title (GTK_WINDOW (dialog->sim_fault_read_dialog), _("Read Fault Simulation Result File and Create Heat Maps"));
  gtk_window_set_resizable (GTK_WINDOW (dialog->sim_fault_read_dialog), FALSE);
  gtk_window_set_modal (GTK_WINDOW (dialog->sim_fault_read_dialog), TRUE) ;

  dialog->dialog_vbox1 = GTK_DIALOG (dialog->sim_fault_read_dialog)->vbox;
  gtk_widget_show (dialog->dialog_vbox1);

  dialog->table = gtk_table_new (9, 3, FALSE);
  gtk_widget_show (dialog->table);
  gtk_container_set_border_width (GTK_CONTAINER (dialog->table), 2);
  gtk_box_pack_start (GTK_BOX (dialog->dialog_vbox1), dialog->table, TRUE, TRUE, 0);

  dialog->label1 = gtk_label_new (_("Results File:"));
  gtk_widget_show (dialog->label1);
  gtk_table_attach (GTK_TABLE (dialog->table), dialog->label1, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_SHRINK),
                    (GtkAttachOptions) (GTK_SHRINK), 2, 2);
  gtk_label_set_justify (GTK_LABEL (dialog->label1), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (dialog->label1), 1, 0.5);

  dialog->path1_entry = gtk_entry_new ();
  gtk_widget_show (dialog->path1_entry);
  gtk_table_attach (GTK_TABLE (dialog->table), dialog->path1_entry, 1, 2, 0, 1,
		  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
		  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);
  gtk_entry_set_activates_default (GTK_ENTRY (dialog->path1_entry), TRUE) ;

  dialog->label2 = gtk_label_new (_("Original Design File:"));
  gtk_widget_show (dialog->label2);
  gtk_table_attach (GTK_TABLE (dialog->table), dialog->label2, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_SHRINK),
                    (GtkAttachOptions) (GTK_SHRINK), 2, 2);
  gtk_label_set_justify (GTK_LABEL (dialog->label2), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (dialog->label2), 1, 0.5);

  dialog->path2_entry = gtk_entry_new ();
  gtk_widget_show (dialog->path2_entry);
  gtk_table_attach (GTK_TABLE (dialog->table), dialog->path2_entry, 1, 2, 1, 2,
		  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
		  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);
  gtk_entry_set_activates_default (GTK_ENTRY (dialog->path2_entry), TRUE) ;

  //
  // Find files
    dialog->input_path1_button = gtk_button_new ();
    gtk_widget_show (dialog->input_path1_button);
    hbox1 = gtk_hbox_new (FALSE, 4) ;
    gtk_widget_show (hbox1) ;
    lbl = gtk_label_new (_("Find a result file...")) ;
    gtk_widget_show (lbl) ;
    gtk_box_pack_start (GTK_BOX (hbox1), lbl, TRUE, TRUE, 0) ;
    gtk_container_add (GTK_CONTAINER (dialog->input_path1_button), hbox1) ;

    dialog->input_path2_button = gtk_button_new ();
    gtk_widget_show (dialog->input_path2_button);
    hbox2 = gtk_hbox_new (FALSE, 2) ;
    gtk_widget_show (hbox2) ;
    lbl = gtk_label_new (_("Find a design file...")) ;
    gtk_widget_show (lbl) ;
    gtk_box_pack_start (GTK_BOX (hbox2), lbl, TRUE, TRUE, 0) ;
    gtk_container_add (GTK_CONTAINER (dialog->input_path2_button), hbox2) ;

    gtk_container_add (GTK_CONTAINER (GTK_DIALOG (dialog->sim_fault_read_dialog)->action_area), dialog->input_path1_button) ;
    gtk_container_add (GTK_CONTAINER (GTK_DIALOG (dialog->sim_fault_read_dialog)->action_area), dialog->input_path2_button) ;
    gtk_dialog_add_button (GTK_DIALOG (dialog->sim_fault_read_dialog), GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL) ;
    gtk_dialog_add_button (GTK_DIALOG (dialog->sim_fault_read_dialog), GTK_STOCK_OK, GTK_RESPONSE_OK) ;
    gtk_dialog_set_default_response (GTK_DIALOG (dialog->sim_fault_read_dialog), GTK_RESPONSE_OK) ;


    g_signal_connect (G_OBJECT (dialog->input_path1_button),  "clicked", (GCallback)(input1_path_button_clicked), dialog->sim_fault_read_dialog);
    g_signal_connect (G_OBJECT (dialog->input_path2_button),  "clicked", (GCallback)(input2_path_button_clicked), dialog->sim_fault_read_dialog);
  }
void input1_path_button_clicked(GtkButton *button, gpointer user_data)
    {

    gchar *pszTempFName = NULL ;
    gchar *pszFName = NULL ;
    GtkWidget *msg = NULL ;

    //calls a dialog for the input file path

        if (NULL == (pszTempFName = get_file_name_from_user (GTK_WINDOW (user_data), _("Results file"), pszFName, FALSE)))
         return ;

        if (g_file_test (pszTempFName, G_FILE_TEST_IS_DIR) == TRUE )
    	{
    	gtk_dialog_run (GTK_DIALOG (msg = gtk_message_dialog_new (GTK_WINDOW (user_data), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, _("Please select a file, not a folder.")))) ;
    	gtk_widget_hide (msg) ;

    	}

        if (NULL != pszFName) g_free (pszFName) ;
         {
            pszFName = pszTempFName ;
            gtk_entry_set_text (GTK_ENTRY (sim_fault_read_dialog.path1_entry), pszTempFName) ;

         }

    }

void input2_path_button_clicked(GtkButton *button, gpointer user_data)
    {

    gchar *pszTempFName = NULL ;
    gchar *pszFName = NULL ;
    GtkWidget *msg = NULL ;

    //calls a dialog for the input file path

        if (NULL == (pszTempFName = get_file_name_from_user (GTK_WINDOW (user_data), _("Original Design file"), pszFName, FALSE)))
         return ;

        if (g_file_test (pszTempFName, G_FILE_TEST_IS_DIR) == TRUE )
    	{
    	gtk_dialog_run (GTK_DIALOG (msg = gtk_message_dialog_new (GTK_WINDOW (user_data), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, _("Please select a file, not a folder.")))) ;
    	gtk_widget_hide (msg) ;

    	}

        if (NULL != pszFName) g_free (pszFName) ;
         {
            pszFName = pszTempFName ;
            gtk_entry_set_text (GTK_ENTRY (sim_fault_read_dialog.path2_entry), pszTempFName) ;

         }

    }



