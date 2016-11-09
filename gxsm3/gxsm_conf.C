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

#if 0

/*
all xrm.Get...
but

 XsmRescourceManager xrmsp("App_View_NCraw"); 
 app_view.C:        show_side_pane = xrmsp.GetBool ("ShowInfoSidePane", TRUE); // TRUE GTK3QQQ

 XsmRescourceManager xrm_osd("App_View_OSD");
 app_view.C:		xrm_osd.Get (flag, &osd_item_enable[i], "0");


app_channelselector.C:	XsmRescourceManager xrm("Channel_Selector","Actual");		
app_channelselector.C:	XsmRescourceManager xrm("Channel_Selector",buff);
app_profile.C:	XsmRescourceManager xrm(profile_res_id);
app_view.C:	XsmRescourceManager xrm("App_View_NCraw");
app_view.C:	XsmRescourceManager xrmsp("App_View_NCraw");
app_view.C:	XsmRescourceManager xrm("App_View");

app_view.C:	XsmRescourceManager xrm("App_View_OSD");

gapp_service.C:	XsmRescourceManager xrm("FilePathSelectionHistory", historyid_gconf_path);
gapp_service.C:	XsmRescourceManager xrm ("Windows", ResName);

gxsm_menucb.C:	XsmRescourceManager xrm("FilingPathMemory");
surface.C:	XsmRescourceManager xrm("FilingPathMemory");

gxsm_resoucetable.C:	XsmRescourceManager xrm("HardwareInterfaces");
gxsm_resoucetable.C:	XsmRescourceManager xrm ("GUI-global", "System");

pcs.C:	XsmRescourceManager xrm("Adjustments", 
xsm.C:	XsmRescourceManager xrm("HardwareInterfaces");
xsm.C:	XsmRescourceManager xrm("Values", SetName);

*/

#include <iostream>
#include <fstream>

#include <string>
#include <config.h>
#include <stdlib.h>

#include "gxsm_conf.h"
#include "xsmdebug.h"


/* restore all GXSM buildin gconf defaults */
extern int force_gconf_defaults;


void write_log (const gchar *key, const gchar *type, const gchar *name, const gchar *defaultv, gint i){
        std::ofstream f;

        f.open ("gxsm3_conf_list", std::ios::out);

        f << key << ", "
          << type << ", "
          << name << ", "
          << defaultv << ", "
          << i
          << std::endl;

        f.close ();
}


/* Generalize Rescoure Management 
 * Prefix allows to save different Value Sets by name
 */
XsmRescourceManager::XsmRescourceManager(const gchar *prefix, const gchar *group){ 
        Xgsettings = NULL;
	if (prefix){
		Prefix = g_strconcat (GXSM_RES_BASE_PATH_DOT".", prefix, NULL);
	        for (gchar *p=Prefix; *p; ++p) if(*p == '/') *p = '.';
	} else
		Prefix = g_strdup (GXSM_RES_BASE_PATH_DOT".global");
	if (group){
		Group = g_strdup (group);
	        for (gchar *p=Group; *p; ++p) if(*p == '/') *p = '.';
	}else
		Group = g_strdup ("default");

        // verify schema for presence and auto manage if necessary
        GSettingsSchemaSource *system_gs_schema_source = g_settings_schema_source_get_default ();
        if (!system_gs_schema_source ){
                g_print ("XsmRescourceManager: no gschema -- g_settings_schema_source_get_default () retured NULL for >%s< group[%s]\n", Prefix, group);
                return;
        }

        gs_schema_source = g_settings_schema_source_lookup (system_gs_schema_source, Prefix, FALSE);
        if (!gs_schema_source){
                g_print ("XsmRescourceManager: no gschema -- g_settings_schema_source_lookup () retured NULL for >%s< group[%s]\n", Prefix, group);
                return;
        }

	Xgsettings = g_settings_new (Prefix);
}

XsmRescourceManager::~XsmRescourceManager(){ 
        if (Xgsettings)
                g_object_unref (Xgsettings);
	g_free(Prefix);
	g_free(Group);
}

void XsmRescourceManager::SetGroup(const gchar *group){
	g_free(Group);
	if (group){
		Group = g_strdup (group);
	        for (gchar *p=Group; *p; ++p) if(*p == '/') *p = '.';
	}else
		Group = g_strdup ("default");
}

gchar* XsmRescourceManager::KeyTranslate(const gchar *name, gint i){
	gchar *name_key = g_strdelimit (g_strdup (name), ". ", '-'); 
	gchar *index = i>=0 ? g_strdup_printf ("%03d",i) : NULL;
	gchar *tmp = g_strconcat (Group, "-", name_key, index, NULL);
	g_free (name_key);
	if (index) 
		g_free (index);
	return tmp;
}

void XsmRescourceManager::PutBool(const gchar *name, gboolean value, gint i){
	gchar *key = KeyTranslate (name, i);
	XSM_DEBUG (DBG_L2, "XsmRM::put-bool: " << key << " = " << value);

        if (!gs_schema_source){
                g_print ("XsmRes.put: g_settings_schema_source invalid/missing. key: type BOOLEAN key=%s : %s\n", key, value?"TRUE":"FALSE");
        } else if (!g_settings_schema_has_key (gs_schema_source, key)){
                g_print ("XsmRes.put: g_settings_schema_is missing_key: type BOOLEAN key=%s : %s\n", key, value?"TRUE":"FALSE");
        } else {
                GVariant *gvar = g_variant_new_boolean (value);
                g_settings_set_value (Xgsettings,
                                      key,
                                      gvar);
                g_variant_unref (gvar);
        }
	g_free(key);
}

void XsmRescourceManager::Put(const gchar *name, double value, gint i){
	gchar *key = KeyTranslate (name, i);
	XSM_DEBUG (DBG_L2, "XsmRM::put-double: " << key << " = " << value);

        if (!gs_schema_source){
                g_print ("XsmRes.put: g_settings_schema_source invalid/missing. key: type DOUBLE key=%s : %f\n", key, value);
        } else if (!g_settings_schema_has_key (gs_schema_source, key)){
                g_print ("XsmRes.put: g_settings_schema_is missing_key: type DOUBLE key=%s : %f\n", key, value);
        } else {
                GVariant *gvar = g_variant_new_double (value);
                g_settings_set_value (Xgsettings,
                                      key,
                                      gvar);
                g_variant_unref (gvar);
        }
	g_free(key);
}
 
void XsmRescourceManager::Put(const gchar *name, int value, gint i){
	gchar *key = KeyTranslate (name, i);

        if (!gs_schema_source){
                g_print ("XsmRes.put: g_settings_schema_source invalid/missing. key: type INT key=%s : %d\n", key, value);
        } else if (!g_settings_schema_has_key (gs_schema_source, key)){
                g_print ("XsmRes.put: g_settings_schema_is missing_key: type INT key=%s : %d\n", key, value);
        } else {
                XSM_DEBUG (DBG_L2, "XsmRM::put-int: " << key << " = " << value);
                GVariant *gvar = g_variant_new_int64 (value);
                g_settings_set_value (Xgsettings,
                                      key,
                                      gvar);
                g_variant_unref (gvar);
        }
	g_free(key);
}

void XsmRescourceManager::Put(const gchar *name, const gchar *value, gint i){
	gchar *key = KeyTranslate (name, i);

        if (!gs_schema_source){
                g_print ("XsmRes.put: g_settings_schema_source invalid/missing. key: type STRING key=%s : %s\n", key, value);
        } else if (!g_settings_schema_has_key (gs_schema_source, key)){
                g_print ("XsmRes.put: g_settings_schema_is missing_key: type STRING key=%s : %s\n", key, value);
        } else {
                XSM_DEBUG (DBG_L2, "XsmRM::put-string: " << key << " = " << value);
                GVariant *gvar = g_variant_new_string (value);
                g_settings_set_value (Xgsettings,
                                      key,
                                      gvar);
                g_variant_unref (gvar);
        }
	g_free(key);
}

#define GXSM_DEFAULT_LOCALE NULL

int XsmRescourceManager::GetBool(const gchar *name, gboolean defaultv, gint i){
	gchar *key = KeyTranslate (name, i);
	gboolean value = FALSE;

        write_log (key, "gboolean", name, defaultv ? "true":"false", i);

        if (force_gconf_defaults || !gs_schema_source){
                g_print ("default forced/bad gss. type BOOLEAN key=%s : %s\n", key, defaultv?"TRUE":"FALSE");
                value = defaultv;
        } else if (!g_settings_schema_has_key (gs_schema_source, key)){
                g_print ("XsmRes.get: g_settings_schema_is missing_key: type BOOLEAN key=%s : %s\n", key, defaultv?"TRUE":"FALSE");
                value = defaultv;
        } else {
                GVariant *gvar = g_settings_get_value (Xgsettings,
                                                       key);
                value = g_variant_get_boolean (gvar);
                g_variant_unref (gvar);
        }
#if 0
	if (force_gconf_defaults || !gce->value){ // force default  or  no default in schema and no value set?
		if (defaultv){
			value = defaultv;
			gconf_client_set_bool (client, key, value, NULL);
		}
	} else {
		value = gconf_client_get_bool (client, key, NULL);
		XSM_DEBUG (DBG_L2, "XsmRM::get-bool: " << key << " = " << value << "  default: " << defaultv);
	}
#endif
	g_free(key);
	return value;
}

int XsmRescourceManager::Get(const gchar *name, double *value, const gchar *defaultv, gint i){
	gchar *key = KeyTranslate (name, i);

        write_log (key, "double", name, defaultv, i);

        if (force_gconf_defaults || !gs_schema_source){
                g_print ("default forced/bad gss. type DOUBLE key=%s : %s\n", key, defaultv);
                *value = atof (defaultv);
        } else if (!g_settings_schema_has_key (gs_schema_source, key)){
                g_print ("XsmRes.get: g_settings_schema_is missing_key: type DOUBLE key=%s : %s\n", key, defaultv);
                *value = atof (defaultv);
        } else {
                GVariant *gvar = g_settings_get_value (Xgsettings,
                                                       key);
                *value = g_variant_get_double (gvar);
        
                g_variant_unref (gvar);
        }
#if 0
	GConfEntry* gce;
	gce = gconf_client_get_entry (client, key, GXSM_DEFAULT_LOCALE, TRUE, NULL);
	if (force_gconf_defaults || !gce->value){ // force default  or  no default in schema and no value set?
		if (defaultv){
			*value = atof (defaultv);
			gconf_client_set_float (client, key, (gfloat) (*value), NULL);
		}
	} else {
		*value = gconf_client_get_float (client, key, NULL);
		XSM_DEBUG (DBG_L2, "XsmRM::get-float: " << key << " = " << *value << "  default: " << defaultv);
	}
#endif
	g_free(key);
	return 0;
}

int XsmRescourceManager::Get(const gchar *name, int *value, const gchar *defaultv, gint i){
	gchar *key = KeyTranslate (name, i);

        write_log (key, "int", name, defaultv, i);

        if (force_gconf_defaults || !gs_schema_source){
                if (defaultv){
                        g_print ("default forced/bad gss. type INT key=%s : %s\n", key, defaultv);
                        *value = atoi (defaultv);
                } else { 
                        g_print ("NO DEFAULT GIVEN. No change to var!! default forced/bad gss. type INT key=%s\n", key);
                }
        } else if (!g_settings_schema_has_key (gs_schema_source, key)){
                g_print ("XsmRes.get: g_settings_schema_is missing_key: type INT key=%s : %s\n", key, defaultv);
                *value = atoi (defaultv);
        } else {
                GVariant *gvar = g_settings_get_value (Xgsettings,
					       key);
                *value = g_variant_get_int64 (gvar);

                g_variant_unref (gvar);
        }
#if 0
	GConfEntry* gce = gconf_client_get_entry (client, key, GXSM_DEFAULT_LOCALE, TRUE, NULL);
	if (force_gconf_defaults || !gce->value){ // force default  or  no default in schema and no value set?
		if (defaultv){
			*value = atoi (defaultv);
			gconf_client_set_int (client, key, *value, NULL);
		}
	} else {
		*value = gconf_client_get_int (client, key, NULL);
		XSM_DEBUG(DBG_L2, "XsmRM::get-int: " << key << " = " << *value << "  default: " << defaultv);
	}
#endif
	g_free(key);
	return 0;
}

int XsmRescourceManager::Get(const gchar *name, gchar **value, const gchar *defaultv, gint i){
	gchar *key = KeyTranslate (name, i);

        write_log (key, "string", name, defaultv, i);

        if (force_gconf_defaults || !gs_schema_source){
                g_print ("default forced/bad gss. type STRING key=%s : %s\n", key, defaultv);
                *value = g_strdup (defaultv);
        } else if (!g_settings_schema_has_key (gs_schema_source, key)){
                g_print ("XsmRes.get: g_settings_schema_is missing_key: type STRING key=%s : %s\n", key, defaultv);
                *value = g_strdup (defaultv);
        } else {
                GVariant *gvar = g_settings_get_value (Xgsettings,
					       key);
                *value = g_strdup (g_variant_get_string (gvar, NULL));

                g_variant_unref (gvar);
        }
#if 0
	GConfEntry* gce = gconf_client_get_entry (client, key, GXSM_DEFAULT_LOCALE, TRUE, NULL);
	if (force_gconf_defaults || !gce->value){ // force default  or  no default in schema and no value set?
		if (defaultv){
		        *value = g_strdup (defaultv);
			gconf_client_set_string (client, key, defaultv, NULL);
		} else {
		        *value = g_strdup ("?!-ERROR-!?");
		}
	} else {
	        *value = g_strdup (gconf_client_get_string (client, key, NULL));
		XSM_DEBUG(DBG_L2, "XsmRM::get-string: " << key << " = " << (*value?*value:"-UnSet-") << "  default: " << defaultv);
	}
#endif
	g_free(key);
	return 0;
}

gchar *XsmRescourceManager::GetStr(const gchar *name, const gchar *defaultv, gint i){
	gchar *value;
	gchar *key = KeyTranslate (name, i);

        write_log (key, "string", name, defaultv, i);

        if (force_gconf_defaults || !gs_schema_source){
                g_print ("default forced/bad gss. type STRING key=%s : %s\n", key, defaultv);
                value = g_strdup (defaultv);
        } else if (!g_settings_schema_has_key (gs_schema_source, key)){
                g_print ("XsmRes.get: g_settings_schema_is missing_key: type STRING key=%s : %s\n", key, defaultv);
                value = g_strdup (defaultv);
        } else {
                GVariant *gvar = g_settings_get_value (Xgsettings,
                                                       key);
                value = g_strdup (g_variant_get_string (gvar, NULL));
                
                g_variant_unref (gvar);
        }
#if 0
	GConfEntry* gce = gconf_client_get_entry (client, key, GXSM_DEFAULT_LOCALE, TRUE, NULL);
	if (force_gconf_defaults || !gce->value){ // force default  or  no default in schema and no value set?
		if (defaultv){
		        value = g_strdup (defaultv);
			gconf_client_set_string (client, key, defaultv, NULL);
		} else {
			g_free(key);
			return NULL;
		}
	} else {
	        value = g_strdup (gconf_client_get_string (client, key, NULL));
		XSM_DEBUG(DBG_L2, "XsmRM::get-string*: " << key << " = " << (value?value:"-UnSet-") << "  default: " << defaultv);
	}
#endif
	g_free(key);
	return value;
}

#endif
