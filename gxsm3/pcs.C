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


/*

to clean dconf database and reset run as user:
dconf reset -f /org/gnome/gxsm3/

The Gtk_EntryControl class is used to create  most of the input fields of GXSM. It handles
the units and supports arrow keys and (optional) adjustments.
Beware: Gtk_EntryControl is derived from Param_Control which contains some of the important
functions and variables. Some of the functions of Param_Control are replaced by functions of
the Gtk_EntryControl class. Look for 'virtual' functions in the header file.

'change' callbacks will be handled by Gtk_EntryControl::Set_Parameter, so this is always a
good starting point.
*/

#include <stdlib.h>
#include <config.h>
#include <glib.h>
#include <glib/gi18n.h>

#include <iostream>
#include "pcs.h"
#include "util.h"
#include "xsmdebug.h"
#include "gxsm/plugin.h"
#include "gapp_service.h"
#include "gnome-res.h"

#define EC_INF 1e133

int total_message_count = 0;
int total_message_count_int = 0;

gint global_pcs_count = 0;
gboolean pcs_schema_writer_first_entry = true;
gboolean pcs_adj_schema_writer_first_entry = true;

// used for PCS VALUE GSCHEMA GENERATION
gchar *current_value_path = NULL;
gchar *generate_pcs_value_gschema_path_add_prev = NULL; // prev. schema name

// used for PCS ADJUSTEMNENTS GSCHEMA GENERATION
gchar *current_pcs_path = NULL;
gchar *generate_pcs_adjustment_gschema_path_add_prev = NULL; // prev. schema name

gboolean pcs_adj_write_missing_schema_entry = false;

Param_Control::Param_Control(UnitObj *U, const char *W, double *V, double VMi, double VMa, const char *p){
	XSM_DEBUG (DBG_L8, "PCS-double:: " << *V << ", " << VMi << ", " << VMa << ", " << p);
	unit = U->Copy ();
	warning = W ? g_strdup(W) : NULL;
	Dval = V; Ival=0; ULval=0; Sval=0;
	Init();
	setMin (VMi);
	setMax (VMa);
	prec = g_strdup(p);
}

Param_Control::Param_Control(UnitObj *U, const char *W, unsigned long *V, double VMi, double VMa, const char *p){
	XSM_DEBUG (DBG_L8, "PCS-ulong:: " << *V << ", " << VMi << ", " << VMa << ", " << p);
	unit = U->Copy ();
	warning = W ? g_strdup(W) : NULL;
	ULval = V; Ival=0; Dval=0; Sval=0;
	Init();
	setMin (VMi);
	setMax (VMa);
	prec = g_strdup(p);
}

Param_Control::Param_Control(UnitObj *U, const char *W, int *V, double VMi, double VMa, const char *p){
	XSM_DEBUG (DBG_L8, "PCS-int:: " << *V << ", " << VMi << ", " << VMa << ", " << p);
	unit = U->Copy ();
	warning = W ? g_strdup(W) : NULL;
	Ival = V; ULval=0; Dval=0; Sval=0;
	Init();
	setMin (VMi);
	setMax (VMa);
	prec = g_strdup(p);
}

Param_Control::Param_Control(UnitObj *U, const char *W, short *V, double VMi, double VMa, const char *p){
	XSM_DEBUG (DBG_L8, "PCS-short:: " << *V << ", " << VMi << ", " << VMa << ", " << p);
	unit = U->Copy ();
	warning = W ? g_strdup(W) : NULL;
	Sval = V; ULval=0; Ival=0; Dval=0;
	Init();
	setMin (VMi);
	setMax (VMa);
	prec = g_strdup(p);
}

Param_Control::~Param_Control(){
	if (warning)
		g_free (warning);
	if (color)
		g_free (color);
	if (warn_color[0])
		g_free (warn_color[0]);
	if (warn_color[1])
		g_free (warn_color[1]);
	g_free(prec);
	if (refname)
		g_free (refname);
	if (info)
		g_free (info);

        if (gsettings_adj_path)
                g_free (gsettings_adj_path);
        if (gsettings_path)
                g_free (gsettings_path);
        if (gsettings_adj_path_dir)
                g_free (gsettings_adj_path_dir);
        if (gsettings_path_dir)
                g_free (gsettings_path_dir);
        if (gsettings_key)
                g_free (gsettings_key);

        if (pcs_settings)
                g_clear_object (&pcs_settings);

        delete (unit);
}

void Param_Control::Init(){
	set_exclude ();
	color = NULL;
	warn_color[0] = NULL;
	warn_color[1] = NULL;
	ChangeNoticeFkt = NULL;
	FktData = NULL;
	refname = NULL; // g_strdup_printf ("pcs%04d", ++global_pcs_count);
	info=NULL;
	ShowMessage_flag=0;
	set_log (PARAM_CONTROL_LOG_MODE_OFF);

        // GSettings 
        pc_head = NULL; 
        pc_next = NULL;
        pc_count = 0;
        gsettings_adj_path_dir = NULL;
        gsettings_path_dir = NULL;
        gsettings_adj_path = NULL;
        gsettings_path = NULL;
        gsettings_key = NULL;
        g_variant_var = NULL;
        pcs_settings = NULL;
}

void Param_Control::Val(double *V){
	Dval = V; Ival=0; Sval=0;
}

void Param_Control::Val(int *V){
	Ival = V; Dval=0; Sval=0;
}

void Param_Control::Val(unsigned long *V){
	ULval=V; Ival = 0; Dval=0; Sval=0;
}

void Param_Control::Val(short *V){
	Sval = V; Ival=0; Dval=0;
}

void Param_Control::setMax(double VMa, double Vmax_warn, const gchar* w_color){
	vMax = VMa;
	vMax_warn = Vmax_warn;
	if (warn_color[1]){
		g_free (warn_color[1]);
		warn_color[1] = NULL;
	}
	if (w_color) warn_color[1] = g_strdup (w_color);
	update_limits ();
}
  
void Param_Control::setMin(double VMi, double Vmin_warn, const gchar* w_color){
	vMin = VMi;
	vMin_warn = Vmin_warn;
	if (warn_color[0]){
		g_free (warn_color[0]);
		warn_color[0] = NULL;
	}
	if (w_color) warn_color[0] = g_strdup (w_color);
	update_limits ();
}

void Param_Control::set_exclude(double V_ex_lo, double V_ex_hi){
	v_ex_lo = V_ex_lo;
	v_ex_hi = V_ex_hi;
	update_limits ();
}

void Param_Control::set_info (const gchar* Info){
	if (info)
		g_free (info);
	info = NULL;
	if (Info)
		info = g_strdup (Info);	
}

void Param_Control::Prec(const char *p){
	g_free(prec);
	prec = g_strdup(p);
}

gint Param_Control::ShowMessage(const char *txt, const char *options, gint default_choice){
	XSM_DEBUG(DBG_L2, txt );
	if (options)
		XSM_DEBUG(DBG_L2, options );

	return default_choice;
}

void Param_Control::Put_Value(){
	gchar *txt = Get_UsrString ();
	XSM_DEBUG(DBG_L2, txt);
        g_free (txt);
}

double Param_Control::Get_dValue(){
	if(Dval)
		return *Dval;
	else
		if(Ival)
			return (double)*Ival;
		else
			if(Sval)
				return (double)*Sval;
			else
				if(ULval)
					return (double)*ULval;
	return 0.;
}

void Param_Control::Set_dValue(double nVal){
	if(Dval)
		*Dval = nVal;
	else
		if(Ival)
			*Ival = (int)nVal;
		else
			if(Sval)
				*Sval = (short)nVal;
			else
				if(ULval)
					*ULval = (unsigned long)nVal;
}

void Param_Control::set_refname(const gchar *ref){
        if (refname) g_free (refname);
        refname = g_strdup(ref);

        if (pc_count == 0){
                if (generate_pcs_gschema)
                        write_pcs_gschema ();
                else
                        get_init_value_from_settings ();
        }
}

gchar *Param_Control::get_refname(){
	gchar *txt;	
        // XSM_DEBUG (DBG_L5, "Refname=" << refname );
	txt = g_strdup(refname);
	return txt;
}


gchar *Param_Control::Get_UsrString(){
	gchar *warn;
	if (color) g_free (color);
	color = NULL;
	if (Get_dValue() <= vMin_warn && warn_color[0]){
		color = g_strdup (warn_color[0]);
		warn = g_strdup_printf ("low (<%g)", vMin_warn);
	}else if (Get_dValue() >= vMax_warn && warn_color[1]){
		color = g_strdup (warn_color[1]);	
		warn = g_strdup_printf ("hi (>%g)", vMax_warn);
	}else
		warn = g_strdup (" ");

	gchar *txt;	
	if (strncmp (prec, "04X", 3) == 0){
                gchar *fmt = g_strdup_printf("%%%s %s %s %s", prec, unit->Symbol(), info?info:" ", warn);
		int h = (int)unit->Base2Usr (Get_dValue ());
		txt = g_strdup_printf(fmt, (int)unit->Base2Usr(Get_dValue()));
                g_free(fmt);
		XSM_DEBUG (DBG_L5, "Param_Control::Get_UsrString -- PCS H Usr out: " << h << " ==> " << txt);
	}
	else{
                txt = g_strdup_printf("%s %s %s", unit->UsrString (Get_dValue()) , info? info:" ", warn);
        }
        g_free(warn);

	return txt;
}


void Param_Control::Set_FromValue(double nVal){
	if (ShowMessage_flag) return;	//do nothing if a message dialog is active
	new_value = nVal;
	if(nVal <= vMax && nVal >= vMin){
		if(nVal >= vMax_warn || nVal <= vMin_warn){
			//check if the current value is already inside the warning range
			if (Get_dValue() >= vMax_warn || Get_dValue() <= vMin_warn){
				Set_dValue(nVal);
			}
			else{
				if (warn_color[0] || warn_color[1]){
					;
				} else {
					//The input exceeds the warning limits
					//Put_Value will reset the entry to the current valid value
					//Changes will eventually be done by the CB of the ShowMessage dialog
					gchar *ref = g_strconcat("[",refname ? refname:" ","]",NULL);
					gchar *txt = g_strdup_printf("Do you really want to\nenter the WARNING range?\n\n%s = %g", ref, new_value);
					ShowMessage (txt, "Warning Limit reached!", 1);
					XSM_DEBUG(DBG_L2, "Warning Limit reached! new_value=" << new_value );
                                        g_free (ref);
                                        g_free (txt);
				}
			}
			Put_Value ();
			return;
		}

		if(nVal >= v_ex_lo && nVal <= v_ex_hi){
			if (Get_dValue() >= v_ex_lo && Get_dValue() <= v_ex_hi){
				Set_dValue(nVal);
			}
			else{
				gchar *ref = g_strconcat("[",refname ? refname:" ","]",NULL);
				gchar *txt = g_strdup_printf("Do you really want to\nenter the EXCLUDE range?\n\n%s = %g", ref, new_value);
				ShowMessage ((const gchar*)txt, "Exclude Limit reached!", 2);
				XSM_DEBUG(DBG_L2, "Exclude Limit reached! new_value=" << new_value );
                                g_free (ref);
                                g_free (txt);
			}
			Put_Value ();
			return;
		}
		Set_dValue(nVal);
	}
	else{
		gchar *ref = g_strconcat("[",refname ? refname:" ","]",NULL);
		gchar *txt = g_strdup_printf("%s %s: %s ... %s", 
					     MLD_VALID_RANGE,
					     ref,
					     unit->UsrString (vMin), 
					     unit->UsrString (vMax));

		if (warning)
			ShowMessage(txt);
		else
			g_print ("%s", txt);

		g_free(txt);
		g_free(ref);
	}
	Put_Value();
}

void Param_Control::Set_Parameter(double value, int flg, int usr2base){
	if(flg){
		if(usr2base)
			Set_FromValue(unit->Usr2Base(value));
		else
			Set_FromValue(value);
	}else{
		gchar *ctxt = Get_UsrString ();
		XSM_DEBUG(DBG_L2, "Set Value [" << ctxt << "] = " );
		std::cin >> ctxt;
		Set_FromValue(unit->Usr2Base(ctxt));
                g_free (ctxt);
	}
	if(ChangeNoticeFkt)
		(*ChangeNoticeFkt)(this, FktData);
}

void Gtk_EntryControl::init_pcs_gsettings_path_and_key (){
        if (refname && gsettings_path == NULL){
                
                gsettings_path_dir = g_strdup_printf ("%s/pcs/%s", GXSM_RES_BASE_PATH, pcs_get_current_gschema_group ());
                gsettings_path = g_strdup_printf ("%s.pcs.%s", GXSM_RES_BASE_PATH_DOT, pcs_get_current_gschema_group ());
                gsettings_key = key_assure (refname);
        }
        gsettings_adj_path_dir = g_strdup_printf ("%s/pcsadjustments/%s", GXSM_RES_BASE_PATH, pcs_get_current_gschema_group ());
        gsettings_adj_path = g_strdup_printf ("%s.pcsadjustments.%s", GXSM_RES_BASE_PATH_DOT, pcs_get_current_gschema_group ());
}

void Gtk_EntryControl::write_pcs_gschema (int array_flag){

        if (generate_pcs_gschema && refname) {
                std::ofstream logf; logf.open ("gxsm-pcs-writer.log",  std::ios::app);
                std::ofstream f;
                std::ifstream fi;

                if (!check_gsettings_path ()){
                        logf << "Error: GSettings Path not set for '" << refname << "'. [" << array_flag << "], " << std::endl;
                        return;
                }

                gchar *tmppathname = g_strdup_printf ("%s.gschema.xml", gsettings_path);
                
                if (current_value_path)
                        g_free (current_value_path);
                current_value_path = tmppathname;
                //                                            x
                logf << "opening pcs-value xml file new for:  " << current_value_path
                     << "  [" << refname << "] array_flag=" << array_flag << " count=" << get_count () << std::endl;

                if (! g_file_test (tmppathname, G_FILE_TEST_EXISTS)){
                        f.open (current_value_path, std::ios::out);
                        f << "<schemalist>" << std::endl;
                        f.close ();
                }

                // -- identify end tag and truncate if exists for continuation
                //  </schema> 
                //</schemalist>
                fi.open (current_value_path, std::ios::in);
                fi.seekg (-28, std::ios_base::end);
                if (!fi.fail ()){
                        gchar x[1024]; x[0]=0;
                        fi.getline (x, 1024);
                        //                        logf << "pcs-end-read: [" << x << "]" << std::endl;
                        if (strcmp (x, "  </schema>") == 0){ // overwrite from here!
                                fi.seekg (-28, std::ios_base::end);
                                off_t pos = fi.tellg ();
                                fi.close ();
                                truncate(current_value_path, pos);
                                //      logf << "  ---> ov from eof-26 at " << pos << std::endl;
                        } else
                                fi.close ();
                } else
                        fi.close ();
                
                logf.close ();

                f.open (current_value_path, std::ios::app);

                
                if (!generate_pcs_value_gschema_path_add_prev)
                        generate_pcs_value_gschema_path_add_prev = g_strdup("dummy");

                if (strcmp (pcs_get_current_gschema_group (), generate_pcs_value_gschema_path_add_prev)){
                        g_free (generate_pcs_value_gschema_path_add_prev);
                        generate_pcs_value_gschema_path_add_prev = g_strdup (pcs_get_current_gschema_group ());

                        f << "  <schema id=\"" << gsettings_path
                          << "\" path=\"/" << gsettings_path_dir << "/\">" 
                          << std::endl;
                }

                gchar *us = Get_UsrString();
                if (!us) us = g_strdup ("N/A");
                const gchar *cpn = ((const gchar*)g_object_get_data( G_OBJECT (entry), "Adjustment_PCS_Name"));
                gchar *pn;
                if (!cpn) pn = g_strdup ("PCS has no adjustment");
                else pn = g_strdup_printf ("Adjustment PCS Name: '%s'", cpn);

                gchar *pcs_id=g_strdup_printf ("pcs%04d", global_pcs_count);

                if (array_flag && pc_count == 1){

                        f << std::endl
                          << "    <key name=\"" << gsettings_key << "\" type=\"ad\">" << std::endl
                          << "      <default>[" << Get_dValue() ;

                } else if (array_flag && pc_next == NULL ){

                        f << "," << Get_dValue() << "]</default>" << std::endl
                          << "      <summary>PCS Remote-ID: '" << refname << "', Default/U: " << us << " </summary>" << std::endl
                          << "      <description>" << pcs_id << ", Array-N: " << pc_count << ", PCS Remote-ID: '" << refname << "', Default/U: " << us << ", " << pn << " </description>" << std::endl
                          << "    </key>"  << std::endl;
                        
                } else if (array_flag){
                        f << "," << Get_dValue() ;
                } else {
                       
                        f << std::endl
                          << "    <key name=\"" << gsettings_key << "\" type=\"d\">" << std::endl
                          << "      <default>" << Get_dValue() << "</default>" << std::endl
                          << "      <summary>PCS Remote-ID: '" << refname << "', Default/U: " << us << " </summary>" << std::endl
                          << "      <description>" << pcs_id << ", PCS Remote-ID: '" << refname << "', Default/U: " << us << ", " << pn << " </description>" << std::endl
                          << "    </key>"  << std::endl;
                }

                g_free (pcs_id);
                g_free (us);
                g_free (pn);

                f << "  </schema>"  << std::endl << std::endl;
                f << "</schemalist>"  << std::endl << std::endl;
                
                f.close ();
        }
}

void Gtk_EntryControl::get_init_value_from_settings (int array_flag){
        if (check_gsettings_path ()){
                XSM_DEBUG_GP (DBG_L2, "PCS init: looking for %s.%s array_flag=%d pcs_count=%d\n", gsettings_path, gsettings_key, array_flag, get_count ());
                
                if (!array_flag && get_count () == 0){
                        if (pcs_settings == NULL)
                                pcs_settings = g_settings_new (gsettings_path);
                        Set_FromValue (g_settings_get_double (pcs_settings, gsettings_key));
                } else
                        init_pcs_via_list ();
        }
}
 
void Gtk_EntryControl::update_value_in_settings (int array_flag){
        if (check_gsettings_path ()){
                
                XSM_DEBUG_GP (DBG_L2, "PCS update value for %s.%s[%d] = %g\n", gsettings_path, gsettings_key, get_count (), Get_dValue () );
                
                if (!array_flag && get_count () == 0){
                        if (pcs_settings == NULL)
                                pcs_settings = g_settings_new (gsettings_path);
                        g_settings_set_double (pcs_settings, gsettings_key, Get_dValue());
                } else { 
                        if (pc_head && get_count () > 0){
                                unsigned int i=0;
                                Param_Control *pc_last;
                                for (Param_Control *pc = pc_head; pc; pc = pc->get_iter_next ())
                                     pc_last = pc;
                                gsize gn = pc_last->get_count ();
                                gdouble *arr = g_new (gdouble, gn);
      
                                for (Param_Control *pc = pc_head; pc && i<gn; pc = pc->get_iter_next (), ++i)
                                        arr[i] = pc->Get_dValue();

                                if (pc_head->check_gsettings_path ()){
                                        if (pcs_settings == NULL)
                                                pcs_settings = g_settings_new (pc_head->get_gsettings_path ());
                                        
                                        g_variant_var = g_variant_new_fixed_array (G_VARIANT_TYPE_DOUBLE, (gconstpointer) arr, gn, sizeof (gdouble));
                                        g_settings_set_value (pcs_settings, pc_head->get_gsettings_key (), g_variant_var);

                                        //if (g_variant_var)
                                        // g_variant_unref (g_variant_var);  // not good to do here? -- settings is holing a  ref?
                                }
                        }
                }
        }
}

void  Gtk_EntryControl::init_pcs_via_list (){
       // must be last element
       if (pc_head && pc_next == NULL){
               if (pc_head->check_gsettings_path ()){
                       XSM_DEBUG_GP (DBG_L2, "PCS init list for %s.%s [%d]\n", pc_head->get_gsettings_path (),  pc_head->get_gsettings_key (),  pc_head->get_count () );
                       gsize gn;
                       if (pcs_settings == NULL)
                               pcs_settings = g_settings_new (pc_head->get_gsettings_path ());

                       g_variant_var = g_settings_get_value (pcs_settings, pc_head->get_gsettings_key ()) ;
                       gdouble *arr = (gdouble*) g_variant_get_fixed_array (g_variant_var, &gn, sizeof (gdouble));

                       guint i=0;
                       for (Param_Control *pc = pc_head; pc && i<gn; pc = pc->get_iter_next (), ++i)
                               pc->Set_FromValue (arr[i]);

                       // g_variant_unref (g_variant_var); // not good to do here? -- settings is holing a  ref?
               }
       }
}

void Gtk_EntryControl::adjustment_callback(GtkAdjustment *adj, Gtk_EntryControl *gpcs){
//	static GTimer* changed_delay = NULL;
	if (!adj) return;
//	if (!adj && changed_delay) { 
//		g_timer_destroy (changed_delay); 
//		changed_delay = NULL; 
//		return;
//	} // cleanup!

	switch (gpcs->log_mode){
	case PARAM_CONTROL_LOG_MODE_OFF:
                gpcs->Set_Parameter (gtk_adjustment_get_value (adj), TRUE, FALSE);
                break;
	case PARAM_CONTROL_LOG_MODE_LOG: {
		double value  = gpcs->Get_dValue ();
		double signum = value >= 0.? 1.:-1.;
		double scale = fabs (gtk_adjustment_get_upper (adj)) / (pow (2.,fabs (gtk_adjustment_get_upper (adj)))-1.);
		double a = signum * scale * (pow (2.,fabs(gtk_adjustment_get_value (adj)))-1.);
		XSM_DEBUG (DBG_L6,
                           "Gtk_EntryControl::adjustment_callback -- get adj[" << (gtk_adjustment_get_upper (adj))
                           << "]: bv (ist)=" << value
                           << " av(adj read)=" << (gtk_adjustment_get_value (adj))
                           << " av2bv(new set)=" << a
                           );
		gpcs->Set_Parameter(a, TRUE, FALSE);
//		gpcs->Set_Parameter(adj->value, TRUE, FALSE);
	} break;
	case PARAM_CONTROL_LOG_MODE_AUTO_RANGING: {
                if (fabs (gtk_adjustment_get_value (adj)) < 0.5 && gtk_adjustment_get_upper (adj) > 1.0){
                        gtk_adjustment_set_upper (adj, 1.0), gtk_adjustment_set_lower (adj,-1.0);
                } else if (fabs (gtk_adjustment_get_value (adj)) < 4.0 && gtk_adjustment_get_upper (adj) > 3.0){
		        gtk_adjustment_set_upper (adj, 3.0), gtk_adjustment_set_lower (adj, -3.0);
		}
		if (fabs (gtk_adjustment_get_value (adj)) > 0.99 && gtk_adjustment_get_upper (adj) < 3.0){
		        gtk_adjustment_set_upper (adj, 3.0), gtk_adjustment_set_lower (adj, -3.0);
		  } else if (fabs (gtk_adjustment_get_value (adj)) > 2.99 && gtk_adjustment_get_upper (adj) < gpcs->vMax){
		        gtk_adjustment_set_upper (adj, gpcs->vMax), gtk_adjustment_set_lower (adj, gpcs->vMin);
		    }
		    //		vMin
	    gpcs->Set_Parameter (gtk_adjustment_get_value (adj), TRUE, FALSE);
	    } break;
	case PARAM_CONTROL_LOG_MODE_AUTO_DUAL_RANGE: { // vMin_warn
                if (gpcs->vMax_warn < 1e110){ // warn set other than default (off)?
                        double  v=gtk_adjustment_get_value (adj);
                        double al=gtk_adjustment_get_lower (adj);
                        double au=gtk_adjustment_get_upper (adj);

                        double l,r;
                        l=r=0.;
                        // value inside "Warn" Range, auto adjust
                        if (al < gpcs->vMin_warn && au > gpcs->vMax_warn && v > gpcs->vMin_warn && v < gpcs->vMax_warn){
                                gtk_adjustment_set_upper (adj, gpcs->vMax_warn), gtk_adjustment_set_lower (adj, gpcs->vMin_warn);
                                l=gpcs->vMin_warn;
                                r=gpcs->vMax_warn;
                        }
                        // value at edge(s) and Warn Range active, switch back to full scale
                        if (fabs (al-gpcs->vMin_warn) < 1e-6 && fabs (au-gpcs->vMax_warn) < 1e-6
                            && (fabs (v-gpcs->vMin_warn) < 1e-6 || fabs (v-gpcs->vMax_warn) < 1e-6)){
                                gtk_adjustment_set_upper (adj, gpcs->vMax), gtk_adjustment_set_lower (adj, gpcs->vMin);
                                l=gpcs->vMin;
                                r=gpcs->vMax;
                        }
                        if (l != 0. && r != 0.){
                                if (gpcs->opt_scale){
                                        gtk_scale_clear_marks (GTK_SCALE (gpcs->opt_scale));
                                        double tic_w = r-l;
                                        double d_tic = AutoSkl(tic_w/11);
                                        double tic_0 = l;
                                        for(double x=AutoNext (tic_0, d_tic); x < r; x += d_tic){
                                                if (fabs(x/d_tic) < 1e-3)
                                                        x=0.;
                                                gchar *tmp = g_strdup_printf("<span size=\"x-small\">%g</span>", x);
                                                gtk_scale_add_mark (GTK_SCALE (gpcs->opt_scale), x, GTK_POS_BOTTOM, tmp);
                                                g_free (tmp);
                                                if (x+d_tic/2 < r)
                                                        gtk_scale_add_mark (GTK_SCALE (gpcs->opt_scale), x+d_tic/2, GTK_POS_BOTTOM, NULL);
                                        }
                                }
                        }
                }
                gpcs->Set_Parameter (gtk_adjustment_get_value (adj), TRUE, FALSE);
        } break;
        default:
                gpcs->Set_Parameter (gtk_adjustment_get_value (adj), TRUE, FALSE);
                break;
	}
}

void Gtk_EntryControl::Put_Value(){
	gchar *txt = Get_UsrString ();
	gtk_entry_set_text (GTK_ENTRY (entry), txt);
        g_free (txt);
        
	if (color) {
		GdkRGBA bgc;
		gdk_rgba_parse (&bgc,color);
                gtk_widget_override_color(GTK_WIDGET (entry), GTK_STATE_FLAG_NORMAL, &bgc); 		
	} else {
                gtk_widget_override_color( GTK_WIDGET(entry), GTK_STATE_FLAG_NORMAL, NULL); 		
	}
        
	if(adj){
		switch (log_mode){
                case PARAM_CONTROL_LOG_MODE_LOG: {
			double value = unit->Usr2Base (Get_dValue ());
			double scale = fabs (gtk_adjustment_get_upper (GTK_ADJUSTMENT(adj))) / (pow (2.,fabs(gtk_adjustment_get_upper (GTK_ADJUSTMENT(adj))))-1.);
			double v = log (1. + fabs(value) / scale) / log (2.);
			gtk_adjustment_set_lower (GTK_ADJUSTMENT (adj), 0.);
                        XSM_DEBUG (DBG_L5,
                                   "put adj[" << (gtk_adjustment_get_upper (GTK_ADJUSTMENT(adj)))
                                   << "]: bv=" << value
                                   << " av=" << v
                                   );
			gtk_adjustment_set_value(GTK_ADJUSTMENT(adj), v);
//			gtk_adjustment_set_value(GTK_ADJUSTMENT(adj), unit->Usr2Base(Get_dValue()));

			double a = scale * (pow (2.,fabs(value))-1.);
                        XSM_DEBUG (DBG_L5,
                                   "Compute Fwd (" << value << ") = " << a
                                   );
			v = log (1. + fabs(a) / scale) / log (2.);
                        XSM_DEBUG (DBG_L5,
                                   "Compute Rev (" << a << ") = " << v
                                   );

		} break;
                default: // PARAM_CONTROL_LOG_MODE_AUTO_RANGING, PARAM_CONTROL_LOG_MODE_OFF
			gtk_adjustment_set_value (GTK_ADJUSTMENT(adj), unit->Usr2Base (Get_dValue()));
                        break;
                }
	}
}

void Gtk_EntryControl::Set_Parameter(double Value=0., int flg=FALSE, int usr2base=FALSE){
	if (ShowMessage_flag) return;	//do nothing if a message dialog is active
	double value;
	GtkWidget *c;
	if(flg){
		if(usr2base)
			value=unit->Usr2Base(Value);
		else
			value=Value;
	}
	else{
		const gchar *ctxt = gtk_entry_get_text (GTK_ENTRY (entry));
		if (strncmp (prec, "04X", 3) == 0){
			int h;
			sscanf (ctxt, "%x", &h);
			value=(double)h;
                        XSM_DEBUG (DBG_L1, "Gtk_EntryControl::Set_Parameter -- PCS H: " << ctxt << " ==> " << value);
		} else
			value=unit->Usr2Base (ctxt);
	}

	Set_FromValue (value);
        update_value_in_settings ();

	if((c=(GtkWidget*)g_object_get_data( G_OBJECT (entry), "HasClient")) && enable_client){
		Gtk_EntryControl *cec = (Gtk_EntryControl *) g_object_get_data( G_OBJECT (c), "Gtk_EntryControl");
                cec->Set_FromValue (new_value);
                cec->update_value_in_settings ();
        }
	if(ChangeNoticeFkt) (*ChangeNoticeFkt)(this, FktData);
}

void Gtk_EntryControl::Set_NewValue(bool set_new_value){
	GtkWidget *c;
	if (!set_new_value){
		ShowMessage_flag=0;
		return;
	}

	Set_dValue(new_value);
	Put_Value ();

	if((c=(GtkWidget*)g_object_get_data( G_OBJECT (entry), "HasClient")) && enable_client)
		((Gtk_EntryControl *)g_object_get_data( G_OBJECT (c), "Gtk_EntryControl"))->Set_FromValue(new_value);

	// The ChangeNoticeFkt commits the current value to the DSP
	if(ChangeNoticeFkt) (*ChangeNoticeFkt)(this, FktData);
	ShowMessage_flag=0;
}

int Gtk_EntryControl::force_button_callback(gpointer ec_object, gpointer dialog){
	--total_message_count;
	((Gtk_EntryControl *)ec_object)->Set_NewValue (TRUE);
	return FALSE;
}

int Gtk_EntryControl::cancel_button_callback(gpointer ec_object, gpointer dialog){
	--total_message_count;
	((Gtk_EntryControl *)ec_object)->Set_NewValue (FALSE);
	return FALSE;
}

int Gtk_EntryControl::quit_callback(gpointer ec_object, gpointer dialog){
	--total_message_count;
	((Gtk_EntryControl *)ec_object)->Set_NewValue (FALSE);
	return FALSE;
}

gint Gtk_EntryControl::ShowMessage(const char *txt, const char *options, int default_choice){

//	gpointer current_object = this;

	if (!txt) 
		return 0;

	++total_message_count_int;
	if (++total_message_count > 6){
		std::cerr << "[" << total_message_count_int << "] Repeted Messag(es), popup blocked: " << txt << std::endl;
		--total_message_count;
		return 0;
	}


	if (options){
		//construction of a warning window
		GtkWidget *dialog;
		GtkWidget *label;
		GtkWidget *vbox;
		GtkWidget *stock_item;
		GtkWidget *text_box;
		GtkWidget *action_box;


		dialog = gtk_window_new (GTK_WINDOW_TOPLEVEL);
		gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);
		gtk_window_set_position (GTK_WINDOW (dialog), GTK_WIN_POS_CENTER);
		gtk_window_set_keep_above (GTK_WINDOW (dialog), TRUE);
		gtk_window_set_title (GTK_WINDOW (dialog), options);

		vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
		gtk_widget_show (vbox);
		gtk_container_add (GTK_CONTAINER (dialog), vbox);

		//warning box consist of a text region a separator and a button region
		text_box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
		gtk_container_set_border_width (GTK_CONTAINER (text_box), 10);
		gtk_box_set_spacing (GTK_BOX (text_box), 10);
		gtk_widget_show (text_box);
		gtk_box_pack_start (GTK_BOX (vbox), text_box, FALSE, FALSE, 0);
		
		GtkWidget *separator = gtk_separator_new (GTK_ORIENTATION_HORIZONTAL);
		gtk_widget_show (separator);
		gtk_box_pack_start (GTK_BOX (vbox), separator, FALSE, FALSE, 0);

		action_box = gtk_button_box_new (GTK_ORIENTATION_HORIZONTAL);
		gtk_container_set_border_width (GTK_CONTAINER (action_box), 10);
		gtk_button_box_set_layout (GTK_BUTTON_BOX (action_box), GTK_BUTTONBOX_END);
		gtk_widget_show (action_box);
		gtk_box_pack_start (GTK_BOX (vbox), action_box, FALSE, FALSE, 0);


		//fill up text area with image and text
		stock_item = gtk_image_new_from_icon_name("dialog-password", GTK_ICON_SIZE_DIALOG);
		gtk_widget_show (stock_item);
		gtk_box_pack_start (GTK_BOX (text_box), stock_item, FALSE, FALSE, 0);

		label = gtk_label_new (N_(txt));
		gtk_widget_show (label);
		gtk_box_pack_start (GTK_BOX (text_box), label, FALSE, FALSE, 0);

		//fill up the button box with the yes and cancel button
		GtkWidget *force_button = gtk_button_new_with_mnemonic (_("_Yes"));
		gtk_widget_show (force_button);
		gtk_box_pack_start (GTK_BOX (action_box), force_button, FALSE, FALSE, 0);
		g_signal_connect_swapped (G_OBJECT (force_button), "clicked",
			    G_CALLBACK (Gtk_EntryControl::force_button_callback), this);
		g_signal_connect_swapped (G_OBJECT (force_button), "clicked",
			    G_CALLBACK (gtk_widget_destroy),  G_OBJECT (dialog));

		GtkWidget *cancel_button = gtk_button_new_with_mnemonic (_("_Cancel"));
		gtk_widget_show (cancel_button);
		gtk_box_pack_start (GTK_BOX (action_box), cancel_button, FALSE, FALSE, 0);
		g_signal_connect_swapped (G_OBJECT (cancel_button), "clicked",
			    G_CALLBACK (Gtk_EntryControl::cancel_button_callback), this);
		g_signal_connect_swapped (G_OBJECT (cancel_button), "clicked",
			    G_CALLBACK (gtk_widget_destroy),  G_OBJECT (dialog));

		//catching a delete event (eg. clicking on the x of the window or pressing Alt+F4)
		g_signal_connect_swapped (G_OBJECT (dialog), "delete-event",
			    G_CALLBACK (Gtk_EntryControl::quit_callback), this);

		gtk_widget_show (dialog);
		ShowMessage_flag=default_choice;

		return 1;
	} else {
                GtkWidget *toplevel = gtk_widget_get_toplevel (entry);  // try to get the underlying window as parent for dialog
                GtkWindow *ww = NULL;
                if (gtk_widget_is_toplevel (toplevel))
                        ww = GTK_WINDOW (toplevel);
                GtkWidget *dialog = gtk_message_dialog_new (ww,
                                                            GTK_DIALOG_DESTROY_WITH_PARENT,
                                                            GTK_MESSAGE_INFO,
                                                            GTK_BUTTONS_CLOSE,
                                                            "%s", txt);
	
		g_signal_connect_swapped (G_OBJECT (dialog), "response",
					  G_CALLBACK (gtk_widget_destroy),
					  G_OBJECT (dialog));
		gtk_widget_show (dialog);

		--total_message_count;
		return default_choice;
	}

	return -1;
}


gint Gtk_EntryControl::update_callback(GtkEditable *editable, void *data){
	Gtk_EntryControl *current_object = ((Gtk_EntryControl *)g_object_get_data( G_OBJECT (editable), "Gtk_EntryControl"));
	current_object->Set_Parameter (atof ( gtk_editable_get_chars (editable, 0 , -1)), FALSE, FALSE);
	return FALSE;
}

void Gtk_EntryControl::pcs_adjustment_configure (){
        UnitObj *unity = new UnitObj(" "," ");
        gchar *tmp = NULL;

	if (! (tmp = (gchar*)g_object_get_data( G_OBJECT (entry), "Adjustment_PCS_Name")))
		return;

	get_pcs_configuartion ();

	tmp = g_strconcat (N_("Configure"), 
			   " ",
			   (gchar*) g_object_get_data( G_OBJECT (entry), 
							 "Adjustment_PCS_Name"),
			   NULL);

        GtkDialogFlags flags =  (GtkDialogFlags) (GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT);
	GtkWidget *dialog = gtk_dialog_new_with_buttons (tmp,
							 GTK_WINDOW (gtk_widget_get_toplevel (entry)), // get underlying window as parent
                                                         flags,
							 _("_OK"), GTK_RESPONSE_ACCEPT,
							 _("_Cancel"), GTK_RESPONSE_REJECT,
							 NULL);
	g_free (tmp);

        BuildParam bp;
        bp.error_text = N_("Value not allowed.");

        gtk_container_add (GTK_CONTAINER (gtk_dialog_get_content_area (GTK_DIALOG (dialog))), bp.grid);

	tmp = g_strdup_printf (N_("Warning: know what you are doing here!" \
                                  "\nInfo: You may use the dconf-editor." \
                                  "\nD-Bus path is:"                    \
                                  "\n %s/%s [%d]"),
                               gsettings_path, gsettings_key, get_count () 
                               );
        bp.grid_add_label (tmp, NULL, 2); bp.new_line ();
	g_free (tmp);	
	bp.grid_add_ec ("Upper Limit", unit, &vMax, -EC_INF, EC_INF, "8g"); bp.new_line ();
	bp.grid_add_ec ("Upper Warn",  unit, &vMax_warn, -EC_INF, EC_INF, "8g"); bp.new_line ();
	bp.grid_add_ec ("Lower Warn",  unit, &vMin_warn, -EC_INF, EC_INF, "8g"); bp.new_line ();
	bp.grid_add_ec ("Lower Limit", unit, &vMin, -EC_INF, EC_INF, "8g"); bp.new_line ();

        bp.grid_add_widget (gtk_separator_new (GTK_ORIENTATION_HORIZONTAL), 2); bp.new_line ();
	bp.grid_add_ec ("Exclude Hi", unit, &v_ex_hi, -EC_INF, EC_INF, "8g"); bp.new_line ();
	bp.grid_add_ec ("Exclude Lo", unit, &v_ex_lo, -EC_INF, EC_INF, "8g"); bp.new_line ();

        if (GTK_IS_SPIN_BUTTON (entry)){
                bp.grid_add_widget (gtk_separator_new (GTK_ORIENTATION_HORIZONTAL), 2); bp.new_line ();
                bp.grid_add_ec ("Step [B1]", unit, &step, -EC_INF, EC_INF, "8g"); bp.new_line ();
                bp.grid_add_ec ("Page [B2]", unit, &page, -EC_INF, EC_INF, "8g"); bp.new_line ();
                bp.grid_add_ec ("Pg10 [B3]", unit, &page10, -EC_INF, EC_INF, "8g"); bp.new_line ();
                bp.grid_add_ec ("Progressive", unity, &progressive, 0., 1., "g"); bp.new_line ();
        }
        gtk_widget_show_all (dialog);
        
	gint result = gtk_dialog_run (GTK_DIALOG (dialog));

  	gtk_widget_destroy (dialog);

        delete unity;

	if (result == GTK_RESPONSE_ACCEPT)
		put_pcs_configuartion ();
}

#define XRM_GET_WD(L, V) tdv = g_strdup_printf ("%g", V); xrm.Get (L, &V, tdv); g_free (tdv)

void Gtk_EntryControl::get_pcs_configuartion (){
	XSM_DEBUG (DBG_L9, "GET-PCS-ADJ:\n");
        gchar *name = (gchar*) g_object_get_data( G_OBJECT (entry), "Adjustment_PCS_Name");

        if (name == NULL){
                XSM_DEBUG (DBG_L8, "GET-PCS-ADJ: return OK. No adjustment/remote requested.");
                return;
        }
        
	XSM_DEBUG (DBG_L8, "GET-PCS-ADJ: " << name << " count=" << get_count ());

        check_gsettings_path ();

        name = g_ascii_strdown (name, -1);
        // name = key_assure (name);
        
        gchar *dotpath =  g_strdup_printf ("%s", gsettings_adj_path);

	XSM_DEBUG (DBG_L8, "GET-PCS-ADJ: " << dotpath << "." << name);

        if (generate_pcs_adj_gschema || pcs_adj_write_missing_schema_entry){

                std::ofstream logf; logf.open ("gxsm-pcs-writer.log",  std::ios::app);
                std::ofstream f;
                std::ifstream fi;
                
                gchar *path    = g_strdup_printf ("/%s/",
                                                  gsettings_adj_path_dir
                                                  );
                
                gchar *tmppathname = g_strdup_printf ("%s.gschema%sxml",
                                                      gsettings_adj_path,
                                                      pcs_adj_write_missing_schema_entry ? ".missing." : "."
                                                      );

                if (current_pcs_path)
                        g_free (current_pcs_path);

                current_pcs_path = tmppathname;

                //                                            x
                logf << "opening pcs-adj xml file new for:    " << current_pcs_path << "[" << name << "] count=" << get_count () << std::endl;
                                
                if (! g_file_test (tmppathname, G_FILE_TEST_EXISTS)){
                        f.open (current_pcs_path,  std::ios::out);
                        f << "<schemalist>" << std::endl;
                        f.close ();
                        pcs_adj_schema_writer_first_entry = false;
                }

                // -- identify end tag and truncate if exists for continuation
                //  </schema> 
                //</schemalist>
                fi.open (current_pcs_path, std::ios::in);
                fi.seekg (-28, std::ios_base::end);
                if (!fi.fail ()){
                        gchar x[1024]; x[0]=0;
                        fi.getline (x, 1024);
                        //                        logf << "pcs-end-read: [" << x << "]" << std::endl;
                        if (strcmp (x, "  </schema>") == 0){ // overwrite from here!
                                fi.seekg (-28, std::ios_base::end);
                                off_t pos = fi.tellg ();
                                fi.close ();
                                truncate(current_pcs_path, pos);
                                //      logf << "  ---> ov from eof-26 at " << pos << std::endl;
                        } else
                                fi.close ();
                } else
                        fi.close ();
                
                logf.close ();

                //======
                
                f.open (current_pcs_path, std::ios::app); // continue writing

                if (!generate_pcs_adjustment_gschema_path_add_prev)
                        generate_pcs_adjustment_gschema_path_add_prev = g_strdup("dummy");

                if (strcmp (pcs_get_current_gschema_group (), generate_pcs_adjustment_gschema_path_add_prev)){
                        g_free (generate_pcs_adjustment_gschema_path_add_prev);
                        generate_pcs_adjustment_gschema_path_add_prev = g_strdup (pcs_get_current_gschema_group ());

                        f << "  <schema id=\"" << dotpath
                          << "\" path=\"" << path << "\">" 
                          << std::endl;
                }

                

                XSM_DEBUG_GP (DBG_L1, "Generating PCS Adjustment Schema for [%s.%s] at %s\n", dotpath, name, current_pcs_path);

                f << std::endl
                  << "    <key name=\"" << name << "\" type=\"ad\">" << std::endl
                  << "      <default>["
                  << vMax << ", "
                  << vMin << ", "
                  << vMax_warn << ", "
                  << vMin_warn << ", "
                  << v_ex_hi << ", "
                  << v_ex_lo << ", "
                  << step << ", "
                  << page << ", "
                  << page10 << ", "
                  << progressive << ", "
                  << " 0, 0"
                  << "      ]"
                  << "      </default>"
                  << std::endl
                  << "      <summary>Array of [Adjustments Absolute Limits: Max, -Min,  Warning Bounds: Max, -Min,  Exclude Range: Lo, -Hi, Spin: Step, Page, Page10, Progressive-mode, 0,0] for '" << name << "' </summary>"
                  << std::endl
                  << "      <description>Configuration for GXSM Entry with ID '" << name << "'. Setup of Min/Max Range (warnign: not checked for validity), Warning Bonds, Exclude Range and Spin Step/Page behavior. </description>"
                  << std::endl
                  << "    </key>"  << std::endl;

                f << "  </schema>"  << std::endl << std::endl;
                f << "</schemalist>" << std::endl << std::endl;

                f.close ();
 
                g_free (path);
                
                pcs_adj_write_missing_schema_entry = false;

        } else {

                // verify schema presence to avoid forced program termination
                GSettingsSchema *gs_schema_source = g_settings_schema_source_lookup (g_settings_schema_source_get_default (), dotpath, FALSE);
                if (!gs_schema_source){
                        g_print ("PCS: g_settings_schema_source_lookup () retured NULL while checking for '%s' Adjustment safed/default value, no schema installed!\n"
                                 "Please run gxsm3 --write-gxsm-pcs-adj-gschema, and copy the schema file(s) to the proper folder(s) and run make install.\n"
                                 "Ignoring problem and continuing with build in default mode.\n"
                                 "==> FYI: Writing missing schema to '%s.gschema%sxml' now.\n",
                                 dotpath,
                                 gsettings_adj_path,
                                 ".missing."
                                 );
                        
                        pcs_adj_write_missing_schema_entry = true;

                        // auto retry/write schema to tmp file
                        get_pcs_configuartion ();

                } else {
                        gdouble *array;
                        gsize n_stores;
                        GSettings *tmp_settings = g_settings_new (dotpath);
                        GVariant *storage = g_settings_get_value (tmp_settings, name);
                        array = (gdouble*) g_variant_get_fixed_array (storage, &n_stores, sizeof (gdouble));
                        if (n_stores < 10)
                                g_print ("PCS GSETTINGS DATA ERROR: g_settings_get_fixed_array returned the wrong number %d of elements:\n"
                                         " ===> Need at a minimum of 10 double values for key %s.%s.\nConfiguration",
                                         n_stores,
                                         dotpath,
                                         name);
                        else {
                                vMax = array[0];
                                vMin = array[1];
                                vMax_warn = array[2];
                                vMin_warn = array[3];
                                v_ex_lo = array[4];
                                v_ex_hi = array[5];
                                step = array[6];
                                page = array[7];
                                page10 = array[8];
                                progressive = array[9];
                        }  

                        g_clear_object (&tmp_settings);
                }
        }       
        g_free (name);
        g_free (dotpath);
}

#define XRM_PUT_WD(L, V) xrm.Put (L, V);

void Gtk_EntryControl::put_pcs_configuartion (){
	XSM_DEBUG(DBG_L2, "PUT-PCS-ADJ:\n");

        gchar *name = (gchar*) g_object_get_data( G_OBJECT (entry), "Adjustment_PCS_Name");

        if (name == NULL)
		return;
        
        if (generate_pcs_adj_gschema)
                return;
     
        name = g_ascii_strdown (name, -1);

        // gchar *dotpath =  g_strdup_printf ("%s.%s", gsettings_adj_path, name);
        gchar *dotpath =  g_strdup_printf ("%s", gsettings_adj_path);

        XSM_DEBUG(DBG_L2, "PCS: storing adjustment configuration [" << dotpath << "." << name << "]\n");

        // verify schema presence to avoid forced program termination
        GSettingsSchema *gs_schema_source = g_settings_schema_source_lookup (g_settings_schema_source_get_default (), dotpath, FALSE);
        if (!gs_schema_source){
                g_print ("PCS: g_settings_schema_source_lookup () retured NULL while checking for '%s' Adjustment safed/default value, no schema installed!\n"
                         "Please update/add schema via gxsm3 --write-gxsm-pcs-adj-gschema, edit file termination tag and run a make install.\n"
                         "Ignoring problem and continuing -- new settings will not be remembered but take effect for this session.\n",
                         dotpath);
        }  else {
        
                gsize n_stores = 10;
                gdouble *array = g_new (gdouble, n_stores);

                array[0] = vMax;
                array[1] = vMin;
                array[2] = vMax_warn;
                array[3] = vMin_warn;
                array[4] = v_ex_lo;
                array[5] = v_ex_hi;
                array[6] = step;
                array[7] = page;
                array[8] = page10;
                array[9] = progressive;

                GSettings *tmp_settings = g_settings_new (dotpath);
                GVariant *storage = g_variant_new_fixed_array (g_variant_type_new ("d"), array, n_stores, sizeof (gdouble));
                g_settings_set_value (tmp_settings, name, storage);

                g_clear_object (&tmp_settings);
        }

        g_free (name);
        g_free (dotpath);

	if (GTK_IS_SPIN_BUTTON (entry)){
		gtk_spin_button_configure (GTK_SPIN_BUTTON (entry), 
                                           GTK_ADJUSTMENT (adj), progressive, 0);	
		gtk_spin_button_set_increments (GTK_SPIN_BUTTON (entry),
                                                step, page);
	}
	if (adj){
	        gtk_adjustment_set_upper (GTK_ADJUSTMENT (adj), vMax);
		gtk_adjustment_set_lower (GTK_ADJUSTMENT (adj), vMin);
	}
}

static gint
ec_gtk_spin_button_sci_output (GtkSpinButton *spin_button)
{   
	gchar *buf = ((Gtk_EntryControl *)g_object_get_data( G_OBJECT (spin_button), "Gtk_EntryControl"))->Get_UsrString();
	if (strcmp (buf, gtk_entry_get_text (GTK_ENTRY (spin_button))))
		gtk_entry_set_text (GTK_ENTRY (spin_button), buf);
	g_free (buf);
	return TRUE;
}

static gint
ec_gtk_spin_button_sci_input (GtkSpinButton *spin_button,
			       gdouble       *new_val)
{
	gchar *err = NULL;
	*new_val = g_strtod (gtk_entry_get_text (GTK_ENTRY (spin_button)), &err);
	*new_val = (gdouble)(((Gtk_EntryControl *)g_object_get_data( G_OBJECT (spin_button), "Gtk_EntryControl"))->Convert2Base (*new_val));

	if (*err)
		return GTK_INPUT_ERROR;
	else
		return FALSE;
}

static void 
ec_pcs_adjustment_configure (GtkWidget *menuitem, Gtk_EntryControl *gpcs){
	gpcs->pcs_adjustment_configure ();
}


static void 
ec_pcs_populate_popup (GtkEntry *entry, GtkMenu *menu, Gtk_EntryControl* gpcs){
	GtkWidget *menuitem;

	menuitem = gtk_separator_menu_item_new ();
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
	gtk_widget_show (menuitem);

	gchar *cfg_label = g_strconcat ("Configure",
					" ",
					g_object_get_data( G_OBJECT (entry), 
                                                           "Adjustment_PCS_Name"),
					NULL);

	menuitem = gtk_menu_item_new_with_label (cfg_label);
	g_signal_connect (menuitem, "activate",
			  G_CALLBACK (ec_pcs_adjustment_configure), gpcs);

	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
	gtk_widget_show (menuitem);
}

void Gtk_EntryControl::InitRegisterCb(double AdjStep, double AdjPage, double AdjProg){
	adj=NULL;
	enable_client = TRUE;

	page10 = 10.*AdjPage;
	page   = AdjPage;
	step   = AdjStep;
	progressive = AdjProg;

        XSM_DEBUG (DBG_L8, "InitRegisterCb -- enter");
        // only for head EC if EC is part of an iter/array
        if (get_count () <= 1){
                get_pcs_configuartion ();
	}
        XSM_DEBUG (DBG_L8, "InitRegisterCb -- put value");
	Put_Value();

        XSM_DEBUG (DBG_L8, "InitRegisterCb -- connect...");
        
	g_object_set_data( G_OBJECT (entry), "Gtk_EntryControl", this);
	g_signal_connect (G_OBJECT (entry), "activate",
                          G_CALLBACK (&Gtk_EntryControl::update_callback),
                          (gpointer) NULL);
	g_signal_connect (G_OBJECT (entry), "focus_out_event",
                          G_CALLBACK (&Gtk_EntryControl::update_callback),
                          (gpointer) NULL);

        XSM_DEBUG (DBG_L8, "InitRegisterCb -- AdjSetup?");
	if(fabs (AdjStep) > 1e-22 && get_count () <= 1){ // only master
                XSM_DEBUG (DBG_L8, "InitRegisterCb -- hookup config menuitem");
		g_signal_connect (G_OBJECT (entry), "populate_popup",
                                  G_CALLBACK (&ec_pcs_populate_popup),
                                  (gpointer) this);
                
		adj = gtk_adjustment_new( Get_dValue (), vMin, vMax, step, page, 0);
		g_signal_connect (G_OBJECT (adj), "value_changed",
                                  G_CALLBACK (Gtk_EntryControl::adjustment_callback), this);

		if (GTK_IS_SPIN_BUTTON (entry)){
			g_signal_connect (G_OBJECT (entry), "output",
                                          G_CALLBACK (&ec_gtk_spin_button_sci_output),
                                          (gpointer) NULL);
			g_signal_connect (G_OBJECT (entry), "input",
                                          G_CALLBACK (&ec_gtk_spin_button_sci_input),
                                          (gpointer) NULL);
                        gtk_spin_button_configure (GTK_SPIN_BUTTON (entry), 
                                                   GTK_ADJUSTMENT (adj), progressive, 0);	
		}
	}
        XSM_DEBUG (DBG_L8, "InitRegisterCb -- done.");
}


GSList *Gtk_EntryControl::AddEntry2RemoteList(const gchar *RefName, GSList *remotelist){
	gchar *help;
	if(refname) 
		g_free (refname);
	refname = g_strdup(RefName);
	
	gtk_widget_set_tooltip_text (entry, RefName);

        if (pc_count == 0){
                if (generate_pcs_gschema)
                        write_pcs_gschema ();
                else
                        get_init_value_from_settings ();
        }

	return g_slist_prepend(remotelist, this);
}
