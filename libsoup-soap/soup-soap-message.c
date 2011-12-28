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

#include <libxml/tree.h>

#define XSD_NAMESPACE "http://www.w3.org/1999/XMLSchema"
#define XSI_NAMESPACE "http://www.w3.org/1999/XMLSchema-instance"
#define SOAP_ENC_NAMESPACE "http://schemas.xmlsoap.org/soap/encoding/"
#define SOAP_ENV_NAMESPACE "http://schemas.xmlsoap.org/soap/envelope/"

#define SOAP_ENCODING_STYLE "http://schemas.xmlsoap.org/soap/encoding/"

struct _SoupSoapMessagePrivate
{
	SoupSoapParamGroup *header;
	SoupSoapParamGroup *body;
	SoupMessageHeaders *message_headers;
	SoupMessageBody *message_body;
};

#define SOUP_SOAP_MESSAGE_GET_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), SOUP_SOAP_TYPE_MESSAGE, SoupSoapMessagePrivate))

enum
{
	PROP_0,

	PROP_MESSAGE_HEADERS,
	PROP_MESSAGE_BODY
};


xmlNodePtr
create_param_node (xmlDocPtr doc,
                   SoupSoapParam *param,
                   xmlNodePtr parent_node)
{
	xmlNodePtr node = xmlNewChild (parent_node, NULL,
	                               BAD_CAST soup_soap_param_get_name (param),
	                               NULL);

	SoupSoapParam *param_child;

	if (SOUP_SOAP_IS_PARAM_GROUP (param))
	{
		GList *elements =
			soup_soap_param_group_get_elements (SOUP_SOAP_PARAM_GROUP (param));
		GList *curr_element = elements;

		while (curr_element)
		{
			param_child = curr_element->data;

			create_param_node (doc, param_child, node);

			curr_element = g_list_next (curr_element);
		}
	}
	else
	{
		xmlChar *content = xmlEncodeEntitiesReentrant (doc,
		                                               BAD_CAST soup_soap_param_get_value (param));
		xmlNodeSetContent (node, content);
		xmlFree (content);
	}

	return node;
}

static void
parse_param (SoupSoapParamGroup *group,
             xmlNodePtr node)
{
	xmlNodePtr current_node, child_node;
	xmlChar *temp;
	gboolean have_element;

	SoupSoapParam *param;
	SoupSoapParamGroup *param_group;

	for (current_node = node->children;
	     current_node != NULL;
	     current_node = current_node->next)
	{
		if (current_node->type == XML_ELEMENT_NODE)
		{
			have_element = FALSE;

			for (child_node = current_node->children;
			     child_node != NULL;
			     child_node = child_node->next)
			{
				if (child_node->type == XML_ELEMENT_NODE)
				{
					have_element = TRUE;
					break;
				}
			}

			if (have_element)
			{
				param_group =
					soup_soap_param_group_new ((gchar *) current_node->name);
				soup_soap_param_group_add (group,
				                           SOUP_SOAP_PARAM (param_group));
				parse_param (param_group, current_node);
			}
			else
			{
				temp = xmlNodeGetContent (current_node);
				param = soup_soap_param_new_value ((gchar *) current_node->name,
				                                   (gchar *) temp);
				soup_soap_param_group_add (group, param);
				xmlFree (temp);
			}
		}
	}
}


G_DEFINE_TYPE (SoupSoapMessage, soup_soap_message, G_TYPE_OBJECT);

static void
soup_soap_message_init (SoupSoapMessage *object)
{
	object->priv = SOUP_SOAP_MESSAGE_GET_PRIVATE (object);
	SoupSoapMessagePrivate *priv = object->priv;

	priv->header = g_object_ref_sink (soup_soap_param_group_new ("Header"));
	priv->body = g_object_ref_sink (soup_soap_param_group_new ("Body"));
	priv->message_headers = NULL;
	priv->message_body = NULL;
}

static void
soup_soap_message_constructed (GObject *object)
{
	SoupSoapMessage *msg = SOUP_SOAP_MESSAGE (object);
	SoupSoapMessagePrivate *priv = msg->priv;

	soup_buffer_free (soup_message_body_flatten (priv->message_body));

	xmlNodePtr current_node, op_node;

	xmlDocPtr doc = xmlParseDoc (BAD_CAST priv->message_body->data);

	if (doc)
	{
		current_node = xmlDocGetRootElement (doc);

		if (current_node &&
		    current_node->name &&
		    xmlStrEqual (current_node->name, BAD_CAST "Envelope"))
		{
			for (current_node = current_node->children;
			     current_node != NULL;
			     current_node = current_node->next)
			{
				if (current_node->type == XML_ELEMENT_NODE)
				{
					if (xmlStrEqual (current_node->name, BAD_CAST "Header"))
						parse_param (priv->header,
						             current_node);
					else if (xmlStrEqual (current_node->name, BAD_CAST "Body"))
					{
						op_node = current_node->children;
						if (op_node)
						{
							soup_soap_param_set_name (SOUP_SOAP_PARAM (priv->body),
							                          (gchar *) op_node->name);
							parse_param (priv->body,
							             op_node);
						}
					}
				}
			}
		}
	}

	xmlFreeDoc (doc);

	G_OBJECT_CLASS (soup_soap_message_parent_class)->constructed (object);
}

static void
soup_soap_message_finalize (GObject *object)
{
	G_OBJECT_CLASS (soup_soap_message_parent_class)->finalize (object);
}

static void
soup_soap_message_set_property (GObject *object,
                                guint prop_id,
                                const GValue *value,
                                GParamSpec *pspec)
{
	g_return_if_fail (SOUP_SOAP_IS_MESSAGE (object));

	SoupSoapMessage *msg = SOUP_SOAP_MESSAGE (object);
	SoupSoapMessagePrivate *priv = msg->priv;

	switch (prop_id)
	{
		case PROP_MESSAGE_HEADERS:
			priv->message_headers = g_value_get_boxed (value);
			break;
		case PROP_MESSAGE_BODY:
			priv->message_body = g_value_get_boxed (value);
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
soup_soap_message_get_property (GObject *object,
                                guint prop_id,
                                GValue *value,
                                GParamSpec *pspec)
{
	g_return_if_fail (SOUP_SOAP_IS_MESSAGE (object));

	SoupSoapMessage *msg = SOUP_SOAP_MESSAGE (object);
	SoupSoapMessagePrivate *priv = msg->priv;

	switch (prop_id)
	{
		case PROP_MESSAGE_HEADERS:
			g_value_set_boxed (value, priv->message_headers);
			break;
		case PROP_MESSAGE_BODY:
			g_value_set_boxed (value, priv->message_body);
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
soup_soap_message_class_init (SoupSoapMessageClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	/*GObjectClass *parent_class = G_OBJECT_CLASS (klass);*/

	g_type_class_add_private (klass, sizeof (SoupSoapMessagePrivate));

	object_class->constructed = soup_soap_message_constructed;
	object_class->finalize = soup_soap_message_finalize;
	object_class->set_property = soup_soap_message_set_property;
	object_class->get_property = soup_soap_message_get_property;

	g_object_class_install_property (object_class,
	                                 PROP_MESSAGE_HEADERS,
	                                 g_param_spec_boxed ("message-headers",
	                                                     "Raw message header",
	                                                     "Set the raw message header to manipulate",
	                                                     SOUP_TYPE_MESSAGE_HEADERS,
	                                                     G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));

	g_object_class_install_property (object_class,
	                                 PROP_MESSAGE_BODY,
	                                 g_param_spec_boxed ("message-body",
	                                                     "Raw message body",
	                                                     "Set the raw message body to manipulate",
	                                                     SOUP_TYPE_MESSAGE_BODY,
	                                                     G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));
}


SoupSoapMessage *
soup_soap_message_new (SoupMessageHeaders *headers,
                       SoupMessageBody *body)
{
	g_return_val_if_fail (headers != NULL, NULL);
	g_return_val_if_fail (body != NULL, NULL);

	return g_object_new (SOUP_SOAP_TYPE_MESSAGE,
	                     "message-headers", headers,
	                     "message-body", body,
	                     NULL);
}

SoupSoapMessage *
soup_soap_message_new_request (SoupMessage *msg)
{
	g_return_val_if_fail (SOUP_IS_MESSAGE (msg), NULL);

	return soup_soap_message_new (msg->request_headers,
	                              msg->request_body);
}

SoupSoapMessage *
soup_soap_message_new_response (SoupMessage *msg)
{
	g_return_val_if_fail (SOUP_IS_MESSAGE (msg), NULL);

	return soup_soap_message_new (msg->response_headers,
	                              msg->response_body);
}

const gchar *
soup_soap_message_get_operation_name (SoupSoapMessage *msg)
{
	g_return_val_if_fail (SOUP_SOAP_IS_MESSAGE (msg), NULL);

	return soup_soap_param_get_name (SOUP_SOAP_PARAM (msg->priv->body));
}

void
soup_soap_message_set_operation_name (SoupSoapMessage *msg,
                                      const gchar *name)
{
	g_return_if_fail (SOUP_SOAP_IS_MESSAGE (msg));

	soup_soap_param_set_name (SOUP_SOAP_PARAM (msg->priv->body),
	                          name);
}

SoupSoapParamGroup *
soup_soap_message_get_header (SoupSoapMessage *msg)
{
	g_return_val_if_fail (SOUP_SOAP_IS_MESSAGE (msg), NULL);

	return msg->priv->header;
}

SoupSoapParamGroup *
soup_soap_message_get_params (SoupSoapMessage *msg)
{
	g_return_val_if_fail (SOUP_SOAP_IS_MESSAGE (msg), NULL);

	return msg->priv->body;
}

void
soup_soap_message_persist (SoupSoapMessage *msg)
{
	g_return_if_fail (SOUP_SOAP_IS_MESSAGE (msg));

	xmlChar *buffer;
	const gchar *contents;

	SoupSoapMessagePrivate *priv = msg->priv;

	xmlDocPtr doc = xmlNewDoc (BAD_CAST "1.0");


	xmlNodePtr envelope_node = xmlNewNode (NULL, BAD_CAST "Envelope");
	xmlSetNs (envelope_node,
	          xmlNewNs (envelope_node,
	                    BAD_CAST SOAP_ENV_NAMESPACE,
	                    BAD_CAST "SOAP-ENV"));
	xmlNewNs (envelope_node,
	          BAD_CAST XSD_NAMESPACE,
	          BAD_CAST "xsd");
	xmlNewNs (envelope_node,
	          BAD_CAST XSI_NAMESPACE,
	          BAD_CAST "xsi");
	xmlNewNs (envelope_node,
	          BAD_CAST SOAP_ENC_NAMESPACE,
	          BAD_CAST "SOAP-ENC");
	xmlSetProp (envelope_node,
	            BAD_CAST "SOAP-ENV:encodingStyle",
	            BAD_CAST SOAP_ENCODING_STYLE);
	xmlDocSetRootElement (doc, envelope_node);

	create_param_node (doc, SOUP_SOAP_PARAM (priv->header), envelope_node);

	xmlNodePtr body_node = xmlNewChild (envelope_node, NULL, BAD_CAST "Body", NULL);
	create_param_node (doc, SOUP_SOAP_PARAM (priv->body), body_node);


	xmlDocDumpFormatMemoryEnc (doc, &buffer, NULL, "UTF-8", 0);

	contents = (gchar *) buffer;
	soup_message_body_truncate (priv->message_body);
	soup_message_body_append (priv->message_body, SOUP_MEMORY_COPY,
	                          contents, strlen (contents));
	soup_message_body_complete (priv->message_body);

	xmlFree (buffer);


	xmlFreeDoc (doc);


	soup_message_headers_set_content_type (priv->message_headers,
	                                       "text/xml", NULL);
}
