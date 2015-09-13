/**
 *modified Software   
 *author Bernd Kernbaum, DK3WX
 *email dk3wx@darc.de 
 * 
 *version 0.2
 *date 2014-10-5
 *
*/

/** 
* @file display.c
* @brief Display functions
* @author John Melton, G0ORX/N6LYT, Doxygen Comments Dave Larsen, KV0S
* @version 0.1
* @date 2009-04-11
*/
// display.c

/* Copyright (C) 
* 2009 - John Melton, G0ORX/N6LYT, Doxygen Comments Dave Larsen, KV0S
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
* 
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
* 
*/


//
// GTK+ 2.0 implementation of Beppe's Display and Decode panel
// see http://www.radioamatore.it/sdr1000/mypowersdr.html fo the original

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <getopt.h>

#include "screensize.h"
#include "command.h"
#include "filter.h"
#include "main.h"
#include "property.h"
#include "spectrum.h"
#include "spectrum_update.h"

//GtkWidget* display;
GtkWidget* displayFixed;

GtkWidget* spectrum;



/* --------------------------------------------------------------------------*/
/** 
* @brief Build the GUI
* 
* @return 
*/
GtkWidget* buildDisplayUI() {
    GtkWidget* label;

    displayFixed=gtk_fixed_new();
    gtk_widget_modify_bg(displayFixed,GTK_STATE_NORMAL,&background);

    spectrum=newSpectrumDisplay(spectrumWIDTH);
    gtk_fixed_put((GtkFixed*)displayFixed,spectrum,0,0);
    gtk_widget_set_size_request(GTK_WIDGET(displayFixed),spectrumWIDTH,spectrumHEIGHT+25);
    gtk_widget_show(displayFixed);

    setSpectrumMode(spectrumPANWATER);

    return displayFixed;
  
}

/* --------------------------------------------------------------------------*/
/** 
* @brief Save the display state. 
*/
void displaySaveState() {
    char string[128];

    sprintf(string,"%d",spectrumUpdatesPerSecond);
    setProperty("spectrum.updates.per.second",string);

    sprintf(string,"%d",spectrumMode);
    setProperty("spectrum.mode",string);

    spectrumSaveState();

}

/* --------------------------------------------------------------------------*/
/** 
* @brief Restore the display state.
*/
void displayRestoreState() {
    char* value;

    value=getProperty("spectrum.updates.per.second");
    spectrumUpdatesPerSecond=SPECTRUM_UPDATES_PER_SECOND;
    if(value) spectrumUpdatesPerSecond=atoi(value);

    value=getProperty("spectrum.mode");
    spectrumMode=spectrumPANWATER;
    if(value) spectrumMode=atoi(value);

    spectrumRestoreState();
}
