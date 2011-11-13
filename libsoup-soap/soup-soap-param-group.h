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

#ifndef _SOUP_SOAP_PARAM_GROUP_H_
#define _SOUP_SOAP_PARAM_GROUP_H_

#include <glib-object.h>

G_BEGIN_DECLS

#define SOUP_SOAP_TYPE_PARAM_GROUP             (soup_soap_param_group_get_type ())
#define SOUP_SOAP_PARAM_GROUP(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), SOUP_SOAP_TYPE_PARAM_GROUP, SoupSoapParamGroup))
#define SOUP_SOAP_PARAM_GROUP_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), SOUP_SOAP_TYPE_PARAM_GROUP, SoupSoapParamGroupClass))
#define SOUP_SOAP_IS_PARAM_GROUP(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SOUP_SOAP_TYPE_PARAM_GROUP))
#define SOUP_SOAP_IS_PARAM_GROUP_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), SOUP_SOAP_TYPE_PARAM_GROUP))
#define SOUP_SOAP_PARAM_GROUP_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), SOUP_SOAP_TYPE_PARAM_GROUP, SoupSoapParamGroupClass))

typedef struct _SoupSoapParamGroupPrivate SoupSoapParamGroupPrivate;
typedef struct _SoupSoapParamGroupClass SoupSoapParamGroupClass;
typedef struct _SoupSoapParamGroup SoupSoapParamGroup;

struct _SoupSoapParamGroupClass
{
	SoupSoapParamClass parent_class;
};

struct _SoupSoapParamGroup
{
	SoupSoapParam parent_instance;

	SoupSoapParamGroupPrivate *priv;
};

GType soup_soap_param_group_get_type (void) G_GNUC_CONST;
SoupSoapParamGroup *soup_soap_param_group_new (const gchar *name);
GList *soup_soap_param_group_get_elements (SoupSoapParamGroup *group);
guint soup_soap_param_group_get_elements_length (SoupSoapParamGroup *group);
void soup_soap_param_group_add (SoupSoapParamGroup *group, SoupSoapParam *param);
void soup_soap_param_group_add_multiple (SoupSoapParamGroup *group, ...);
void soup_soap_param_group_add_multiple_valist (SoupSoapParamGroup *group, va_list var_args);
SoupSoapParam *soup_soap_param_group_get (SoupSoapParamGroup *group, const gchar *name);
void soup_soap_param_group_get_multiple (SoupSoapParamGroup *group, ...);
void soup_soap_param_group_get_multiple_valist (SoupSoapParamGroup *group, va_list var_args);

G_END_DECLS

#endif /* _SOUP_SOAP_PARAM_GROUP_H_ */
