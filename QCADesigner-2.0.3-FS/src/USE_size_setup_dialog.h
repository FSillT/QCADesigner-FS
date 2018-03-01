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
// Header for the simulation fault setup dialog. This  //
// dialog allows the user to choose from among the      //
// available simulation faults.                        //
//                                                      //
//////////////////////////////////////////////////////////

#ifndef _USE_SIZE_SETUP_DIALOG_H_
#define _USE_SIZE_SETUP_DIALOG_H_

#include <gtk/gtk.h>

int get_USE_size_from_user (GtkWindow *parent, int USE_size);

#endif /* _USE_SIZE_SETUP_DIALOG_H_ */
