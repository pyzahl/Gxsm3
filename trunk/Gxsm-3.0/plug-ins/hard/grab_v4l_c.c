/* Gnome gxsm - Gnome X Scanning Microscopy
 * universal STM/AFM/SARLS/SPALEED/... controlling and
 * data analysis software
 *
 * Gxsm Hardware Interface Plugin Name: grab_v4l.C
 * ===============================================
 * 
 * Copyright (C) 1999 The Free Software Foundation
 *
 * Authors: Percy Zahl <zahl@fkp.uni-hannover.de>
 * additional features: Andreas Klust <klust@fkp.uni-hannover.de>
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

/* irnore this module for docuscan
% PlugInModuleIgnore
 */


#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include <string.h>
#include <sys/types.h>

#include <unistd.h>
#include <linux/videodev.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <fcntl.h>
#include <signal.h>
#include <linux/types.h> 
#include <linux/videodev.h>
#include <errno.h>

/*
 * GXSM V4L Hardware Interface Class -- plain c module
 * ============================================================
 */

gint gxsm_v4l_open_video4l ();
gint gxsm_v4l_close_video4l ();
gint gxsm_v4l_maxwidth  ();
gint gxsm_v4l_maxheight ();
gint gxsm_v4l_win_width  ();
gint gxsm_v4l_win_height ();
gint gxsm_v4l_grab_video4l ();
gint gxsm_v4l_get_pixel (int *r, int *g, int *b);

int    video_dev_fd=0;
struct video_capability cap;
struct video_picture pic;
struct video_window win;
struct video_capture vicap;
struct video_channel vidcan;
struct video_mbuf vidbuf;
struct video_buffer buffer;
struct video_mmap mapbuf;
guchar *bigbuf;
guchar *buf;
int gray[3];
guchar  *src;


gint gxsm_v4l_open_video4l (){
//    struct v4l2_capability v4l2_cap;
//    struct v4l2_requestbuffers reqbuf;
    struct {
        void *start;
        size_t length;
    } *buffers;
    unsigned int i;
//-------------------------------------------------------------------
              /* OPEN VIDEO DEVICE */
//-------------------------------------------------------------------	
	
	video_dev_fd = open("/dev/video", O_RDWR );

	if (video_dev_fd<0){
		printf("ERROR: THERE IS NO CAMERA CONECTED\n");
		exit(-1);
	}

//--------------------------------------------------------------------
		/* VIDEO CAPABILITIES*/
//---------------------------------------------------------------------

	if (-1==ioctl(video_dev_fd,VIDIOCGCAP,&cap)){
		perror("ioctl VIDIOCGCAP");
		exit(-1);
	}
	printf("------------------------------------\n");
	printf("------------------------------------\n");
	printf("name      -> %s\n", cap.name);
	printf("type      -> %d\n", cap.type);
	printf("channels  -> %d\n", cap.channels);
	printf("audios    -> %d\n", cap.audios );
	printf("maxwidth  -> %d\n", cap.maxwidth );
	printf("maxheight -> %d\n", cap.maxheight);
	printf("minwidth  -> %d\n", cap.minwidth );
	printf("minheight -> %d\n", cap.minheight );
	printf("------------------------------------\n");

//---------------------------------------------------------------------
		/* FRAME GRABBERS DETECTION*/
//---------------------------------------------------------------------

	if (-1==ioctl(video_dev_fd,VIDIOCSFBUF,&buffer))
		printf ("Not a decteced frame grabber\n");
	else {
 
 		if (-1==ioctl(video_dev_fd,VIDIOCGFBUF,&buffer)) {
			perror ("ioctl VIDIOCGFBUF");
 			exit(-1);
		}
  		printf("------------------------------------\n"); 
  		printf ("add -> %d\n", buffer.base);
  		printf ("height -> %d\n", buffer.height);
  		printf ("width -> %d\n", buffer.width);
  		printf ("depth -> %d\n", buffer.depth);
  		printf ("BPL -> %d\n", buffer.bytesperline);
	}
	printf("------------------------------------\n");					

/*
	reqbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	reqbuf.memory = V4L2_MEMORY_MMAP;
	reqbuf.count = 20;

	if (-1 == ioctl (video_dev_fd, VIDIOC_REQBUFS, &reqbuf)) {
	    if (errno == EINVAL)
                printf ("Video capturing or mmap-streaming is not supported\n");
	    else
                perror ("ioctl VIDIOC_REQBUFS");

	    //	    exit (EXIT_FAILURE);
	}

	if (-1 == ioctl (video_dev_fd, VIDIOC_QUERYCAP, &v4l2_cap)) {
	    if (errno == EINVAL)
                printf ("not supported request by kernel\n");
	    else
		perror ("ioctl V4L2 VIDIOC_QUERYCAP");
	} else {
	    printf("------------------------------------\n");
	    printf("v4l2 driver    -> %s\n", v4l2_cap.driver);
	    printf("v4l2 card      -> %s\n", v4l2_cap.card);
	    printf("v4l2 bus info  -> %s\n", v4l2_cap.bus_info);
	    printf("v4l2 driver    -> %s\n", v4l2_cap.driver);
	    printf("v4l2 version   -> %06X\n", v4l2_cap.version);
	    printf("v4l2 capabilities -> %X\n", v4l2_cap.capabilities);
	    printf("---| V4L2_CAP_VIDEO_CAPTURE .. %s\n", v4l2_cap.capabilities&V4L2_CAP_VIDEO_CAPTURE ? "Yes":"No");
	    printf("   | V4L2_CAP_VIDEO_OUTPUT ... %s\n", v4l2_cap.capabilities&V4L2_CAP_VIDEO_OUTPUT ? "Yes":"No");
	    printf("   | V4L2_CAP_VIDEO_OVERLAY .. %s\n", v4l2_cap.capabilities&V4L2_CAP_VIDEO_OVERLAY ? "Yes":"No");
	    printf("   | V4L2_CAP_VBI_CAPTURE .... %s\n", v4l2_cap.capabilities&V4L2_CAP_VBI_CAPTURE ? "Yes":"No");
	    printf("   | V4L2_CAP_VBI_OUTPUT... .. %s\n", v4l2_cap.capabilities&V4L2_CAP_VBI_OUTPUT ? "Yes":"No");
	    printf("   | V4L2_CAP_TUNER .......... %s\n", v4l2_cap.capabilities&V4L2_CAP_TUNER ? "Yes":"No");
	    printf("   | V4L2_CAP_AUDIO .......... %s\n", v4l2_cap.capabilities&V4L2_CAP_AUDIO ? "Yes":"No");
	    printf("   | V4L2_CAP_READWRITE ...... %s\n", v4l2_cap.capabilities&V4L2_CAP_READWRITE ? "Yes":"No");
	    printf("   | V4L2_CAP_ASYNCIO ........ %s\n", v4l2_cap.capabilities&V4L2_CAP_ASYNCIO ? "Yes":"No");
	    printf("   | V4L2_CAP_STREAMING ...... %s\n", v4l2_cap.capabilities&V4L2_CAP_STREAMING ? "Yes":"No");
	    printf("------------------------------------\n");
	}
*/

	return 0;
}

gint gxsm_v4l_close_video4l (){
	close (video_dev_fd);
	video_dev_fd = 0;
	return 0;
}

inline gint gxsm_v4l_maxwidth  () { return cap.maxwidth; }
inline gint gxsm_v4l_maxheight () { return cap.maxheight; }

inline gint gxsm_v4l_win_width  () { return win.width; }
inline gint gxsm_v4l_win_height () { return win.height; }

gint gxsm_v4l_grab_video4l (){ 

	int frame;
	int i;

//-----------------------------------------------------------------------
		/* CAPTURE FRAME */
//-----------------------------------------------------------------------

/*
	win.x = 0;
	win.y = 0;
	win.width = COLS;
	win.height = ROWS;
	win.clipcount = 0;
	win.chromakey = 1;
	win.flags = VIDEO_CLIPMAP_SIZE;
	
	if (-1== ioctl(video_dev_fd, VIDIOCSWIN,&win)){
		perror ("ioctl VIDIOCSWIN");
		exit (-1);
	}
*/
	if (ioctl(video_dev_fd, VIDIOCGWIN, &win) < 0) {
		perror("ioctl VIDIOCGWIN");
		close (video_dev_fd);
		exit(1);
	}	

	fprintf(stderr, "V4L Window [%d x %d]...\n", win.width, win.height);

//---------------------------------------------------------------------
		/* IMAGE PROPERTIES*/
//---------------------------------------------------------------------

	pic.depth = 24;
	pic.palette = VIDEO_PALETTE_RGB24;
	pic.brightness = 100;
	pic.contrast = 30;
	pic.whiteness = 0;
	pic.colour = 0;
	pic.hue = 0;
	
	
	if (-1==ioctl( video_dev_fd, VIDIOCSPICT, &pic )){perror("ioctl VIDIOCSPICT");
		exit(-1);}
	
	ioctl( video_dev_fd, VIDIOCGPICT, &pic );

	printf("------------------------------------\n");
	printf("brightness -> %d \n", pic.brightness/256 );
	printf("hue -> %d\n", pic.hue/256);
	printf("colour -> %d\n", pic.colour/256 );
	printf("contrast -> %d \n", pic.contrast/256 );
	printf("whiteness -> %d\n", pic.whiteness/256 );
	printf("depth -> %d\n", pic.depth );
	printf("palette -> %d \n", pic.palette );
	printf("------------------------------------\n");

//-----------------------------------------------------------------------
		/* MAPPING BUFFER */
//------------------------------------------------------------------------


	if (-1==ioctl(video_dev_fd,VIDIOCGMBUF,&vidbuf)){
		perror("ioctl VIDIOCGMBUF"); 
		exit(-1);
	}
	
	printf("------------------------------------\n");
	printf("size  -> %d\n",vidbuf.size);
	printf("frames -> %d\n",vidbuf.frames);
	printf("offsets -> %d\n",vidbuf.offsets);
	printf("------------------------------------\n");

	bigbuf = (guchar *)mmap(0,vidbuf.size, PROT_READ | PROT_WRITE, MAP_SHARED, video_dev_fd, 0);


	mapbuf.width  = win.width;
	mapbuf.height = win.height;
	mapbuf.format = VIDEO_PALETTE_RGB24;


//----------------------------------------------------------------------
		    /* SET BUFFERS*/
//----------------------------------------------------------------------		 

	for(frame=0; frame<vidbuf.frames;frame++){					// turn on both of the buffers
		mapbuf.frame = frame;							// to start capture process.
		if (ioctl(video_dev_fd,VIDIOCMCAPTURE, &mapbuf)<0){				// Now they can store images.
			perror("VIDIOCMCAPTURE");
			exit(-1);}
		}
	
	frame = 0;

    
//---------------------------------------------------------------------
		       /* CAPTURING*/
//---------------------------------------------------------------------		

	while (1){
		i = -1;
		while(i<0){
	
			i= ioctl(video_dev_fd,VIDIOCSYNC, &frame);				// Wait until the actual buffer
			if(i < 0 && errno == EINTR) continue;				// is full. When it happends 
			if (i < 0) {							// it start to capture to	
				perror ("VIDIOCSYNC");					// the other buffer.
				exit(-1);
			}
		}
		break;
	}


	buf = bigbuf + vidbuf.offsets[frame];
	mapbuf.frame = frame;
						
	if (ioctl(video_dev_fd,VIDIOCMCAPTURE, &mapbuf)<0) {
		perror("VIDIOCMCAPTURE");		// Turn on the buffer that
		exit(-1);
	}								// was being used.

	frame++;


	if (frame>=vidbuf.frames) frame=0;

	src = buf;

	return 0;
}

#define READ_VIDEO_PIXEL(buf, format, depth, r, g, b)			\
{									\
	switch (format)							\
	{								\
		case VIDEO_PALETTE_GREY:				\
			switch (depth)					\
			{						\
				case 4:					\
				case 6:					\
				case 8:					\
					(r) = (g) = (b) = (*buf++ << 8);\
					break;				\
									\
				case 16:				\
					(r) = (g) = (b) = 		\
						*((unsigned short *) buf);	\
					buf += 2;			\
					break;				\
			}						\
			break;						\
									\
									\
		case VIDEO_PALETTE_RGB565:				\
		{							\
			unsigned short tmp = *(unsigned short *)buf;	\
			(r) = tmp&0xF800;				\
			(g) = (tmp<<5)&0xFC00;				\
			(b) = (tmp<<11)&0xF800;				\
			buf += 2;					\
		}							\
		break;							\
									\
		case VIDEO_PALETTE_RGB555:				\
			(r) = (buf[0]&0xF8)<<8;				\
			(g) = ((buf[0] << 5 | buf[1] >> 3)&0xF8)<<8;	\
			(b) = ((buf[1] << 2 ) & 0xF8)<<8;		\
			buf += 2;					\
			break;						\
									\
		case VIDEO_PALETTE_RGB24:				\
			(r) = buf[0] << 8; (g) = buf[1] << 8; 		\
			(b) = buf[2] << 8;				\
			buf += 3;					\
			break;						\
									\
		default:						\
			fprintf(stderr, 				\
				"Format %d not yet supported\n",	\
				format);				\
	}								\
}						

inline gint gxsm_v4l_get_pixel (int *r, int *g, int *b){
	READ_VIDEO_PIXEL(src, pic.palette, pic.depth, *r, *g, *b);
	return (*r + *g + *b);
}


