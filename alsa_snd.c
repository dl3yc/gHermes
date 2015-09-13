/** 
* @file alsa_snd.c
* @brief Alsa Sound System
* @author Bernd Kernbaum DK3WX
* @version 0.1
* @date 2014-11-18
*/
//alsa_snd.c

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

#define ALSA_PCM_NEW_HW_PARAMS_API
#include <stdio.h>
#include <math.h>

#include <alsa/asoundlib.h>

int main() {
  long loops;
  int rc;
  int size;
  snd_pcm_t *handle;
  snd_pcm_hw_params_t *params;
  unsigned int val;
  int dir;
  snd_pcm_uframes_t frames;
  char *buffer;
  int i;
  short cwSin;
  double deltaf;
  double phase;
  int vu;
  
  /* Open PCM device for playback. */
  rc = snd_pcm_open(&handle, "default",SND_PCM_STREAM_PLAYBACK, 0);
  if (rc < 0) {
    fprintf(stderr, 
            "unable to open pcm device: %s\n",
            snd_strerror(rc));
    exit(1);
  }
  
  /* Allocate a hardware parameters object. */
  snd_pcm_hw_params_alloca(&params);
  /* Fill it in with default values. */
  snd_pcm_hw_params_any(handle, params);
  /* Set the desired hardware parameters. */
  /* Interleaved mode */
  snd_pcm_hw_params_set_access(handle, params,SND_PCM_ACCESS_RW_INTERLEAVED);
  /* Signed 16-bit little-endian format */
  snd_pcm_hw_params_set_format(handle, params,SND_PCM_FORMAT_S16_LE);
  /* Two channels (stereo) */
  snd_pcm_hw_params_set_channels(handle, params, 2);
  /* 44100 bits/second sampling rate (CD quality) */
  val = 48000;
  snd_pcm_hw_params_set_rate_near(handle, params,&val, &dir);
  fprintf(stderr,"set_rate:val: %d\n",val);
  fprintf(stderr,"set_rate:dir: %d\n",dir);
  /* Set period size to 32 frames. */
  frames = 1024;
  snd_pcm_hw_params_set_period_size_near(handle,params, &frames, &dir);
  fprintf(stderr,"set_period:frames: %d\n",frames);
  fprintf(stderr,"set_period:dir: %d\n",dir);

  /* Write the parameters to the driver */
  rc = snd_pcm_hw_params(handle, params);
  if (rc < 0) {fprintf(stderr,"unable to set hw parameters: %s\n",snd_strerror(rc));
     exit(1);
     }
  /* Use a buffer large enough to hold one period */
  //snd_pcm_hw_params_get_period_size(params, &frames,&dir);
  size = frames * 2 *2; /* 2 bytes/sample, 2 channels */
  buffer = (char *) malloc(size);

  /* We want to loop for 5 seconds */
  snd_pcm_hw_params_get_period_time(params,&val, &dir);
  /* 5 seconds in microseconds divided by 
   * period time */
  loops = 10000000 / val;
  
  //fprintf(stderr,"val: %d\n",val);
  //fprintf(stderr,"dir: %d\n",dir);
  fprintf(stderr,"size: %d\n",size);
  
  //deltaf = 2*M_PI*freq/sampleRate;
  deltaf = 600.0/(double)48000*2*M_PI;
  phase = 0;
  vu = 8000;
  
  while (loops > 0) {
    loops--;
    //Lautstaerke am Ende
    if (loops < 100) { vu -= 80; };
    i = 0;
    do {
     cwSin = (short)(vu * sin(phase)); 
    //Kanal links
    // buffer[i++] = (char) (cwSin >> 8);
     buffer[i++] = (char) cwSin;
    // i++;
     buffer[i++] = (char) (cwSin >> 8);
    //Kanal rechts 
     //buffer[i++] = (char) (cwSin >> 8);
     buffer[i++] = (char) cwSin;
     buffer[i++] = (char) (cwSin >> 8);
     //i++;

     phase += deltaf;
     
    } while (i<(size));
    
    rc = snd_pcm_writei(handle, buffer, frames);
    if (rc == -EPIPE) {
      /* EPIPE means underrun */
      fprintf(stderr, "underrun occurred\n");
      snd_pcm_prepare(handle);
    } else if (rc < 0) {
      fprintf(stderr,
              "error from writei: %s\n",
              snd_strerror(rc));
    }  else if (rc != (int)frames) {
      fprintf(stderr, 
              "short write, write %d frames\n", rc);
    }
  }
  fprintf(stderr,"end\n");
  snd_pcm_drain(handle);
  snd_pcm_close(handle);
  
  free(buffer);
  
  return 0;
}
