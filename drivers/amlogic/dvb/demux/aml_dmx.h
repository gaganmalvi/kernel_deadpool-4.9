/*
 * drivers/amlogic/dvb/demux/aml_dmx.h
 *
 * Copyright (C) 2017 Amlogic, Inc. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 */

#ifndef _AML_DMX_H_
#define _AML_DMX_H_

#include "sw_demux/swdemux.h"
#include "sc2_demux/ts_output.h"
#include "sc2_demux/ts_input.h"
#include "sc2_demux/mem_desc.h"
#include "demux.h"
#include "dvbdev.h"
#include <dmxdev.h>

struct sw_demux_ts_feed {
	struct dmx_ts_feed ts_feed;

	dmx_ts_cb ts_cb;
	struct out_elem *ts_out_elem;
	int cb_id;
	int type;
	int ts_type;
	int pes_type;
	int pid;
	int state;
	int format;
};

struct sw_demux_sec_filter {
	struct dmx_section_filter section_filter;

	struct swdmx_secfilter *secf;
	int state;
};

struct sw_demux_sec_feed {
	struct dmx_section_feed sec_feed;

	int sec_filter_num;
	struct sw_demux_sec_filter *filter;

	struct out_elem *sec_out_elem;
	dmx_section_cb sec_cb;
	int cb_id;
	int pid;
	int check_crc;
	int type;
	int state;
};

struct aml_dmx {
	struct dmx_demux dmx;
	struct dmxdev dev;
	void *priv;
	int id;

	u8 ts_index;
	int demod_sid;
	int local_sid;
	struct in_elem *sc2_input;

	enum dmx_input_source source;
	struct swdmx_demux *swdmx;
	struct swdmx_ts_parser *tsp;

	int ts_feed_num;
	struct sw_demux_ts_feed *ts_feed;

	int sec_feed_num;
	struct sw_demux_sec_feed *section_feed;

	struct list_head frontend_list;

	u16 pids[DMX_PES_OTHER];
#define EACH_DMX_MAX_PCR_NUM		4
	u16 pcr_index[EACH_DMX_MAX_PCR_NUM];
	struct dmx_frontend mem_fe;

	int buf_warning_level;	//percent, used/total, default is 60

#define MAX_SW_DEMUX_USERS 10
	int users;
	/*protect many user operate*/
	struct mutex *pmutex;
	/*protect register operate*/
	spinlock_t *pslock;

	int init;

	/*dvr sec mem*/
	__u32 sec_dvr_buff;
	__u32 sec_dvr_size;
	void *dvr_ts_output;
};

void dmx_init_hw(int sid_num, int *sid_info);
int dmx_init(struct aml_dmx *pdmx, struct dvb_adapter *dvb_adapter);
int dmx_destroy(struct aml_dmx *pdmx);
int dmx_regist_dmx_class(void);
int dmx_unregist_dmx_class(void);
int dmx_get_stc(struct dmx_demux *dmx, unsigned int num,
		u64 *stc, unsigned int *base);
int dmx_get_pcr(struct dmx_demux *dmx, unsigned int num,	u64 *pcr);

#endif
