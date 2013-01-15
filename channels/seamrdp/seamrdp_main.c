/**
 * FreeRDP: A Remote Desktop Protocol client.
 * Seamrdp : Seamless RDP channel
 *
 * Copyright 2012 Ulteo SAS
 * http://www.ulteo.com
 * Author Alexandre CONFIANT-LATOUR <a.confiant@ulteo.com> 2012
 *
 * This program is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; version 2
 * of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <freerdp/types.h>
#include <freerdp/freerdp.h>
#include <freerdp/constants.h>
#include <freerdp/utils/memory.h>
#include <freerdp/utils/svc_plugin.h>

#include <guacamole/client.h>
#include <guacamole/socket.h>

typedef struct seamrdp_plugin
{
	rdpSvcPlugin plugin;
} seamrdpPlugin;

static void seamrdp_process_connect(rdpSvcPlugin* plugin)
{
	/* Vchannel connection callback */
	printf("Seamrdp connect\n");
}

static void seamrdp_process_receive(rdpSvcPlugin* plugin, STREAM* data_in)
{
	/* Vchannel receive proc (receive data FROM server) */
	/* - Get data as a stream
		 - Parse it
		 - Generate an event
		 - Send it to the main process "with svc_plugin_send_event"
		 - Free stream
	*/
	int len = strlen((char*) stream_get_data(data_in));
	RDP_EVENT *event = malloc(sizeof(RDP_EVENT));

	/* Create a new event and copy data from stream */
	event->event_class = RDP_EVENT_CLASS_SEAMRDP;
	event->event_type = 0;
	event->on_event_free_callback = NULL;
	event->user_data = malloc(len+1);
	strncpy(event->user_data, (const char*) stream_get_data(data_in), len);
	((char*)(event->user_data))[len] = '\0';

	/*printf("Seamrdp input : %s\n", ((char*)(event->user_data)));*/

	/* Send the event to the main program */
	svc_plugin_send_event(plugin, event);

	stream_free(data_in);
}

static void seamrdp_process_event(rdpSvcPlugin* plugin, RDP_EVENT* event)
{
	/* Vchannel send proc (send data to server) */
	/* - Get data as an event
		 - Convert-it to a stream
		 - Send it to the server with "svc_plugin_send"
		 - Free event
	*/
	int len = strlen(event->user_data);
	STREAM *stream = stream_new(len+1);

	/* Copy event data to stream */
	stream_write(stream, event->user_data, len);
	stream_write_uint8(stream, 0);

	printf("Seamrdp output : %s\n", ((char*)(stream_get_data(stream))));

	/* Send the stream to the server */
	svc_plugin_send(plugin, stream);

	freerdp_event_free(event);
}

static void seamrdp_process_terminate(rdpSvcPlugin* plugin)
{
	/* Vchannel close callback */
	printf("Seamrdp terminate\n");
}

DEFINE_SVC_PLUGIN(seamrdp, "seamrdp", CHANNEL_OPTION_INITIALIZED | CHANNEL_OPTION_ENCRYPT_RDP | CHANNEL_OPTION_COMPRESS_RDP | CHANNEL_OPTION_SHOW_PROTOCOL)