
#include <gtk/gtk.h>
#include <math.h>
#include <stdio.h>

#include "screensize.h"
#include "bandstack.h"
#include "command.h"
#include "zoom.h"
#include "main.h"
#include "property.h"
#include "ozy.h"

GtkWidget* zoomFrame;
GtkWidget* zoomTable;

GtkWidget* zoomButton1;
GtkWidget* zoomButton2;
GtkWidget* zoomButton4;
GtkWidget* currentZoomButton;

int zoom=1;
/* --------------------------------------------------------------------------*/
void selectZoom(GtkWidget* widget) {
    GtkWidget* label;
    char temp[80];

    if(currentZoomButton) {
        label=gtk_bin_get_child((GtkBin*)currentZoomButton);
        gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &white);
        gtk_widget_modify_fg(label, GTK_STATE_PRELIGHT, &black);
    }
    label=gtk_bin_get_child((GtkBin*)widget);
    gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &buttonSelected);
    gtk_widget_modify_fg(label, GTK_STATE_PRELIGHT, &buttonSelected);
    currentZoomButton=widget;

    if(widget==zoomButton1) {
        zoom=1;
    } else if(widget==zoomButton2) {
        zoom=2;
    } else if(widget==zoomButton4) {
        zoom=4;
    } 

}

/* --------------------------------------------------------------------------*/
void setZoom(int zoom) {
    GtkWidget* widget;
    switch(zoom) {
        case 1:
            widget=zoomButton1;
            break;
        case 2:
            widget=zoomButton2;
            break;
        case 4:
            widget=zoomButton4;
            break;
    }
    selectZoom(widget);
}

/* --------------------------------------------------------------------------*/

void zoomButtonCallback(GtkWidget* widget,gpointer data) {
    selectZoom(widget);
}

/* --------------------------------------------------------------------------*/
/** 
* @brief Build Zoom User Interface 
* 
* @return GtkWidget pointer 
*/
GtkWidget* buildZoomUI() {

    GtkWidget* label;


    zoomFrame=gtk_frame_new("Zoom Display");
    gtk_widget_modify_bg(zoomFrame,GTK_STATE_NORMAL,&background);
    gtk_widget_modify_fg(gtk_frame_get_label_widget(GTK_FRAME(zoomFrame)),GTK_STATE_NORMAL,&white);

    zoomTable=gtk_table_new(1,3,TRUE);

    // zoom settings
    zoomButton1 = gtk_button_new_with_label ("1");
    gtk_widget_modify_bg(zoomButton1, GTK_STATE_NORMAL, &buttonBackground);
    label=gtk_bin_get_child((GtkBin*)zoomButton1);
    gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &white);
    gtk_widget_set_size_request(GTK_WIDGET(zoomButton1),BUTTON_WIDTH,BUTTON_HEIGHT);
    g_signal_connect(G_OBJECT(zoomButton1),"clicked",G_CALLBACK(zoomButtonCallback),NULL);
    gtk_widget_show(zoomButton1);
    gtk_table_attach_defaults(GTK_TABLE(zoomTable),zoomButton1,0,1,0,1);

    zoomButton2 = gtk_button_new_with_label ("2");
    gtk_widget_modify_bg(zoomButton2, GTK_STATE_NORMAL, &buttonBackground);
    label=gtk_bin_get_child((GtkBin*)zoomButton2);
    gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &white);
    gtk_widget_set_size_request(GTK_WIDGET(zoomButton2),BUTTON_WIDTH,BUTTON_HEIGHT);
    g_signal_connect(G_OBJECT(zoomButton2),"clicked",G_CALLBACK(zoomButtonCallback),NULL);
    gtk_widget_show(zoomButton2);
    gtk_table_attach_defaults(GTK_TABLE(zoomTable),zoomButton2,1,2,0,1);

    zoomButton4 = gtk_button_new_with_label ("4");
    gtk_widget_modify_bg(zoomButton4, GTK_STATE_NORMAL, &buttonBackground);
    label=gtk_bin_get_child((GtkBin*)zoomButton4);
    gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &white);
    gtk_widget_set_size_request(GTK_WIDGET(zoomButton4),BUTTON_WIDTH,BUTTON_HEIGHT);
    g_signal_connect(G_OBJECT(zoomButton4),"clicked",G_CALLBACK(zoomButtonCallback),NULL);
    gtk_widget_show(zoomButton4);
    gtk_table_attach_defaults(GTK_TABLE(zoomTable),zoomButton4,2,3,0,1);

    gtk_container_add(GTK_CONTAINER(zoomFrame),zoomTable);
    gtk_widget_show(zoomTable);
    gtk_widget_show(zoomFrame);

    setZoom(zoom);
    return zoomFrame;

}
