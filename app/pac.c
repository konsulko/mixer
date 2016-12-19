/*
 * Copyright (C) 2016 Konsulko Group
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <limits.h>
#include <assert.h>

#include <pulse/pulseaudio.h>
#include <sys/queue.h>

#include "pacontrolmodel.h"
#include "pac.h"

/* FIXME: move these into a pac context */
static pa_threaded_mainloop* m;
TAILQ_HEAD(pac_cstateq, pac_cstate);
static struct pac_cstateq cstateq;

static void add_one_cstate(int type, int index, const pa_cvolume *cvolume)
{
	struct pac_cstate *cstate;
	int i;

	cstate = pa_xnew(struct pac_cstate, 1);
	cstate->type = type;
	cstate->index = index;
	cstate->cvolume.channels = cvolume->channels;
	for (i = 0; i < cvolume->channels; i++)
		cstate->cvolume.values[i] = cvolume->values[i];

	TAILQ_INSERT_TAIL(&cstateq, cstate, tailq);
}

static void get_source_list_cb(pa_context *c,
		const pa_source_info *i,
		int eol,
		void *data)
{
	int chan;

	if (eol < 0) {
		fprintf(stderr, "get source list: %s\n",
				pa_strerror(pa_context_errno(c)));

		pa_threaded_mainloop_stop(m);
		return;
	}

	if (!eol) {
		assert(i);
		add_one_cstate(C_SOURCE, i->index, &i->volume);
		for (chan = 0; chan < i->channel_map.channels; chan++) {
			add_one_control(data, i->index, i->description,
					C_SOURCE, chan,
					channel_position_string[i->channel_map.map[chan]],
					i->volume.values[chan]);
		}
	}
}

static void get_sink_list_cb(pa_context *c,
		const pa_sink_info *i,
		int eol,
		void *data)
{
	int chan;

	if(eol < 0) {
		fprintf(stderr, "get sink list: %s\n",
				pa_strerror(pa_context_errno(c)));

		pa_threaded_mainloop_stop(m);
		return;
	}

	if(!eol) {
		assert(i);
		add_one_cstate(C_SINK, i->index, &i->volume);
		for (chan = 0; chan < i->channel_map.channels; chan++) {
			add_one_control(data, i->index, i->description,
					C_SINK, chan,
					channel_position_string[i->channel_map.map[chan]],
					i->volume.values[chan]);
		}
	}
}

static void context_state_cb(pa_context *c, void *data) {
	pa_operation *o;

	assert(c);
	switch (pa_context_get_state(c)) {
		case PA_CONTEXT_CONNECTING:
		case PA_CONTEXT_AUTHORIZING:
		case PA_CONTEXT_SETTING_NAME:
			break;
		case PA_CONTEXT_READY:
			/* Fetch the controls of interest */
			if (!(o = pa_context_get_source_info_list(c, get_source_list_cb, data))) {
				fprintf(stderr, "get source info list: %s\n",
					pa_strerror(pa_context_errno(c)));
				return;
			}
			pa_operation_unref(o);
			if (!(o = pa_context_get_sink_info_list(c, get_sink_list_cb, data))) {
				fprintf(stderr, "get sink info list: %s\n",
					pa_strerror(pa_context_errno(c)));
				return;
			}
			break;
		case PA_CONTEXT_TERMINATED:
			pa_threaded_mainloop_stop(m);
			break;
		case PA_CONTEXT_FAILED:
		default:
			fprintf(stderr, "PA connection failed: %s\n",
					pa_strerror(pa_context_errno(c)));
			pa_threaded_mainloop_stop(m);
	}
}

static void pac_set_source_volume_cb(pa_context *c, int success, void *userdata __attribute__((unused))) {
	assert(c);
	if (!success)
		fprintf(stderr, "Set source volume: %s\n",
				pa_strerror(pa_context_errno(c)));
}

static void pac_set_sink_volume_cb(pa_context *c, int success, void *userdata __attribute__((unused))) {
	assert(c);
	if (!success)
		fprintf(stderr, "Set source volume: %s\n",
				pa_strerror(pa_context_errno(c)));
}

void pac_set_volume(pa_context *c, uint32_t type, uint32_t idx, uint32_t channel, uint32_t volume)
{
	pa_operation *o;
	struct pac_cstate *cstate;

	TAILQ_FOREACH(cstate, &cstateq, tailq)
		if (cstate->index == idx)
			break;
	cstate->cvolume.values[channel] = volume;

	if (type == C_SOURCE) {
		if (!(o = pa_context_set_source_volume_by_index(c, idx, &cstate->cvolume, pac_set_source_volume_cb, NULL))) {
			fprintf(stderr, "set source #%d channel #%d volume: %s\n",
				idx, channel, pa_strerror(pa_context_errno(c)));
			return;
		}
		pa_operation_unref(o);
	} else if (type == C_SINK) {
		if (!(o = pa_context_set_sink_volume_by_index(c, idx, &cstate->cvolume, pac_set_sink_volume_cb, NULL))) {
			fprintf(stderr, "set sink #%d channel #%d volume: %s\n",
				idx, channel, pa_strerror(pa_context_errno(c)));
			return;
		}
		pa_operation_unref(o);
	}
}

pa_context *pac_init(void *this, const char *name) {
	pa_context *c;
	pa_mainloop_api *mapi;
	char *server = NULL;
	char *client = pa_xstrdup(name);

	TAILQ_INIT(&cstateq);

	if (!(m = pa_threaded_mainloop_new())) {
		fprintf(stderr, "pa_mainloop_new() failed.\n");
		return NULL;
	}

	pa_threaded_mainloop_set_name(m, "pa_mainloop");
	mapi = pa_threaded_mainloop_get_api(m);

	if (!(c = pa_context_new(mapi, client))) {
		fprintf(stderr, "pa_context_new() failed.\n");
		goto exit;
	}

	pa_context_set_state_callback(c, context_state_cb, this);
	if (pa_context_connect(c, server, 0, NULL) < 0) {
		fprintf(stderr, "pa_context_connect(): %s", pa_strerror(pa_context_errno(c)));
		goto exit;
	}

	if (pa_threaded_mainloop_start(m) < 0) {
		fprintf(stderr, "pa_mainloop_run() failed.\n");
		goto exit;
	}

	return c;

exit:
	if (c)
		pa_context_unref(c);

	if (m)
		pa_threaded_mainloop_free(m);

	pa_xfree(client);

	return NULL;
}
