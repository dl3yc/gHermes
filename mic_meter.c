
// mic_meter.c

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "screensize.h"
#include "main.h"
#include "mic_meter.h"
//#include "mic_meter_update.h"
#include "property.h"
#include "ozy.h"
#include "preamp.h"

GtkWidget* mic_meterFixed;
GtkWidget* mic_meter;
GdkPixmap* mic_meterPixmap;

//GtkWidget* buttonSIGNAL;
//GtkWidget* buttonSAV;

int mic_meterPeak=0;
int mic_meterPeakCount=0;
int mic_meterPeakSamples=10;

int mic_meterX=0;

//void plotSignal(float* samples);
//void drawSignal();
//void updateOff();


/* --------------------------------------------------------------------------*/
/** 
* @brief  Callback when mic_meter is created
* 
* @param widget
* @param event
* 
* @return 
*/
gboolean mic_meter_configure_event(GtkWidget* widget,GdkEventConfigure* event) {
    GdkGC* gc;

    if(mic_meterPixmap) g_object_unref(mic_meterPixmap);

    mic_meterPixmap=gdk_pixmap_new(widget->window,widget->allocation.width,widget->allocation.height,-1);

    gc=gdk_gc_new(widget->window);
    gdk_gc_set_rgb_fg_color(gc,&black);
    gdk_draw_rectangle(mic_meterPixmap,
                       gc,
                       TRUE,
                       0,0,
                       widget->allocation.width,
                       widget->allocation.height);

    g_object_unref(gc);

    return TRUE;
}

/* --------------------------------------------------------------------------*/
/** 
* @brief Callback when mic_meter is exposed - paint it from the pixmap
* 
* @param widget
* @param event
* 
* @return 
*/
gboolean mic_meter_expose_event(GtkWidget* widget,GdkEventExpose* event) {
    gdk_draw_drawable(widget->window,
                    widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
                    mic_meterPixmap,
                    event->area.x, event->area.y,
                    event->area.x, event->area.y,
                    event->area.width, event->area.height);
    return FALSE;
}


/* --------------------------------------------------------------------------*/
/** 
* @brief Draw the mic_meter signal 
*/
void mic_meterDrawSignal() {

    // get the mic_meter context - just copy the window GC and modify
    GdkGC* gc;
    PangoContext *context;
    PangoLayout *layout;
    char temp[128];

    int i;
    if(mic_meter->window) {

        gc=gdk_gc_new(mic_meter->window);
        gdk_gc_copy(gc,mic_meter->style->black_gc);

        // black background
        gdk_draw_rectangle(mic_meterPixmap,gc,TRUE,0,0,mic_meter->allocation.width,mic_meter->allocation.height);

        // draw the mic_meter
        gdk_gc_set_rgb_fg_color(gc,&green);
        gdk_draw_rectangle(mic_meterPixmap,gc,TRUE,20,0,mic_meterX,mic_meter->allocation.height/2);
	
        // draw the levels
        context = gdk_pango_context_get_for_screen(gdk_screen_get_default ());
        layout = pango_layout_new(context);
        pango_layout_set_width(layout,120*PANGO_SCALE);
        pango_layout_set_alignment(layout,PANGO_ALIGN_LEFT);
        sprintf(temp,"<span foreground='#7AAA6E' background='#000000' font_desc='Sans 6'>    0.2   0.4   0.6   0.8   1</span>");
        pango_layout_set_markup(layout,temp,-1);
        gdk_draw_layout(GDK_DRAWABLE(mic_meterPixmap),gc,20,20,layout);


        // update the peak
        if(mic_meterX>mic_meterPeak) {
            mic_meterPeak=mic_meterX;
            mic_meterPeakCount=0;
        }
        if(mic_meterPeakCount++ >= mic_meterPeakSamples) {
            mic_meterPeak=mic_meterX;
            mic_meterPeakCount=0;
        }
        gdk_gc_set_rgb_fg_color(gc,&red);
        gdk_draw_line(mic_meterPixmap,gc,20+mic_meterPeak,0,20+mic_meterPeak,mic_meter->allocation.height/2);
        gdk_draw_line(mic_meterPixmap,gc,20+mic_meterPeak-1,0,20+mic_meterPeak-1,mic_meter->allocation.height/2);

        gdk_gc_set_rgb_fg_color(gc,&grey);
        gdk_draw_rectangle(mic_meterPixmap,gc,FALSE,0,0,mic_meter->allocation.width-1,mic_meter->allocation.height-1);

        //g_object_unref(context);
        //g_object_unref(layout);
        g_object_unref(gc);

        gtk_widget_queue_draw(mic_meter);
    }
}

/* --------------------------------------------------------------------------*/
/** 
* @brief Plot the mic_meter signal 
* 
* @param sample
*/
void mic_meterPlotSignal(float sample) {

    // plot the mic_meter
    float val=sample *100;
   // val+=multimic_meterCalibrationOffset + getFilterSizeCalibrationOffset()+preampOffset;
   // Soundcard entfernt festen Wert für HPSDR -41.0f
   // val+=-41.0f + getFilterSizeCalibrationOffset()+preampOffset;

    //mic_meterDbm=(int)val;
    mic_meterX=(int)val;
    //mic_meter *= 100;
    if(mic_meterX<=0) mic_meterX=1;
}

/* --------------------------------------------------------------------------*/
/** 
* @brief Turn off the mic_meter update
*/
void mic_meterUpdateOff() {

    // get the mic_meter context - just copy the window GC and modify
    GdkGC* gc;
    if(mic_meter->window) {
        gc=gdk_gc_new(mic_meter->window);
        gdk_gc_copy(gc,mic_meter->style->black_gc);
        gdk_draw_rectangle(mic_meterPixmap,gc,TRUE,0,0,mic_meter->allocation.width,mic_meter->allocation.height);

        // update the mic_meter
        gtk_widget_queue_draw_area(mic_meter,0,0,mic_meter->allocation.width,mic_meter->allocation.height);
    }
}

void updateMic_Meter(float sample) {
           mic_meterPlotSignal(sample);
           mic_meterDrawSignal();
       //     mic_meterDbmDrawSignal();
}



/* --------------------------------------------------------------------------*/
/** 
* @brief Build the mic_meter user interface
* 
* @return 
*/
GtkWidget* buildMic_MeterUI() {
   mic_meterFixed=gtk_fixed_new();
    gtk_widget_modify_bg(mic_meterFixed,GTK_STATE_NORMAL,&background);
    //gtk_widget_modify_bg(mic_meterFixed,GTK_STATE_NORMAL,&black);

    // mic_meter
    mic_meter=gtk_drawing_area_new();
    gtk_widget_set_size_request(GTK_WIDGET(mic_meter),160,30);
    g_signal_connect(G_OBJECT (mic_meter),"configure_event",G_CALLBACK(mic_meter_configure_event),NULL);
    g_signal_connect(G_OBJECT (mic_meter),"expose_event",G_CALLBACK(mic_meter_expose_event),NULL);
    gtk_widget_show(mic_meter);
    gtk_fixed_put((GtkFixed*)mic_meterFixed,mic_meter,0,0);
    gtk_widget_set_size_request(GTK_WIDGET(mic_meterFixed),160,35);
    gtk_widget_show(mic_meterFixed);

    return mic_meterFixed;
}


