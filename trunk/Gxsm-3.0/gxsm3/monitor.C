/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 8 c-style: "K&R" -*- */

/* Gxsm - Gnome X Scanning Microscopy
 * universal STM/AFM/SARLS/SPALEED/... controlling and
 * data analysis software
 * 
 * Copyright (C) 1999,2000,2001,2002,2003 Percy Zahl
 *
 * Authors: Percy Zahl <zahl@users.sf.net>
 * additional features: Andreas Klust <klust@users.sf.net>
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


#include <iostream>
#include <time.h>
#include "monitor.h"
#include "meldungen.h"
#include "unit.h"
#include "xsmdebug.h"

// may be enabled via configure.ac or here directly
// #define GXSM_MONITOR_VMEMORY_USAGE
#ifdef GXSM_MONITOR_VMEMORY_USAGE

gint parseLine (char* line){
        // This assumes that a digit will be found and the line ends in " Kb".
        gint i = strlen(line);
        const char* p = line;
        while (*p <'0' || *p > '9') p++;
        line[i-3] = '\0';
        i = atoi(p);
        return i;
}

// use: getValue ("VmSize:");
gint getValue (const gchar *what){ //Note: this value is in KB!
        FILE* file = fopen("/proc/self/status", "r");
        gint result = -1;
        gchar line[128];
        
        while (fgets(line, 128, file) != NULL){
                if (strncmp(line, what, strlen(what)) == 0){
                        result = parseLine(line);
                        break;
                }
        }
        fclose(file);
        return result;
}

#endif

Monitor::Monitor(const gchar *name){
        dt=10.0;

        if(name)
                logname = g_strdup(name);
        else{
                char buf[64];
                time_t t;
                time(&t);               // 012345678901234567890123456789
                strcpy(buf, ctime(&t)); // Thu Jun 25 12:44:55 1998
                buf[7]=0; buf[24]=0;
                logname = g_strdup_printf("Ev_%3s%4s.log", &buf[4], &buf[20]);
                XSM_DEBUG (DBG_L2, "LogName:>" << logname << "<" );
        }

        for(int i=0; i < MAXMONITORFIELDS; ++i)
                Fields[i] = NULL;

        LogEvent("Monitor", "startup");
}

Monitor::~Monitor(){
        if(logname)
                g_free(logname);
        for(gchar **field = Fields; *field; ++field)
                g_free(*field);
}

void Monitor::SetLogName(char *name){
        if(logname)
                g_free(logname);
        logname = g_strdup(name);
}

gint Monitor::Load(gchar *fname){
        std::ifstream f;
        return 0;
}

gint Monitor::Save(gchar *fname){
        std::ofstream f;
        time_t t;
        time(&t); 

        f.open(fname, std::ios::out | std::ios::trunc);
        if(!f.good()){
                return 1;
        }
        f << "# Monitor Data\n" << "# Date: " << ctime(&t) << "#" << fname << "\n";
  
        //  foralllines...
        f << "\n";

        f.close();
        return 0;
}

void Monitor::Messung(float val, gchar *txt){
        /*
          char buf[256];
          time_t t;
          time(&t);               // 012345678901234567890123456789
          strcpy(buf, ctime(&t)); // Thu Jun 25 12:44:55 1998
          buf[24]=0;
 
          strcpy(Field[0], buf);
          sprintf(Field[1], "%6.0f  ", val);
          if(!nlast)
          f0=fn;
          sprintf(Field[2], "------  ");
          sprintf(Field[3], "------  ");
          if(txt)
          sprintf(Field[LastField-1], "*X: %3d %s", nlast, txt);
          else
          sprintf(Field[LastField-1], "*X: %3d", nlast);

          AppLine();
        */
}

void Monitor::LogEvent(const gchar *Action, const gchar *Entry){
        PutEvent(Action, Entry);
}

void Monitor::PutEvent(const gchar *Action, const gchar *Entry){
        //  return;
        for(gchar **field = Fields; *field; ++field){
                g_free(*field);
                *field=NULL;
        }
        time_t t;
        time(&t);
        Fields[0] = g_strdup(ctime(&t)); Fields[0][24]=' ';
        Fields[1] = g_strdup_printf("%12s : ",Action);
        Fields[2] = g_strdup_printf("%s : ",Entry);
        Fields[3] = NULL; //g_strdup_printf("%10.6f ",0.0);
        Fields[4] = NULL; //g_strdup_printf("%10.6f ",0.0);
        Fields[5] = NULL; //g_strdup_printf("%10.6f ",0.0);
        Fields[6] = NULL;

        AppLine();
}

gint Monitor::AppLine(){
        // Autologging
        if(logname){
                std::ofstream f;
                f.open(logname, std::ios::out | std::ios::app);
                if(!f.good()){
                        std::cerr << ERR_SORRY << "\n" << ERR_FILEWRITE << ": " << logname << std::endl;
                        return 1;
                }
    
                for(gchar **field = Fields; *field; ++field)
                        f << *field;

#ifdef GXSM_MONITOR_VMEMORY_USAGE
                f << " RealTime: " << g_get_real_time ();
                f << " VmSize: " << getValue ("VmSize:") << " kB";
#endif

                f << "\n";
                f.close();
        }
        return 0;
}

// END
