/** 
* @file keyer_setup.c
* @brief Header files for the keyer functions
* @author Bernd Kernbaum DK3WX
* @version 0.1
* @date 2014-09-03
*/
// keyer_setup.c

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

#include <gtk/gtk.h>
#include "main.h"
#include "keyer.h"
#include "keyer_setup.h"


GtkWidget* setupKeyerFixed;

GtkWidget* sidetoneLabel;
GtkWidget* sidetoneButton;
GtkWidget* SidetoneVolumeLabel;
GtkWidget* SidetoneVolumeLabelButton;
GtkWidget* CWHangTimeLabel;
GtkWidget* CWHangTimeLabelButton;
GtkWidget* CWPTTDelayLabel;
GtkWidget* CWPTTDelayLabelButton;
GtkWidget* KeyerWightLabel;
GtkWidget* KeyerWightLabelButton;
GtkWidget* keyerSpacingCheckButton;
GtkWidget* box;
GtkWidget* label;
GtkWidget* mode_0;
GtkWidget* mode_A;
GtkWidget* mode_B;



void sidetoneChanged(GtkSpinButton* spinButton,gpointer data);
void SidetoneVolumeLabelChanged(GtkSpinButton* spinButton,gpointer data);
void CWHangTimeLabelChanged(GtkSpinButton* spinButton,gpointer data);
void CWPTTDelayLabelChanged(GtkSpinButton* spinButton,gpointer data);
void KeyerWightLabelChanged(GtkSpinButton* spinButton,gpointer data);
void keyerSpacingChanged(GtkToggleButton* toggleButton,gpointer data);
void mode_0ButtonCallback(GtkWidget* widget,gpointer data);
void mode_AButtonCallback(GtkWidget* widget,gpointer data);
void mode_BButtonCallback(GtkWidget* widget,gpointer data);

/* --------------------------------------------------------------------------*/
/** 
* @brief keyer setup UI
*/
GtkWidget* keyerSetupUI() {
        setupKeyerFixed=gtk_fixed_new();

        // add keyer controls * Sidetone Frequency Hz (bits [11:0])
        sidetoneLabel=gtk_label_new("Sidetone [Hz]");
        gtk_widget_show(sidetoneLabel);
        gtk_fixed_put((GtkFixed*)setupKeyerFixed,sidetoneLabel,10,10);
        sidetoneButton=gtk_spin_button_new_with_range(100,4000,50);
        gtk_spin_button_set_value((GtkSpinButton*)sidetoneButton,(double)cw_sidetone_frequency);
        g_signal_connect(G_OBJECT(sidetoneButton),"value-changed",G_CALLBACK(sidetoneChanged),NULL);
        gtk_widget_show(sidetoneButton);
        gtk_fixed_put((GtkFixed*)setupKeyerFixed,sidetoneButton,250,10);

        // add keyer controls * CW Sidetone Volume (bits [7:0])
        SidetoneVolumeLabel=gtk_label_new("Sidetone Volume");
        gtk_widget_show(SidetoneVolumeLabel);
        gtk_fixed_put((GtkFixed*)setupKeyerFixed,SidetoneVolumeLabel,10,40);
        SidetoneVolumeLabelButton=gtk_spin_button_new_with_range(0,128,5);
        gtk_spin_button_set_value((GtkSpinButton*)SidetoneVolumeLabelButton,(double)cw_sidetone_volume);
        g_signal_connect(G_OBJECT(SidetoneVolumeLabelButton),"value-changed",G_CALLBACK(SidetoneVolumeLabelChanged),NULL);
        gtk_widget_show(SidetoneVolumeLabelButton);
        gtk_fixed_put((GtkFixed*)setupKeyerFixed,SidetoneVolumeLabelButton,250,40);

        // add keyer controls * CW Hang Time ms (bits [9:0])
        CWHangTimeLabel=gtk_label_new("Hang Time [ms]");
        gtk_widget_show(CWHangTimeLabel);
        gtk_fixed_put((GtkFixed*)setupKeyerFixed,CWHangTimeLabel,10,70);
        CWHangTimeLabelButton=gtk_spin_button_new_with_range(0,512,5);
        gtk_spin_button_set_value((GtkSpinButton*)CWHangTimeLabelButton,(double)cw_hang_time);
        g_signal_connect(G_OBJECT(CWHangTimeLabelButton),"value-changed",G_CALLBACK(CWHangTimeLabelChanged),NULL);
        gtk_widget_show(CWHangTimeLabelButton);
        gtk_fixed_put((GtkFixed*)setupKeyerFixed,CWHangTimeLabelButton,250,70);

        // add keyer controls * CW PTT Delay ms (bits [7:0])
        CWPTTDelayLabel=gtk_label_new("CW PTT Delay [ms]");
        gtk_widget_show(CWPTTDelayLabel);
        gtk_fixed_put((GtkFixed*)setupKeyerFixed,CWPTTDelayLabel,10,100);
        CWPTTDelayLabelButton=gtk_spin_button_new_with_range(0,256,5);
        gtk_spin_button_set_value((GtkSpinButton*)CWPTTDelayLabelButton,(double)cw_ptt_delay);
        g_signal_connect(G_OBJECT(CWPTTDelayLabelButton),"value-changed",G_CALLBACK(CWPTTDelayLabelChanged),NULL);
        gtk_widget_show(CWPTTDelayLabelButton);
        gtk_fixed_put((GtkFixed*)setupKeyerFixed,CWPTTDelayLabelButton,250,100);

        // add keyer controls * Keyer Wight (bits [6:0] (0 - 100))
        KeyerWightLabel=gtk_label_new("Keyer Wight [%]");
        gtk_widget_show(KeyerWightLabel);
        gtk_fixed_put((GtkFixed*)setupKeyerFixed,KeyerWightLabel,10,130);
        KeyerWightLabelButton=gtk_spin_button_new_with_range(0,100,5);
        gtk_spin_button_set_value((GtkSpinButton*)KeyerWightLabelButton,(double)keyer_wight);
        g_signal_connect(G_OBJECT(KeyerWightLabelButton),"value-changed",G_CALLBACK(KeyerWightLabelChanged),NULL);
        gtk_widget_show(KeyerWightLabelButton);
        gtk_fixed_put((GtkFixed*)setupKeyerFixed,KeyerWightLabelButton,250,130);

        // add keyer_spacing controls
        keyerSpacingCheckButton=gtk_check_button_new_with_label("Keyer Spacing");
        gtk_toggle_button_set_active((GtkToggleButton*)keyerSpacingCheckButton,(gboolean)keyer_spacing);
        gtk_widget_show(keyerSpacingCheckButton);
        gtk_fixed_put((GtkFixed*)setupKeyerFixed,keyerSpacingCheckButton,250,180);
        g_signal_connect(G_OBJECT(keyerSpacingCheckButton),"toggled",G_CALLBACK(keyerSpacingChanged),NULL);

        box=gtk_hbox_new(FALSE,5);
        label=gtk_label_new("Key Mode:					");
        gtk_widget_show(label);
        gtk_box_pack_start(GTK_BOX(box),label,FALSE,FALSE,2);
        mode_0=gtk_radio_button_new_with_label(NULL,"Straight		");
        gtk_widget_show(mode_0);
        gtk_box_pack_start(GTK_BOX(box),mode_0,FALSE,FALSE,2);
        g_signal_connect(G_OBJECT(mode_0),"clicked",G_CALLBACK(mode_0ButtonCallback),NULL);
        mode_A=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(mode_0),"Mode A		");
        gtk_widget_show(mode_A);
        gtk_box_pack_start(GTK_BOX(box),mode_A,FALSE,FALSE,2);
        g_signal_connect(G_OBJECT(mode_A),"clicked",G_CALLBACK(mode_AButtonCallback),NULL);
        mode_B=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(mode_0),"Mode B	");
        gtk_widget_show(mode_B);
        gtk_box_pack_start(GTK_BOX(box),mode_B,FALSE,FALSE,2);
        g_signal_connect(G_OBJECT(mode_B),"clicked",G_CALLBACK(mode_BButtonCallback),NULL);
        gtk_widget_show(box);
        gtk_fixed_put((GtkFixed*)setupKeyerFixed,box,10,210);
 
        switch(keyer_mode) {
        case 0:
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(mode_0),TRUE);
            break;
        case 1:
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(mode_A),TRUE);
            break;
        case 2:
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(mode_B),TRUE);
            break;
        }

        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(keyerSpacingCheckButton),keyer_spacing);


        gtk_widget_set_size_request(GTK_WIDGET(setupKeyerFixed),600,510);
        gtk_widget_show(setupKeyerFixed);

    return setupKeyerFixed;

}


/* --------------------------------------------------------------------------*/
/** 
* @brief sidetone changed
* 
* @param spinButton
* @param data
*/
void sidetoneChanged(GtkSpinButton* spinButton,gpointer data) {
    cw_sidetone_frequency =gtk_spin_button_get_value(spinButton);
    ctrlSidetoneChanged();
    setKeyerChanged();
}

/* --------------------------------------------------------------------------*/
/** 
* @brief cw_sidetone_volume
* 
* @param spinButton
* @param data
*/
void SidetoneVolumeLabelChanged(GtkSpinButton* spinButton,gpointer data) {
    cw_sidetone_volume =gtk_spin_button_get_value(spinButton);
    ctrlSidetoneVolumeChanged();
    setKeyerChanged();
}

/* --------------------------------------------------------------------------*/
/** 
* @brief cw_hang_time
* 
* @param spinButton
* @param data
*/
void CWHangTimeLabelChanged(GtkSpinButton* spinButton,gpointer data) {
    cw_hang_time =gtk_spin_button_get_value(spinButton);
    ctrlHangTimeChanged();
    setKeyerChanged();
}

/* --------------------------------------------------------------------------*/
/** 
* @brief cw_ptt_delay
* 
* @param spinButton
* @param data
*/
void CWPTTDelayLabelChanged(GtkSpinButton* spinButton,gpointer data) {
    cw_ptt_delay =gtk_spin_button_get_value(spinButton);
    ctrlPttDelayChanged();
    setKeyerChanged();
}

/* --------------------------------------------------------------------------*/
/** 
* @brief keyer_wight
* 
* @param spinButton
* @param data
*/
void KeyerWightLabelChanged(GtkSpinButton* spinButton,gpointer data) {
    keyer_wight =gtk_spin_button_get_value(spinButton);
    ctrlKeyerWightChanged();
    setKeyerChanged();
}

/** 
* @brief keyer_spacing changed
* 
* @param spinButton
* @param data
*/
void keyerSpacingChanged(GtkToggleButton* button,gpointer data) {
    keyer_spacing=gtk_toggle_button_get_active(button);
    ctrlKeyerSpacingChanged();
    setKeyerChanged();
}

void mode_0ButtonCallback(GtkWidget* widget,gpointer data) {
    if(GTK_TOGGLE_BUTTON(widget)->active) {
      keyer_mode = 0;
      ctrlKeyerModeChanged();
    setKeyerChanged();
    }
}

void mode_AButtonCallback(GtkWidget* widget,gpointer data) {
    if(GTK_TOGGLE_BUTTON(widget)->active) {
       keyer_mode = 1;
      ctrlKeyerModeChanged();
    setKeyerChanged();
    }
}

void mode_BButtonCallback(GtkWidget* widget,gpointer data) {
    if(GTK_TOGGLE_BUTTON(widget)->active) {
        keyer_mode = 2;
      ctrlKeyerModeChanged();
    setKeyerChanged();
    }
}

void updateKeyerSetup() {
   // gtk_spin_button_set_value((GtkSpinButton*)sidetoneButton,(double)cw_sidetone_frequency);
}
