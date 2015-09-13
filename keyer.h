/** 
* @file keyer.h
* @brief Header files for the keyer functions
* @author Bernd Kernbaum DK3WX
* @version 0.1
* @date 2014-09-03
*/
// keyer.h

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

double cw_sidetone_frequency;
double cw_hang_time;
double cw_ptt_delay;
double cw_sidetone_volume;
gboolean cw_ext;
double keyer_wight;
gboolean keyer_spacing;
unsigned char keyer_mode;
double keyer_speed;

int keyChanged_16;
int keyChanged_1E;
int keyChanged_20;
unsigned char control_16[5]; // Keyer Mode Speed Weight Spacing
unsigned char control_1E[5]; // Keyer CW int Sidetone Vol PTT delay
unsigned char control_20[5]; // Keyer CW Hang time Sidetone Frequ.

int aRit;
int aRitChanged;

void keyerSaveState();
void keyerRestoreState();
GtkWidget* buildKeyerUI();
GtkWidget* buttonCW;

void ctrlSidetoneChanged(void);
void ctrlSidetoneVolumeChanged(void);
void ctrlHangTimeChanged(void);
void ctrlPttDelayChanged(void);
void ctrlKeyerWightChanged(void);
void ctrlKeyerSpacingChanged(void);
void ctrlKeyerModeChanged(void);
