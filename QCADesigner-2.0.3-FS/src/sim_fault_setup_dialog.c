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
// Please use complete names in variables and functions //
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
#include "callbacks.h"
#include "fault_simulation.h"
#include "sim_fault_setup_dialog.h"
#include "file_selection_window.h"

typedef struct
  {
  GtkWidget *sim_fault_setup_dialog;
  GtkWidget *dialog_vbox1;
  GtkWidget *table;


  GtkWidget *lbl_high_th;
  GtkWidget *lbl_low_th;
  GtkWidget *lbl_sample_interval;
  GtkWidget *lbl_latency;
  GtkWidget *high_th_entry;
  GtkWidget *low_th_entry;
  GtkWidget *sample_interval_entry;
  GtkWidget *latency_entry;
  GtkWidget *kind_of_fault_cmb;
  GtkWidget *lbl_kind_of_fault;
  GtkWidget *lbl_num_simulations;
  GtkWidget *lbl_inicial_id;
  GtkWidget *number_of_simulations_entry;
  GtkWidget *inicial_ID_entry;

  GtkWidget *lbl_prob_engine;
  GtkWidget *lbl_fault_class;
  GtkWidget *combo;
  GSList    *vbox1_group;
  GtkWidget *chkVacancy;
  GtkWidget *chkInterstitial;
  GtkWidget *chkDopant;
  GtkWidget *chkDislocation;
  GtkWidget *probVacancy_entry;
  GtkWidget *probInterstitial_entry;
  GtkWidget *probDopant_entry;
  GtkWidget *probDislocation_entry;

  GSList    *vbox2_group;
  GtkWidget *initial_shift_entry;
  GtkWidget *final_shift_entry;
  GtkWidget *p_engine_shifts_cmb;
  GtkWidget *fixed_phase_entry;
  GtkWidget *frequency_shift_entry;
  GtkWidget *lbl_initial_shift;
  GtkWidget *lbl_final_shift;
  GtkWidget *lbl_p_engine_shifts;
  GtkWidget *lbl_fixed_phase;
  GtkWidget *lbl_frequency_shift;

  GtkWidget *action_area;
  GtkWidget *sim_fault_setup_dialog_ok_button;
  GtkWidget *sim_fault_setup_dialog_cancel_button;

  } sim_fault_setup_dialog_D;

static sim_fault_setup_dialog_D sim_fault_setup_dialog = {NULL};

static void create_sim_fault_setup_dialog (sim_fault_setup_dialog_D *dialog, fault_simulation_OP *pbo) ;
static void kind_of_fault_cmb_changed(GtkComboBox *combo, sim_fault_setup_dialog_D *dialog, gpointer user_data) ;
static void p_engine_shifts_cmb_changed(GtkComboBox *combo, sim_fault_setup_dialog_D *dialog, gpointer user_data) ;
static void combo_changed(GtkComboBox *combo, sim_fault_setup_dialog_D *dialog, gpointer userdata);

void get_sim_fault_from_user (GtkWindow *parent, fault_simulation_OP *pbo)
  {

  char sz[16] = "" ;
  if (NULL == sim_fault_setup_dialog.sim_fault_setup_dialog)
    create_sim_fault_setup_dialog (&sim_fault_setup_dialog, pbo) ;

  gtk_window_set_transient_for (GTK_WINDOW (sim_fault_setup_dialog.sim_fault_setup_dialog), parent) ;

  //g_snprintf (sz, 16, "%f", pbo->p_engine) ;
  //gtk_entry_set_text (GTK_ENTRY (sim_fault_setup_dialog.combo), sz) ;

  //g_snprintf (sz, 16, "%f", pbo->kind_of_fault) ;
  //gtk_entry_set_text (GTK_ENTRY (sim_fault_setup_dialog.kind_of_fault_cmb), sz) ;

  //g_snprintf (sz, 16, "%f", pbo->p_engine) ;
  //gtk_entry_set_text (GTK_ENTRY (sim_fault_setup_dialog.p_engine_shifts_cmb), sz) ;
  gtk_combo_box_get_active(GTK_COMBO_BOX (sim_fault_setup_dialog.p_engine_shifts_cmb));
  gtk_combo_box_get_active(GTK_COMBO_BOX (sim_fault_setup_dialog.kind_of_fault_cmb));
  gtk_combo_box_get_active(GTK_COMBO_BOX (sim_fault_setup_dialog.combo));


  g_snprintf (sz, 64, "%f", pbo->initial_shift) ;
  gtk_entry_set_text (GTK_ENTRY (sim_fault_setup_dialog.initial_shift_entry), sz) ;

  g_snprintf (sz, 64, "%f", pbo->final_shift) ;
  gtk_entry_set_text (GTK_ENTRY (sim_fault_setup_dialog.final_shift_entry), sz) ;

  g_snprintf (sz, 16, "%d", pbo->fixed_clock_phase) ;
  gtk_entry_set_text (GTK_ENTRY (sim_fault_setup_dialog.fixed_phase_entry), sz) ;

  g_snprintf (sz, 64, "%f", pbo->frequency_shift) ;
  gtk_entry_set_text (GTK_ENTRY (sim_fault_setup_dialog.frequency_shift_entry), sz) ;

  g_snprintf (sz, 64, "%f", pbo->high_th) ;
  gtk_entry_set_text (GTK_ENTRY (sim_fault_setup_dialog.high_th_entry), sz) ;

  g_snprintf (sz, 64, "%f", pbo->low_th) ;
  gtk_entry_set_text (GTK_ENTRY (sim_fault_setup_dialog.low_th_entry), sz) ;

  g_snprintf (sz, 64, "%f", pbo->sample_interval) ;
  gtk_entry_set_text (GTK_ENTRY (sim_fault_setup_dialog.sample_interval_entry), sz) ;

  g_snprintf (sz, 64, "%f", pbo->latency) ;
  gtk_entry_set_text (GTK_ENTRY (sim_fault_setup_dialog.latency_entry), sz) ;

  g_snprintf (sz, 16, "%d", pbo->number_of_simulations) ;
  gtk_entry_set_text (GTK_ENTRY (sim_fault_setup_dialog.number_of_simulations_entry), sz) ;

  g_snprintf (sz, 16, "%d", pbo->inicial_ID) ;
  gtk_entry_set_text (GTK_ENTRY (sim_fault_setup_dialog.inicial_ID_entry), sz) ;

  g_snprintf (sz, 64, "%f", pbo->probvacancy) ;
  gtk_entry_set_text (GTK_ENTRY (sim_fault_setup_dialog.probVacancy_entry), sz) ;

  g_snprintf (sz, 64, "%f", pbo->probinterstitial) ;
  gtk_entry_set_text (GTK_ENTRY (sim_fault_setup_dialog.probInterstitial_entry), sz) ;

  g_snprintf (sz, 64, "%f", pbo->probdopant) ;
  gtk_entry_set_text (GTK_ENTRY (sim_fault_setup_dialog.probDopant_entry), sz) ;

  g_snprintf (sz, 64, "%f", pbo->probdislocation) ;
  gtk_entry_set_text (GTK_ENTRY (sim_fault_setup_dialog.probDislocation_entry), sz) ;

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (sim_fault_setup_dialog.chkVacancy),
    pbo->vacancy) ;

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (sim_fault_setup_dialog.chkInterstitial),
    pbo->interstitial) ;

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (sim_fault_setup_dialog.chkDopant),
    pbo->dopant) ;

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (sim_fault_setup_dialog.chkDislocation),
    pbo->dislocation) ;

  if (GTK_RESPONSE_OK == gtk_dialog_run (GTK_DIALOG (sim_fault_setup_dialog.sim_fault_setup_dialog)))
    {
    pbo->number_of_simulations =         atoi (gtk_entry_get_text (GTK_ENTRY (sim_fault_setup_dialog.number_of_simulations_entry))) ;
    pbo->inicial_ID =         atoi (gtk_entry_get_text (GTK_ENTRY (sim_fault_setup_dialog.inicial_ID_entry))) ;
    pbo->p_engine =           gtk_combo_box_get_active (GTK_COMBO_BOX(sim_fault_setup_dialog.combo)) ;
    pbo->vacancy =           gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (sim_fault_setup_dialog.chkVacancy)) ;
    pbo->interstitial =        gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (sim_fault_setup_dialog.chkInterstitial)) ;
    pbo->dopant =           gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (sim_fault_setup_dialog.chkDopant)) ;
    pbo->dislocation =        gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (sim_fault_setup_dialog.chkDislocation)) ;
    pbo->probdislocation =       atof (gtk_entry_get_text (GTK_ENTRY (sim_fault_setup_dialog.probDislocation_entry))) ;
    pbo->probdopant =        atof (gtk_entry_get_text (GTK_ENTRY (sim_fault_setup_dialog.probDopant_entry))) ;
    pbo->probinterstitial =        atof (gtk_entry_get_text (GTK_ENTRY (sim_fault_setup_dialog.probInterstitial_entry))) ;
    pbo->probvacancy =        atof (gtk_entry_get_text (GTK_ENTRY (sim_fault_setup_dialog.probVacancy_entry))) ;
    pbo->high_th =        atof (gtk_entry_get_text (GTK_ENTRY (sim_fault_setup_dialog.high_th_entry))) ;
    pbo->low_th =        atof (gtk_entry_get_text (GTK_ENTRY (sim_fault_setup_dialog.low_th_entry))) ;
    pbo->sample_interval =        atof (gtk_entry_get_text (GTK_ENTRY (sim_fault_setup_dialog.sample_interval_entry))) ;
    pbo->latency =        atof (gtk_entry_get_text (GTK_ENTRY (sim_fault_setup_dialog.latency_entry))) ;

    pbo->initial_shift =        atof (gtk_entry_get_text (GTK_ENTRY (sim_fault_setup_dialog.initial_shift_entry))) ;
    pbo->final_shift =        atof (gtk_entry_get_text (GTK_ENTRY (sim_fault_setup_dialog.final_shift_entry))) ;
    pbo->fixed_clock_phase =     atoi (gtk_entry_get_text (GTK_ENTRY (sim_fault_setup_dialog.fixed_phase_entry))) ;
    pbo->p_engine_shifts =       gtk_combo_box_get_active (GTK_COMBO_BOX(sim_fault_setup_dialog.p_engine_shifts_cmb)) ;
    pbo->kind_of_fault =       gtk_combo_box_get_active (GTK_COMBO_BOX(sim_fault_setup_dialog.kind_of_fault_cmb)) ;
    pbo->frequency_shift =        atof (gtk_entry_get_text (GTK_ENTRY (sim_fault_setup_dialog.frequency_shift_entry))) ;

    //project_options.SIMULATION_MODE= gtk_combo_box_get_active (GTK_COMBO_BOX(sim_fault_setup_dialog.kind_of_fault_cmb)) ;

    }

  gtk_widget_hide (sim_fault_setup_dialog.sim_fault_setup_dialog) ;

  if (NULL != parent)
    gtk_window_present (parent) ;
  }

static void create_sim_fault_setup_dialog (sim_fault_setup_dialog_D *dialog, fault_simulation_OP *pbo)
  {
  GtkWidget *hbox = NULL, *lbl = NULL ;

  if (NULL != dialog->sim_fault_setup_dialog) return ;

  dialog->sim_fault_setup_dialog = gtk_dialog_new ();
  gtk_window_set_title (GTK_WINDOW (dialog->sim_fault_setup_dialog), _("Fault Simulation Setup"));
  gtk_window_set_resizable (GTK_WINDOW (dialog->sim_fault_setup_dialog), FALSE);
  gtk_window_set_modal (GTK_WINDOW (dialog->sim_fault_setup_dialog), TRUE) ;
  gtk_window_set_default_size (GTK_WINDOW (dialog->sim_fault_setup_dialog), 200,200);

  dialog->dialog_vbox1 = GTK_DIALOG (dialog->sim_fault_setup_dialog)->vbox;
  gtk_widget_show (dialog->dialog_vbox1);

  dialog->table = gtk_table_new (13, 2, TRUE);
  gtk_widget_show (dialog->table);
  gtk_container_set_border_width (GTK_CONTAINER (dialog->table), 2);
  gtk_box_pack_start (GTK_BOX (dialog->dialog_vbox1), dialog->table, TRUE, TRUE, 0);

  dialog->lbl_high_th = gtk_label_new (_("High Threshold:"));
  gtk_widget_show (dialog->lbl_high_th);
  gtk_table_attach (GTK_TABLE (dialog->table), dialog->lbl_high_th, 0, 1, 0, 1,
		  (GtkAttachOptions) (GTK_SHRINK),
  		  (GtkAttachOptions) (GTK_SHRINK), 2, 2);
  gtk_label_set_justify (GTK_LABEL (dialog->lbl_high_th), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (dialog->lbl_high_th), 1, 0.5);

  dialog->high_th_entry = gtk_entry_new ();
  gtk_widget_show (dialog->high_th_entry);
  gtk_table_attach (GTK_TABLE (dialog->table), dialog->high_th_entry, 1, 2, 0, 1,
		  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
		  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);
  gtk_entry_set_activates_default (GTK_ENTRY (dialog->high_th_entry), TRUE) ;

  dialog->lbl_low_th = gtk_label_new (_("Low Threshold:"));
  gtk_widget_show (dialog->lbl_low_th);
  gtk_table_attach (GTK_TABLE (dialog->table), dialog->lbl_low_th, 0, 1, 1, 2,
		  (GtkAttachOptions) (GTK_SHRINK),
  		  (GtkAttachOptions) (GTK_SHRINK), 2, 2);
  gtk_label_set_justify (GTK_LABEL (dialog->lbl_low_th), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (dialog->lbl_low_th), 1, 0.5);

  dialog->low_th_entry = gtk_entry_new ();
  gtk_widget_show (dialog->low_th_entry);
  gtk_table_attach (GTK_TABLE (dialog->table), dialog->low_th_entry, 1, 2, 1, 2,
		  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
		  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);
  gtk_entry_set_activates_default (GTK_ENTRY (dialog->low_th_entry), TRUE) ;

  dialog->lbl_sample_interval = gtk_label_new (_("Sample Interval:"));
  gtk_widget_show (dialog->lbl_sample_interval);
  gtk_table_attach (GTK_TABLE (dialog->table), dialog->lbl_sample_interval, 0, 1, 2, 3,
		  (GtkAttachOptions) (GTK_SHRINK),
  		  (GtkAttachOptions) (GTK_SHRINK), 2, 2);
  gtk_label_set_justify (GTK_LABEL (dialog->lbl_sample_interval), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (dialog->lbl_sample_interval), 1, 0.5);

  dialog->sample_interval_entry = gtk_entry_new ();
  gtk_widget_show (dialog->sample_interval_entry);
  gtk_table_attach (GTK_TABLE (dialog->table), dialog->sample_interval_entry, 1, 2, 2, 3,
		  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
		  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);
  gtk_entry_set_activates_default (GTK_ENTRY (dialog->sample_interval_entry), TRUE) ;

  dialog->lbl_latency = gtk_label_new (_("Latency:"));
  gtk_widget_show (dialog->lbl_latency);
  gtk_table_attach (GTK_TABLE (dialog->table), dialog->lbl_latency, 0, 1, 3, 4,
		  (GtkAttachOptions) (GTK_SHRINK),
  		  (GtkAttachOptions) (GTK_SHRINK), 2, 2);
  gtk_label_set_justify (GTK_LABEL (dialog->lbl_latency), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (dialog->lbl_latency), 1, 0.5);

  dialog->latency_entry = gtk_entry_new ();
  gtk_widget_show (dialog->latency_entry);
  gtk_table_attach (GTK_TABLE (dialog->table), dialog->latency_entry, 1, 2, 3, 4,
		  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
		  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);
  gtk_entry_set_activates_default (GTK_ENTRY (dialog->latency_entry), TRUE) ;

  dialog->lbl_num_simulations = gtk_label_new (_("Iterations:"));
  gtk_widget_show (dialog->lbl_num_simulations);
  gtk_table_attach (GTK_TABLE (dialog->table), dialog->lbl_num_simulations, 0, 1, 4, 5,
		  (GtkAttachOptions) (GTK_SHRINK),
		  (GtkAttachOptions) (GTK_SHRINK), 2, 2);
  gtk_label_set_justify (GTK_LABEL (dialog->lbl_num_simulations), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (dialog->lbl_num_simulations), 1, 0.5);

  dialog->number_of_simulations_entry = gtk_entry_new ();
  gtk_widget_show (dialog->number_of_simulations_entry);
  gtk_table_attach (GTK_TABLE (dialog->table), dialog->number_of_simulations_entry, 1, 2, 4, 5,
		  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
		  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);
  gtk_entry_set_activates_default (GTK_ENTRY (dialog->number_of_simulations_entry), TRUE) ;

  dialog->lbl_inicial_id = gtk_label_new (_("Inicial ID:"));
  gtk_widget_show (dialog->lbl_inicial_id);
  gtk_table_attach (GTK_TABLE (dialog->table), dialog->lbl_inicial_id, 0, 1, 5, 6,
		  (GtkAttachOptions) (GTK_SHRINK),
		  (GtkAttachOptions) (GTK_SHRINK), 2, 2);
  gtk_label_set_justify (GTK_LABEL (dialog->lbl_inicial_id), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (dialog->lbl_inicial_id), 1, 0.5);

  dialog->inicial_ID_entry = gtk_entry_new ();
  gtk_widget_show (dialog->inicial_ID_entry);
  gtk_table_attach (GTK_TABLE (dialog->table), dialog->inicial_ID_entry, 1, 2, 5, 6,
		  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
		  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);
  gtk_entry_set_activates_default (GTK_ENTRY (dialog->inicial_ID_entry), TRUE) ;

  dialog->lbl_kind_of_fault = gtk_label_new (_("Class of faults:"));
  gtk_widget_show (dialog->lbl_kind_of_fault);
  gtk_table_attach (GTK_TABLE (dialog->table), dialog->lbl_kind_of_fault, 0, 1, 6, 7,
    		  (GtkAttachOptions) (GTK_SHRINK),
    		  (GtkAttachOptions) (GTK_SHRINK), 2, 2);
  gtk_label_set_justify (GTK_LABEL (dialog->lbl_kind_of_fault), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (dialog->lbl_kind_of_fault), 1, 0.5);

  dialog->kind_of_fault_cmb = gtk_combo_box_new_text();
  gtk_widget_show (dialog->kind_of_fault_cmb);
  gtk_table_attach (GTK_TABLE (dialog->table), dialog->kind_of_fault_cmb, 1, 2, 6, 7,
    		  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
    		  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);
  gtk_combo_box_insert_text( GTK_COMBO_BOX( dialog->kind_of_fault_cmb ), 0, "Defect insertion" );
  gtk_combo_box_insert_text( GTK_COMBO_BOX( dialog->kind_of_fault_cmb ), 1, "Clock phases shifts" );

/////////////////////////////////////////////////////////////////////////////////////

  dialog->lbl_initial_shift = gtk_label_new (_("Min. Shift (pi rad):"));
  //gtk_widget_show (dialog->lbl_kind_of_fault);
  gtk_table_attach (GTK_TABLE (dialog->table), dialog->lbl_initial_shift, 0, 1, 7, 8,
    		  (GtkAttachOptions) (GTK_SHRINK),
    		  (GtkAttachOptions) (GTK_SHRINK), 2, 2);
  gtk_label_set_justify (GTK_LABEL (dialog->lbl_initial_shift), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (dialog->lbl_initial_shift), 1, 0.5);

  dialog->initial_shift_entry = gtk_entry_new();
  //gtk_widget_show (dialog->kind_of_fault_cmb);
  gtk_table_attach (GTK_TABLE (dialog->table), dialog->initial_shift_entry, 1, 2, 7, 8,
    		  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
    		  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);
  gtk_entry_set_activates_default (GTK_ENTRY (dialog->initial_shift_entry), TRUE) ;

  dialog->lbl_final_shift = gtk_label_new (_("Max. Shift (pi rad):"));
  //gtk_widget_show (dialog->lbl_kind_of_fault);
  gtk_table_attach (GTK_TABLE (dialog->table), dialog->lbl_final_shift, 0, 1, 8, 9,
    		  (GtkAttachOptions) (GTK_SHRINK),
    		  (GtkAttachOptions) (GTK_SHRINK), 2, 2);
  gtk_label_set_justify (GTK_LABEL (dialog->lbl_final_shift), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (dialog->lbl_final_shift), 1, 0.5);

  dialog->final_shift_entry = gtk_entry_new();
  //gtk_widget_show (dialog->kind_of_fault_cmb);
  gtk_table_attach (GTK_TABLE (dialog->table), dialog->final_shift_entry, 1, 2, 8, 9,
    		  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
    		  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);
  gtk_entry_set_activates_default (GTK_ENTRY (dialog->final_shift_entry), TRUE) ;

  dialog->lbl_p_engine_shifts = gtk_label_new (_("Probability of shifts:"));
  //gtk_widget_show (dialog->lbl_kind_of_fault);
  gtk_table_attach (GTK_TABLE (dialog->table), dialog->lbl_p_engine_shifts, 0, 1, 9, 10,
    		  (GtkAttachOptions) (GTK_SHRINK),
    		  (GtkAttachOptions) (GTK_SHRINK), 2, 2);
  gtk_label_set_justify (GTK_LABEL (dialog->lbl_p_engine_shifts), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (dialog->lbl_p_engine_shifts), 1, 0.5);

  dialog->p_engine_shifts_cmb = gtk_combo_box_new_text();
  //gtk_widget_show (dialog->kind_of_fault_cmb);
  gtk_table_attach (GTK_TABLE (dialog->table), dialog->p_engine_shifts_cmb, 1, 2, 9, 10,
    		  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
    		  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);
  gtk_combo_box_insert_text( GTK_COMBO_BOX( dialog->p_engine_shifts_cmb ), 3, "Sequential" );
  gtk_combo_box_insert_text( GTK_COMBO_BOX( dialog->p_engine_shifts_cmb ), 4, "Random shifts" );
  gtk_combo_box_insert_text( GTK_COMBO_BOX( dialog->p_engine_shifts_cmb ), 5, "Fixed clock zone" );

  dialog->lbl_fixed_phase = gtk_label_new (_("Fixed clock zone:"));
  //gtk_widget_show (dialog->lbl_kind_of_fault);
  gtk_table_attach (GTK_TABLE (dialog->table), dialog->lbl_fixed_phase, 0, 1, 10, 11,
    		  (GtkAttachOptions) (GTK_SHRINK),
    		  (GtkAttachOptions) (GTK_SHRINK), 2, 2);
  gtk_label_set_justify (GTK_LABEL (dialog->lbl_fixed_phase), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (dialog->lbl_fixed_phase), 1, 0.5);

  dialog->fixed_phase_entry = gtk_entry_new();
  //gtk_widget_show (dialog->kind_of_fault_cmb);
  gtk_table_attach (GTK_TABLE (dialog->table), dialog->fixed_phase_entry, 1, 2, 10, 11,
    		  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
    		  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);
  gtk_entry_set_activates_default (GTK_ENTRY (dialog->fixed_phase_entry), TRUE) ;

  dialog->lbl_frequency_shift = gtk_label_new (_("Frequency shift:"));
    gtk_table_attach (GTK_TABLE (dialog->table), dialog->lbl_frequency_shift, 0, 1, 11, 12,
      		  (GtkAttachOptions) (GTK_SHRINK),
      		  (GtkAttachOptions) (GTK_SHRINK), 2, 2);
    gtk_label_set_justify (GTK_LABEL (dialog->lbl_frequency_shift), GTK_JUSTIFY_LEFT);
    gtk_misc_set_alignment (GTK_MISC (dialog->lbl_frequency_shift), 1, 0.5);

    dialog->frequency_shift_entry = gtk_entry_new();
    //gtk_widget_show (dialog->kind_of_fault_cmb);
    gtk_table_attach (GTK_TABLE (dialog->table), dialog->frequency_shift_entry, 1, 2, 11, 12,
      		  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
      		  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);
    gtk_entry_set_activates_default (GTK_ENTRY (dialog->fixed_phase_entry), TRUE) ;


////////////////////////////////////////////////////////////////////////////////////

  dialog->lbl_prob_engine = gtk_label_new (_("Probability engine:"));
  //gtk_widget_show (dialog->lbl_prob_engine);
  gtk_table_attach (GTK_TABLE (dialog->table), dialog->lbl_prob_engine, 0, 1, 7, 8,
  		  (GtkAttachOptions) (GTK_SHRINK),
  		  (GtkAttachOptions) (GTK_SHRINK), 2, 2);
  gtk_label_set_justify (GTK_LABEL (dialog->lbl_prob_engine), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (dialog->lbl_prob_engine), 1, 0.5);

  dialog->combo = gtk_combo_box_new_text();
  //gtk_widget_show (dialog->combo);
  gtk_table_attach (GTK_TABLE (dialog->table), dialog->combo, 1, 2, 7, 8,
  		  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
  		  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);
  gtk_combo_box_insert_text( GTK_COMBO_BOX( dialog->combo ), 0, "Assignable" );
  gtk_combo_box_insert_text( GTK_COMBO_BOX( dialog->combo ), 1, "Uniform" );
  gtk_combo_box_insert_text( GTK_COMBO_BOX( dialog->combo ), 2, "Step" );

  dialog->lbl_fault_class = gtk_label_new (_("Faults to be simulated:"));
  //gtk_widget_show (dialog->lbl_fault_class);
  gtk_table_attach (GTK_TABLE (dialog->table), dialog->lbl_fault_class, 0, 1, 8, 9,
                    (GtkAttachOptions) (GTK_SHRINK),
                    (GtkAttachOptions) (GTK_SHRINK), 2, 2);
  gtk_label_set_justify (GTK_LABEL (dialog->lbl_fault_class), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (dialog->lbl_fault_class), 1, 0.5);

  dialog->chkVacancy = gtk_check_button_new_with_label (_("Vacancy")) ;
  //gtk_widget_show (dialog->chkVacancy) ;
  gtk_table_attach (GTK_TABLE (dialog->table), dialog->chkVacancy, 0, 1, 9, 10,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);

  dialog->probVacancy_entry = gtk_entry_new ();
  //gtk_widget_show (dialog->probVacancy_entry);
  gtk_table_attach (GTK_TABLE (dialog->table), dialog->probVacancy_entry, 1, 2, 9, 10,
		  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
		  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);
  gtk_entry_set_activates_default (GTK_ENTRY (dialog->probVacancy_entry), TRUE) ;

  dialog->chkInterstitial = gtk_check_button_new_with_label (_("Interstitial")) ;
  //gtk_widget_show (dialog->chkInterstitial) ;
  gtk_table_attach (GTK_TABLE (dialog->table), dialog->chkInterstitial, 0, 1, 10, 11,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);

  dialog->probInterstitial_entry = gtk_entry_new ();
  //gtk_widget_show (dialog->probInterstitial_entry);
  gtk_table_attach (GTK_TABLE (dialog->table), dialog->probInterstitial_entry, 1, 2, 10, 11,
		  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
		  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);
  gtk_entry_set_activates_default (GTK_ENTRY (dialog->probInterstitial_entry), TRUE) ;

  dialog->chkDopant = gtk_check_button_new_with_label (_("Dopant")) ;
  //gtk_widget_show (dialog->chkDopant) ;
  gtk_table_attach (GTK_TABLE (dialog->table), dialog->chkDopant, 0, 1, 11, 12,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);

  dialog->probDopant_entry = gtk_entry_new ();
  //gtk_widget_show (dialog->probDopant_entry);
  gtk_table_attach (GTK_TABLE (dialog->table), dialog->probDopant_entry, 1, 2, 11, 12,
		  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
		  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);
  gtk_entry_set_activates_default (GTK_ENTRY (dialog->probDopant_entry), TRUE) ;

  dialog->chkDislocation = gtk_check_button_new_with_label (_("Dislocation")) ;
  //gtk_widget_show (dialog->chkDislocation) ;
  gtk_table_attach (GTK_TABLE (dialog->table), dialog->chkDislocation, 0, 1, 12, 13,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);

  dialog->probDislocation_entry = gtk_entry_new ();
  //gtk_widget_show (dialog->probDislocation_entry);
  gtk_table_attach (GTK_TABLE (dialog->table), dialog->probDislocation_entry, 1, 2, 12, 13,
		  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
		  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);
  gtk_entry_set_activates_default (GTK_ENTRY (dialog->probDislocation_entry), TRUE) ;

  gtk_dialog_add_button (GTK_DIALOG (dialog->sim_fault_setup_dialog), GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL) ;
  gtk_dialog_add_button (GTK_DIALOG (dialog->sim_fault_setup_dialog), GTK_STOCK_OK, GTK_RESPONSE_OK) ;
  gtk_dialog_set_default_response (GTK_DIALOG (dialog->sim_fault_setup_dialog), GTK_RESPONSE_OK) ;

  g_signal_connect (G_OBJECT (dialog->kind_of_fault_cmb),  "changed", (GCallback)(kind_of_fault_cmb_changed), dialog);
  g_signal_connect (G_OBJECT (dialog->p_engine_shifts_cmb),  "changed", (GCallback)(p_engine_shifts_cmb_changed), dialog);
  g_signal_connect (G_OBJECT (dialog->combo),  "changed", (GCallback)(combo_changed), dialog);

 }

//static void combo_changed(GtkComboBox *combo, gpointer user_data)
static void kind_of_fault_cmb_changed(GtkComboBox *kind_of_fault_cmb, sim_fault_setup_dialog_D *dialog, gpointer user_data)
{

    int kind_of_fault = (int)gtk_combo_box_get_active (GTK_COMBO_BOX (kind_of_fault_cmb)) ;
    //int p_engine=pbo->p_engine;

    if (kind_of_fault==0) //Defect insertion
	{
	gtk_widget_show (GTK_WIDGET(dialog->chkDislocation)) ;
	gtk_widget_show (GTK_WIDGET(dialog->chkDopant)) ;
	gtk_widget_show (GTK_WIDGET(dialog->chkInterstitial)) ;
	gtk_widget_show (GTK_WIDGET(dialog->chkVacancy)) ;
	gtk_widget_show (GTK_WIDGET(dialog->combo)) ;
	gtk_widget_show (GTK_WIDGET(dialog->lbl_fault_class)) ;
	gtk_widget_show (GTK_WIDGET(dialog->lbl_prob_engine)) ;

	if ((int)gtk_combo_box_get_active (GTK_COMBO_BOX (dialog->combo))==0)
	    {
	gtk_widget_show (GTK_WIDGET(dialog->probDislocation_entry)) ;
	gtk_widget_show (GTK_WIDGET(dialog->probDopant_entry)) ;
	gtk_widget_show (GTK_WIDGET(dialog->probInterstitial_entry)) ;
	gtk_widget_show (GTK_WIDGET(dialog->probVacancy_entry)) ;
	    }

	gtk_widget_hide (GTK_WIDGET(dialog->initial_shift_entry)) ;
	gtk_widget_hide (GTK_WIDGET(dialog->final_shift_entry)) ;
	gtk_widget_hide (GTK_WIDGET(dialog->lbl_initial_shift)) ;
	gtk_widget_hide (GTK_WIDGET(dialog->lbl_final_shift)) ;
	gtk_widget_hide (GTK_WIDGET(dialog->p_engine_shifts_cmb)) ;
	gtk_widget_hide (GTK_WIDGET(dialog->lbl_p_engine_shifts)) ;
	gtk_widget_hide (GTK_WIDGET(dialog->fixed_phase_entry)) ;
	gtk_widget_hide (GTK_WIDGET(dialog->lbl_fixed_phase)) ;
	gtk_widget_hide (GTK_WIDGET(dialog->lbl_frequency_shift)) ;
	gtk_widget_hide (GTK_WIDGET(dialog->frequency_shift_entry)) ;

	}
    if (kind_of_fault==1) //Phase shifts
	{
	gtk_widget_show (GTK_WIDGET(dialog->initial_shift_entry)) ;
	gtk_widget_show (GTK_WIDGET(dialog->final_shift_entry)) ;
	gtk_widget_show (GTK_WIDGET(dialog->lbl_initial_shift)) ;
	gtk_widget_show (GTK_WIDGET(dialog->lbl_final_shift)) ;
	gtk_widget_show (GTK_WIDGET(dialog->p_engine_shifts_cmb)) ;
	gtk_widget_show (GTK_WIDGET(dialog->lbl_p_engine_shifts)) ;
	gtk_widget_show (GTK_WIDGET(dialog->lbl_frequency_shift)) ;
	gtk_widget_show (GTK_WIDGET(dialog->frequency_shift_entry)) ;

	if ((int)gtk_combo_box_get_active (GTK_COMBO_BOX (dialog->p_engine_shifts_cmb))==2)
	{
	gtk_widget_show (GTK_WIDGET(dialog->fixed_phase_entry)) ;
	gtk_widget_show (GTK_WIDGET(dialog->lbl_fixed_phase)) ;
	}

	gtk_widget_hide (GTK_WIDGET(dialog->chkDislocation)) ;
	gtk_widget_hide (GTK_WIDGET(dialog->chkDopant)) ;
	gtk_widget_hide (GTK_WIDGET(dialog->chkInterstitial)) ;
	gtk_widget_hide (GTK_WIDGET(dialog->chkVacancy)) ;
	gtk_widget_hide (GTK_WIDGET(dialog->combo)) ;
	gtk_widget_hide (GTK_WIDGET(dialog->lbl_fault_class)) ;
	gtk_widget_hide (GTK_WIDGET(dialog->lbl_prob_engine)) ;
	gtk_widget_hide (GTK_WIDGET(dialog->probDislocation_entry)) ;
	gtk_widget_hide (GTK_WIDGET(dialog->probDopant_entry)) ;
	gtk_widget_hide (GTK_WIDGET(dialog->probInterstitial_entry)) ;
	gtk_widget_hide (GTK_WIDGET(dialog->probVacancy_entry)) ;

	//gtk_widget_hide (GTK_WIDGET(dialog->lbl_fixed_phase)) ;
	//gtk_widget_hide (GTK_WIDGET(dialog->fixed_phase_entry)) ;

	}

}

//static void combo_changed(GtkComboBox *combo, gpointer user_data)
static void p_engine_shifts_cmb_changed(GtkComboBox *p_engine_shifts_cmb, sim_fault_setup_dialog_D *dialog, gpointer user_data)
{

    int p_engine_shifts = (int)gtk_combo_box_get_active (GTK_COMBO_BOX (p_engine_shifts_cmb)) ;

    if (p_engine_shifts==0) //All possibilities
	{
	gtk_widget_hide (GTK_WIDGET(dialog->lbl_fixed_phase)) ;
	gtk_widget_hide (GTK_WIDGET(dialog->fixed_phase_entry)) ;
	}
    if (p_engine_shifts==1) //Random
	{
	gtk_widget_hide (GTK_WIDGET(dialog->lbl_fixed_phase)) ;
	gtk_widget_hide (GTK_WIDGET(dialog->fixed_phase_entry)) ;
	}
    if (p_engine_shifts==2) //Fixed
	{
	gtk_widget_show (GTK_WIDGET(dialog->lbl_fixed_phase)) ;
	gtk_widget_show (GTK_WIDGET(dialog->fixed_phase_entry)) ;
	}

}

//static void combo_changed(GtkComboBox *combo, gpointer user_data)
static void combo_changed(GtkComboBox *combo, sim_fault_setup_dialog_D *dialog, gpointer user_data)
{

    int p_engine = (int)gtk_combo_box_get_active (GTK_COMBO_BOX (combo)) ;
    //int p_engine=pbo->p_engine;

    if (p_engine==0) //Assignable
	{
	//gtk_label_set_text(GTK_WIDGET(dialog->lbl_num_simulations),"Iterations:");
	gtk_widget_show (GTK_WIDGET(dialog->probDislocation_entry)) ;
	gtk_widget_show (GTK_WIDGET(dialog->probDopant_entry)) ;
	gtk_widget_show (GTK_WIDGET(dialog->probInterstitial_entry)) ;
	gtk_widget_show (GTK_WIDGET(dialog->probVacancy_entry)) ;
	}
    if (p_engine==1) //Uniform
	{
	//gtk_label_set_text(GTK_WIDGET(dialog->lbl_num_simulations),"Iterations:");
	gtk_widget_hide (GTK_WIDGET(dialog->probDislocation_entry)) ;
	gtk_widget_hide (GTK_WIDGET(dialog->probDopant_entry)) ;
	gtk_widget_hide (GTK_WIDGET(dialog->probInterstitial_entry)) ;
	gtk_widget_hide (GTK_WIDGET(dialog->probVacancy_entry)) ;
	}
    if (p_engine==2) //Step
	{
	//gtk_label_set_text(GTK_WIDGET(dialog->lbl_num_simulations),"Iterations /cell:");
	gtk_widget_hide (GTK_WIDGET(dialog->probDislocation_entry)) ;
	gtk_widget_hide (GTK_WIDGET(dialog->probDopant_entry)) ;
	gtk_widget_hide (GTK_WIDGET(dialog->probInterstitial_entry)) ;
	gtk_widget_hide (GTK_WIDGET(dialog->probVacancy_entry)) ;
	}

}

