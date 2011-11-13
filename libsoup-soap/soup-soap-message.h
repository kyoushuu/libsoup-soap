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

#ifndef _SOUP_SOAP_MESSAGE_H_
#define _SOUP_SOAP_MESSAGE_H_

#include <glib-object.h>

G_BEGIN_DECLS

#define SOUP_SOAP_TYPE_MESSAGE             (soup_soap_message_get_type ())
#define SOUP_SOAP_MESSAGE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), SOUP_SOAP_TYPE_MESSAGE, SoupSoapMessage))
#define SOUP_SOAP_MESSAGE_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), SOUP_SOAP_TYPE_MESSAGE, SoupSoapMessageClass))
#define SOUP_SOAP_IS_MESSAGE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SOUP_SOAP_TYPE_MESSAGE))
#define SOUP_SOAP_IS_MESSAGE_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), SOUP_SOAP_TYPE_MESSAGE))
#define SOUP_SOAP_MESSAGE_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), SOUP_SOAP_TYPE_MESSAGE, SoupSoapMessageClass))

typedef struct _SoupSoapMessagePrivate SoupSoapMessagePrivate;
typedef struct _SoupSoapMessageClass SoupSoapMessageClass;
typedef struct _SoupSoapMessage SoupSoapMessage;

struct _SoupSoapMessageClass
{
	GObjectClass parent_class;
};

struct _SoupSoapMessage
{
	GObject parent_instance;

	SoupSoapMessagePrivate *priv;
};

GType soup_soap_message_get_type (void) G_GNUC_CONST;
SoupSoapMessage *soup_soap_message_new (SoupMessageHeaders *headers, SoupMessageBody *body);
SoupSoapMessage *soup_soap_message_new_request (SoupMessage *msg);
SoupSoapMessage *soup_soap_message_new_response (SoupMessage *msg);
const gchar *soup_soap_message_get_operation_name (SoupSoapMessage *msg);
void soup_soap_message_set_operation_name (SoupSoapMessage *msg, const gchar *name);
SoupSoapParamGroup *soup_soap_message_get_header (SoupSoapMessage *msg);
SoupSoapParamGroup *soup_soap_message_get_params (SoupSoapMessage *msg);
void soup_soap_message_persist (SoupSoapMessage *msg);
G_END_DECLS

#endif /* _SOUP_SOAP_MESSAGE_H_ */
