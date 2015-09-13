/** 
* @file hamlibserv.c
* @brief Header files for the keyer functions
* @author Bernd Kernbaum DK3WX
* @version 0.1
* @date 2014-10-23
*/
//hamlibserv.c

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

#include<stdio.h> //printf
#include<string.h> //memset
#include<stdlib.h> //exit(0);
#include<arpa/inet.h>
#include<sys/socket.h>
#include<unistd.h> //write
#include<pthread.h> //for threading , link with lpthread

//#include "vfo.h"

#define BUFLEN 1024  //Max length of buffer
#define PORT 4532   //The port on which to listen for incoming data
extern  long long frequencyA;
extern  int mode;
extern int meterX;
extern double cw_sidetone_frequency;
//  From hamlibserver.py by Jim Ahlstrom, N2ADR
// "rigctl -m 2 -r localhost"
char dump1[] = {
//length 197                                                            15                      
0x20,0x30,0x0a,0x32,0x0a,0x32,0x0a,0x31,0x35,0x30,0x30,0x30,0x30,0x2e,0x30,  //0
0x30,0x30,0x30,0x30,0x30,0x20,0x33,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x2e,  //1
0x30,0x30,0x30,0x30,0x30,0x30,0x20,0x20,0x30,0x78,0x39,0x30,0x30,0x61,0x66,  //2
0x20,0x2d,0x31,0x20,0x2d,0x31,0x20,0x30,0x78,0x31,0x30,0x20,0x30,0x30,0x30,  //3
0x30,0x30,0x33,0x20,0x30,0x78,0x33,0x0a,0x30,0x20,0x30,0x20,0x30,0x20,0x30,  //4
0x20,0x30,0x20,0x30,0x20,0x30,0x0a,0x31,0x35,0x30,0x30,0x30,0x30,0x2e,0x30,  //5
0x30,0x30,0x30,0x30,0x30,0x20,0x33,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x2e,  //6
0x30,0x30,0x30,0x30,0x30,0x30,0x20,0x20,0x30,0x78,0x39,0x30,0x30,0x61,0x66,  //7
0x20,0x2d,0x31,0x20,0x2d,0x31,0x20,0x30,0x78,0x31,0x30,0x20,0x30,0x30,0x30,  //8
0x30,0x30,0x33,0x20,0x30,0x78,0x33,0x0a,0x30,0x20,0x30,0x20,0x30,0x20,0x30,  //9
0x20,0x30,0x20,0x30,0x20,0x30,0x0a,0x30,0x20,0x30,0x0a,0x30,0x20,0x30,0x0a,  //10
0x30,0x0a,0x30,0x0a,0x30,0x0a,0x30,0x0a,0x0a,0x0a,0x30,0x78,0x30,0x0a,0x30,  //11
0x78,0x30,0x0a,0x30,0x78,0x30,0x0a,0x30,0x78,0x30,0x0a,0x30,0x78,0x30,0x0a,  //12
0x30,0x0a };
char dump[] = {
// length   285
 0x20,0x32,0x0a,0x32,0x0a,0x32,0x0a,0x31,0x35,0x30,0x30,0x30,0x30,0x2e,0x30,0x30
,0x30,0x30,0x30,0x30,0x20,0x31,0x35,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x2e
,0x30,0x30,0x30,0x30,0x30,0x30,0x20,0x30,0x78,0x31,0x66,0x66,0x20,0x2d,0x31,0x20
,0x2d,0x31,0x20,0x30,0x78,0x31,0x30,0x30,0x30,0x30,0x30,0x30,0x33,0x20,0x30,0x78
,0x33,0x0a,0x30,0x20,0x30,0x20,0x30,0x20,0x30,0x20,0x30,0x20,0x30,0x20,0x30,0x0a
,0x30,0x20,0x30,0x20,0x30,0x20,0x30,0x20,0x30,0x20,0x30,0x20,0x30,0x0a,0x30,0x78
,0x31,0x66,0x66,0x20,0x31,0x0a,0x30,0x78,0x31,0x66,0x66,0x20,0x30,0x0a,0x30,0x20
,0x30,0x0a,0x30,0x78,0x31,0x65,0x20,0x32,0x34,0x30,0x30,0x0a,0x30,0x78,0x32,0x20
,0x35,0x30,0x30,0x0a,0x30,0x78,0x31,0x20,0x38,0x30,0x30,0x30,0x0a,0x30,0x78,0x31
,0x20,0x32,0x34,0x30,0x30,0x0a,0x30,0x78,0x32,0x30,0x20,0x31,0x35,0x30,0x30,0x30
,0x0a,0x30,0x78,0x32,0x30,0x20,0x38,0x30,0x30,0x30,0x0a,0x30,0x78,0x34,0x30,0x20
,0x32,0x33,0x30,0x30,0x30,0x30,0x0a,0x30,0x20,0x30,0x0a,0x39,0x39,0x39,0x30,0x0a
,0x39,0x39,0x39,0x30,0x0a,0x31,0x30,0x30,0x30,0x30,0x0a,0x30,0x0a,0x31,0x30,0x20
,0x0a,0x31,0x30,0x20,0x32,0x30,0x20,0x33,0x30,0x20,0x0a,0x30,0x78,0x33,0x65,0x66
,0x66,0x66,0x66,0x66,0x66,0x0a,0x30,0x78,0x33,0x65,0x66,0x66,0x66,0x66,0x66,0x66
,0x0a,0x30,0x78,0x37,0x66,0x66,0x66,0x66,0x66,0x66,0x66,0x0a,0x30,0x78,0x37,0x66
,0x66,0x66,0x66,0x66,0x66,0x66,0x0a,0x30,0x78,0x37,0x66,0x66,0x66,0x66,0x66,0x66
,0x66,0x0a,0x30,0x78,0x37,0x66,0x66,0x66,0x66,0x66,0x66,0x66,0x0a};

char vfo[] = {'V','F','O',0x0a};
char freq[] = {'1','2','3','4','5','6','7','8','9','0',0x0a};
char null[] = {'0',0x0a};
char mode_re[] = {'D','I','G','U',0x0a,'8','0','0',0x0a};
char pwr[] =   {'0','.','0','1','9','6','0','7','8',0x0a};
char smeter[] ={'-','1','8',0x0a,'5','6','7','8','9','0',0x0a};

int dump_len = 285; //197;
static pthread_t hamlibserv_thread_id;

//the thread function
void *connection_handler(void *);
void* hamlibserv_thread(void* arg);


void hamlibserv_init() {

    int rc;
    //init_alaw_tables();
    
    rc=pthread_create(&hamlibserv_thread_id,NULL,hamlibserv_thread,NULL);
    if(rc != 0) {
        fprintf(stderr,"pthread_create failed on hamlibserv_thread: rc=%d\n", rc);
    }

}
  
  

void* hamlibserv_thread(void* arg)
 {
  int socket_desc , client_sock , c;
  struct sockaddr_in server , client;
  //Create socket
  socket_desc = socket(AF_INET , SOCK_STREAM , 0);
  if (socket_desc == -1)
   {
    fprintf(stderr,"Could not create socket\n");
   }
  fprintf(stderr,"Socket created\n");
  //Prepare the sockaddr_in structure
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = INADDR_ANY;
  server.sin_port = htons( 4532 );
  //Bind
  if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
   {
   //print the error message
   fprintf(stderr,"bind failed. Error\n");
   //return 1;
   }
  fprintf(stderr,"bind done\n");
  //Listen
  listen(socket_desc , 3);
  //Accept and incoming connection
  fprintf(stderr,"Waiting for incoming connections...\n");
  c = sizeof(struct sockaddr_in);

  pthread_t thread_id;
  while( (client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c)) )
    {
    fprintf(stderr,"Connection accepted\n");
    if( pthread_create( &thread_id , NULL , connection_handler , (void*) &client_sock) < 0)
     {
      fprintf(stderr,"could not create thread\n");
      //return 1;
     }
  //Now join the thread , so that we dont terminate before the thread
  //pthread_join( thread_id , NULL);
  fprintf(stderr,"Handler assigned\n");
   }
  if (client_sock < 0)
   {
    fprintf(stderr,"accept failed\n");
    //return 1;
   }
  //return 0;
}


/*
* This will handle connection for each client
* */
void *connection_handler(void *socket_desc)
{
  //Get the socket descriptor
  int sock = *(int*)socket_desc;
  int read_size;
  char *message , client_message[BUFLEN];
  int lf,lm,ls;

 //Receive a message from client
 while( (read_size = recv(sock , client_message , BUFLEN , 0)) > 0 )
  {
        //Send the message back to client
        if (client_message[0] == 0x5c) { write(sock , dump , dump_len);};
        if (client_message[0] == 'v') { write(sock , vfo , 4);};
        if (client_message[0] == 'f') {lf=freqtochar(); write(sock , freq , lf);};
        if (client_message[0] == 'm') {lm=modetochar(); write(sock , mode_re , lm);};
        if (client_message[0] == 'l') if(strstr(client_message, "STRENGTH")) {ls=smetertochar(); write(sock , smeter , ls);}
                                       else if(strstr(client_message, "RFPOWER")) { write(sock , pwr , 10);};
        if (client_message[0] == 't') { write(sock , null , 2);};
        memset(client_message, 0, BUFLEN);
  }  
 if(read_size == 0)
  {
   fprintf(stderr,"Client disconnected\n");
   fflush(stdout);
  }
  else if(read_size == -1)
  {
   fprintf(stderr,"recv failed\n");
   }
 return 0;
} 

// returns: the length of the number string
int itoa(int value, char *sp, int radix)
{
    char tmp[16];// be careful with the length of the buffer
    char *tp = tmp;
    int i;
    unsigned v;

    int sign = (radix == 10 && value < 0);    
    if (sign)
        v = -value;
    else
        v = (unsigned)value;

    while (v || tp == tmp)
    {
        i = v % radix;
        v /= radix; // v/=radix uses less CPU clocks than v=v/radix does
        if (i < 10)
          *tp++ = i+'0';
        else
          *tp++ = i + 'a' - 10;
    }

    int len = tp - tmp;

    if (sign) 
    {
        *sp++ = '-';
        len++;
    }

    while (tp > tmp)
        *sp++ = *--tp;

    return len;
}


int smetertochar(void)
{ int i=0,l;
  l=meterX-54;
  if (l<0) {l=-l;smeter[i++] = '-';};
  if (l>99) l=99;
  if (l<10) { smeter[i++] = l+0x30;}
   else { smeter[i++] = l/10 + 0x30;  smeter[i++] = l%10 + 0x30;};
  smeter[i++] = '\0';
  return i;
}

int freqtochar(void)
{ int i,l;
  long long qrg;
  qrg = frequencyA;
  if (mode==3) qrg = frequencyA - cw_sidetone_frequency;
  if (mode==4) qrg = frequencyA + cw_sidetone_frequency;
  char fchar[10];
  l=itoa((int)qrg,fchar,10);
  strcpy(freq,fchar);
  strcat(freq,"\n");
  return strlen(freq);
  
}

int modetochar(void)
{    
     switch(mode) {
        case 0: strcpy(mode_re,"LSB");  break;
        case 1: strcpy(mode_re,"USB");  break;
        case 2: strcpy(mode_re,"DSB");  break;
        case 3: strcpy(mode_re,"CW");  break;
        case 4: strcpy(mode_re,"CW");  break;
        case 5: strcpy(mode_re,"FMN");  break;
        case 6: strcpy(mode_re,"AM");   break;
        case 7: strcpy(mode_re,"DIGU"); break;
        case 8: strcpy(mode_re,"SPEC"); break;
        case 9: strcpy(mode_re,"DIGL"); break;
        case 10:strcpy(mode_re,"SAM");  break;
        case 11:strcpy(mode_re,"DRM");  break;
    }
    strcat(mode_re,"\n");
    //todo filter BW
    strcat(mode_re,"800");
    strcat(mode_re,"\n");
    
    return strlen(mode_re);
    
}