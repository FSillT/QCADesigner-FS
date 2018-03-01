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
// Header file for the fault simulation.                //
//                                                      //
//                                                      //
//                                                      //
//////////////////////////////////////////////////////////

#ifndef _FAULT_SIMULATION_H_
#define _FAULT_SIMULATION_H_

#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <assert.h>
#include "callback_helpers.h"

typedef struct
  {
	gint out_clear;
	gint *cells_w_dopant_fault;
	gint *cells_w_interstitial_fault;
	gint *cells_w_vacancy_fault;
	gint *cells_w_dislocation_fault;
	gint *total_faults_by_cell;
	gint total_dopant_faults;
	gint total_interstitial_faults;
	gint total_vacancy_faults;
	gint total_dislocation_faults;
   } output_description_OP;

typedef struct
     {
   	int total_interval_clockzone[12][4];
   	int faulty_interval_clockzone[12][4];
   	int number_of_shifts[4][2];
      } output_description_shift_sim_OP;

typedef struct
  {
	//-- general options --//
	unsigned long int number_of_simulations;
	unsigned long int inicial_ID;
	double high_th;
	double low_th;
	double sample_interval;
	double latency;
	int kind_of_fault;

	//------ defects ------ //
	double probvacancy;
	double probinterstitial;
	double probdopant;
	double probdislocation;
	gboolean vacancy;
	gboolean interstitial;
	gboolean dopant;
	gboolean dislocation;
	int p_engine;

	//-----phase shifts-----//
	double initial_shift;
	double final_shift;
	unsigned int fixed_clock_phase;
	int p_engine_shifts;
	double frequency_shift;

  } fault_simulation_OP;

  typedef struct
    {
        gchar *path_results_file;
        gchar *path_design_file;

    } analysis_result_fault_simulation_OP;

   typedef struct
      {
    	gint total_samples ;
    	gint level_errors ;
    	gint clear_simulations ;
      } analysis_fault_simulation_OP;

#endif /* _FAULT_SIMULATION_H_ */

 void start_clock_phase_shifts_simulation(project_OP project_options, char *pszFName, VectorTable *pvt, int ns, int countsym);
 void start_fault_simulation_ud(project_OP project_options, char *pszFName, VectorTable *pvt, int n, int totalcell, int countsym);
 void start_fault_simulation_uniform(project_OP project_options, char *pszFName, VectorTable *pvt, int n, int totalcell, int countsym);
 void start_fault_simulation_step(project_OP project_options, char *pszFName, VectorTable *pvt, int ns, int ncell, int totalcell, int countsym);
 void fill_out_the_result_file_defects(char *pszFName, int countsym, int totalcell, int *reg_cell, int *reg_fault_code, fault_simulation_OP fault_simulation_options );
 void fill_out_the_result_ph_shifts(char *pszFName, int countsym, double phase_shift, fault_simulation_OP fault_simulation_options );
 void error_detection(FILE *fp, char *filename, SIMULATION_OUTPUT *sim_output_ref);
float synthetize_and_create_hm(gchar *resultFName, gchar *designFName, gchar *configFName, int number_of_simulations);
float synthetize_phase_shifts_sim_results(gchar *resultFName, gchar *configFName, int number_of_simulations);
