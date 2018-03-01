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
// The fault simulation.                                //
//                                                      //
// Dayane Alfenas Reis                                  //
//                                                      //
//////////////////////////////////////////////////////////

#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <string.h>

#ifdef GTK_GUI
  #include "support.h"
#else
  #define _(s) s
#endif /* def GTK_GUI */
#include "objects/QCADCell.h"
#include "custom_widgets.h"
#include "global_consts.h"
#include "fault_simulation.h"
#include "callback_helpers.h"
#include "design.h"
#include "callbacks.h"
#include "fileio.h"
#include "fileio_helpers.h"

//!Options for the fault simulation engine
extern analysis_fault_simulation_OP analysis_summary = {0, 0, 0};
extern analysis_result_fault_simulation_OP analysis_result = {NULL, NULL, 0, 0, 0, NULL, 0, NULL};
//This variable is used by multiple source files
extern fault_simulation_OP fault_simulation_options = {1, 1, 0.8, -0.8, 0.1, 0, 1, 0.01, 0.01, 0.01, 0.01, TRUE, TRUE, TRUE, TRUE, 0, 0, 1, 0, 0, 0, 0 } ;
extern double phase_shift[4];
extern project_OP project_options;

//#define REDUCE_DEREF
#define ID_SAMPLES_PER_LINE 10
#define HIGH_HIGH_IMPORTANT 0.99
#define HIGH_IMPORTANT 0.75
#define IMPORTANT 0.5
#define LOW_IMPORTANT 0.25
#define LOW_LOW_IMPORTANT 0.01

#define max(A,B) ((A>B) ? (A):(B))

int count_of_faults, n_errors, *reg_cell=NULL, *reg_fault_code=NULL;

void start_fault_simulation_ud(project_OP project_options, char *pszFName, VectorTable *pvt, int n, int totalcell, int countsym)
{
	int volatile nrand, icell, idot, ilastDot;
	float fdesloc;
	GList *id_cell = NULL, *id_cell2 = NULL;
	SIMULATION_OUTPUT sim_output;
	char *psz = NULL ;
	DESIGN *modified_design= NULL;
	EXP_ARRAY *ar = NULL ;
	QCADDesignObject *obj = NULL ;
	FILE *fp = NULL ;


	modified_design=design_copy(project_options.design); //make a copy from the current design, so the faults can be simulated.

    count_of_faults=0;

   reg_cell = malloc((sizeof(int)*totalcell*4));
   reg_fault_code = malloc((sizeof(int)*totalcell*4));

		    //insert faults in the design according to the probability
	    	icell=0;
		for (id_cell = modified_design->lstLayers; id_cell!=NULL; id_cell=id_cell->next)
		    {
		    for (id_cell2=QCAD_LAYER(id_cell->data)->lstObjs; id_cell2!=NULL; id_cell2=id_cell2->next)

		     {
		     if (QCAD_IS_CELL(id_cell2->data)==TRUE)
		        {
		    	icell=icell+1;

		    	//Dopant fault: takes out one point which is chosen randomly
		    	if (fault_simulation_options.dopant==TRUE)
		    	{

		    		nrand=(rand()%(int)(1/fault_simulation_options.probdopant)); //Chooses a random number in order to know if the fault will be inserted into a cell (in fact)

		    		if (nrand==0) //The number that has been chosen is equal to zero - means that the fault will be inserted.
		    		{


		    			ilastDot=QCAD_CELL(id_cell2->data)->number_of_dots-1; //This is the ID from the last dot in a cell.
		    			idot=(rand()%3); //Chooses randomly a point to be taken out

		    			//Swap values between last dot in a cell (which will be taken out) and the dot that was chosen randomly
		    			QCAD_CELL(id_cell2->data)->cell_dots[idot].charge=QCAD_CELL(id_cell2->data)->cell_dots[ilastDot].charge;
		    			QCAD_CELL(id_cell2->data)->cell_dots[idot].diameter=QCAD_CELL(id_cell2->data)->cell_dots[ilastDot].diameter;
		    			QCAD_CELL(id_cell2->data)->cell_dots[idot].potential=QCAD_CELL(id_cell2->data)->cell_dots[ilastDot].potential;
		    			QCAD_CELL(id_cell2->data)->cell_dots[idot].spin=QCAD_CELL(id_cell2->data)->cell_dots[ilastDot].spin;
		    			QCAD_CELL(id_cell2->data)->cell_dots[idot].x=QCAD_CELL(id_cell2->data)->cell_dots[ilastDot].x;
		    			QCAD_CELL(id_cell2->data)->cell_dots[idot].y=QCAD_CELL(id_cell2->data)->cell_dots[ilastDot].y;

		    			(QCAD_CELL(id_cell2->data)->number_of_dots--); //Takes out a point

		    			//Registro da falha
		    			reg_cell[count_of_faults]=icell;
		    			reg_fault_code[count_of_faults]=3;

		    			count_of_faults++;



		    		}

		    	}

		    	//Interstitial fault: Shifts a cell
		    	if (fault_simulation_options.interstitial==TRUE)
		    	{

		    		nrand=(rand()%(int)(1/fault_simulation_options.probinterstitial)); //Chooses a random number in order to know if the fault will be inserted into a cell (in fact)

		    		if (nrand==0) //The number that has been chosen is equal to zero - means that the fault will be inserted.
		    		{

		    		     //A shift is calculated for this cell
		    		     fdesloc= -1.5*+3.0*((float)rand()/RAND_MAX);

		    		     //The entire cell is shifted (all of its dots)
		    		     QCAD_CELL(id_cell2->data)->cell_dots[0].x=QCAD_CELL(id_cell2->data)->cell_dots[0].x+fdesloc;
		    		     QCAD_CELL(id_cell2->data)->cell_dots[0].y=QCAD_CELL(id_cell2->data)->cell_dots[0].y+fdesloc;
		    		     QCAD_CELL(id_cell2->data)->cell_dots[1].x=QCAD_CELL(id_cell2->data)->cell_dots[1].x+fdesloc;
		    		     QCAD_CELL(id_cell2->data)->cell_dots[1].y=QCAD_CELL(id_cell2->data)->cell_dots[1].y+fdesloc;
		    		     QCAD_CELL(id_cell2->data)->cell_dots[2].x=QCAD_CELL(id_cell2->data)->cell_dots[2].x+fdesloc;
		    		     QCAD_CELL(id_cell2->data)->cell_dots[2].y=QCAD_CELL(id_cell2->data)->cell_dots[2].y+fdesloc;
		    		     QCAD_CELL(id_cell2->data)->cell_dots[3].x=QCAD_CELL(id_cell2->data)->cell_dots[3].x+fdesloc;
		    		     QCAD_CELL(id_cell2->data)->cell_dots[3].y=QCAD_CELL(id_cell2->data)->cell_dots[3].y+fdesloc;
		    		     //and also its bounding box
		    		     QCAD_CELL(id_cell2->data)->parent_instance.bounding_box.xWorld=QCAD_CELL(id_cell2->data)->parent_instance.bounding_box.xWorld+fdesloc;
		    		     QCAD_CELL(id_cell2->data)->parent_instance.bounding_box.yWorld=QCAD_CELL(id_cell2->data)->parent_instance.bounding_box.yWorld+fdesloc;

		    		     //Registro da falha
		    		     reg_cell[count_of_faults]=icell;
		    		     reg_fault_code[count_of_faults]=2;

		    		     count_of_faults++;

		    		    }
		    	}

		    	//Dislocation fault: Rotates a cell
		    	if (fault_simulation_options.dislocation==TRUE)
		    	{

		    		nrand=(rand()%(int)(1/fault_simulation_options.probdislocation)); //Chooses a random number in order to know if the fault will be inserted into a cell (in fact)

		    		if (nrand==0) //The number that has been chosen is equal to zero - means that the fault will be inserted.
		    		{
		    		//A rotation angle is calculated for this cell
		    		fdesloc= 2*PI*((float)rand()/RAND_MAX);
		    		//The dots of this cell is rotated
		    		qcad_cell_rotate_dots (QCAD_CELL(id_cell2->data), fdesloc);

		    		//Registro da falha
		    		reg_cell[count_of_faults]=icell;
		    		reg_fault_code[count_of_faults]=4;

		    		count_of_faults++;

		    		}

		    	}

		    	//Vacancy fault: Takes the cell out of the design - excluding the output cell
		    	if ((fault_simulation_options.vacancy==TRUE) && (QCAD_CELL(id_cell2->data)->cell_function!=QCAD_CELL_OUTPUT))
		    	{

		    		nrand=(rand()%(int)(1/fault_simulation_options.probvacancy)); //Chooses a random number in order to know if the fault will be inserted into a cell (in fact)

		    		if (nrand==0) //The number that has been chosen is equal to zero - means that the fault will be inserted.
		    		{


		    		    //Will it be necessary?
		    		    if (QCAD_CELL(id_cell2->data)->cell_function==QCAD_CELL_FIXED)
		    			qcad_cell_set_function (QCAD_CELL(id_cell2->data), QCAD_CELL_NORMAL);


		    		    QCAD_CELL(id_cell2->data)->number_of_dots=0; //Takes out ALL dots from the cell

		    		   //Registro da falha
		    		   reg_cell[count_of_faults]=icell;
		    		   reg_fault_code[count_of_faults]=1;

		    		   count_of_faults++;


		    		    }

		    	}


		    }
		  }
		  }
		     //Save a file for the design where the faults was inserted - just for checking
		     // create_file (g_strdup_printf ("%s%s%d", _(pszFName), "_alterado", n), modified_design);
		     //Run the simulation
		      project_options.sim_data = run_simulation (project_options.SIMULATION_ENGINE, project_options.SIMULATION_TYPE, project_options.SIMULATION_MODE, modified_design, pvt);

	    	  if (NULL != project_options.sim_data)
	    	  {
	    	  sim_output.sim_data = project_options.sim_data;
	    	  sim_output.bus_layout = modified_design->bus_layout;
	    	  sim_output.bFakeIOLists=FALSE ;
	    	  }

	    	  //Gives a name to the file
	    	  psz = g_strdup_printf ("%s%d", _(pszFName), n) ;

	    	  //Create an output file for the design
	    	   if (NULL == (fp = fopen (psz, "w")))
	    	   return ;
	    	    else
	    	    create_simulation_output_file_fp (fp, &sim_output) ;

	    	    fclose (fp) ;

	    	    //create_file (g_strdup_printf ("%s%d%s", _(pszFName), n, "_design"), modified_design) ;

	    	    reg_cell[count_of_faults]=NULL;

	    	    fill_out_the_result_file_defects(pszFName, n, totalcell, reg_cell, reg_fault_code, fault_simulation_options);


}

void start_fault_simulation_uniform(project_OP project_options, char *pszFName, VectorTable *pvt, int n, int totalcell, int countsym)
{
	int nrand, icell, idot, ilastDot, size;
	float fdesloc;
	GList *id_cell = NULL, *id_cell2 = NULL;
	SIMULATION_OUTPUT sim_output;
	char *psz = NULL ;
	DESIGN *modified_design= NULL;
	EXP_ARRAY *ar = NULL ;
	QCADDesignObject *obj = NULL ;
	FILE *fp = NULL ;


	modified_design=design_copy(project_options.design); //make a copy from the current design, so the faults can be simulated.

	count_of_faults=0;

        reg_cell = (int *) malloc(sizeof(int)*totalcell*4);
        reg_fault_code = (int *) malloc(sizeof(int)*totalcell*4);

		    //insert faults in the design according to the probability
	    	icell=0;
	    	for (id_cell = modified_design->lstLayers; id_cell!=NULL; id_cell=id_cell->next)
	    	{
	    	for (id_cell2=QCAD_LAYER(id_cell->data)->lstObjs; id_cell2!=NULL; id_cell2=id_cell2->next)

	    	{
	    	if (QCAD_IS_CELL(id_cell2->data)==TRUE)
	    	{

		    	icell=icell+1;

		    	//Dopant fault: takes out one point which is chosen randomly
		    	if (fault_simulation_options.dopant==TRUE)
		    	{
		    		nrand=1+(rand()%totalcell); //Chooses a random number in order to know if the fault will be inserted into a cell (in fact)

		    		if (icell==nrand) //The number that has been chosen is equal to the cell's id. It means that the fault will be inserted.
		    		{


		    			ilastDot=QCAD_CELL(id_cell2->data)->number_of_dots-1; //This is the ID from the last dot in a cell.
		    			idot=(rand()%3); //Chooses randomly a point to be taken out

		    			//Swap values between last dot in a cell (which will be taken out) and the dot that was chosen randomly
		    			QCAD_CELL(id_cell2->data)->cell_dots[idot].charge=QCAD_CELL(id_cell2->data)->cell_dots[ilastDot].charge;
		    			QCAD_CELL(id_cell2->data)->cell_dots[idot].diameter=QCAD_CELL(id_cell2->data)->cell_dots[ilastDot].diameter;
		    			QCAD_CELL(id_cell2->data)->cell_dots[idot].potential=QCAD_CELL(id_cell2->data)->cell_dots[ilastDot].potential;
		    			QCAD_CELL(id_cell2->data)->cell_dots[idot].spin=QCAD_CELL(id_cell2->data)->cell_dots[ilastDot].spin;
		    			QCAD_CELL(id_cell2->data)->cell_dots[idot].x=QCAD_CELL(id_cell2->data)->cell_dots[ilastDot].x;
		    			QCAD_CELL(id_cell2->data)->cell_dots[idot].y=QCAD_CELL(id_cell2->data)->cell_dots[ilastDot].y;

		    			(QCAD_CELL(id_cell2->data)->number_of_dots--); //Takes out a point

		    			//Registro da falha
		    			reg_cell[count_of_faults]=icell;
		    			reg_fault_code[count_of_faults]=3;

		    			count_of_faults++;


		    		}

		    	}

		    	//Interstitial fault: Shifts a cell
		    	if (fault_simulation_options.interstitial==TRUE)
		    	{

		    		 nrand=1+(rand()%totalcell); //Chooses a random number in order to know if the fault will be inserted into a cell (in fact)

		    		 if (icell==nrand) //The number that has been chosen is equal to the cell's id. It means that the fault will be inserted.
		    		    {

		    		     //A shift is calculated for this cell
		    		     fdesloc= -1.5*+3.0*((float)rand()/RAND_MAX);

		    		     //The entire cell is shifted (all of its dots)
		    		     QCAD_CELL(id_cell2->data)->cell_dots[0].x=QCAD_CELL(id_cell2->data)->cell_dots[0].x+fdesloc;
		    		     QCAD_CELL(id_cell2->data)->cell_dots[0].y=QCAD_CELL(id_cell2->data)->cell_dots[0].y+fdesloc;
		    		     QCAD_CELL(id_cell2->data)->cell_dots[1].x=QCAD_CELL(id_cell2->data)->cell_dots[1].x+fdesloc;
		    		     QCAD_CELL(id_cell2->data)->cell_dots[1].y=QCAD_CELL(id_cell2->data)->cell_dots[1].y+fdesloc;
		    		     QCAD_CELL(id_cell2->data)->cell_dots[2].x=QCAD_CELL(id_cell2->data)->cell_dots[2].x+fdesloc;
		    		     QCAD_CELL(id_cell2->data)->cell_dots[2].y=QCAD_CELL(id_cell2->data)->cell_dots[2].y+fdesloc;
		    		     QCAD_CELL(id_cell2->data)->cell_dots[3].x=QCAD_CELL(id_cell2->data)->cell_dots[3].x+fdesloc;
		    		     QCAD_CELL(id_cell2->data)->cell_dots[3].y=QCAD_CELL(id_cell2->data)->cell_dots[3].y+fdesloc;
		    		     //and also its bounding box
		    		     QCAD_CELL(id_cell2->data)->parent_instance.bounding_box.xWorld=QCAD_CELL(id_cell2->data)->parent_instance.bounding_box.xWorld+fdesloc;
		    		     QCAD_CELL(id_cell2->data)->parent_instance.bounding_box.yWorld=QCAD_CELL(id_cell2->data)->parent_instance.bounding_box.yWorld+fdesloc;

		    			//Registro da falha
		    			reg_cell[count_of_faults]=icell;
		    			reg_fault_code[count_of_faults]=2;

		    			count_of_faults++;


		    		    }

		    	}

		    	//Dislocation fault: Rotates a cell
		    	if (fault_simulation_options.dislocation==TRUE)
		    	{

		    		nrand=1+(rand()%totalcell); //Chooses a random number in order to know if the fault will be inserted into a cell (in fact)

		    		if (icell==nrand) //The number that has been chosen is equal to the cell's id. It means that the fault will be inserted.
		    		{
		    		//A rotation angle is calculated for this cell
		    		fdesloc= 2*PI*((float)rand()/RAND_MAX);
		    		//The dots of this cell is rotated
		    		qcad_cell_rotate_dots (QCAD_CELL(id_cell2->data), fdesloc);

	    			//Registro da falha
	    			reg_cell[count_of_faults]=icell;
	    			reg_fault_code[count_of_faults]=4;

	    			count_of_faults++;


		    		}

		    	}

		    	//Vacancy fault: Takes the cell out of the design
		    	if ((fault_simulation_options.vacancy==TRUE) && (QCAD_CELL(id_cell2->data)->cell_function!=QCAD_CELL_OUTPUT))
		    	{

		    		nrand=1+(rand()%totalcell); //Chooses a random number in order to know if the fault will be inserted into a cell (in fact)

		    		if (icell==nrand) //The number that has been chosen is equal to the cell's id. It means that the fault will be inserted.
		    		    {


		    		    //Will it be necessary?
		    		    if (QCAD_CELL(id_cell2->data)->cell_function==QCAD_CELL_FIXED)
		    			qcad_cell_set_function (QCAD_CELL(id_cell2->data), QCAD_CELL_NORMAL);


		    		    QCAD_CELL(id_cell2->data)->number_of_dots=0; //Takes out ALL dots from the cell

		    		    //Registro da falha
		    		    reg_cell[count_of_faults]=icell;
		    		    reg_fault_code[count_of_faults]=1;

		    		    count_of_faults++;


		    		    }

		    	}

		    }
		  }
		  }
		     //Save a file for the design where the faults was inserted - just for checking
		     // create_file (g_strdup_printf ("%s%s%d", _(pszFName), "_alterado", n), modified_design);
		     //Run the simulation
		      project_options.sim_data = run_simulation (project_options.SIMULATION_ENGINE, project_options.SIMULATION_TYPE, project_options.SIMULATION_MODE, modified_design, pvt);

	    	  if (NULL != project_options.sim_data)
	    	  {
	    	  sim_output.sim_data = project_options.sim_data;
	    	  sim_output.bus_layout = modified_design->bus_layout;
	    	  sim_output.bFakeIOLists=FALSE ;
	    	  }

	    	  //Gives a name to the file
	    	  psz = g_strdup_printf ("%s%d", _(pszFName), n) ;

	    	  //Create an output file for the design
	    	   if (NULL == (fp = fopen (psz, "w")))
	    	   return ;
	    	    else
	    	    create_simulation_output_file_fp (fp, &sim_output) ;

	    	    fclose (fp) ;

	    	    //create_file (g_strdup_printf ("%s%d%s", _(pszFName), n, "_design"), modified_design) ;

	    	    reg_cell[count_of_faults]=NULL;

	    	    fill_out_the_result_file_defects(pszFName, n, totalcell, reg_cell, reg_fault_code, fault_simulation_options);



}

void start_fault_simulation_step(project_OP project_options, char *pszFName, VectorTable *pvt, int ns, int ncell, int totalcell, int countsym)
{

	int nrand, icell, idot, ilastDot, flag, n, count;
	float fdesloc;
	GList *id_cell = NULL, *id_cell2 = NULL;
	SIMULATION_OUTPUT sim_output;
	char *psz = NULL ;
	DESIGN *modified_design= NULL;
	EXP_ARRAY *ar = NULL ;
	QCADDesignObject *obj = NULL ;
	FILE *fp = NULL ;


	//insert faults in the design according to the probability
	modified_design=design_copy(project_options.design); //make a copy from the original design, so the faults can be simulated.

        reg_cell = (int *) malloc(1);
        reg_fault_code = (int *) malloc(1);
        count=1;



	    for (id_cell = modified_design->lstLayers; id_cell!=NULL; id_cell=id_cell->next)
	    {
	    for (id_cell2=QCAD_LAYER(id_cell->data)->lstObjs; id_cell2!=NULL; id_cell2=id_cell2->next)
	    {
	    	if (QCAD_IS_CELL(id_cell2->data)==TRUE)
	    	{
		    if (count==ncell)
		    {

		    flag=0;
		    while (flag==0)
		    {

		    	nrand=1+(rand()%4); //Chooses a random number in order to know which fault will be inserted into a cell

		    	if (nrand==1 && (fault_simulation_options.vacancy==TRUE) ) //Vacancy fault: Takes the cell out of the design
		    	    {


		    		if ((QCAD_CELL(id_cell2->data)->cell_function==QCAD_CELL_OUTPUT) || (QCAD_CELL(id_cell2->data)->cell_function==QCAD_CELL_INPUT))
						{
			    		QCAD_CELL(id_cell2->data)->cell_dots[0].charge=0;
			    		QCAD_CELL(id_cell2->data)->cell_dots[1].charge=0;
			    		QCAD_CELL(id_cell2->data)->cell_dots[2].charge=0;
			    		QCAD_CELL(id_cell2->data)->cell_dots[3].charge=0;
						}


	    		    if (QCAD_CELL(id_cell2->data)->cell_function==QCAD_CELL_FIXED)
			    	qcad_cell_set_function (QCAD_CELL(id_cell2->data), QCAD_CELL_NORMAL);

			        if (QCAD_CELL(id_cell2->data)->number_of_dots!=0)
	    		    QCAD_CELL(id_cell2->data)->number_of_dots=0; //Takes out ALL dots from the cell

			    flag=1;
			    //break;

	    		    //Registro da falha
	    		    reg_cell[0]=ncell;
	    		    reg_fault_code[0]=1;

		    	    }

		    	if (nrand==2 && fault_simulation_options.interstitial==TRUE) //Interstitial fault: Shifts a cell
		    	    {

			    //A shift is calculated for this cell
			    fdesloc= -1.5*+3.0*((float)rand()/RAND_MAX);

			    //The entire cell is shifted (all of its dots)
			    QCAD_CELL(id_cell2->data)->cell_dots[0].x=QCAD_CELL(id_cell2->data)->cell_dots[0].x+fdesloc;
			    QCAD_CELL(id_cell2->data)->cell_dots[0].y=QCAD_CELL(id_cell2->data)->cell_dots[0].y+fdesloc;
			    QCAD_CELL(id_cell2->data)->cell_dots[1].x=QCAD_CELL(id_cell2->data)->cell_dots[1].x+fdesloc;
			    QCAD_CELL(id_cell2->data)->cell_dots[1].y=QCAD_CELL(id_cell2->data)->cell_dots[1].y+fdesloc;
			    QCAD_CELL(id_cell2->data)->cell_dots[2].x=QCAD_CELL(id_cell2->data)->cell_dots[2].x+fdesloc;
			    QCAD_CELL(id_cell2->data)->cell_dots[2].y=QCAD_CELL(id_cell2->data)->cell_dots[2].y+fdesloc;
			    QCAD_CELL(id_cell2->data)->cell_dots[3].x=QCAD_CELL(id_cell2->data)->cell_dots[3].x+fdesloc;
			    QCAD_CELL(id_cell2->data)->cell_dots[3].y=QCAD_CELL(id_cell2->data)->cell_dots[3].y+fdesloc;
			    //and also its bounding box
			    QCAD_CELL(id_cell2->data)->parent_instance.bounding_box.xWorld=QCAD_CELL(id_cell2->data)->parent_instance.bounding_box.xWorld+fdesloc;
			    QCAD_CELL(id_cell2->data)->parent_instance.bounding_box.yWorld=QCAD_CELL(id_cell2->data)->parent_instance.bounding_box.yWorld+fdesloc;

			    flag=1;
			    //break;

	    		    //Registro da falha
	    		    reg_cell[0]=ncell;
	    		    reg_fault_code[0]=2;

		    	    }

		    	if (nrand==3 && fault_simulation_options.dopant==TRUE) //Dopant fault: takes out one point which is chosen randomly
		    	    {

			    ilastDot=QCAD_CELL(id_cell2->data)->number_of_dots-1; //This is the ID from the last dot in a cell.
			    idot=(rand()%3); //Chooses randomly a point to be taken out

			    //Swap values between last dot in a cell (which will be taken out) and the dot that was chosen randomly
			    QCAD_CELL(id_cell2->data)->cell_dots[idot].charge=QCAD_CELL(id_cell2->data)->cell_dots[ilastDot].charge;
			    QCAD_CELL(id_cell2->data)->cell_dots[idot].diameter=QCAD_CELL(id_cell2->data)->cell_dots[ilastDot].diameter;
			    QCAD_CELL(id_cell2->data)->cell_dots[idot].potential=QCAD_CELL(id_cell2->data)->cell_dots[ilastDot].potential;
			    QCAD_CELL(id_cell2->data)->cell_dots[idot].spin=QCAD_CELL(id_cell2->data)->cell_dots[ilastDot].spin;
			    QCAD_CELL(id_cell2->data)->cell_dots[idot].x=QCAD_CELL(id_cell2->data)->cell_dots[ilastDot].x;
			    QCAD_CELL(id_cell2->data)->cell_dots[idot].y=QCAD_CELL(id_cell2->data)->cell_dots[ilastDot].y;

			    (QCAD_CELL(id_cell2->data)->number_of_dots--); //Takes out a point


			    flag=1;
			    //break;

	    		    //Registro da falha
	    		    reg_cell[0]=ncell;
	    		    reg_fault_code[0]=3;

		    	    }

		    	if (nrand==4 && fault_simulation_options.dislocation==TRUE) //Dislocation fault: Rotates a cell
		    	    {
			    //A rotation angle is calculated for this cell
			    fdesloc= 2*PI*((float)rand()/RAND_MAX);
			    //The dots of this cell is rotated
			    qcad_cell_rotate_dots (QCAD_CELL(id_cell2->data), fdesloc);

			    flag=1;
			    //break;

	    		    //Registro da falha
	    		    reg_cell[0]=ncell;
	    		    reg_fault_code[0]=4;

		    	    }
		   }

		  //Run the simulation
		  project_options.sim_data = run_simulation (project_options.SIMULATION_ENGINE, project_options.SIMULATION_TYPE, project_options.SIMULATION_MODE, modified_design, pvt);

	    	  if (NULL != project_options.sim_data)
	    	  {
	    	  sim_output.sim_data = project_options.sim_data;
	    	  sim_output.bus_layout = modified_design->bus_layout;
	    	  sim_output.bFakeIOLists=FALSE ;
	    	  }

	    	  //Gives a name to the file
	    	  psz = g_strdup_printf ("%s%d", _(pszFName), countsym) ;

	    	  //Create an output file for the simulation
	    	  if (NULL == (fp = fopen (psz, "w")))
	    	   return ;
	    	  else
	    	  create_simulation_output_file_fp (fp, &sim_output) ;

	    	  fclose (fp) ;

	    	  //create_file (g_strdup_printf ("%s%d%s", _(pszFName), countsym, "_design"), modified_design) ;

	    	  reg_cell[1]=NULL;

	    	  fill_out_the_result_file_defects(pszFName, countsym, totalcell, reg_cell, reg_fault_code, fault_simulation_options);
	//count++;
	return;
	}
	else
	count++;
    }

    }
    //break;
    }
        g_free(reg_cell);
        g_free(reg_fault_code);

}

void start_clock_phase_shifts_simulation(project_OP project_options, char *pszFName, VectorTable *pvt, int ns, int countsym)
{

	int nrand, icell, idot, ilastDot, flag, n, count;
	float fdesloc;
	GList *id_cell = NULL, *id_cell2 = NULL;
	SIMULATION_OUTPUT sim_output;
	char *psz = NULL ;
	DESIGN *modified_design= NULL;
	EXP_ARRAY *ar = NULL ;
	QCADDesignObject *obj = NULL ;
	FILE *fp = NULL ;


		  //Run the simulation
		  project_options.sim_data = run_simulation (project_options.SIMULATION_ENGINE, project_options.SIMULATION_TYPE, project_options.SIMULATION_MODE, project_options.design, pvt);

	    	  if (NULL != project_options.sim_data)
	    	  {
	    	  sim_output.sim_data = project_options.sim_data;
	    	  sim_output.bus_layout = project_options.design->bus_layout;
	    	  sim_output.bFakeIOLists=FALSE ;
	    	  }

	    	  //Gives a name to the file
	    	  psz = g_strdup_printf ("%s%d", _(pszFName), countsym) ;

	    	  //Create an output file for the simulation
	    	  if (NULL == (fp = fopen (psz, "w")))
	    	   return ;
	    	  else
	    	  create_simulation_output_file_fp (fp, &sim_output) ;

	    	  fclose (fp) ;

	    	  fill_out_the_result_file_ph_shifts(pszFName, countsym, fault_simulation_options);


}


void fill_out_the_result_file_defects(char *pszFName, int countsym, int totalcell, int *reg_cell, int *reg_fault_code, fault_simulation_OP fault_simulation_options )
    {

        gint  m, m_outputs, n_samples, n_inputs, n_outputs, n_traces, i, flag, last_sym;
        char *filename=NULL, *original_filename=NULL, *results_filename=NULL, *config_filename=NULL;
        FILE *fp = NULL, *fp1 = NULL ;
        DESIGN *ref_design = NULL;
        gboolean bRet;
        QCADDesignObject *obj = NULL ;

        SIMULATION_OUTPUT *sim_output_ref=NULL;
        simulation_data *sim_data = NULL ;
        char *pszLine = NULL;

        GtkWidget *msg = NULL ;

        //Abre o arquivo de referencia da simulacao
        //concatena o caminho, o nome base e o indice
        original_filename=g_strdup_printf ("%s%s", _(pszFName), "_original");
        //obtem a saida da simulacao
        sim_output_ref=open_simulation_output_file (original_filename);

        //obtem numero de curvas
        n_traces=sim_output_ref->sim_data->number_of_traces;

        //obtem numero total de simulacoes
        //if (fault_simulation_options.p_engine==2)
            //last_sym=countsym*totalcell;
        //else
            last_sym=countsym;


        if (n_traces== 0)
    	return;

        //conta numero de entradas e saidas
        n_inputs=0;
        n_outputs=0;
        for (m = 0 ; m< n_traces ; m++)
           {
            //identifica as curvas correspondentes as saidas
            if (sim_output_ref->sim_data->trace[m].trace_function == QCAD_CELL_INPUT)
                n_inputs++;
            if (sim_output_ref->sim_data->trace[m].trace_function == QCAD_CELL_OUTPUT)
                n_outputs++;
           }
///
        results_filename=g_strdup_printf ("%s%s", _(pszFName), "_results");
        if (countsym==1)
            {
            if (NULL == (fp = fopen (results_filename, "w")))
            return;
            }
        else
            {
            if (NULL == (fp = fopen (results_filename, "a")))
            return ;
            }
        ///
        results_filename=g_strdup_printf ("%s%s", _(pszFName), "_results");

          //concatena o caminho, o nome base e o indice
          filename=g_strdup_printf ("%s%d", _(pszFName), countsym);

          //chama a funcao que procura as falhas em simulacao

          fprintf (fp, g_strdup_printf ("%s%d%s", "\n[FAULT SIMULATION FILE ", countsym, "]\n")) ;

          error_detection(fp, filename, sim_output_ref);

          fprintf (fp, "[FAULTS x CELLS CROSS REFERENCE]\n") ;

             fprintf (fp, "DOPANT=") ;
             flag=0;
             for (i=0; reg_cell[i]!=NULL; i++)
             {
             if (reg_fault_code[i]==3)
        	 {
        	 fprintf (fp, g_strdup_printf ("%d%s", reg_cell[i], " ")) ;
        	 flag=1;
        	 }
             }
             if (flag==0)
        	fprintf (fp, "NO FAULTS ") ;

             fprintf (fp, "\nINSTERSTITIAL=") ;
             flag=0;
             for (i=0; reg_cell[i]!=NULL; i++)
             {
             if (reg_fault_code[i]==2)
        	 {
        	 fprintf (fp, g_strdup_printf ("%d%s", reg_cell[i], " ")) ;
        	 flag=1;
        	 }
             }
             if (flag==0)
        	fprintf (fp, "NO FAULTS ") ;

             fprintf (fp, "\nDISLOCATION=") ;
             flag=0;
             for (i=0; reg_cell[i]!=NULL; i++)
             {
             if (reg_fault_code[i]==4)
        	 {
        	 fprintf (fp, g_strdup_printf ("%d%s", reg_cell[i], " ")) ;
        	 flag=1;
        	 }
             }
             if (flag==0)
        	 fprintf (fp, "NO FAULTS ") ;

             fprintf (fp, "\nVACANCY=") ;
             flag=0;
             for (i=0; reg_cell[i]!=NULL; i++)
             {
             if (reg_fault_code[i]==1)
        	 {
        	 fprintf (fp, g_strdup_printf ("%d%s", reg_cell[i], " ")) ;
        	 flag=1;
        	 }
             }
             if (flag==0)
        	 fprintf (fp, "NO FAULTS ") ;

          fprintf (fp, "\n[#FAULTS x CELLS CROSS REFERENCE]") ;



          fprintf (fp, g_strdup_printf ("%s%d%s", "\n[#FAULT SIMULATION FILE ", countsym, "]\n")) ;

        //Escreve no arquivo de configuracao
                      config_filename=g_strdup_printf ("%s%s", _(pszFName), "_config");
                      if (NULL == (fp1 = fopen (config_filename, "w+")))
                      return ;
                      else
                      {
                          fprintf (fp1, "\n[FAULT SIMULATION CONFIGURATION]\n") ;

                          fprintf (fp1, g_strdup_printf ("%s%d%s", "DOPANT=", fault_simulation_options.dopant, "\n")) ;
                          fprintf (fp1, g_strdup_printf ("%s%d%s", "INTERSTITIAL=", fault_simulation_options.interstitial, "\n")) ;
                          fprintf (fp1, g_strdup_printf ("%s%d%s", "DISLOCATION=", fault_simulation_options.dislocation, "\n")) ;
                          fprintf (fp1, g_strdup_printf ("%s%d%s", "VACANCY=", fault_simulation_options.vacancy, "\n")) ;

                          if (fault_simulation_options.p_engine == 1)
                          fprintf (fp1, "FAULT PROBALITY ENGINE=UNIFORM\n") ;

                          if (fault_simulation_options.p_engine == 0)
                          {
                          fprintf (fp1, "FAULT PROBALITY ENGINE=USER DEFINED-VALUES BELOW\n") ;
                          fprintf (fp1, g_strdup_printf ("%s%f%s", "DOPANT=", fault_simulation_options.probdopant, "\n")) ;
                          fprintf (fp1, g_strdup_printf ("%s%f%s", "INTERSTITIAL=", fault_simulation_options.probinterstitial, "\n")) ;
                          fprintf (fp1, g_strdup_printf ("%s%f%s", "DISLOCATION=", fault_simulation_options.probdislocation, "\n")) ;
                          fprintf (fp1, g_strdup_printf ("%s%f%s", "VACANCY=", fault_simulation_options.probvacancy, "\n")) ;
                          }

                          if (fault_simulation_options.p_engine == 2)
                          {
                          fprintf (fp1, "FAULT PROBALITY ENGINE=STEP\n") ;
                          }

                           //fprintf (fp, "[FAULT SIMULATION ANALYSIS]\n") ;
                           fprintf (fp1, g_strdup_printf ("%s%s%s", "REFERENCE_FILE=", original_filename, "\n")) ;
                           fprintf (fp1, g_strdup_printf ("%s%d%s", "NUMBER_OF_SIMULATIONS=", last_sym, "\n")) ;
                           fprintf (fp1, g_strdup_printf ("%s%d%s", "TOTAL_CELLS=", totalcell, "\n")) ;

                           fprintf (fp1, g_strdup_printf ("%s%d%s", "INPUTS=", n_inputs, "\n")) ;
                           fprintf (fp1, g_strdup_printf ("%s%d%s", "OUTPUTS=", n_outputs, "\n")) ;

                           fprintf (fp1, "[#FAULT SIMULATION CONFIGURATION]\n") ;

                           fclose(fp1);
                      }

                      fclose (fp) ;
    }

void fill_out_the_result_file_ph_shifts(char *pszFName, int countsym, fault_simulation_OP fault_simulation_options )
    {

        gint  m, m_outputs, n_samples, n_inputs, n_outputs, n_traces, i, flag, last_sym;
        char *filename=NULL, *original_filename=NULL, *results_filename=NULL, *config_filename=NULL;
        FILE *fp = NULL, *fp1 = NULL ;
        DESIGN *ref_design = NULL;
        gboolean bRet;
        QCADDesignObject *obj = NULL ;

        SIMULATION_OUTPUT *sim_output_ref=NULL;
        simulation_data *sim_data = NULL ;
        char *pszLine = NULL;

        GtkWidget *msg = NULL ;

        //Abre o arquivo de referencia da simulacao
        //concatena o caminho, o nome base e o indice
        original_filename=g_strdup_printf ("%s%s", _(pszFName), "_original");
        //obtem a saida da simulacao
        sim_output_ref=open_simulation_output_file (original_filename);

        //obtem numero de curvas
        n_traces=sim_output_ref->sim_data->number_of_traces;

        //obtem numero total de simulacoes
        //if (fault_simulation_options.p_engine_shifts==0)
            //last_sym=fault_simulation_options.number_of_simulations*15;
        //else
            //last_sym=fault_simulation_options.number_of_simulations;
            last_sym=countsym;


        if (n_traces== 0)
    	return;

        //conta numero de entradas e saidas
        n_inputs=0;
        n_outputs=0;
        for (m = 0 ; m< n_traces ; m++)
           {
            //identifica as curvas correspondentes as saidas
            if (sim_output_ref->sim_data->trace[m].trace_function == QCAD_CELL_INPUT)
                n_inputs++;
            if (sim_output_ref->sim_data->trace[m].trace_function == QCAD_CELL_OUTPUT)
                n_outputs++;
           }

        results_filename=g_strdup_printf ("%s%s", _(pszFName), "_results");
        if (countsym==1)
            {
            if (NULL == (fp = fopen (results_filename, "w")))
            return;
            }
        else
            {
            if (NULL == (fp = fopen (results_filename, "a")))
            return ;
            }

              //concatena o caminho, o nome base e o indice
              filename=g_strdup_printf ("%s%d", _(pszFName), countsym);

              //chama a funcao que procura as falhas em simulacao

              fprintf (fp, g_strdup_printf ("%s%d%s", "\n[FAULT SIMULATION FILE ", countsym, "]\n")) ;

              error_detection(fp, filename, sim_output_ref);

              fprintf (fp, "[PHASE SHIFTS]\n") ;

              fprintf (fp, "CLOCK 0  (pi rad)=") ;
              fprintf (fp, g_strdup_printf ("%f%s", (float)phase_shift[0], "\n")) ;

              fprintf (fp, "CLOCK 1  (pi rad)=") ;
              fprintf (fp, g_strdup_printf ("%f%s", (float)phase_shift[1], "\n")) ;

              fprintf (fp, "CLOCK 2  (pi rad)=") ;
              fprintf (fp, g_strdup_printf ("%f%s", (float)phase_shift[2], "\n")) ;

              fprintf (fp, "CLOCK 3  (pi rad)=") ;
              fprintf (fp, g_strdup_printf ("%f%s", (float)phase_shift[3], "\n")) ;

              fprintf (fp, "[#PHASE SHIFTS]\n") ;

              fprintf (fp, g_strdup_printf ("%s%d%s", "[#FAULT SIMULATION FILE ", countsym, "]\n")) ;

              //Escreve no arquivo de configuracao
              config_filename=g_strdup_printf ("%s%s", _(pszFName), "_config");
              if (NULL == (fp1 = fopen (config_filename, "w+")))
              return ;
              else
              {
              fprintf (fp1, "\n[FAULT SIMULATION CONFIGURATION]\n") ;
              fprintf (fp1, "\nMODE=CLOCK PHASE SHIFTS \n") ;

              if (fault_simulation_options.p_engine_shifts == 0)
        	  fprintf (fp1, "FAULT PROBALITY ENGINE=SEQUENTIAL SHIFTS\n") ;
              if (fault_simulation_options.p_engine_shifts == 1)
        	  fprintf (fp1, "FAULT PROBALITY ENGINE=RANDOM SHIFTS\n") ;

              if (fault_simulation_options.p_engine_shifts == 2)
        	  {
        	  fprintf (fp1, "FIXED SHIFTS\n") ;
        	  fprintf (fp1, g_strdup_printf ("%s%f%s", "PHASE=", fault_simulation_options.fixed_clock_phase, "\n")) ;
        	  }
              fprintf (fp1, g_strdup_printf ("%s%f%s", "MIN_SHIFT_(pi_rad)=", (float)fault_simulation_options.initial_shift, "\n")) ;
              fprintf (fp1, g_strdup_printf ("%s%f%s", "MAX_SHIFT_(pi_rad)=", (float)fault_simulation_options.final_shift, "\n")) ;

              fprintf (fp1, g_strdup_printf ("%s%s%s", "REFERENCE_FILE=", original_filename, "\n")) ;
              fprintf (fp1, g_strdup_printf ("%s%d%s", "NUMBER_OF_SIMULATIONS=", last_sym, "\n")) ;

              fprintf (fp1, g_strdup_printf ("%s%d%s", "INPUTS=", n_inputs, "\n")) ;
              fprintf (fp1, g_strdup_printf ("%s%d%s", "OUTPUTS=", n_outputs, "\n")) ;

              fprintf (fp1, "[#FAULT SIMULATION CONFIGURATION]\n") ;

              fclose (fp1) ;
              }

        fclose (fp) ;

    }


void error_detection(FILE *fp, char *filename, SIMULATION_OUTPUT *sim_output_ref)
    {
    SIMULATION_OUTPUT *sim_output=NULL;
    int m, r, n_traces, i_output, ep, count_dif_samples, buffer_samples[3], flag_err;
    int icSamples, Nix, idx, sample_idx, out_clockzone, Nix_inicial, Nix_final, found, flag, pulse_w, count_ltcy;
    int icSamples_ref, n_traces_ref, Nix_ref, found_ref, count_ltcy_ref, dif, dif_OK;

    //obtem a saida da simulacao
    sim_output=open_simulation_output_file (filename);

    i_output=1;
    //obtem numero de curvas simulacao
    icSamples=sim_output->sim_data->number_samples;
    n_traces=sim_output->sim_data->number_of_traces;

    //obtem numero de curvas referencia
    icSamples_ref=sim_output_ref->sim_data->number_samples;
    n_traces_ref=sim_output_ref->sim_data->number_of_traces;

    flag_err=0;


	fprintf (fp, "[SAMPLES WITH LEVEL ERRORS]") ;

	for (m = 0 ; m< n_traces ; m++)
	{
		Nix_inicial=0;
		Nix_final=icSamples - 1;
		found=0;
		count_ltcy=0;
		found_ref=0;
		count_ltcy_ref=0;
		dif_OK=0;


		//identifica as curvas correspondentes as saidas
		if (sim_output->sim_data->trace[m].trace_function == QCAD_CELL_OUTPUT)
		{

		count_dif_samples=0;
		out_clockzone = sim_output->sim_data->trace[m].clockzone;
		n_errors=0;

		flag=0;
		pulse_w=0;

		//medicao da largura do clock da saida
		for (Nix = 0 ; Nix< icSamples - 1; Nix++)
		    {
		     while ((sim_output->sim_data->clock_data[out_clockzone].data[Nix] == sim_output->sim_data->clock_data[out_clockzone].data[Nix+1]) && sim_output->sim_data->clock_data[out_clockzone].data[Nix] < fault_simulation_options.high_th)
			 {
			 pulse_w++;
			 flag=1;
			 Nix++;
			 }
		     if (flag==1)
		   break;
		    }


		fprintf (fp, g_strdup_printf ("%s%d%s", "\n[OUTPUT ", i_output, "]")) ;
		fprintf (fp, "\ndata_labels=%s\n", sim_output->sim_data->trace[m].data_labels) ;

		for (Nix_ref = 0 ; Nix_ref< icSamples_ref - 1; Nix_ref++)
		{
		//borda de subida do clock
		    if ((sim_output_ref->sim_data->clock_data[out_clockzone].data[Nix_ref] < sim_output_ref->sim_data->clock_data[out_clockzone].data[Nix_ref+1]) && found_ref==0)
		    {
			if  (count_ltcy_ref>=(int)fault_simulation_options.latency)
			break;
			else
			count_ltcy_ref++;

			found_ref=1; //acha a borda apenas uma vez

		    }

		   else
		    {
		      if (sim_output_ref->sim_data->clock_data[out_clockzone].data[Nix_ref] >= sim_output_ref->sim_data->clock_data[out_clockzone].data[Nix_ref+1])
		      found_ref=0;
		    }
		}

		for (Nix = 0 ; Nix< icSamples - 1; Nix++)
		{
		//borda de subida do clock
		    if ((sim_output->sim_data->clock_data[out_clockzone].data[Nix] < sim_output->sim_data->clock_data[out_clockzone].data[Nix+1]) && found==0)
		    {
			if  (count_ltcy>=(int)fault_simulation_options.latency)
			{
			Nix_inicial = floor(Nix-pulse_w*fault_simulation_options.sample_interval);
			Nix_final=Nix;

			if (dif_OK==0)
			{
			dif=Nix-Nix_ref;
			dif_OK=1;
			}


			for (Nix = Nix_inicial ; Nix< Nix_final ; Nix += ID_SAMPLES_PER_LINE)
			{

			for (idx = 0 ; idx < ID_SAMPLES_PER_LINE ; idx++)
			    {
			    if ((sample_idx = Nix + idx) <= Nix_final)
				{
				if ((((sim_output->sim_data->trace[m].data[sample_idx] <= fault_simulation_options.low_th) &&
	        		 (sim_output_ref->sim_data->trace[m].data[sample_idx-dif] >= fault_simulation_options.low_th)) ||
	        		((sim_output->sim_data->trace[m].data[sample_idx] >= fault_simulation_options.high_th) &&
	        		 (sim_output_ref->sim_data->trace[m].data[sample_idx-dif] <= fault_simulation_options.high_th))) ||
	        		(((sim_output->sim_data->trace[m].data[sample_idx] >= fault_simulation_options.low_th) &&
	        		 (sim_output_ref->sim_data->trace[m].data[sample_idx-dif] <= fault_simulation_options.low_th)) ||
	        		((sim_output->sim_data->trace[m].data[sample_idx] <= fault_simulation_options.high_th) &&
	        		(sim_output_ref->sim_data->trace[m].data[sample_idx-dif] >= fault_simulation_options.high_th))))

				    {

					fprintf (fp, "%d ", sample_idx) ;
					flag_err=1;
					n_errors++;

				    }
				}
			    }



			}
		        }
			count_ltcy++;
			found=1; //acha a borda apenas uma vez

		    }

		   else
		    {
		      if (sim_output->sim_data->clock_data[out_clockzone].data[Nix] >= sim_output->sim_data->clock_data[out_clockzone].data[Nix+1])
		      found=0;
		    }



		}
		if (n_errors==0)
		fprintf (fp, "NO ERRORS") ;

		fprintf (fp, g_strdup_printf ("%s%d%s", "\n[#OUTPUT ", i_output, "]")) ;

		    i_output++;
		}
	}

	if (flag_err==0)
	{

	fprintf (fp, "\nERROR-FREE SIMULATION") ;
	fprintf (fp, "\n[#SAMPLES WITH LEVEL ERRORS]\n") ;
	}
	else
	{
	fprintf (fp, "\n[#SAMPLES WITH LEVEL ERRORS]\n") ;
	}
    return;
   }

float synthetize_and_create_hm(gchar *resultFName, gchar *designFName, gchar *configFName, int number_of_simulations)
    {

      gint teste;
      FILE *result_file = NULL, *design_file = NULL, *config_file = NULL, *fp = NULL;
      gint number_of_cells, number_of_outputs;
      gint Nix1, Nix2, id, id2, id3, icell;
      char *pszLine = NULL , *pszValue = NULL , *pszItr = NULL ;
      gdouble **heatmap_by_output, *general_heatmap, quocient;
      DESIGN **original_design= NULL, *modified_design= NULL;
      GList *id_cell= NULL, *id_cell2= NULL;
      gint *unclear_simulations;
      gfloat  percent_working;


      //Abre o arquivo de resultados
      if (NULL == (result_file = file_open_and_buffer (resultFName)))
        return 0;

      //Abre o arquivo de configuracoes
      if (NULL == (config_file = file_open_and_buffer (configFName)))
        return 0;

      //Abre o arquivo do design
      if (NULL == (design_file = file_open_and_buffer (designFName)))
        return;

      set_progress_bar_visible (TRUE) ;
      set_progress_bar_label (_("Analyzing results...")) ;
      set_progress_bar_fraction (0.0) ;

      //Extracao do resultado
      //Numero de simulacoes, celulas e outputs
        if (!SkipPast (config_file, '\0', "[FAULT SIMULATION CONFIGURATION]", NULL))
          return 0;

        analysis_summary.total_samples=0 ;
        analysis_summary.level_errors=0 ;
        analysis_summary.clear_simulations=0 ;

        while (TRUE)
          {
          if (NULL == (pszLine = ReadLine (config_file, '\0', TRUE))) break ;

          if (!strcmp ("[#FAULT SIMULATION CONFIGURATION]", pszLine))
            {
            g_free (pszLine) ;
            break ;
            }
                  tokenize_line (pszLine, strlen (pszLine), &pszValue, '=') ;

                  if (!strcmp (pszLine, "NUMBER_OF_SIMULATIONS"))
                    number_of_simulations = atoi (pszValue) ;
                  else
                  if (!strcmp (pszLine, "TOTAL_CELLS"))
                      number_of_cells = atoi (pszValue) ;
                  else
                  if (!strcmp (pszLine, "OUTPUTS"))
                      number_of_outputs = atoi (pszValue) ;


                  g_free (pszLine) ;
                  g_free (ReadLine (config_file, '\0', FALSE)) ;

             }
        file_close_and_unbuffer (config_file);

        //Saidas
        output_description_OP **outputs_simulations = (output_description_OP **) malloc(number_of_simulations * sizeof(output_description_OP*));
        for (id=0; id<number_of_simulations; id++)
        {
           outputs_simulations[id]= (output_description_OP *) malloc(number_of_outputs * sizeof(output_description_OP));
           for (id2=0;id2<number_of_outputs;id2++)
               {
               outputs_simulations[id][id2].out_clear= 0;
               outputs_simulations[id][id2].total_dislocation_faults=0;
               outputs_simulations[id][id2].total_dopant_faults=0;
               outputs_simulations[id][id2].total_interstitial_faults=0;
               outputs_simulations[id][id2].total_vacancy_faults=0;


               outputs_simulations[id][id2].cells_w_dislocation_fault = (int *) malloc(number_of_cells * sizeof(int));
               outputs_simulations[id][id2].cells_w_dopant_fault = (int *) malloc(number_of_cells * sizeof(int));
               outputs_simulations[id][id2].cells_w_vacancy_fault = (int *) malloc(number_of_cells * sizeof(int));
               outputs_simulations[id][id2].cells_w_interstitial_fault = (int *) malloc(number_of_cells * sizeof(int));
               outputs_simulations[id][id2].total_faults_by_cell = (int *) malloc(number_of_cells * sizeof(int));

               for (id3=0;id3<number_of_cells;id3++)
        	{

               outputs_simulations[id][id2].cells_w_dislocation_fault[id3]=0;
               outputs_simulations[id][id2].cells_w_dopant_fault[id3]=0;
               outputs_simulations[id][id2].cells_w_interstitial_fault[id3]=0;
               outputs_simulations[id][id2].cells_w_vacancy_fault[id3]=0;
               outputs_simulations[id][id2].total_faults_by_cell[id3]=0;
        	}

               }


        }



        for(Nix1=0; Nix1< number_of_simulations; Nix1++)
        {

            while (TRUE)
            {

            if (NULL == (pszLine = ReadLine (result_file, '\0', TRUE))) break ;

            if (!strcmp ("[#FAULTS x CELLS CROSS REFERENCE]", pszLine))
            {
            g_free (pszLine) ;
            break ;
            }

            if (!strcmp (g_strdup_printf ("%s%d%s", "[#FAULT SIMULATION FILE ", Nix1+1, "]"), pszLine))
            {
            g_free (pszLine) ;
            break ;
            }

            //for para outputs
            for(Nix2=0; Nix2< number_of_outputs; Nix2++)
            {
        	while (TRUE)
        	{
        	if (NULL == (pszLine = ReadLine (result_file, '\0', TRUE))) break ;

        	if (!strcmp (g_strdup_printf ("%s%d%s", "[#OUTPUT ", Nix2+1, "]"), pszLine))
        	{
        	g_free (pszLine) ;
        	break ;
        	}

        	if (!strcmp (pszLine, "NO ERRORS"))
        	outputs_simulations[Nix1][Nix2].out_clear=1;

        	g_free (pszLine) ;
        	g_free (ReadLine (result_file, '\0', FALSE)) ;

        	}

            }

            //[FAULTS x CELLS CROSS REFERENCE]
            while (TRUE)
            {

                if (NULL == (pszLine = ReadLine (result_file, '\0', TRUE))) break ;

                if (!strcmp ("ERROR-FREE SIMULATION",pszLine))
                analysis_summary.clear_simulations++;

    		 if (!strcmp ("[#FAULTS x CELLS CROSS REFERENCE]", pszLine))
                     {
                     //g_free (pszLine) ;
                     break ;
                     }

    		 tokenize_line (pszLine, strlen (pszLine), &pszValue, '=') ;

		 if (pszValue!= NULL && strcmp(pszValue,"NO FAULTS ")!=0)
		 {

		  if (!strcmp (pszLine, "DOPANT"))
		  {
		      pszItr = pszValue ;
		      while (NULL != (pszValue = next_space_separated_value (&pszItr)))
			  {

			     for(Nix2=0; Nix2< number_of_outputs; Nix2++)
			     {
			     //if (outputs_simulations[Nix1][Nix2].out_clear==0)
			     //{

				 outputs_simulations[Nix1][Nix2].cells_w_dopant_fault[atoi(pszValue)-1] = 1;
				 set_progress_bar_fraction (get_file_percent (result_file)) ;

			     //}
			     }
			  }

		 }

		  if (!strcmp (pszLine, "INSTERSTITIAL"))
		  {

		      pszItr = pszValue ;
		      while (NULL != (pszValue = next_space_separated_value (&pszItr)))
			  {

			     for(Nix2=0; Nix2< number_of_outputs; Nix2++)
			     {
			     //if (outputs_simulations[Nix1][Nix2].out_clear==0)
			     //{

				 outputs_simulations[Nix1][Nix2].cells_w_interstitial_fault[atoi(pszValue)-1] = 1;
				 set_progress_bar_fraction (get_file_percent (result_file)) ;

			     //}
			     }
			  }

		 }

		  if (!strcmp (pszLine, "VACANCY"))
		  {

		      pszItr = pszValue ;
		      while (NULL != (pszValue = next_space_separated_value (&pszItr)))
			  {

			     for(Nix2=0; Nix2< number_of_outputs; Nix2++)
			     {
			     //if (outputs_simulations[Nix1][Nix2].out_clear==0)
			     //{

				 outputs_simulations[Nix1][Nix2].cells_w_vacancy_fault[atoi(pszValue)-1] = 1;
				 set_progress_bar_fraction (get_file_percent (result_file)) ;

			     //}
			     }
			  }
		 }

		  if (!strcmp (pszLine, "DISLOCATION"))
		  {

		      pszItr = pszValue ;
		      while (NULL != (pszValue = next_space_separated_value (&pszItr)))
			  {

			     for(Nix2=0; Nix2< number_of_outputs; Nix2++)
			     {
			     //if (outputs_simulations[Nix1][Nix2].out_clear==0)
			     //{

				 outputs_simulations[Nix1][Nix2].cells_w_dislocation_fault[atoi(pszValue)-1] = 1;
				 set_progress_bar_fraction (get_file_percent (result_file)) ;

			     //}
			     }
			  }

		 }

		 }

            g_free (pszLine) ;
            g_free (ReadLine (result_file, '\0', FALSE)) ;
            }
                        //for

            g_free (pszLine) ;
            g_free (ReadLine (result_file, '\0', FALSE)) ;

            }

            //g_free (pszLine) ;
            //g_free (ReadLine (result_file, '\0', FALSE)) ;
        }

        //Le, para cada celula, as falhas em cada saida e registra
        for (id3=0; id3<number_of_cells; id3++)
        	for (id2=0; id2<number_of_outputs; id2++)
        		for (id=0; id<number_of_simulations; id++)
        		{
        			//if (outputs_simulations[id][id2].out_clear==0)
        			//{
        			outputs_simulations[id][id2].total_faults_by_cell[id3]=
        			outputs_simulations[id][id2].cells_w_dislocation_fault[id3]+
        			outputs_simulations[id][id2].cells_w_dopant_fault[id3]+
        			outputs_simulations[id][id2].cells_w_interstitial_fault[id3]+
        			outputs_simulations[id][id2].cells_w_vacancy_fault[id3];
        			//}
        		}

        heatmap_by_output = (double **) malloc(number_of_outputs * sizeof(double*));
        general_heatmap = (double *) malloc(number_of_cells * sizeof(double));

        for (id2=0; id2<number_of_outputs; id2++)
          {
		heatmap_by_output[id2]= (double *) malloc(number_of_cells * sizeof(double));
                   for (id3=0;id3<number_of_cells;id3++)
                       {
                       heatmap_by_output[id2][id3]= 0;
                       general_heatmap[id3]=0;
                       }

           }


        //Faz soma do peso de cada celula no erro nas varias simulacoes por saida
        unclear_simulations = (int *) malloc(number_of_cells * sizeof(int));
        for (id3=0;id3<number_of_cells;id3++)
        	unclear_simulations[id3]= 0;

        for (id2=0; id2<number_of_outputs; id2++)
            {
        	for (id3=0; id3<number_of_cells; id3++)
        	    {
        		unclear_simulations[id3]=0;
        		for (id=0; id<number_of_simulations; id++)
        		{
        			//heatmap_by_output[id3]=heatmap_by_output[id3]+(outputs_simulations[id][id2].total_faults_by_cell[id3])/(number_of_cells * number_of_simulations);
        			if (outputs_simulations[id][id2].out_clear==0)
        			heatmap_by_output[id2][id3]=heatmap_by_output[id2][id3]+outputs_simulations[id][id2].total_faults_by_cell[id3];

        			unclear_simulations[id3]=unclear_simulations[id3]+outputs_simulations[id][id2].total_faults_by_cell[id3];
        			//}

        		}
        		if(unclear_simulations[id3]!=0)
        		heatmap_by_output[id2][id3]=heatmap_by_output[id2][id3]/unclear_simulations[id3];

        		//if (number_of_outputs>1)
        		//{
            	if (heatmap_by_output[id2][id3]>general_heatmap[id3])
            		general_heatmap[id3]=heatmap_by_output[id2][id3];
        		//}
        		//else
        			//general_heatmap[id3]=heatmap_by_output[id2][id3];
        	    }

        	//Acha o valor maximo de unclear_simulations por saida para ser o valor geral
        	//unclear_simulations[number_of_outputs]=unclear_simulations[0];
        	//for (Nix1=0;Nix1<number_of_outputs;Nix1++)
            	//if (unclear_simulations[number_of_outputs]<unclear_simulations[Nix1])
            		//unclear_simulations[number_of_outputs]=unclear_simulations[Nix1];

		//Abre o desenho para alterar Heat Maps
		open_project_file_fp (design_file, &original_design);

		modified_design=design_copy(original_design);
		icell=0;
		for (id_cell = modified_design->lstLayers; id_cell!=NULL; id_cell=id_cell->next)
		{
		for (id_cell2=QCAD_LAYER(id_cell->data)->lstObjs; id_cell2!=NULL; id_cell2=id_cell2->next)

		{
		    if (QCAD_IS_CELL(id_cell2->data)==TRUE)
		    {

        			if (heatmap_by_output[id2][icell]<= LOW_LOW_IMPORTANT)
        			    {
        			    QCAD_DESIGN_OBJECT(id_cell2->data)->clr.red=13056;
        			    QCAD_DESIGN_OBJECT(id_cell2->data)->clr.green=13056;
        			    QCAD_DESIGN_OBJECT(id_cell2->data)->clr.blue=65535;
        			    }
        			else
            			if (heatmap_by_output[id2][icell]> LOW_LOW_IMPORTANT && heatmap_by_output[id2][icell]<= LOW_IMPORTANT)
            			    {
            			    QCAD_DESIGN_OBJECT(id_cell2->data)->clr.red=13056;
            			    QCAD_DESIGN_OBJECT(id_cell2->data)->clr.green=65535;
            			    QCAD_DESIGN_OBJECT(id_cell2->data)->clr.blue=65535;
            			    }
            			else
        			    if (heatmap_by_output[id2][icell]> LOW_IMPORTANT && heatmap_by_output[id2][icell]<= IMPORTANT)
            			    {
            			    QCAD_DESIGN_OBJECT(id_cell2->data)->clr.red=13056;
            			    QCAD_DESIGN_OBJECT(id_cell2->data)->clr.green=65535;
            			    QCAD_DESIGN_OBJECT(id_cell2->data)->clr.blue=13056;
            			    }
        			else
        			    if (heatmap_by_output[id2][icell]>IMPORTANT && heatmap_by_output[id2][icell]<=HIGH_IMPORTANT)
        			    {
        			    QCAD_DESIGN_OBJECT(id_cell2->data)->clr.red=65535;
        			    QCAD_DESIGN_OBJECT(id_cell2->data)->clr.green=65535;
        			    QCAD_DESIGN_OBJECT(id_cell2->data)->clr.blue=13056;
        			    }
        			else
        			    if (heatmap_by_output[id2][icell]>HIGH_IMPORTANT && heatmap_by_output[id2][icell]<=HIGH_HIGH_IMPORTANT)
        			    {
        			    QCAD_DESIGN_OBJECT(id_cell2->data)->clr.red=65535;
        			    QCAD_DESIGN_OBJECT(id_cell2->data)->clr.green=13056;
        			    QCAD_DESIGN_OBJECT(id_cell2->data)->clr.blue=65535;
        			    }
        			else
        			    if (heatmap_by_output[id2][icell]>HIGH_HIGH_IMPORTANT)
        			    {
        			    QCAD_DESIGN_OBJECT(id_cell2->data)->clr.red=65535;
        			    QCAD_DESIGN_OBJECT(id_cell2->data)->clr.green=13056;
        			    QCAD_DESIGN_OBJECT(id_cell2->data)->clr.blue=13056;
        			    }


        		    icell++;
        		    }
		}
		}
		    create_file (g_strdup_printf ("%s%s%d", _(resultFName), "_heatmap_output_", id2+1), modified_design) ;

        	if (id2==(number_of_outputs-1))
        	    {

        		//Abre o desenho para alterar Heat Maps
        		modified_design=design_copy(original_design);
        		icell=0;
        		for (id_cell = modified_design->lstLayers; id_cell!=NULL; id_cell=id_cell->next)
        		{
        		for (id_cell2=QCAD_LAYER(id_cell->data)->lstObjs; id_cell2!=NULL; id_cell2=id_cell2->next)

        		{
        		    if (QCAD_IS_CELL(id_cell2->data)==TRUE)
        		    {

        			if (general_heatmap[icell]<= LOW_LOW_IMPORTANT)
        			    {
        			    QCAD_DESIGN_OBJECT(id_cell2->data)->clr.red=13056;
        			    QCAD_DESIGN_OBJECT(id_cell2->data)->clr.green=13056;
        			    QCAD_DESIGN_OBJECT(id_cell2->data)->clr.blue=65535;
        			    }
        			else
            			if (general_heatmap[icell]> LOW_LOW_IMPORTANT && general_heatmap[icell]<= LOW_IMPORTANT)
            			    {
            			    QCAD_DESIGN_OBJECT(id_cell2->data)->clr.red=13056;
            			    QCAD_DESIGN_OBJECT(id_cell2->data)->clr.green=65535;
            			    QCAD_DESIGN_OBJECT(id_cell2->data)->clr.blue=65535;
            			    }
            			else
        			    if (general_heatmap[icell]> LOW_IMPORTANT && general_heatmap[icell]<= IMPORTANT)
            			    {
        			    QCAD_DESIGN_OBJECT(id_cell2->data)->clr.red=13056;
                		    QCAD_DESIGN_OBJECT(id_cell2->data)->clr.green=65535;
                		    QCAD_DESIGN_OBJECT(id_cell2->data)->clr.blue=13056;
            			    }
        			else
        			    if (general_heatmap[icell]>IMPORTANT && general_heatmap[icell]<=HIGH_IMPORTANT)
        			    {
            			    QCAD_DESIGN_OBJECT(id_cell2->data)->clr.red=65535;
            			    QCAD_DESIGN_OBJECT(id_cell2->data)->clr.green=65535;
            			    QCAD_DESIGN_OBJECT(id_cell2->data)->clr.blue=13056;
        			    }
        			else
        			    if (general_heatmap[icell]>HIGH_IMPORTANT && general_heatmap[icell]<=HIGH_HIGH_IMPORTANT)
        			    {
            			    QCAD_DESIGN_OBJECT(id_cell2->data)->clr.red=65535;
            			    QCAD_DESIGN_OBJECT(id_cell2->data)->clr.green=13056;
            			    QCAD_DESIGN_OBJECT(id_cell2->data)->clr.blue=65535;
        			    }
        			else
        			    if (general_heatmap[icell]>HIGH_HIGH_IMPORTANT)
        			    {
            			    QCAD_DESIGN_OBJECT(id_cell2->data)->clr.red=65535;
            			    QCAD_DESIGN_OBJECT(id_cell2->data)->clr.green=13056;
            			    QCAD_DESIGN_OBJECT(id_cell2->data)->clr.blue=13056;
        			    }


        		    icell++;
        		    }
        		}
        		}
        		    create_file (g_strdup_printf ("%s%s", _(resultFName), "_general_heatmap"), modified_design) ;

        	    }

            }

         create_file (g_strdup_printf ("%s%s", _(resultFName), "_analysis"), modified_design) ;

	 if (NULL == (fp = fopen (g_strdup_printf ("%s%s", _(resultFName), "_analysis"), "w+")))
	 return 0;
	 else
	{

	 fprintf (fp, "\n[SUMMARY OF ERRORS ON ALL FILES]\n") ;
	 percent_working=((float)(analysis_summary.clear_simulations)/((float)(number_of_simulations)))*100;
	 fprintf (fp, g_strdup_printf ("%s%d%s", "ALL_SIMULATIONS=", number_of_simulations, "\n")) ;
	 fprintf (fp, g_strdup_printf ("%s%d%s", "CLEAR_SIMULATIONS=", analysis_summary.clear_simulations, "\n")) ;
	 fprintf (fp, g_strdup_printf ("%s%f%s", "PERCENT_OF_CIRCUITS_WORKING=", percent_working, "%\n")) ;
	 fprintf (fp, "[#SUMMARY OF ERRORS ON ALL FILES]\n") ;

	 file_close_and_unbuffer (fp) ;
	}

	    g_free(outputs_simulations);
	    g_free(unclear_simulations);
	    g_free(heatmap_by_output);
	    g_free(general_heatmap);

        file_close_and_unbuffer (result_file) ;
        file_close_and_unbuffer (design_file) ;

        set_progress_bar_visible (FALSE) ;
        return percent_working;
 }

float synthetize_phase_shifts_sim_results(gchar *resultFName, gchar *configFName, int number_of_simulations)
    {

      gint teste;
      FILE *result_file = NULL, *config_file = NULL, *fp = NULL;
      gint number_of_cells, number_of_outputs, totals_1, totals_2;
      gint Nix1, Nix2, id, id2, id3, id4, shifts_count;
      char *pszLine = NULL , *pszValue = NULL , *pszItr = NULL ;
      gdouble **heatmap_by_output, *general_heatmap, quocient;
      DESIGN **original_design= NULL, *modified_design= NULL;
      GList *id_cell= NULL, *id_cell2= NULL;
      gint *unclear_simulations;
      gfloat  min_of_range[12], max_of_range[12], percent_working;
      double min_shift, max_shift;
      int iaux;


      //Abre o arquivo de resultados
      if (NULL == (result_file = file_open_and_buffer (resultFName)))
        return 0;

      //Abre o arquivo de configuracoes
      if (NULL == (config_file = file_open_and_buffer (configFName)))
        return 0;

      set_progress_bar_visible (TRUE) ;
      set_progress_bar_label (_("Analyzing results...")) ;
      set_progress_bar_fraction (0.0) ;

      //Extracao do resultado
      //Numero de simulacoes, celulas e outputs
        if (!SkipPast (config_file, '\0', "[FAULT SIMULATION CONFIGURATION]", NULL))
          return 0;

        analysis_summary.total_samples=0 ;
        analysis_summary.level_errors=0 ;
        analysis_summary.clear_simulations=0 ;

        while (TRUE)
          {
          if (NULL == (pszLine = ReadLine (config_file, '\0', TRUE))) break ;

          if (!strcmp ("[#FAULT SIMULATION CONFIGURATION]", pszLine))
            {
            g_free (pszLine) ;
            break ;
            }
                  tokenize_line (pszLine, strlen (pszLine), &pszValue, '=') ;

                  if (!strcmp (pszLine, "NUMBER_OF_SIMULATIONS"))
                    number_of_simulations = atoi (pszValue) ;
                  else
                  if (!strcmp (pszLine, "OUTPUTS"))
                      number_of_outputs = atoi (pszValue) ;
                  else
                  if (!strcmp (pszLine, "MIN_SHIFT_(pi_rad)"))
                      min_shift = atof(pszValue) ;
                  else
                  if (!strcmp (pszLine, "MAX_SHIFT_(pi_rad)"))
                      max_shift = atof(pszValue) ;


                  g_free (pszLine) ;
                  g_free (ReadLine (config_file, '\0', FALSE)) ;

             }
        file_close_and_unbuffer (config_file);

        //Calculo do maximo de cada faixa
        for (id3=0;id3<12;id3++)
            {
            max_of_range[id3]=min_shift*PI+(float)(id3+1)*PI*(float)(fabs(max_shift-min_shift)/12);
            min_of_range[id3]=min_shift*PI+(float)(id3)*PI*(float)(fabs(max_shift-min_shift)/12);
            }

        //Saidas
        output_description_shift_sim_OP general_result;

        //output_description_shift_sim_OP outputs_simulations = malloc(number_of_outputs * sizeof(output_description_shift_sim_OP));
        //for (id2=0; id2<number_of_outputs; id2++)
        //{
        //    outputs_simulations[id2]= (int *) malloc(number_of_outputs * sizeof(int));
        //}
        output_description_shift_sim_OP *outputs_simulations =  (output_description_shift_sim_OP *) malloc(number_of_outputs * sizeof(output_description_shift_sim_OP));


        int **out_clear =  (int **) malloc(number_of_simulations * sizeof(int*));
        for (id=0; id<number_of_simulations; id++)
        {
        out_clear[id]= (int *) malloc(number_of_outputs * sizeof(int));
        }

        for (id2=0;id2<number_of_outputs;id2++)
        {
            for (id=0; id<number_of_simulations; id++)
            {
            out_clear[id][id2]= 0;
            }

               for (id4=0;id4<4;id4++)
        	{
        	outputs_simulations[id2].number_of_shifts[id4][0]=0;
        	outputs_simulations[id2].number_of_shifts[id4][1]=0;
		    if (id2==0)
		    {
		    general_result.number_of_shifts[id4][0]=0;
		    general_result.number_of_shifts[id4][1]=0;
		    }
        	for (id3=0;id3<12;id3++)
        	{
                outputs_simulations[id2].total_interval_clockzone[id3][id4]=0;
                outputs_simulations[id2].faulty_interval_clockzone[id3][id4]=0;
		    if (id2==0)
                    {
		    general_result.total_interval_clockzone[id3][id4]=0;
		    general_result.faulty_interval_clockzone[id3][id4]=0;
                    }
               }
               }
        }

        for(Nix1=0; Nix1< number_of_simulations; Nix1++)
        {

            //while (TRUE)
            //{

            //if (NULL == (pszLine = ReadLine (result_file, '\0', TRUE))) break ;

            //if (!strcmp (g_strdup_printf ("%s%d%s", "[#FAULT SIMULATION FILE ", Nix1+1, "]"), pszLine))
            //{
            //g_free (pszLine) ;
            //break ;
            //}

            //for para outputs
            for(Nix2=0; Nix2< number_of_outputs; Nix2++)
            {
        	while (TRUE)
        	{
        	if (NULL == (pszLine = ReadLine (result_file, '\0', TRUE))) break ;

        	if (!strcmp (g_strdup_printf ("%s%d%s", "[#OUTPUT ", Nix2+1, "]"), pszLine))
        	{
        	g_free (pszLine) ;
        	break ;
        	}

        	if (!strcmp (pszLine, "NO ERRORS"))
        	out_clear[Nix1][Nix2]=1;

        	g_free (pszLine) ;
        	g_free (ReadLine (result_file, '\0', FALSE)) ;
        	}
             }

            //[PHASE SHIFTS]
            shifts_count=0;
            while (TRUE)
            {

                if (NULL == (pszLine = ReadLine (result_file, '\0', TRUE))) break ;

                if (!strcmp ("ERROR-FREE SIMULATION",pszLine))
                analysis_summary.clear_simulations++;

                if (!strcmp (g_strdup_printf ("%s%d%s", "[#FAULT SIMULATION FILE ", Nix1+1, "]"), pszLine))
                     {
    		     if (shifts_count>0)
    		     {
    		     shifts_count--;
    			 for(Nix2=0; Nix2< number_of_outputs; Nix2++)
    			 {
    			 outputs_simulations[Nix2].number_of_shifts[shifts_count][0]++;

    			 if (out_clear[Nix1][Nix2]==0)
    			 outputs_simulations[Nix2].number_of_shifts[shifts_count][1]++;

    			 }
    		      }
    		     break ;
                     }

    		 tokenize_line (pszLine, strlen (pszLine), &pszValue, '=') ;

		 if (pszValue!= NULL && strcmp(pszValue,"0.000000")!=0)
		 {

		  if (!strcmp (pszLine, "CLOCK 0  (pi rad)"))
		  {
			     shifts_count++;
			     for(Nix2=0; Nix2< number_of_outputs; Nix2++)
			     {
				for (id3=0;id3<12;id3++)
				{
				if (fabs(atof(pszValue))<=(double)fabs(max_of_range[id3]))
				    break;
				}

				outputs_simulations[Nix2].total_interval_clockzone[id3][0]++;

			     if (out_clear[Nix1][Nix2]==0)
				 outputs_simulations[Nix2].faulty_interval_clockzone[id3][0]++;



			     set_progress_bar_fraction (get_file_percent (result_file)) ;

			     }

		 }

		  if (!strcmp (pszLine, "CLOCK 1  (pi rad)"))
		  {
			     shifts_count++;
			     for(Nix2=0; Nix2< number_of_outputs; Nix2++)
			     {
				for (id3=0;id3<12;id3++)
				{
				if (fabs(atof(pszValue))<=(double)fabs(max_of_range[id3]))
				    break;
				}

				outputs_simulations[Nix2].total_interval_clockzone[id3][1]++;

			     if (out_clear[Nix1][Nix2]==0)
				 outputs_simulations[Nix2].faulty_interval_clockzone[id3][1]++;



			     set_progress_bar_fraction (get_file_percent (result_file)) ;

			     }

		 }

		  if (!strcmp (pszLine, "CLOCK 2  (pi rad)"))
		  {
			    shifts_count++;
			     for(Nix2=0; Nix2< number_of_outputs; Nix2++)
			     {
				for (id3=0;id3<12;id3++)
				{
				if (fabs(atof(pszValue))<=(double)fabs(max_of_range[id3]))
				    break;
				}

				outputs_simulations[Nix2].total_interval_clockzone[id3][2]++;

			     if (out_clear[Nix1][Nix2]==0)
				 outputs_simulations[Nix2].faulty_interval_clockzone[id3][2]++;



			     set_progress_bar_fraction (get_file_percent (result_file)) ;

			     }

		 }

		  if (!strcmp (pszLine, "CLOCK 3  (pi rad)"))
		  {
			     shifts_count++;
			     for(Nix2=0; Nix2< number_of_outputs; Nix2++)
			     {
				for (id3=0;id3<12;id3++)
				{
				if (fabs(atof(pszValue))<=(double)fabs(max_of_range[id3]))
				    break;
				}

				outputs_simulations[Nix2].total_interval_clockzone[id3][3]++;


			     if (out_clear[Nix1][Nix2]==0)
				 outputs_simulations[Nix2].faulty_interval_clockzone[id3][3]++;


			     set_progress_bar_fraction (get_file_percent (result_file)) ;

			     }
		 }

		 }

            g_free (pszLine) ;
            g_free (ReadLine (result_file, '\0', FALSE)) ;
            }

            g_free (pszLine) ;
            g_free (ReadLine (result_file, '\0', FALSE)) ;

        }

        //Le as falhas em cada saida e faz o resumo como o maximo

        for (id4=0;id4<4;id4++)
            for (id3=0;id3<12;id3++)
        	    for (id2=0; id2<number_of_outputs; id2++)
        	    {
        		if (id2+1<number_of_outputs)
        		{
        		    general_result.faulty_interval_clockzone[id3][id4]=
        			max(outputs_simulations[id2].faulty_interval_clockzone[id3][id4],
        			outputs_simulations[id2+1].faulty_interval_clockzone[id3][id4]);

        		    general_result.total_interval_clockzone[id3][id4]=
        			max(outputs_simulations[id2].total_interval_clockzone[id3][id4],
        			outputs_simulations[id2+1].total_interval_clockzone[id3][id4]);

        		    if(id3==11) //evita o processamento desnecessario
        			{
        			general_result.number_of_shifts[id4][0]=outputs_simulations[id2].number_of_shifts[id4][0];

        			general_result.number_of_shifts[id4][1]=max(outputs_simulations[id2].number_of_shifts[id4][1],
        				outputs_simulations[id2+1].number_of_shifts[id4][1]);
        			}
        		}
        	        else
        	        {
        		    general_result.faulty_interval_clockzone[id3][id4]=
        			max(outputs_simulations[id2].faulty_interval_clockzone[id3][id4],
        				general_result.faulty_interval_clockzone[id3][id4]);

        		    general_result.total_interval_clockzone[id3][id4]=
        			max(outputs_simulations[id2].total_interval_clockzone[id3][id4],
        				general_result.total_interval_clockzone[id3][id4]);

        		    if(id3==11) //evita o processamento desnecessario
        			{
        			general_result.number_of_shifts[id4][0]=outputs_simulations[id2].number_of_shifts[id4][0];

        			general_result.number_of_shifts[id4][1]=max(outputs_simulations[id2].number_of_shifts[id4][1],
        				general_result.number_of_shifts[id4][1]);
        			}
        	            break;
        	        }
        	    }
        //escreve os resultados

        create_file (g_strdup_printf ("%s%s", _(resultFName), "_analysis"), project_options.design) ;

	 if (NULL == (fp = fopen (g_strdup_printf ("%s%s", _(resultFName), "_analysis"), "w+")))
	 return 0;
	 else
	{

	 fprintf (fp, "\n[SUMMARY OF ERRORS ON ALL FILES]\n") ;
	 percent_working=((float)(analysis_summary.clear_simulations)/((float)(number_of_simulations)))*100;
	 fprintf (fp, g_strdup_printf ("%s%d%s", "ALL_SIMULATIONS=", number_of_simulations, "\n")) ;
	 fprintf (fp, g_strdup_printf ("%s%d%s", "CLEAR_SIMULATIONS=", analysis_summary.clear_simulations, "\n")) ;
	 fprintf (fp, g_strdup_printf ("%s%f%s", "PERCENT_OF_CIRCUITS_WORKING=", percent_working, "%\n")) ;
	 fprintf (fp, "[#SUMMARY OF ERRORS ON ALL FILES]\n") ;
	 fprintf (fp, "\n[DETAILED REPORT]\n") ;
	 for (id2=0;id2<number_of_outputs;id2++)
	     {
	     fprintf (fp, g_strdup_printf ("%s%d%s", "\n[OUTPUT ", id2+1, "]\n")) ;

	     fprintf (fp, "\n[SHIFTS RANGES x FAULT EVENTS]\n") ;
	     for (id3=0;id3<12;id3++)

		     {
		     totals_1=0;
		     totals_2=0;

		     fprintf (fp, g_strdup_printf ("%s%d%s", "\n[SHIFT RANGE ", id3+1, "]\n")) ;
		     fprintf (fp, g_strdup_printf ("%s%f%s%f%s", "VALUE=", min_of_range[id3], " to ", max_of_range[id3], "\n")) ;
		     for (id4=0;id4<4;id4++)
		     {
		     fprintf (fp, g_strdup_printf ("%s%d%s", "[CLOCK ", id4, "]\n")) ;
		     fprintf (fp, g_strdup_printf ("%s%d%s", "NUMBER_OF_SHIFTS=", outputs_simulations[id2].total_interval_clockzone[id3][id4], "\n")) ;
		     fprintf (fp, g_strdup_printf ("%s%d%s", "NUMBER_OF_FAULTS=", outputs_simulations[id2].faulty_interval_clockzone[id3][id4], "\n")) ;
		     fprintf (fp, g_strdup_printf ("%s%d%s", "[#CLOCK ", id4, "]\n")) ;
		     totals_1=totals_1+outputs_simulations[id2].total_interval_clockzone[id3][id4];
		     totals_2=totals_2+outputs_simulations[id2].faulty_interval_clockzone[id3][id4];
		     }
		     fprintf (fp, "[TOTALS]\n") ;
		     fprintf (fp, g_strdup_printf ("%s%d%s", "TOTAL_OF_SHIFTS=", totals_1, "\n")) ;
		     fprintf (fp, g_strdup_printf ("%s%d%s", "TOTAL_OF_FAULTS=", totals_2, "\n")) ;
		     fprintf (fp, "[#TOTALS]\n") ;

		     fprintf (fp, g_strdup_printf ("%s%d%s", "[#SHIFT RANGE ", id3+1, "]\n")) ;
		     }
	     fprintf (fp, "\n[#SHIFTS RANGES x FAULT EVENTS]\n") ;


	     fprintf (fp, "\n[NUMBER OF PHASES SHIFTED x FAULT EVENTS]\n") ;
	     for (id4=0;id4<4;id4++)
	     {
		     fprintf (fp, g_strdup_printf ("%s%d%s", "[", id4+1, " PHASE(S) SHIFTED]\n")) ;
		     fprintf (fp, g_strdup_printf ("%s%d%s", "TOTAL=", (int)outputs_simulations[id2].number_of_shifts[id4][0], "\n")) ;
		     fprintf (fp, g_strdup_printf ("%s%d%s", "FAULT_EVENTS=", (int)outputs_simulations[id2].number_of_shifts[id4][1], "\n")) ;
		     fprintf (fp, g_strdup_printf ("%s%d%s", "[#", id4+1, " PHASE(S) SHIFTED]\n")) ;
	     }
	     fprintf (fp, "[#NUMBER OF PHASES SHIFTED x FAULT EVENTS]\n") ;

	     fprintf (fp, g_strdup_printf ("%s%d%s", "\n[#OUTPUT ", id2+1, "]\n")) ;
	     }
	     if (number_of_outputs>1)
	     {
	     fprintf (fp, "\n[OVERALL CIRCUIT (ALL OUTPUTS)]\n") ;

	     fprintf (fp, "\n[SHIFTS RANGES x FAULT EVENTS]\n") ;
	     for (id3=0;id3<12;id3++)

		     {
		     totals_1=0;
		     totals_2=0;

		     fprintf (fp, g_strdup_printf ("%s%d%s", "\n[SHIFT RANGE ", id3+1, "]\n")) ;
		     fprintf (fp, g_strdup_printf ("%s%f%s%f%s", "VALUE=", min_of_range[id3], " to ", max_of_range[id3], "\n")) ;
		     for (id4=0;id4<4;id4++)
		     {
		     fprintf (fp, g_strdup_printf ("%s%d%s", "[CLOCK ", id4, "]\n")) ;
		     fprintf (fp, g_strdup_printf ("%s%d%s", "NUMBER_OF_SHIFTS=", (int)general_result.total_interval_clockzone[id3][id4], "\n")) ;
		     fprintf (fp, g_strdup_printf ("%s%d%s", "NUMBER_OF_FAULTS=", (int)general_result.faulty_interval_clockzone[id3][id4], "\n")) ;
		     fprintf (fp, g_strdup_printf ("%s%d%s", "[#CLOCK ", id4, "]\n")) ;
		     totals_1=totals_1+general_result.total_interval_clockzone[id3][id4];
		     totals_2=totals_2+general_result.faulty_interval_clockzone[id3][id4];
		     }
		     fprintf (fp, "[TOTALS]\n") ;
		     fprintf (fp, g_strdup_printf ("%s%d%s", "TOTAL_OF_SHIFTS=", totals_1, "\n")) ;
		     fprintf (fp, g_strdup_printf ("%s%d%s", "TOTAL_OF_FAULTS=", totals_2, "\n")) ;
		     fprintf (fp, "[#TOTALS]\n") ;

		     fprintf (fp, g_strdup_printf ("%s%d%s", "[#SHIFT RANGE ", id3+1, "]\n")) ;
		     }
	     fprintf (fp, "\n[#SHIFTS RANGES x FAULT EVENTS]\n") ;


	     fprintf (fp, "\n[NUMBER OF PHASES SHIFTED x FAULT EVENTS]\n") ;
	     for (id4=0;id4<4;id4++)
	     {
		     fprintf (fp, g_strdup_printf ("%s%d%s", "[", id4+1, " PHASE(S) SHIFTED]\n")) ;
		     fprintf (fp, g_strdup_printf ("%s%d%s", "TOTAL=", general_result.number_of_shifts[id4][0], "\n")) ;
		     fprintf (fp, g_strdup_printf ("%s%d%s", "FAULT_EVENTS=", general_result.number_of_shifts[id4][1], "\n")) ;
		     fprintf (fp, g_strdup_printf ("%s%d%s", "[#", id4+1, " PHASE(S) SHIFTED]\n")) ;
	     }

	     fprintf (fp, "[#NUMBER OF PHASES SHIFTED x FAULT EVENTS]\n") ;
	     fprintf (fp, "\n[#OVERALL CIRCUIT (ALL OUTPUTS)]\n") ;

	 }
	 fprintf (fp, "\n[#DETAILED REPORT]\n") ;

	}
	file_close_and_unbuffer (fp) ;
        file_close_and_unbuffer (result_file) ;

        set_progress_bar_visible (FALSE) ;
        return percent_working;
 }
