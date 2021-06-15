/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * drivers/amlogic/media/video_processor/videotunnel/videotunnel_priv.h
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

#ifndef __VIDEO_TUNNEL_PRIV_H
#define __VIDEO_TUNNEL_PRIV_H

#include <linux/device.h>
#include <linux/miscdevice.h>
#include <linux/ioctl.h>
#include <linux/types.h>
#include <linux/kfifo.h>
#include <linux/rbtree.h>
#include <linux/list.h>
#include <linux/idr.h>
#include <linux/kref.h>
#include <linux/rtmutex.h>

#include "uapi/videotunnel.h"

#define  VT_POOL_SIZE 32
#define  VT_MAX_WAIT_MS 4
#define  VT_FENCE_WAIT_MS 3000

union vt_ioctl_arg {
	struct vt_alloc_id_data alloc_data;
	struct vt_ctrl_data ctrl_data;
	struct vt_buffer_data buffer_data;
};

/*
 * struct vt_dev - the metadata of the videotunnel device node
 * @mdev:			the actual misc device
 * @instance_lock:	mutex procting the instances
 * @instance_idr:	an idr space for allocating instance ids
 * @instances:		an rb tree for all the existing video instance
 * @session_lock:	an semaphore protect sessions
 * @sessions:		an rb tree for all the existing video session
 * @dev_name:		the device name
 */
struct vt_dev {
	struct miscdevice mdev;
	struct mutex instance_lock; /* protect the instances */
	struct idr instance_idr;
	struct rb_root instances;

	struct rw_semaphore session_lock;
	struct rb_root sessions;

	char *dev_name;
	struct dentry *debug_root;
};

/*
 * struct videotunbel_session - a process block lock address space data
 * @node:			node in the tree of all session
 * @dev:			backpointer to videotunnel device
 * @instances_head:	an list of all instance allocated in this session
 * @role:			producer or consumer
 * @name:			used for debugging
 * @display_name:	used for debugging (unique version of @name)
 * @display_serial:	used for debugging (to make display_name unique)
 * @task:			used for debugging
 * @cid:			connection id
 */
struct vt_session {
	struct rb_node node;
	struct vt_dev *dev;
	struct list_head instances_head;

	enum vt_role_e role;
	pid_t pid;
	char *name;
	char *display_name;
	int display_serial;
	struct task_struct *task;
	long cid;
	struct dentry *debug_root;
};

/*
 * struct vt_buffer - videtunnel buffer
 * @node:			node in the sessions buffer rb tree
 * @file_buffer:	file get from buffer fd
 * @buffer_fd_pro:	fd in the producer side
 * @buffer_fd_con:	fd in the consumer side
 * @file_fence:		fence file get from release fence
 * @session_pro:	vt_session that queued this buffer
 * @cid_pro:		producer session connection id
 * @item:			buffer item
 */
struct vt_buffer {
	struct file *file_buffer;
	int buffer_fd_pro;
	int buffer_fd_con;

	struct fence *file_fence;
	struct vt_session *session_pro;
	long cid_pro;
	struct vt_buffer_item item;
};

/*
 * struct vt_cmd - videotunnel cmd
 * @cmd:		command
 * @cmd_data:	command data
 * @client_id:	producer pid
 */
struct vt_cmd {
	enum vt_video_cmd_e cmd;
	int cmd_data;
	int client_id;
};

/*
 * struct vt_instance - an instance that holds the buffer
 * @ref:		reference count
 * @id:			unique id allocated by device->idr
 * @node:		node in the videotunel device rbtree
 * @lock:		proctect fifo
 * @dev:		backpointer to device
 * @entry:		list node in session list
 * @consumer:		consumer session on this instance
 * @wait_consumer:	cosnumer wait queue for buffer
 * @producer:		producer session on this instance
 * @wait_producer:	producer wait queue for buffer
 * @cmd_lock:			protect cmd fifo
 * @wait_cmd:			consumer wait queue for vt cmd
 * @fcount:				file operation count
 * @fifo_to_consumer:	fifo that queued the buffer transfer to consumer
 * @fifo_to_producer:	fifo that queued the buffer transfer to producer
 */

struct vt_instance {
	struct kref ref;
	int id;
	struct vt_dev *dev;
	struct rb_node node;

	struct mutex lock; /* proctect fd fifo */
	struct list_head entry;
	struct vt_session *consumer;
	wait_queue_head_t wait_consumer;
	struct vt_session *producer;
	wait_queue_head_t wait_producer;

	struct mutex cmd_lock; /* protect cmd fifo */
	wait_queue_head_t wait_cmd;
	DECLARE_KFIFO_PTR(fifo_cmd, struct vt_cmd*);

	struct dentry *debug_root;
	int fcount;

	DECLARE_KFIFO_PTR(fifo_to_consumer, struct vt_buffer*);
	DECLARE_KFIFO_PTR(fifo_to_producer, struct vt_buffer*);

	struct vt_buffer vt_buffers[VT_POOL_SIZE];
};

#endif /* __VIDEO_TUNNEL_PRIV_H */
