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

#include <config.h>
#include <glib/gi18n.h>

#include <libsoup/soup.h>
#include <libsoup-soap/soup-soap.h>

struct _SoupSoapParamGroupPrivate
{
	GList *elements;
};

#define SOUP_SOAP_PARAM_GROUP_GET_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), SOUP_SOAP_TYPE_PARAM_GROUP, SoupSoapParamGroupPrivate))



G_DEFINE_TYPE (SoupSoapParamGroup, soup_soap_param_group, SOUP_SOAP_TYPE_PARAM);

static void
soup_soap_param_group_init (SoupSoapParamGroup *object)
{
	object->priv = SOUP_SOAP_PARAM_GROUP_GET_PRIVATE (object);
	SoupSoapParamGroupPrivate *priv = object->priv;

	priv->elements = NULL;
}

static void
soup_soap_param_group_finalize (GObject *object)
{
	SoupSoapParamGroup *group = SOUP_SOAP_PARAM_GROUP (object);
	SoupSoapParamGroupPrivate *priv = group->priv;

	if (priv->elements)
		g_list_free_full (priv->elements, g_object_unref);

	G_OBJECT_CLASS (soup_soap_param_group_parent_class)->finalize (object);
}

static void
soup_soap_param_group_class_init (SoupSoapParamGroupClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	/*SoupSoapParamClass *parent_class = SOUP_SOAP_PARAM_CLASS (klass);*/

	g_type_class_add_private (klass, sizeof (SoupSoapParamGroupPrivate));

	object_class->finalize = soup_soap_param_group_finalize;
}


SoupSoapParamGroup *
soup_soap_param_group_new (const gchar *name)
{
	g_return_val_if_fail (name != NULL && *name != '\0', NULL);

	return g_object_new (SOUP_SOAP_TYPE_PARAM_GROUP,
	                     "name", name,
	                     NULL);
}

GList *
soup_soap_param_group_get_elements (SoupSoapParamGroup *group)
{
	g_return_val_if_fail (SOUP_SOAP_IS_PARAM_GROUP (group), NULL);

	SoupSoapParamGroupPrivate *priv = group->priv;

	return g_list_copy (priv->elements);
}

guint
soup_soap_param_group_get_elements_length (SoupSoapParamGroup *group)
{
	g_return_val_if_fail (SOUP_SOAP_IS_PARAM_GROUP (group), 0);

	SoupSoapParamGroupPrivate *priv = group->priv;

	return g_list_length (priv->elements);
}

void
soup_soap_param_group_add (SoupSoapParamGroup *group,
                           SoupSoapParam *param)
{
	g_return_if_fail (SOUP_SOAP_IS_PARAM_GROUP (group));
	g_return_if_fail (SOUP_SOAP_IS_PARAM (param));

	SoupSoapParamGroupPrivate *priv = group->priv;

	priv->elements = g_list_append (priv->elements, g_object_ref_sink (param));
}

void
soup_soap_param_group_add_multiple (SoupSoapParamGroup *group,
                                    ...)
{
	g_return_if_fail (SOUP_SOAP_IS_PARAM_GROUP (group));

	va_list var_args;

	va_start (var_args, group);
	soup_soap_param_group_add_multiple_valist (group, var_args);
	va_end (var_args);
}

void
soup_soap_param_group_add_multiple_valist (SoupSoapParamGroup *group,
                                           va_list var_args)
{
	g_return_if_fail (SOUP_SOAP_IS_PARAM_GROUP (group));

	SoupSoapParamGroupPrivate *priv = group->priv;

	SoupSoapParam *param;
	GList *new_elements = NULL;

	while ((param = va_arg (var_args, SoupSoapParam *)))
	{
		new_elements = g_list_prepend (new_elements, g_object_ref_sink (param));
	}

	new_elements = g_list_reverse (new_elements);
	priv->elements = g_list_concat (priv->elements, new_elements);
}

SoupSoapParam *
soup_soap_param_group_get (SoupSoapParamGroup *group,
                           const gchar *name)
{
	g_return_val_if_fail (SOUP_SOAP_IS_PARAM_GROUP (group), NULL);
	g_return_val_if_fail (name != NULL && *name != '\0', NULL);

	SoupSoapParamGroupPrivate *priv = group->priv;

	SoupSoapParam *param;
	GList *elements = priv->elements;

	while (elements)
	{
		param = elements->data;

		if (g_strcmp0 (name, soup_soap_param_get_name (param)) == 0)
			return param;

		elements = g_list_next (elements);
	}

	return NULL;
}

void
soup_soap_param_group_get_multiple (SoupSoapParamGroup *group,
                                    ...)
{
	g_return_if_fail (SOUP_SOAP_IS_PARAM_GROUP (group));

	va_list var_args;

	va_start (var_args, group);
	soup_soap_param_group_get_multiple_valist (group, var_args);
	va_end (var_args);
}

void
soup_soap_param_group_get_multiple_valist (SoupSoapParamGroup *group,
                                           va_list var_args)
{
	g_return_if_fail (SOUP_SOAP_IS_PARAM_GROUP (group));

	const gchar *name;
	SoupSoapParam **param;

	while ((name = va_arg (var_args, const gchar *)))
	{
		param = va_arg (var_args, SoupSoapParam **);
		if (param)
			*param = soup_soap_param_group_get (group, name);
	}
}
