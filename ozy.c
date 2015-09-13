/**
 *modified Software   
 *author Bernd Kernbaum, DK3WX
 *email dk3wx@darc.de 
 * 
 *version 0.2
 *date 2014-10-5
 *
*/

/* 
 * File:   ozy.c
 * Author: jm57878
 *
 * Created on 10 March 2009, 20:26
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>
#include <time.h>
#include <sys/timeb.h>
#include <math.h>
#include <gtk/gtk.h>

#include "ozy.h"
#include "ozy_buffers.h"
#include "ozy_ringbuffer.h"
#include "property.h"
#include "spectrum_buffers.h"
#include "dttsp.h"
#include "util.h"

#include "filter.h"
#include "volume.h"
#include "mode.h"
#include "audiostream.h"
#include "transmit.h"
#include "main.h"
#include "vfo.h"
#include "metis.h"
#include "twotone.h"
#include "keyer.h"

//#define OZY_BUFFERS

/*
 *   ozy interface
 */

#define USB_TIMEOUT -7
//static struct OzyHandle* ozy;

static char ozy_firmware_version[9];
int mercury_software_version=0;
int penelope_software_version=0;
int ozy_software_version=0;

int penelopeForwardPower=0;
int alexForwardPower=0;
int alexReversePower=0;
int AIN3=0;
int AIN4=0;
int AIN6=0;
int IO1=1; // 1 is inactive
int IO2=1;
int IO3=1;

static pthread_t ep6_ep2_io_thread_id;
static pthread_t ep4_io_thread_id;
static pthread_t ozy_input_buffer_thread_id;
static pthread_t ozy_spectrum_buffer_thread_id;

static long rxFrequency=7056000;
static int rxFrequency_changed=1;
static long txFrequency=7056000;
static int txFrequency_changed=1;

unsigned char output_buffer[OZY_BUFFER_SIZE];
int output_buffer_index=8;

unsigned char bandscope_buffer[8192];
int bandscope_buffer_index=0;

static int force_write=0;

static unsigned char control_in[5]={0x00,0x00,0x00,0x00,0x00};

unsigned char control_out[5]={0x00,0x00,0x00,0x00,0x00};

int output_sample_increment=2; // 1=48000 2=96000 4=192000

int buffer_size=BUFFER_SIZE;

float left_input_buffer[BUFFER_SIZE];
float right_input_buffer[BUFFER_SIZE];

float mic_left_buffer[BUFFER_SIZE];
float mic_right_buffer[BUFFER_SIZE];

float left_output_buffer[BUFFER_SIZE];
float right_output_buffer[BUFFER_SIZE];

float left_tx_buffer[BUFFER_SIZE];
float right_tx_buffer[BUFFER_SIZE];


int samples=0;

int left_sample;
int right_sample;
int mic_sample;


float left_sample_float;
float right_sample_float;
float mic_sample_float;

short left_rx_sample;
short right_rx_sample;
short left_tx_sample;
short right_tx_sample;

int frames=0;
int usb_output_buffers=0;

int show_software_serial_numbers=1;

unsigned char spectrum_samples[SPECTRUM_BUFFER_SIZE];

int lt2208ADCOverflow=0;

int speed=1;           // default 96K
int class=0;           // default other
int lt2208Dither=1;    // default dither on
int lt2208Random=1;    // default random 0n
int alexAttenuation=0; // default alex attenuation 0Db
int micSource=1;       // default mic source Penelope
int clock10MHz=2;      // default 10 MHz clock source Mercury
int clock122_88MHz=1;  // default 122.88 MHz clock source Mercury
int preamp=0;          // default preamp off

int sampleRate=96000;  // default 48k

int mox=0;
int ptt=0;
int dot=0;
int dash=0;

int xmit=0;  // not transmitting

int twotone=0;       // 
int driveLevel=255;    // default is max
int driveLevelChanged=0; // force drive level to be sent
int micboost=0;       // Microfon vorverstärker

int timing=0;
static struct timeb start_time;
static struct timeb end_time;
static int sample_count=0;

static int metis=1;
static char interface[128];

int alexRxAntenna=0;
int alexTxAntenna=0;
int alexRxOnlyAntenna=0;

float vswr=0.0;
float mic_meter_float= 0.8;

int ritFrequencyAChanged=0;
int dbTest = 0;
int tone1 = 600;
int tone2 = 800;

void ozy_set_metis() {
    metis=1;
}

void ozy_set_interface(char* iface) {
    strcpy(interface,iface);
}

void setTwoTone(int state) {
    twotone=state;
}

void setMicBoost(int state) {
    micboost=state;
}

void setDriveLevelChanged(int level) {
    driveLevel=level;
    driveLevelChanged=1;
}

void setKeyerChanged(viod) {
   keyChanged_20=1;
}

char* ozy_get_interface() {
    return interface;
}

void process_bandscope_buffer(char* buffer) {
    int i;

    for(i=0;i<512;i++) {
        bandscope_buffer[bandscope_buffer_index++]=buffer[i];
    }

    if(bandscope_buffer_index>=SPECTRUM_BUFFER_SIZE) {
        memcpy(spectrum_samples,bandscope_buffer,SPECTRUM_BUFFER_SIZE);
        bandscope_buffer_index=0;
    }
}

/* --------------------------------------------------------------------------*/
/** 
* @brief Process the ozy input buffer
* 
* @param buffer
*/
void process_ozy_input_buffer(char* buffer) {
    int i,j;
    int b=0;
    unsigned char ozy_samples[8*8];
    int bytes;
    double gain;
    int sum;

    if(buffer[b++]==SYNC && buffer[b++]==SYNC && buffer[b++]==SYNC) {
        // extract control bytes
        control_in[0]=buffer[b++];
        control_in[1]=buffer[b++];
        control_in[2]=buffer[b++];
        control_in[3]=buffer[b++];
        control_in[4]=buffer[b++];

        ptt=(control_in[0]&0x01)==0x01;
        dash=(control_in[0]&0x02)==0x02;
        dot=(control_in[0]&0x04)==0x04;

        xmit=mox|ptt|dot|dash;

        int *vfoState=malloc(sizeof(int));
        *vfoState=xmit;
        g_idle_add(vfoTransmit,(gpointer)vfoState);

//if(xmit!=last_xmit) {
//    fprintf(stderr,"ozy: xmit=%d mox=%d ptt=%d dot=%d dash=%d\n", xmit,mox,ptt,dot,dash);
//    fprintf(stderr,"ozy: testing=%d tuning=%d\n", testing,tuning);
//}
        switch((control_in[0]>>3)&0x1F) {

        case 0:
            lt2208ADCOverflow=control_in[1]&0x01;
            IO1=(control_in[1]&0x02)?0:1;
            IO2=(control_in[1]&0x04)?0:1;
            IO3=(control_in[1]&0x08)?0:1;
            if(mercury_software_version!=control_in[2]) {
                mercury_software_version=control_in[2];
                fprintf(stderr,"  Mercury Software version: %d (0x%0X)\n",mercury_software_version,mercury_software_version);
            }
            if(penelope_software_version!=control_in[3]) {
                penelope_software_version=control_in[3];
                fprintf(stderr,"  Penelope Software version: %d (0x%0X)\n",penelope_software_version,penelope_software_version);
            }
            if(ozy_software_version!=control_in[4]) {
                ozy_software_version=control_in[4];
                fprintf(stderr,"  Ozy Software version: %d (0x%0X)\n",ozy_software_version,ozy_software_version);
            }
            break;
        case 1:
            penelopeForwardPower=(control_in[1]<<8)+control_in[2]; // from Penelope or Hermes
            
            alexForwardPower=(control_in[3]<<8)+control_in[4]; // from Alex or Apollo
            break;
        case 2:
            alexReversePower=(control_in[1]<<8)+control_in[2]; // from Alex or Apollo
            AIN3=(control_in[3]<<8)+control_in[4]; // from Pennelope or Hermes
            break;
        case 3:
            AIN4=(control_in[1]<<8)+control_in[2]; // from Pennelope or Hermes
            AIN6=(control_in[3]<<8)+control_in[4]; // from Pennelope or Hermes
            break;
        }

if(xmit) {
    float fwd=(float)alexForwardPower/100.0F;
    float rev=(float)alexReversePower/100.0F;

    float gamma=sqrt(rev/fwd);
    vswr=(1.0F+gamma)/(1.0F-gamma);
    //fprintf(stderr,"fwd=%f rev=%f vswr=%f\n",fwd,rev,vswr);
}
        // extract the 63 samples
        sum=0;
        for(i=0;i<63;i++) {

            left_sample   = (int)((signed char) buffer[b++]) << 16;
            left_sample  += (int)((unsigned char)buffer[b++]) << 8;
            left_sample  += (int)((unsigned char)buffer[b++]);
            right_sample  = (int)((signed char) buffer[b++]) << 16;
            right_sample += (int)((unsigned char)buffer[b++]) << 8;
            right_sample += (int)((unsigned char)buffer[b++]);
            mic_sample    = (int)((signed char) buffer[b++]) << 8;
            mic_sample   += (int)((unsigned char)buffer[b++]);

	    
            left_sample_float=(float)left_sample/8388607.0; // 24 bit sample
            right_sample_float=(float)right_sample/8388607.0; // 24 bit sample
            mic_sample_float=((float)mic_sample/32767.0)*micGain*10; // 16 bit sample
	    if(sum < mic_sample){sum = mic_sample;};

            // add to buffer
            if(xmit) { // mute the input
                left_input_buffer[samples]=0.0;
                right_input_buffer[samples]=0.0;
            } else {
                left_input_buffer[samples]=left_sample_float;
                right_input_buffer[samples]=right_sample_float;
            }

            if(control_1E[1]) {
                mic_left_buffer[samples]=0.0f;
                mic_right_buffer[samples]=0.0f;
            } else {
                mic_left_buffer[samples]=mic_sample_float;
                mic_right_buffer[samples]=0.0f;
            }
            samples++;

            if(timing) {
                sample_count++;
                if(sample_count==sampleRate) {
                    ftime(&end_time);
                    fprintf(stderr,"%d samples in %ld ms\n",sample_count,((end_time.time*1000)+end_time.millitm)-((start_time.time*1000)+start_time.millitm));
                    sample_count=0;
                    ftime(&start_time);
                }
            }

            //key_thread_process(1.0, dash, dot, TRUE);

            // when we have enough samples give them to DttSP and get the results
            if(samples==buffer_size) {
                // process the input
                Audio_Callback (left_input_buffer,right_input_buffer,
                                left_output_buffer,right_output_buffer, buffer_size, 0);

                // transmit
                if(xmit) {
                    if(tuning) {		                  
                         tuningPhase1 =sineWave(mic_left_buffer,buffer_size,tuningPhase1,(double)tone1);
			  if (twotone) {tuningPhase2 =addWave(mic_left_buffer,buffer_size,tuningPhase2,(double)tone2);};
                        //tuningPhase=sineWave(mic_right_buffer,buffer_size,tuningPhase,(double)cwPitch);
                    } else if(testing) {
                        // leave alone
                    } else if(mode==modeCWU || mode==modeCWL) {
                        // CWtoneExchange(mic_left_buffer, mic_right_buffer, buffer_size);
                    }
                }

                // process the output
                Audio_Callback (mic_left_buffer,mic_right_buffer,
                                left_tx_buffer,right_tx_buffer, buffer_size, 1);


                gain= 0.9; //1.0;wegen Übersteuerung

                for(j=0;j<buffer_size;j+=output_sample_increment) {
                    left_rx_sample=(short)(left_output_buffer[j]*32767.0);
                    right_rx_sample=(short)(right_output_buffer[j]*32767.0);

                    if(xmit) {
                        left_tx_sample=(short)(left_tx_buffer[j]*32767.0*gain);
                        right_tx_sample=(short)(right_tx_buffer[j]*32767.0*gain);
                    } else {
                        left_tx_sample=0;
                        right_tx_sample=0;
                    }

                    output_buffer[output_buffer_index++]=left_rx_sample>>8;
                    output_buffer[output_buffer_index++]=left_rx_sample;
                    output_buffer[output_buffer_index++]=right_rx_sample>>8;
                    output_buffer[output_buffer_index++]=right_rx_sample;
                    output_buffer[output_buffer_index++]=left_tx_sample>>8;
                    output_buffer[output_buffer_index++]=left_tx_sample;
                    output_buffer[output_buffer_index++]=right_tx_sample>>8;
                    output_buffer[output_buffer_index++]=right_tx_sample;

                    if(output_buffer_index>=OZY_BUFFER_SIZE) {
                        output_buffer[0]=SYNC;
                        output_buffer[1]=SYNC;
                        output_buffer[2]=SYNC;

                        // set mox
                       control_out[0]=control_out[0]&0xFE;
                       control_out[0]=control_out[0]|(xmit&0x01);  
                        
                        if(splitChanged || aRitChanged) {
                            output_buffer[3]=control_out[0];
                            output_buffer[4]=control_out[1];
                            output_buffer[5]=control_out[2];
                            output_buffer[6]=control_out[3];
			   output_buffer[7]=control_out[4];
                            if(aRit) {
                                output_buffer[7]=control_out[4]|0x04;
                                if (!bSplit) {ritFrequencyAChanged =1;}
			       
                            }
                             if(bSplit) {
                                output_buffer[7]=control_out[4]|0x04;
			      
                            }
                            if (splitChanged) {splitChanged=0;}
                            if (aRitChanged) {aRitChanged=0;}
			    fprintf(stderr,"Con:  %d\n", output_buffer[7]);
			    
                        } else if(frequencyAChanged) {
		  	    fprintf(stderr,"rit:  %d\n",aRit);
                 	    fprintf(stderr,"split:  %d\n",bSplit);

                                output_buffer[3]=control_out[0]|0x02; // Mercury and Penelope
                            if(bSplit || aRit) {
                                output_buffer[3]=control_out[0]|0x04;   // Mercury (1)
                                if (!bSplit) {ritFrequencyAChanged =1;}
                             }
                            output_buffer[4]=ddsAFrequency>>24;
                            output_buffer[5]=ddsAFrequency>>16;
                            output_buffer[6]=ddsAFrequency>>8;
                            output_buffer[7]=ddsAFrequency;
                            frequencyAChanged=0;
			       fprintf(stderr,"ACh:  %d\n", output_buffer[3]);
                                fprintf(stderr,"ddsAFrequency=%d\n",ddsAFrequency);
                        } else if(ritFrequencyAChanged) {
                                output_buffer[3]=control_out[0]|0x02; // Penelope
                                output_buffer[4]=ddsAPlusRit>>24;
                                output_buffer[5]=ddsAPlusRit>>16;
                                output_buffer[6]=ddsAPlusRit>>8;
                                output_buffer[7]=ddsAPlusRit;
                                ritFrequencyAChanged=0;
                                fprintf(stderr,"ddsAPlusRit  =%d\n",ddsAPlusRit);
                        } else if(frequencyBChanged) {
                            if(bSplit) {
                                output_buffer[3]=control_out[0]|0x02; // Penelope
                                output_buffer[4]=ddsBFrequency>>24;
                                output_buffer[5]=ddsBFrequency>>16;
                                output_buffer[6]=ddsBFrequency>>8;
                                output_buffer[7]=ddsBFrequency;
                             //   fprintf(stderr,"ddsBFrequency=%d\n",ddsBFrequency);
                            }
                            frequencyBChanged=0;
                        } else if(driveLevelChanged) { 
                            output_buffer[3]=0x12|(xmit&0x01);
                            output_buffer[4]=driveLevel;
                            output_buffer[5]=micboost;
                            output_buffer[6]=0;
                            output_buffer[7]=0;
                            driveLevelChanged=0;
                        } else if(keyChanged_16) { 
                            output_buffer[3]=control_16[0];
                            output_buffer[4]=control_16[1];
                            output_buffer[5]=control_16[2];
                            output_buffer[6]=control_16[3];
                            output_buffer[7]=control_16[4];
                            keyChanged_16=0;
                        } else if(keyChanged_1E) { 
                            output_buffer[3]=control_1E[0];
                            output_buffer[4]=control_1E[1];
                            output_buffer[5]=control_1E[2];
                            output_buffer[6]=control_1E[3];
                            output_buffer[7]=control_1E[4];
                            keyChanged_16=1;
                            keyChanged_1E=0;
                        } else if(keyChanged_20) { 
                            output_buffer[3]=control_20[0];
                            output_buffer[4]=control_20[1];
                            output_buffer[5]=control_20[2];
                            output_buffer[6]=control_20[3];
                            output_buffer[7]=control_20[4];
                            keyChanged_1E=1;
                            keyChanged_20=0;
                        } else {
                            output_buffer[3]=control_out[0];
                            output_buffer[4]=control_out[1];
                            output_buffer[5]=control_out[2];
                            output_buffer[6]=control_out[3];
                            output_buffer[7]=control_out[4];
                            if(bSplit) { output_buffer[7]=control_out[4]|0x04;} 
                            if(aRit)   { output_buffer[7]=control_out[4]|0x04;} 
                        }

                        metis_write(0x02,output_buffer,OZY_BUFFER_SIZE);
                        output_buffer_index=8;
                    }

                }
                dbTest=0;
                samples=0;
                frames++;
            }
        }
        mic_meter_float=((float)sum/32767.0f); // 16 bit sample

    } else {
        time_t t;
        struct tm* gmt;
        time(&t);
        gmt=gmtime(&t);

        fprintf(stderr,"%s: process_ozy_input_buffer: did not find sync\n",
                asctime(gmt));
        dump_ozy_buffer("input buffer",buffer);
        exit(1);
    }
}

void ozy_prime() {
    int i;
    int bytes;


    output_buffer[0]=SYNC;
    output_buffer[1]=SYNC;
    output_buffer[2]=SYNC;
    output_buffer[3]=control_out[0];
    output_buffer[4]=control_out[1];
    output_buffer[5]=control_out[2];
    output_buffer[6]=control_out[3];
    output_buffer[7]=control_out[4];

    for(i=8;i<OZY_BUFFER_SIZE;i++) {
        output_buffer[i]=0;
    }

    for(i=0;i<2;i++) {
      metis_write(0x02,output_buffer,OZY_BUFFER_SIZE);
    }
}

/* --------------------------------------------------------------------------*/
/** 
* @brief Ozy input buffer thread
* 
* @param arg
* 
* @return 
*/
void* ozy_input_buffer_thread(void* arg) {
    struct ozy_buffer* buffer;

    while(1) {
        // wait for an ozy buffer
        sem_wait(ozy_input_buffer_sem);
        buffer=get_ozy_input_buffer();
        if(buffer==NULL) {
            fprintf(stderr,"ozy_input_buffer_thread: get_ozy_buffer returned NULL!\n");
        } else {
            process_ozy_input_buffer(buffer->buffer);
            free_ozy_buffer(buffer);
        }

    }
}

/* --------------------------------------------------------------------------*/
/** 
* @brief Ozy spectrum buffer thread
* 
* @param arg
* 
* @return 
*/
void* ozy_spectrum_buffer_thread(void* arg) {
    struct spectrum_buffer* spectrum_buffer;
    while(1) {
        sem_wait(spectrum_input_buffer_sem);
        spectrum_buffer=get_spectrum_input_buffer();
        memcpy(spectrum_samples,spectrum_buffer->buffer,SPECTRUM_BUFFER_SIZE);
        free_spectrum_buffer(spectrum_buffer);
    }
}


/* --------------------------------------------------------------------------*/
/** 
* @brief Get the spectrum samples
* 
* @param samples
*/
void getSpectrumSamples(char *samples) {
    memcpy(samples,spectrum_samples,SPECTRUM_BUFFER_SIZE);
}

/* --------------------------------------------------------------------------*/
/** 
* @brief Set the MOX
* 
* @param state
*/
void setMOX(int state) {
    mox=state;
}

/* --------------------------------------------------------------------------*/
/** 
* @brief Set the speed
* 
* @param speed
*/
void setSpeed(int s) {
    speed=s;
    control_out[1]=control_out[1]&0xFC;
    control_out[1]=control_out[1]|s;
    if(s==SPEED_48KHZ) {
        output_sample_increment=1;
        sampleRate=48000;
    } else if(s==SPEED_96KHZ) {
        output_sample_increment=2;
        sampleRate=96000;
    } else if(s==SPEED_192KHZ) {
        output_sample_increment=4;
        sampleRate=192000;
    }

    SetSampleRate((double)sampleRate);
    SetRXOsc(0,0,0.0);
    SetTXOsc(1,0.0);
    setFilter(filter);
    setMode(mode);
    SetRXOutputGain(0,0,volume/100.0);
    //SetKeyerSampleRate((float)sampleRate);
}

/* --------------------------------------------------------------------------*/
/** 
* @brief Set the 10 mHz source
* 
* @param source
*/
void set10MHzSource(int source) {
    clock10MHz=source;
    control_out[1]=control_out[1]&0xF3;
    control_out[1]=control_out[1]|(clock10MHz<<2);
}

/* --------------------------------------------------------------------------*/
/** 
* @brief Set the 122 mHz source
* 
* @param source
*/
void set122MHzSource(int source) {
    clock122_88MHz=source;
    control_out[1]=control_out[1]&0xEF;
    control_out[1]=control_out[1]|(source<<4);
}

/* --------------------------------------------------------------------------*/
/** 
* @brief Set the configuration
* 
* @param config
*/
void setConfig(int config) {
    control_out[1]=control_out[1]&0x9F;
    control_out[1]=control_out[1]|(config<<5);
}


/* --------------------------------------------------------------------------*/
/** 
* @brief Set the mic source
* 
* @param source
*/
void setMicSource(int source) {
    micSource=source;
    control_out[1]=control_out[1]&0x7F;
    control_out[1]=control_out[1]|(source<<7);
}

/* --------------------------------------------------------------------------*/
/** 
* @brief Set the class
* 
* @param class
*/
void setClass(int c) {
    class=c;
    control_out[2]=control_out[2]&0xFE;
    control_out[2]=control_out[2]|c;
}

/* --------------------------------------------------------------------------*/
/** 
* @brief Set the OC outputs
* 
* @param outputs
*/
void setOCOutputs(int outputs) {
    control_out[2]=control_out[2]&0x01;
    control_out[2]=control_out[2]|(outputs<<1);
}

/* --------------------------------------------------------------------------*/
/** 
* @brief Set the Alex attenuation
* 
* @param attenuation
*/
void setAlexAttenuation(int attenuation) {
    alexAttenuation=attenuation;
    control_out[3]=control_out[3]&0xFC;
    control_out[3]=control_out[3]|attenuation;
}

/* --------------------------------------------------------------------------*/
/** 
* @brief Set the preamplifer gain
* 
* @param gain
*/
void setPreamp(int p) {
    preamp=p;
    control_out[3]=control_out[3]&0xFB;
    control_out[3]=control_out[3]|(p<<2);
}

/* --------------------------------------------------------------------------*/
/** 
* @brief Set the LT2208 dither
* 
* @param dither
*/
void setLT2208Dither(int dither) {
    lt2208Dither=dither;
    control_out[3]=control_out[3]&0xF7;
    control_out[3]=control_out[3]|(dither<<3);
}

/* --------------------------------------------------------------------------*/
/** 
* @brief Set the LT2208 random
* 
* @param random
*/
void setLT2208Random(int random) {
    lt2208Random=random;
    control_out[3]=control_out[3]&0xEF;
    control_out[3]=control_out[3]|(random<<4);
}

/* --------------------------------------------------------------------------*/
/** 
* @brief Initialize Ozy
* 
* @param sample_rate
* 
* @return 
*/
int ozy_init() {
    int rc;

    fprintf(stderr,"ozy_init\n");

    //
    setSpeed(speed);

    // setup defaults
    control_out[0] = MOX_DISABLED;
    control_out[1] = CONFIG_BOTH
            | MERCURY_122_88MHZ_SOURCE
            | MERCURY_10MHZ_SOURCE
            | speed
            | MIC_SOURCE_PENELOPE;
    control_out[2] = MODE_OTHERS;
    control_out[3] = ALEX_ATTENUATION_0DB
            | LT2208_GAIN_OFF
            | LT2208_DITHER_ON
            | LT2208_RANDOM_ON;
    control_out[4] = 0;

    ozyRestoreState();

    tuningPhase1=0.0;
    tuningPhase2=0.0;

 
   int found;
   long i;
   metis_discover(interface);
   for(i=0;i<30000;i++) {
            if(metis_found()>0) {
                fprintf(stderr,"Metis discovered after %ld\n",i);
                break;
            }
        }
        if(metis_found()<=0) {
            return (-3);
        }

    
    force_write=0;

#ifdef OZY_BUFFERS
    // create buffers of ozy
    create_ozy_ringbuffer(128*512);
    create_ozy_buffers(128);
#endif
    create_spectrum_buffers(8);
    


    metis_start_receive_thread();



    return rc;
}


/* --------------------------------------------------------------------------*/
/** 
* @brief Get the ADC Overflow 
* 
* @return 
*/
int getADCOverflow() {
    int result=lt2208ADCOverflow;
    lt2208ADCOverflow=0;
    return result;
}

/* --------------------------------------------------------------------------*/
/** 
* @brief Get Ozy FX2 firmware version
* 
* @return 
*/
char* get_ozy_firmware_version() {
    return ozy_firmware_version;
}

/* --------------------------------------------------------------------------*/
/** 
* @brief Get Mercury software version
* 
* @return 
*/
int get_mercury_software_version() {
    return mercury_software_version;
}

/* --------------------------------------------------------------------------*/
/** 
* @brief Get Penelope software version
* 
* @return 
*/
int get_penelope_software_version() {
    return penelope_software_version;
}

/* --------------------------------------------------------------------------*/
/** 
* @brief Get Ozy software version
* 
* @return 
*/
int get_ozy_software_version() {
    return ozy_software_version;
}

/* --------------------------------------------------------------------------*/
/** 
* @brief save Ozy state
* 
* @return 
*/
void ozySaveState() {
    char string[128];
    sprintf(string,"%d",clock10MHz);
    setProperty("10MHzClock",string);
    sprintf(string,"%d",clock122_88MHz);
    setProperty("122_88MHzClock",string);
    sprintf(string,"%d",micSource);
    setProperty("micSource",string);
    sprintf(string,"%d",class);
    setProperty("class",string);
    sprintf(string,"%d",lt2208Dither);
    setProperty("lt2208Dither",string);
    sprintf(string,"%d",lt2208Random);
    setProperty("lt2208Random",string);
    sprintf(string,"%d",alexAttenuation);
    setProperty("alexAttenuation",string);
    sprintf(string,"%d",preamp);
    setProperty("preamp",string);
    sprintf(string,"%d",speed);
    setProperty("speed",string);
    sprintf(string,"%d",sampleRate);
    setProperty("sampleRate",string);
    sprintf(string,"%d",twotone);
    setProperty("twoTone",string);
    sprintf(string,"%d",micboost);
    setProperty("micBoost",string);

}

/* --------------------------------------------------------------------------*/
/** 
* @brief resore Ozy state
* 
* @return 
*/
void ozyRestoreState() {
    char *value;

    value=getProperty("10MHzClock");
    if(value) {
        set10MHzSource(atoi(value));
    }
    value=getProperty("122_88MHzClock");
    if(value) {
        set122MHzSource(atoi(value));
    }
    value=getProperty("micSource");
    if(value) {
        setMicSource(atoi(value));
    }
    value=getProperty("class");
    if(value) {
        //setMode(atoi(value));
        setClass(atoi(value));
    }
    value=getProperty("lt2208Dither");
    if(value) {
        setLT2208Dither(atoi(value));
    }
    value=getProperty("lt2208Random");
    if(value) {
        setLT2208Random(atoi(value));
    }
    value=getProperty("alexAttenuation");
    if(value) {
        setAlexAttenuation(atoi(value));
    }
    value=getProperty("preamp");
    if(value) {
        setPreamp(atoi(value));
    }
    value=getProperty("speed");
    if(value) {
        setSpeed(atoi(value));
    }
    value=getProperty("sampleRate");
    if(value) {
        sampleRate=atoi(value);
    }
    value=getProperty("twoTone");
    if(value) {
        twotone=atoi(value);
    }
    value=getProperty("micBoost");
    if(value) {
        micboost=atoi(value);
    }
}

void setAlexRxAntenna(int a) {
    alexRxAntenna=a;
    if(!xmit) {
        control_out[4]=control_out[4]&0xFC;
        control_out[4]=control_out[4]|a;
    }
}

void setAlexTxAntenna(int a) {
    alexTxAntenna=a;
    if(xmit) {
        control_out[4]=control_out[4]&0xFC;
        control_out[4]=control_out[4]|a;
    }
}

void setAlexRxOnlyAntenna(int a) {
    alexRxOnlyAntenna=a;
    if(!xmit) {
        control_out[3]=control_out[3]&0x9F;
        control_out[3]=control_out[3]|(a<<5);

        if(a!=0) {
            control_out[3]=control_out[3]|0x80;
        } else {
            control_out[3]=control_out[3]&0x7F;
        }
    }
}
