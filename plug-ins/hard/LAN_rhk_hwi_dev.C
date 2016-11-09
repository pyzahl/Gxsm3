/* Gxsm - Gnome X Scanning Microscopy
 * universal STM/AFM/SARLS/SPALEED/... controlling and
 * data analysis software
 * 
 * Copyright (C) 1999,2000,2001 Percy Zahl
 *
 * Authors: Percy Zahl <zahl@users.sf.net>
 * additional features: Farid El Gabaly <farid.elgabaly@uam.es>, 
 * Juan de la Figuera <juan.delafiguera@uam.es>
 * WWW Home: http://gxsm.sf.net
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 */

/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 8 c-style: "K&R" -*- */

/* irnore this module for docuscan
% PlugInModuleIgnore
 */

// "C++" headers
#include <iostream>

// "C" headers
#include <cstdio>
#include <cstring>

// system headers
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>

// Gxsm headers
#include "glbvars.h"

#include "LAN_rhk_hwi.h"


// use "gxsm3 --debug-level=5" (5 or higher) to enable debug!
#define	INTERNET_DEBUG(S) XSM_DEBUG (DBG_L4, S)

/* Constructor for Gxsm Internet device support base class
 * ==================================================
 * - open device
 * - init things
 * - ...
 */
LAN_rhk_hwi_dev::LAN_rhk_hwi_dev(){
	char host_name[40];
	char *tport;
	int port;
	struct sockaddr_in pin;
	struct hostent *server_host_name;

	strcpy(host_name, xsmres.DSPDev);
	tport=strstr(host_name,":");
	if (tport==NULL)
		{
			INTERNET_DEBUG("No : in remote address name, assuming 5027");
			port=5027;
		}
	else
		{
			tport[0]=0;
			tport++;
			port=atoi(tport);
		}

	INTERNET_DEBUG("open socket to " << host_name << ", port " << port);
	max_points_per_line = 262145; // Should be returned by the external program.

	if ((server_host_name = gethostbyname(host_name)) == 0) {
		INTERNET_DEBUG("Error resolving local host");
		exit(1);
	}

	bzero(&pin, sizeof(pin));
	pin.sin_family = AF_INET;
	pin.sin_addr.s_addr = htonl(INADDR_ANY);
	pin.sin_addr.s_addr = ((struct in_addr *)(server_host_name->h_addr))->s_addr;
	pin.sin_port = htons(port);

	if ((socket_descriptor = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		INTERNET_DEBUG("Error opening socket\n");
		exit(1);
	}


	if (connect(socket_descriptor, (struct sockaddr *)&pin, sizeof(pin)) == -1) {
		INTERNET_DEBUG("Error connecting to socket\n");
		exit(1);
	}

}

/* Destructor
 * close device
 */
LAN_rhk_hwi_dev::~LAN_rhk_hwi_dev(){
	INTERNET_DEBUG("close remote connection");

	close(socket_descriptor);

}

int LAN_rhk_hwi_dev::SendCommand(char *Cmd){
	int n;
	n=strlen(Cmd);
	return(write(socket_descriptor, Cmd, n));
}

size_t LAN_rhk_hwi_dev::ReadData(void *Data, size_t count) {
	struct pollfd list[1];

	list[0].fd=socket_descriptor;
	list[0].events=POLLIN;
	while (!poll(list,1,0)) {
		gapp->check_events();
	}
	return(recv(socket_descriptor, Data, count,MSG_WAITALL ));
}

void LAN_rhk_hwi_dev::ExecCmd(int Cmd){
	// Exec "DSP" command (additional stuff)
	INTERNET_DEBUG("Exec Cmd 0x" << std::hex << Cmd);
	// Put CMD to DSP for execution.
	// Wait until done and
	// call whenever you are waiting longer for sth.:
	// gapp->check_events(); // inbetween!
}

int LAN_rhk_hwi_dev::WaitExec(int data){
	return 0; // not needed today, always returns 0
}

// you may want this later for mover and other tasks...
void LAN_rhk_hwi_dev::SetParameter(PARAMETER_SET &hps, int scanflg){
}

void LAN_rhk_hwi_dev::GetParameter(PARAMETER_SET &hps){
}

int LAN_rhk_hwi_dev::ReadScanData(int y_index, int num_srcs, Mem2d *m[MAX_SRCS_CHANNELS]){
	static time_t t0 = 0; // only for demo
	int len = m[0]->GetNx();
	int count;
	SHT *linebuffer = new SHT[len];
	char txt[20];

	// create dummy data (need to read some stuff from driver/tmp buffer...)
	// should use ReadData (see above) later!
	// and should not block...

	//   INTERNET_DEBUG("ReadData:" << y_index);

	if (t0 == 0) t0 = time (NULL);
	int drift = (int) (time (NULL) - t0);
	if (y_index < 0) // 2D HS Area Capture/Scan
		for (int k=0; k<m[0]->GetNy (); ++k)
			for (int i=0; i<num_srcs; ++i){
				count=sprintf(txt, "read %d\n", len);
				write(socket_descriptor, txt, count);
				ReadData(linebuffer, len*sizeof(short));
				write(socket_descriptor, txt, count); // This is ugly!!! So far a hack for reading one
				ReadData(linebuffer, len*sizeof(short)); // channel and dropping the forward image!!!!
				if (m[i])
					m[i]->PutDataLine (k, linebuffer);
			}
	else
		for (int i=0; i<num_srcs; ++i){
			count=sprintf(txt, "read %d\n", len);
			write(socket_descriptor, txt, count);
			ReadData(linebuffer, len*sizeof(short));
			write(socket_descriptor, txt, count); // This is ugly!!! So far a hack for reading one
			count=ReadData(linebuffer, len*sizeof(short)); // channel and dropping the forward image!!!!
			if (m[i])
				m[i]->PutDataLine (y_index, linebuffer);
		}
	delete[] linebuffer;
	return 0;
}

int LAN_rhk_hwi_dev::ReadProbeData(int nsrcs, int nprobe, int kx, int ky, Mem2d *m, double scale){
	// later
	return 1;
}


/*
 * provide some info about the connected hardware/driver/etc.
 */
gchar* LAN_rhk_hwi_dev::get_info(){
	return g_strdup("*--GXSM LAN/Internet HwI base class--*\n"
			"Internet device: do not know\n"
			"*--Features--*\n"
			"SCAN: Yes\n"
			"PROBE: No\n"
			"ACPROBE: No\n"
			"*--EOF--*\n"
			);
}
