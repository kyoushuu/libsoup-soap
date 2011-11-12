/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * LibSoup-SOAP - SOAP Support for LibSoup
 * Copyright (C) 2011  Arnel A. Borja <kyoushuu@yahoo.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _SOUP_SOAP_PARAM_H_
#define _SOUP_SOAP_PARAM_H_

#include <glib-object.h>

G_BEGIN_DECLS

#define SOUP_SOAP_TYPE_PARAM             (soup_soap_param_get_type ())
#define SOUP_SOAP_PARAM(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), SOUP_SOAP_TYPE_PARAM, SoupSoapParam))
#define SOUP_SOAP_PARAM_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), SOUP_SOAP_TYPE_PARAM, SoupSoapParamClass))
#define SOUP_SOAP_IS_PARAM(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SOUP_SOAP_TYPE_PARAM))
#define SOUP_SOAP_IS_PARAM_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), SOUP_SOAP_TYPE_PARAM))
#define SOUP_SOAP_PARAM_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), SOUP_SOAP_TYPE_PARAM, SoupSoapParamClass))

typedef struct _SoupSoapParamPrivate SoupSoapParamPrivate;
typedef struct _SoupSoapParamClass SoupSoapParamClass;
typedef struct _SoupSoapParam SoupSoapParam;

struct _SoupSoapParamClass
{
	GInitiallyUnownedClass parent_class;
};

struct _SoupSoapParam
{
	GInitiallyUnowned parent_instance;

	SoupSoapParamPrivate *priv;
};

GType soup_soap_param_get_type (void) G_GNUC_CONST;
SoupSoapParam *soup_soap_param_new (const gchar *name);
const gchar *soup_soap_param_get_name (SoupSoapParam *param);
void soup_soap_param_set_name (SoupSoapParam *param, const gchar *name);
const gchar *soup_soap_param_get_value (SoupSoapParam *param);
void soup_soap_param_set_value (SoupSoapParam *param, const gchar *value);
gchar *soup_soap_param_get_string (SoupSoapParam *param, GError **error);
void soup_soap_param_set_string (SoupSoapParam *param, const gchar *string);
gboolean soup_soap_param_get_boolean (SoupSoapParam *param, GError **error);
void soup_soap_param_set_boolean (SoupSoapParam *param, gboolean value);
gint soup_soap_param_get_integer (SoupSoapParam *param, GError **error);
void soup_soap_param_set_integer (SoupSoapParam *param, gint value);
gdouble soup_soap_param_get_double (SoupSoapParam *param, GError **error);
void soup_soap_param_set_double (SoupSoapParam *param, gdouble value);
guchar *soup_soap_param_get_base64_binary (SoupSoapParam *param, gsize *value_len, GError **error);
void soup_soap_param_set_base64_binary (SoupSoapParam *param, const guchar *value, gsize value_len);
gchar *soup_soap_param_get_base64_string (SoupSoapParam *param, GError **error);
void soup_soap_param_set_base64_string (SoupSoapParam *param, const gchar *value);

typedef enum
{
	SOUP_SOAP_PARAM_ERROR_UNKNOWN_ENCODING,
	SOUP_SOAP_PARAM_ERROR_INVALID_VALUE
} SoupSoapParamError;

#define SOUP_SOAP_PARAM_ERROR soup_soap_param_error_quark()

GQuark soup_soap_param_error_quark (void);

G_END_DECLS

#endif /* _SOUP_SOAP_PARAM_H_ */
