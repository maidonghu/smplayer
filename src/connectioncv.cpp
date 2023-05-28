/*  smplayer, GUI front-end for mplayer.
    Copyright (C) 2006-2023 Ricardo Villalba <ricardo@smplayer.info>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "connectioncv.h"
#include "mconnection.h"

#include <QCoreApplication>
#include <QDebug>

#include <fcntl.h>
#include <sys/mman.h>
#include <stdint.h>
#include <errno.h>

ConnectionCV::ConnectionCV(VideoLayerRender * parent)
	: ConnectionBase(parent)
	, shm_fd(0)
	, image_buffer(0)
	, buffer_size(0)
#ifdef COPY_BUFFER
	, copy_buffer(0)
	, copy_buffer_size(0)
#endif
{
	buffer_name = QString("/smplayer-%1").arg(QCoreApplication::applicationPid());
	mconnection = new MConnection(this, buffer_name);
	start_connection();
}

ConnectionCV::~ConnectionCV() {
	delete mconnection;
	stop_connection();

#ifdef COPY_BUFFER
	if (copy_buffer != 0) {
		free(copy_buffer);
	}
#endif
}

void ConnectionCV::stop() {
	qDebug("ConnectionCV::stop");
	//stop_connection();
}

void ConnectionCV::start() {
	qDebug("ConnectionCV::start");
	//start_connection();
}

void ConnectionCV::init_slot(int width, int height, int bytes, int aspect) {
	qDebug("ConnectionCV::init_slot: %d %d %d %d", width, height, bytes, aspect);

	int image_width = width;
	int image_height = height;
	int image_bytes = bytes;

	uint32_t format = ConnectionBase::UYVY;
	if (bytes == 1) {
		buffer_size = image_width * image_height * 2;
		format = ConnectionBase::I420;
	} else {
		buffer_size = image_width * image_height * image_bytes;
	}

	shm_fd = shm_open(buffer_name.toLatin1().constData(), O_RDONLY, S_IRUSR);
	if (shm_fd == -1) {
		qDebug("ConnectionCV::init_slot: shm_open failed");
		return;
	}

	image_buffer = (unsigned char*) mmap(NULL, buffer_size, PROT_READ, MAP_SHARED, shm_fd, 0);
	if (image_buffer == MAP_FAILED) {
		qDebug("ConnectionCV::init_slot: mmap failed");
		return;
	}

	video_window->init(image_width, image_height, image_bytes, format, image_buffer);
	video_window->setColorspace(ConnectionBase::CSP_BT_709, ConnectionBase::PRIM_BT_709);
}

void ConnectionCV::render_slot() {
	//qDebug("ConnectionCV::render_slot");
	if (image_buffer) {
		#ifdef COPY_BUFFER
		if (copy_buffer == 0 || copy_buffer_size < buffer_size) {
			qDebug("ConnectionCV::render_slot: creating copy_buffer");
			if (copy_buffer != 0) free(copy_buffer);
			copy_buffer = (unsigned char *) malloc(buffer_size);
			copy_buffer_size = buffer_size;
			video_window->setImageBuffer(copy_buffer);
		}
		memcpy(copy_buffer, image_buffer, buffer_size);
		#endif
		video_window->render();
	}
}

void ConnectionCV::stop_slot() {
	qDebug("ConnectionCV::stop_slot");
}

void ConnectionCV::start_connection() {
	mconnection->startConnection();
}

void ConnectionCV::stop_connection() {
	mconnection->stopConnection();

	// Destroy the shared buffer
	if (munmap(image_buffer, buffer_size) == -1) {
		qDebug("ConnectionCV::stop_connection: munmap failed");
	}
	image_buffer = 0;
}

#include "moc_connectioncv.cpp"
