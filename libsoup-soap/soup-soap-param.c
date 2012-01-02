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

#include <errno.h>
#include <stdlib.h>

struct _SoupSoapParamPrivate
{
	gchar *name;
	gchar *value;
};

#define SOUP_SOAP_PARAM_GET_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), SOUP_SOAP_TYPE_PARAM, SoupSoapParamPrivate))

enum
{
	PROP_0,

	PROP_NAME,
	PROP_VALUE,
};


/* Parts taken from GLib
 *
 *
 * gutf8.c - Operations on UTF-8 strings.
 *
 * Copyright (C) 1999 Tom Tromey
	 * Copyright (C) 2000 Red Hat, Inc.
		 *
 *
 * gkeyfile.c - key file parser
 *
 *  Copyright 2004  Red Hat, Inc.
 *  Copyright 2009-2010  Collabora Ltd.
 *  Copyright 2009  Nokia Corporation
 *
 * Written by Ray Strode <rstrode@redhat.com>
 *            Matthias Clasen <mclasen@redhat.com>
 *
 *
 * GLib is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
	 *
 * GLib is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with GLib; see the file COPYING.LIB.  If not,
 * write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *   Boston, MA 02111-1307, USA.
 */

gchar *
_g_utf8_make_valid (const gchar *name)
{
	GString *string;
	const gchar *remainder, *invalid;
	gint remaining_bytes, valid_bytes;

	g_return_val_if_fail (name != NULL, NULL);

	string = NULL;
	remainder = name;
	remaining_bytes = strlen (name);

	while (remaining_bytes != 0)
	{
		if (g_utf8_validate (remainder, remaining_bytes, &invalid))
			break;
		valid_bytes = invalid - remainder;

		if (string == NULL)
			string = g_string_sized_new (remaining_bytes);

		g_string_append_len (string, remainder, valid_bytes);
		/* append U+FFFD REPLACEMENT CHARACTER */
		g_string_append (string, "\357\277\275");

		remaining_bytes -= valid_bytes + 1;
		remainder = invalid + 1;
	}

	if (string == NULL)
		return g_strdup (name);

	g_string_append (string, remainder);

	g_assert (g_utf8_validate (string->str, -1, NULL));

	return g_string_free (string, FALSE);
}

static gchar *
parse_value_as_string (const gchar *value,
                       GError **error)
{
	gchar *string_value, *p, *q;

	string_value = g_new (gchar, strlen (value) + 1);

	p = (gchar *) value;
	q = string_value;
	while (*p)
	{
		if (*p == '\\')
		{
			p++;

			switch (*p)
			{
				case 's':
					*q = ' ';
					break;

				case 'n':
					*q = '\n';
					break;

				case 't':
					*q = '\t';
					break;

				case 'r':
					*q = '\r';
					break;

				case '\\':
					*q = '\\';
					break;

				case '\0':
					g_set_error_literal (error, SOUP_SOAP_PARAM_ERROR,
					                     SOUP_SOAP_PARAM_ERROR_INVALID_VALUE,
					                     _("Key file contains escape character "
					                       "at end of line"));
					break;

				default:
					*q++ = '\\';
					*q = *p;

					if (*error == NULL)
					{
						gchar sequence[3];

						sequence[0] = '\\';
						sequence[1] = *p;
						sequence[2] = '\0';

						g_set_error (error, SOUP_SOAP_PARAM_ERROR,
							         SOUP_SOAP_PARAM_ERROR_INVALID_VALUE,
							         _("Key file contains invalid escape "
							           "sequence '%s'"), sequence);
					}
					break;
			}
		}
		else
			*q = *p;

		if (*p == '\0')
			break;

		q++;
		p++;
	}

	*q = '\0';

	return string_value;
}

static gchar *
parse_string_as_value (const gchar *string,
                       gboolean escape_separator)
{
	gchar *value, *p, *q;
	gsize length;
	gboolean parsing_leading_space;

	length = strlen (string) + 1;

	/* Worst case would be that every character needs to be escaped.
	 * In other words every character turns to two characters
	 */
	value = g_new (gchar, 2 * length);

	p = (gchar *) string;
	q = value;
	parsing_leading_space = TRUE;
	while (p < (string + length - 1))
	{
		gchar escaped_character[3] = { '\\', 0, 0 };

		switch (*p)
		{
			case ' ':
				if (parsing_leading_space)
				{
					escaped_character[1] = 's';
					strcpy (q, escaped_character);
					q += 2;
				}
				else
				{
					*q = *p;
					q++;
				}
				break;
			case '\t':
				if (parsing_leading_space)
				{
					escaped_character[1] = 't';
					strcpy (q, escaped_character);
					q += 2;
				}
				else
				{
					*q = *p;
					q++;
				}
				break;
			case '\n':
				escaped_character[1] = 'n';
				strcpy (q, escaped_character);
				q += 2;
				break;
			case '\r':
				escaped_character[1] = 'r';
				strcpy (q, escaped_character);
				q += 2;
				break;
			case '\\':
				escaped_character[1] = '\\';
				strcpy (q, escaped_character);
				q += 2;
				parsing_leading_space = FALSE;
				break;
			default:
				*q = *p;
				q++;
				parsing_leading_space = FALSE;
				break;
		}
		p++;
	}
	*q = '\0';

	return value;
}

static gint
parse_value_as_integer (const gchar *value,
                        GError **error)
{
	gchar *eof_int;
	glong long_value;
	gint int_value;

	errno = 0;
	long_value = strtol (value, &eof_int, 10);

	if (*value == '\0' || (*eof_int != '\0' && !g_ascii_isspace(*eof_int)))
	{
		gchar *value_utf8 = _g_utf8_make_valid (value);
		g_set_error (error, SOUP_SOAP_PARAM_ERROR,
		             SOUP_SOAP_PARAM_ERROR_INVALID_VALUE,
		             _("Value '%s' cannot be interpreted "
		               "as a number."), value_utf8);
		g_free (value_utf8);

		return 0;
	}

	int_value = long_value;
	if (int_value != long_value || errno == ERANGE)
	{
		gchar *value_utf8 = _g_utf8_make_valid (value);
		g_set_error (error,
		             SOUP_SOAP_PARAM_ERROR,
		             SOUP_SOAP_PARAM_ERROR_INVALID_VALUE,
		             _("Integer value '%s' out of range"),
		             value_utf8);
		g_free (value_utf8);

		return 0;
	}

	return int_value;
}

static gdouble
parse_value_as_double  (const gchar *value,
                        GError **error)
{
	gchar *end_of_valid_d;
	gdouble double_value = 0;

	double_value = g_ascii_strtod (value, &end_of_valid_d);

	if (*end_of_valid_d != '\0' || end_of_valid_d == value)
	{
		gchar *value_utf8 = _g_utf8_make_valid (value);
		g_set_error (error, SOUP_SOAP_PARAM_ERROR,
		             SOUP_SOAP_PARAM_ERROR_INVALID_VALUE,
		             _("Value '%s' cannot be interpreted "
		               "as a float number."),
		             value_utf8);
		g_free (value_utf8);
	}

	return double_value;
}

static gboolean
parse_value_as_boolean (const gchar *value,
                        GError **error)
{
	gchar *value_utf8;

	if (strcmp (value, "true") == 0 || strcmp (value, "1") == 0)
		return TRUE;
	else if (strcmp (value, "false") == 0 || strcmp (value, "0") == 0)
		return FALSE;

	value_utf8 = _g_utf8_make_valid (value);
	g_set_error (error, SOUP_SOAP_PARAM_ERROR,
	             SOUP_SOAP_PARAM_ERROR_INVALID_VALUE,
	             _("Value '%s' cannot be interpreted "
	               "as a boolean."), value_utf8);
	g_free (value_utf8);

	return FALSE;
}

static gchar *
parse_boolean_as_value (gboolean value)
{
	if (value)
		return g_strdup ("true");
	else
		return g_strdup ("false");
}

static guchar *
parse_value_as_base64_binary (const gchar *value,
                              gsize *out_len,
                              GError **error)
{
	gchar *value_utf8;
	gsize result_len = 0;
	guchar *result;

	result = g_base64_decode (value, &result_len);

	if (result_len == 0)
	{
		value_utf8 = _g_utf8_make_valid (value);
		g_set_error (error, SOUP_SOAP_PARAM_ERROR,
		             SOUP_SOAP_PARAM_ERROR_INVALID_VALUE,
		             _("Value '%s' cannot be interpreted "
		               "as base64 encoded."), value_utf8);
		g_free (value_utf8);
		return NULL;
	}
	else
	{
		if (out_len) *out_len = result_len;
		return result;
	}
}


GQuark
soup_soap_param_error_quark (void)
{
	return g_quark_from_static_string ("soup-soap-param-error-quark");
}


G_DEFINE_TYPE (SoupSoapParam, soup_soap_param, G_TYPE_INITIALLY_UNOWNED);

static void
soup_soap_param_init (SoupSoapParam *object)
{
	object->priv = SOUP_SOAP_PARAM_GET_PRIVATE (object);
	SoupSoapParamPrivate *priv = object->priv;

	priv->name = NULL;
	priv->value = NULL;
}

static void
soup_soap_param_finalize (GObject *object)
{
	SoupSoapParam *param = SOUP_SOAP_PARAM (object);
	SoupSoapParamPrivate *priv = param->priv;

	g_free (priv->name);
	g_free (priv->value);

	G_OBJECT_CLASS (soup_soap_param_parent_class)->finalize (object);
}

static void
soup_soap_param_set_property (GObject *object,
                              guint prop_id,
                              const GValue *value,
                              GParamSpec *pspec)
{
	g_return_if_fail (SOUP_SOAP_IS_PARAM (object));

	SoupSoapParam *param = SOUP_SOAP_PARAM (object);

	switch (prop_id)
	{
		case PROP_NAME:
			soup_soap_param_set_name (param,
			                          g_value_get_string (value));
			break;
		case PROP_VALUE:
			soup_soap_param_set_value (param,
			                           g_value_get_string (value));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
soup_soap_param_get_property (GObject *object,
                              guint prop_id,
                              GValue *value,
                              GParamSpec *pspec)
{
	g_return_if_fail (SOUP_SOAP_IS_PARAM (object));

	SoupSoapParam *param = SOUP_SOAP_PARAM (object);

	switch (prop_id)
	{
		case PROP_NAME:
			g_value_set_string (value,
			                    soup_soap_param_get_name (param));
			break;
		case PROP_VALUE:
			g_value_set_string (value,
			                    soup_soap_param_get_value (param));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
soup_soap_param_class_init (SoupSoapParamClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	/*GInitiallyUnownedClass *parent_class = G_INITIALLY_UNOWNED_CLASS (klass);*/

	g_type_class_add_private (klass, sizeof (SoupSoapParamPrivate));

	object_class->finalize = soup_soap_param_finalize;
	object_class->set_property = soup_soap_param_set_property;
	object_class->get_property = soup_soap_param_get_property;

	g_object_class_install_property (object_class,
	                                 PROP_NAME,
	                                 g_param_spec_string ("name",
	                                                      "Param name",
	                                                      "The name of the param of a SOAP operation",
	                                                      "no-name-set",
	                                                      G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT));

	g_object_class_install_property (object_class,
	                                 PROP_VALUE,
	                                 g_param_spec_string ("value",
	                                                      "Param value",
	                                                      "The value of the param of a SOAP operation as string",
	                                                      "",
	                                                      G_PARAM_READABLE | G_PARAM_WRITABLE));

}


SoupSoapParam *
soup_soap_param_new (const gchar *name)
{
	g_return_val_if_fail (name != NULL && *name != '\0', NULL);

	return g_object_new (SOUP_SOAP_TYPE_PARAM,
	                     "name", name,
	                     NULL);
}

SoupSoapParam *
soup_soap_param_new_value (const gchar *name,
                           const gchar *value)
{
	g_return_val_if_fail (name != NULL && *name != '\0', NULL);

	SoupSoapParam *param = soup_soap_param_new (name);
	soup_soap_param_set_value (param, value);

	return param;
}

SoupSoapParam *
soup_soap_param_new_string (const gchar *name,
                            const gchar *value)
{
	g_return_val_if_fail (name != NULL && *name != '\0', NULL);

	SoupSoapParam *param = soup_soap_param_new (name);
	soup_soap_param_set_string (param, value);

	return param;
}

SoupSoapParam *
soup_soap_param_new_boolean (const gchar *name,
                             gboolean value)
{
	g_return_val_if_fail (name != NULL && *name != '\0', NULL);

	SoupSoapParam *param = soup_soap_param_new (name);
	soup_soap_param_set_boolean (param, value);

	return param;
}

SoupSoapParam *
soup_soap_param_new_integer (const gchar *name,
                             gint value)
{
	g_return_val_if_fail (name != NULL && *name != '\0', NULL);

	SoupSoapParam *param = soup_soap_param_new (name);
	soup_soap_param_set_integer (param, value);

	return param;
}

SoupSoapParam *
soup_soap_param_new_double (const gchar *name,
                            gdouble value)
{
	g_return_val_if_fail (name != NULL && *name != '\0', NULL);

	SoupSoapParam *param = soup_soap_param_new (name);
	soup_soap_param_set_double (param, value);

	return param;
}

SoupSoapParam *
soup_soap_param_new_base64_binary (const gchar *name,
                                   const guchar *value,
                                   gsize value_len)
{
	g_return_val_if_fail (name != NULL && *name != '\0', NULL);

	SoupSoapParam *param = soup_soap_param_new (name);
	soup_soap_param_set_base64_binary (param, value, value_len);

	return param;
}

SoupSoapParam *
soup_soap_param_new_base64_string (const gchar *name,
                                   const gchar *value)
{
	g_return_val_if_fail (name != NULL && *name != '\0', NULL);

	SoupSoapParam *param = soup_soap_param_new (name);
	soup_soap_param_set_base64_string (param, value);

	return param;
}

const gchar *
soup_soap_param_get_name (SoupSoapParam *param)
{
	g_return_val_if_fail (SOUP_SOAP_IS_PARAM (param), NULL);

	return param->priv->name;
}

void
soup_soap_param_set_name (SoupSoapParam *param,
                          const gchar *name)
{
	g_return_if_fail (SOUP_SOAP_IS_PARAM (param));

	SoupSoapParamPrivate *priv = param->priv;

	g_free (priv->name);
	priv->name = g_strdup (name);
}

const gchar *
soup_soap_param_get_value (SoupSoapParam *param)
{
	g_return_val_if_fail (SOUP_SOAP_IS_PARAM (param), NULL);

	return param->priv->value;
}

void
soup_soap_param_set_value (SoupSoapParam *param,
                           const gchar *value)
{
	g_return_if_fail (SOUP_SOAP_IS_PARAM (param));

	SoupSoapParamPrivate *priv = param->priv;

	g_free (priv->value);
	priv->value = g_strdup (value);
}

gchar *
soup_soap_param_get_string (SoupSoapParam *param,
                            GError **error)
{
	const gchar *value;
	gchar *string_value;
	GError *param_error;

	g_return_val_if_fail (SOUP_SOAP_IS_PARAM (param), NULL);

	param_error = NULL;

	value = soup_soap_param_get_value (param);

	if (!g_utf8_validate (value, -1, NULL))
	{
		gchar *value_utf8 = _g_utf8_make_valid (value);
		g_set_error (error, SOUP_SOAP_PARAM_ERROR,
		             SOUP_SOAP_PARAM_ERROR_UNKNOWN_ENCODING,
		             _("Value '%s' is not UTF-8"), value_utf8);
		g_free (value_utf8);

		return NULL;
	}

	string_value = parse_value_as_string (value, &param_error);

	if (param_error)
	{
		if (g_error_matches (param_error,
		                     SOUP_SOAP_PARAM_ERROR,
		                     SOUP_SOAP_PARAM_ERROR_INVALID_VALUE))
		{
			g_set_error (error, SOUP_SOAP_PARAM_ERROR,
			             SOUP_SOAP_PARAM_ERROR_INVALID_VALUE,
			             _("Value cannot be interpreted."));
			g_error_free (param_error);
		}
		else
			g_propagate_error (error, param_error);
	}

	return string_value;
}

void
soup_soap_param_set_string (SoupSoapParam *param,
                            const gchar *string)
{
	gchar *value;

	g_return_if_fail (SOUP_SOAP_IS_PARAM (param));
	g_return_if_fail (string != NULL);

	value = parse_string_as_value (string, FALSE);
	soup_soap_param_set_value (param, value);
	g_free (value);
}

gboolean
soup_soap_param_get_boolean (SoupSoapParam *param,
                             GError **error)
{
	GError *param_error = NULL;
	const gchar *value;
	gboolean bool_value;

	g_return_val_if_fail (SOUP_SOAP_IS_PARAM (param), FALSE);

	value = soup_soap_param_get_value (param);

	bool_value = parse_value_as_boolean (value, &param_error);

	if (param_error)
	{
		if (g_error_matches (param_error,
		                     SOUP_SOAP_PARAM_ERROR,
		                     SOUP_SOAP_PARAM_ERROR_INVALID_VALUE))
		{
			g_set_error (error, SOUP_SOAP_PARAM_ERROR,
			             SOUP_SOAP_PARAM_ERROR_INVALID_VALUE,
			             _("Value cannot be interpreted."));
			g_error_free (param_error);
		}
		else
			g_propagate_error (error, param_error);
	}

	return bool_value;
}

void
soup_soap_param_set_boolean (SoupSoapParam *param,
                             gboolean value)
{
	gchar *result;

	g_return_if_fail (SOUP_SOAP_IS_PARAM (param));

	result = parse_boolean_as_value (value);
	soup_soap_param_set_value (param, result);
	g_free (result);
}

gint
soup_soap_param_get_integer (SoupSoapParam *param,
                             GError **error)
{
	GError *param_error;
	const gchar *value;
	gint int_value;

	g_return_val_if_fail (SOUP_SOAP_IS_PARAM (param), -1);

	param_error = NULL;

	value = soup_soap_param_get_value (param);

	int_value = parse_value_as_integer (value, &param_error);

	if (param_error)
	{
		if (g_error_matches (param_error,
		                     SOUP_SOAP_PARAM_ERROR,
		                     SOUP_SOAP_PARAM_ERROR_INVALID_VALUE))
		{
			g_set_error (error, SOUP_SOAP_PARAM_ERROR,
			             SOUP_SOAP_PARAM_ERROR_INVALID_VALUE,
			             _("Value cannot be interpreted."));
			g_error_free (param_error);
		}
		else
			g_propagate_error (error, param_error);
	}

	return int_value;
}

void
soup_soap_param_set_integer (SoupSoapParam *param,
                             gint value)
{
	gchar *result;

	g_return_if_fail (SOUP_SOAP_IS_PARAM (param));

	result = g_strdup_printf ("%d", value);
	soup_soap_param_set_value (param, result);
	g_free (result);
}

gdouble
soup_soap_param_get_double (SoupSoapParam *param,
                            GError **error)
{
	GError *param_error;
	const gchar *value;
	gdouble double_value;

	g_return_val_if_fail (SOUP_SOAP_IS_PARAM (param), -1);

	param_error = NULL;

	value = soup_soap_param_get_value (param);

	double_value = parse_value_as_double (value, &param_error);

	if (param_error)
	{
		if (g_error_matches (param_error,
		                     SOUP_SOAP_PARAM_ERROR,
		                     SOUP_SOAP_PARAM_ERROR_INVALID_VALUE))
		{
			g_set_error (error, SOUP_SOAP_PARAM_ERROR,
			             SOUP_SOAP_PARAM_ERROR_INVALID_VALUE,
			             _("Value cannot be interpreted."));
			g_error_free (param_error);
		}
		else
			g_propagate_error (error, param_error);
	}

	return double_value;
}

void
soup_soap_param_set_double (SoupSoapParam *param,
                            gdouble value)
{
	gchar result[G_ASCII_DTOSTR_BUF_SIZE];

	g_return_if_fail (SOUP_SOAP_IS_PARAM (param));

	g_ascii_dtostr (result, sizeof (result), value);
	soup_soap_param_set_value (param, result);
}

guchar *
soup_soap_param_get_base64_binary (SoupSoapParam *param,
                                   gsize *value_len,
                                   GError **error)
{
	GError *param_error;
	const gchar *value;
	guchar *base64_value;
	gsize result_len = 0;

	g_return_val_if_fail (SOUP_SOAP_IS_PARAM (param), NULL);

	param_error = NULL;

	value = soup_soap_param_get_value (param);

	base64_value = parse_value_as_base64_binary (value, &result_len, &param_error);

	if (param_error)
	{
		if (g_error_matches (param_error,
		                     SOUP_SOAP_PARAM_ERROR,
		                     SOUP_SOAP_PARAM_ERROR_INVALID_VALUE))
		{
			g_set_error (error, SOUP_SOAP_PARAM_ERROR,
			             SOUP_SOAP_PARAM_ERROR_INVALID_VALUE,
			             _("Value cannot be interpreted."));
			g_error_free (param_error);
		}
		else
			g_propagate_error (error, param_error);
	}
	else if (value_len)
		*value_len = result_len;

	return base64_value;
}

void
soup_soap_param_set_base64_binary (SoupSoapParam *param,
                                   const guchar *value,
                                   gsize value_len)
{
	gchar *result;

	g_return_if_fail (SOUP_SOAP_IS_PARAM (param));

	result = g_base64_encode (value, value_len);
	soup_soap_param_set_value (param, result);
	g_free (result);
}

gchar *
soup_soap_param_get_base64_string (SoupSoapParam *param,
                                   GError **error)
{
	return (gchar *) soup_soap_param_get_base64_binary (param, NULL, error);
}

void
soup_soap_param_set_base64_string (SoupSoapParam *param,
                                   const gchar *value)
{
	soup_soap_param_set_base64_binary (param,
	                                   (const guchar *) value,
	                                   strlen (value));
}
