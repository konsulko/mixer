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

#include <pulse/pulseaudio.h>
#include <sys/queue.h>

#ifdef __cplusplus
extern "C" void pac_set_volume(pa_context *, uint32_t, uint32_t, uint32_t, uint32_t);
extern "C" pa_context *pac_init(void *, const char *);
#else
static char * channel_position_string[] =
{
	"Mono",
	"Front Left",
	"Front Right",
	"Center",
	"Rear Center",
	"Rear Left",
	"Rear Right",
	"LFE",
	"Left Center",
	"Right Center",
	"Side Left",
	"Side Right",
};

enum control_type
{
	C_SOURCE,
	C_SINK
};

struct pac_cstate
{
	TAILQ_ENTRY(pac_cstate) tailq;
	int type;
	uint32_t index;
	pa_cvolume cvolume;
};

#endif
