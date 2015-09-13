/** 
* @file keyer.c
* @brief Header files for the keyer functions
* @author Bernd Kernbaum DK3WX
* @version 0.1
* @date 2014-09-03
*/
// keyer.c

/* Copyright (C) 
* 2014 - Bernd Kernbaum DK3WX
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



#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <getopt.h>
#include "property.h"

#include "screensize.h"
#include "command.h"
#include "dttsp.h"
#include "main.h"
#include "ozy.h"

GtkWidget* keyerFrame;
GtkWidget* CWFrame;
GtkWidget* CWTable;
GtkWidget* buttonCW;
/*
* Setup
* Sidetone Frequency Hz (bits [11:0])
* CW Hang Time ms (bits [9:0])
* CW PTT Delay ms (bits [7:0])
* CW Sidetone Volume (bits [7:0])
* ++ CW ext/int on main Window
* Keyer Wight (bits [6:0] (0 - 100))
* Keyer Spacing (0=off; 1=0n)
* Keyer Mode (bits [1:0] (00 = Straight; 01 = Mode A; 10 = Mode B))
* ++ Keyer Speed on main Window
*/

double cw_sidetone_frequency;
double cw_hang_time;
double cw_ptt_delay;
double cw_sidetone_volume;
gboolean cw_ext;
double keyer_wight;
gboolean keyer_spacing;
unsigned char keyer_mode;
double keyer_speed;

int keyChanged_16=0;
int keyChanged_1E=0;
int keyChanged_20=0;
unsigned char control_16[5]={0x16,0x00,0x40,0x94,0xB2}; // Keyer Mode Speed Weight Spacing
unsigned char control_1E[5]={0x1E,0x00,0x7F,0x00,0x00}; // Keyer CW int Sidetone Vol PTT delay
unsigned char control_20[5]={0x20,0x80,0x00,0x26,0x00}; // Keyer CW Hang time Sidetone Frequ.

int aRit=0;
int aRitChanged=0;

GtkWidget* keyerScale;

/* --------------------------------------------------------------------------*/
/** 
* @brief  Select the keyer
* 
* @param widget
*/
void selectKeyer(GtkWidget* widget) {
    GtkWidget* label;
    char temp[80];
}

/* --------------------------------------------------------------------------*/
/** 
* @brief CW button Callback 
* 
* @param widget -- pointer to the parent widget, 
* @param data -- pointer to the data.
*/
void cwButtonCallback(GtkWidget* widget,gpointer data) {
    GtkWidget* label;
    char c[80];
  

    if (control_1E[1]) {
      control_1E[1]=0;aRit=0;
    } else {
      control_1E[1]=0x01;aRit=1;  
    };
     
    aRitChanged=1;
    
    label=gtk_bin_get_child((GtkBin*)widget);
    if(aRit) {
        gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &red);
        gtk_widget_modify_fg(label, GTK_STATE_PRELIGHT, &red);
    } else {
        gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &white);
        gtk_widget_modify_fg(label, GTK_STATE_PRELIGHT, &black);
    }

   setKeyerChanged();
}

/* --------------------------------------------------------------------------*/
/** 
* @brief  Callback when keyer values changes
* 
* @param widget
* @param data
*/
void keyerChanged(GtkWidget* widget,gpointer data) {
    unsigned char ks;

    keyer_speed=gtk_range_get_value((GtkRange*)keyerScale);
    ks = (char)keyer_speed;
    ks &= 0x3F;
    control_16[3] &= 0xC0;
    control_16[3] |= ks;
    setKeyerChanged();
}

void ctrlSidetoneChanged(void) {
    control_20[3] = (int)cw_sidetone_frequency>>4;
    control_20[4] = (int)cw_sidetone_frequency;
    control_20[4] &= 0x0f;
}
void ctrlSidetoneVolumeChanged(void) {
    control_1E[2] = (char)cw_sidetone_volume;
}

void ctrlHangTimeChanged(void) {
    control_20[1] = (int)cw_hang_time>>2;
    control_20[2] = (int)cw_hang_time;
    control_20[2] &= 0x03;
}

void ctrlPttDelayChanged(void) {
    control_1E[3] = (char)cw_ptt_delay;
}

void ctrlKeyerWightChanged(void) {
    unsigned char kw;
    kw = (char)keyer_wight;
    kw &= 0x7F;
    control_16[4] &= 0x80;
    control_16[4] |= kw;
}

void ctrlKeyerSpacingChanged(void) {
    control_16[4] &= 0x7F;
    if (keyer_spacing) {control_16[4] |= 0x80;}; 
}

void ctrlKeyerModeChanged(void) {
    control_16[3] &= 0x3F;
    if (keyer_mode == 1) {control_16[3] |= 0x40;}; 
    if (keyer_mode == 2) {control_16[3] |= 0x80;}; 
}


/* --------------------------------------------------------------------------*/




/** 
* @brief  Build the GUI
* 
* @return 
*/
GtkWidget* buildKeyerUI() {
    GtkWidget* label;


    CWFrame=gtk_frame_new("Internal Keyer");
    gtk_widget_modify_bg(CWFrame,GTK_STATE_NORMAL,&background);
    gtk_widget_modify_fg(gtk_frame_get_label_widget(GTK_FRAME(CWFrame)),GTK_STATE_NORMAL,&white);

    CWTable=gtk_table_new(1,8,TRUE);

    // CW On/Off settings
    buttonCW = gtk_button_new_with_label ("On");
    gtk_widget_modify_bg(buttonCW, GTK_STATE_NORMAL, &buttonBackground);
    label=gtk_bin_get_child((GtkBin*)buttonCW);
    gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &white);
    gtk_widget_set_size_request(GTK_WIDGET(buttonCW),BUTTON_WIDTH,BUTTON_HEIGHT);
    g_signal_connect(G_OBJECT(buttonCW),"clicked",G_CALLBACK(cwButtonCallback),NULL);
    gtk_widget_show(buttonCW);
    gtk_table_attach_defaults(GTK_TABLE(CWTable),buttonCW,0,1,0,1);



    keyerFrame=gtk_frame_new("CW Speed");
    gtk_widget_modify_bg(keyerFrame,GTK_STATE_NORMAL,&background);
    gtk_widget_modify_fg(gtk_frame_get_label_widget(GTK_FRAME(keyerFrame)),GTK_STATE_NORMAL,&white);

    keyerScale=gtk_hscale_new_with_range(10.0,30.0,1.0);
    g_signal_connect(G_OBJECT(keyerScale),"value-changed",G_CALLBACK(keyerChanged),NULL);
    gtk_range_set_value((GtkRange*)keyerScale,keyer_speed);
    gtk_widget_set_size_request(GTK_WIDGET(keyerScale),360,30);
    gtk_widget_show(keyerScale);
    gtk_container_add(GTK_CONTAINER(keyerFrame),keyerScale);
    //gtk_widget_set_size_request(keyerFrame,200,30);
    gtk_widget_show(keyerFrame);
    
    gtk_table_attach_defaults(GTK_TABLE(CWTable),keyerFrame,1,8,0,1);

    gtk_container_add(GTK_CONTAINER(CWFrame),CWTable);
    gtk_widget_show(CWTable);
    gtk_widget_show(CWFrame);

    //setKeyerChanged();

    return CWFrame;
  
}

/* --------------------------------------------------------------------------*/
/** 
* @brief Save the keyer state
*/
void keyerSaveState() {
    char string[128];
    sprintf(string,"%f",keyer_speed);
    setProperty("keyer_speed",string);

    sprintf(string,"%f",keyer_wight);
    setProperty("keyer_wight",string);

    sprintf(string,"%f",cw_sidetone_volume);
    setProperty("cw_sidetone_volume",string);

    sprintf(string,"%f",cw_ptt_delay);
    setProperty("cw_ptt_delay",string);

    sprintf(string,"%f",cw_hang_time);
    setProperty("cw_hang_time",string);

    sprintf(string,"%f",cw_sidetone_frequency);
    setProperty("cw_sidetone_frequency",string);

    sprintf(string,"%d",keyer_mode);
    setProperty("keyer_mode",string);

    sprintf(string,"%d",keyer_spacing);
    setProperty("keyer_spacing",string);

    sprintf(string,"%d",cw_ext);
    setProperty("cw_ext",string);
}

/* --------------------------------------------------------------------------*/
/** 
* @brief Restore the keyer state
*/
void keyerRestoreState() {
    char* value;
    value=getProperty("keyer_speed");
    if(value) keyer_speed=atof(value); else keyer_speed=20.0f;

    value=getProperty("keyer_wight");
    if(value) keyer_wight=atof(value); else keyer_wight=50.0f;

    value=getProperty("cw_sidetone_volume");
    if(value) cw_sidetone_volume=atof(value); else cw_sidetone_volume=120.0f;

    value=getProperty("cw_ptt_delay");
    if(value) cw_ptt_delay=atof(value); else cw_ptt_delay=0.0f;

    value=getProperty("cw_hang_time");
    if(value) cw_hang_time=atof(value); else cw_hang_time=300.0f;

    value=getProperty("cw_sidetone_frequency");
    if(value) cw_sidetone_frequency=atof(value); else cw_sidetone_frequency=600.0f;

    value=getProperty("keyer_mode");
    if(value) keyer_mode=atoi(value); else keyer_mode=3;

    value=getProperty("keyer_spacing");
    if(value) keyer_spacing=atoi(value); else keyer_spacing=0;

    value=getProperty("cw_ext");
    if(value) cw_ext=atoi(value); else cw_ext=0;

    //keyerChanged();
    ctrlSidetoneChanged();
    ctrlSidetoneVolumeChanged();
    ctrlHangTimeChanged();
    ctrlPttDelayChanged();
    ctrlKeyerWightChanged();
    ctrlKeyerSpacingChanged();
    ctrlKeyerModeChanged();

    setKeyerChanged();


}
