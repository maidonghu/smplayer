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

#ifndef CONNECTIONCV_H
#define CONNECTIONCV_H

#include "connectionbase.h"
#include "videolayerrender.h"

#define COPY_BUFFER

class MConnection;

class ConnectionCV : public ConnectionBase
{
	Q_OBJECT

public:
	ConnectionCV(VideoLayerRender * parent = 0);
	~ConnectionCV();

	bool isVOSupported(const QString & vo) { return (vo == "corevideo" || vo == "sharedbuffer"); }

public slots:
	virtual void start();
	virtual void stop();

protected slots:
	void init_slot(int width, int height, int bytes, int aspect);
	void render_slot();
	void stop_slot();

protected:
	void start_connection();
	void stop_connection();

	MConnection * mconnection;
	QString buffer_name;
	int shm_fd;
	unsigned char * image_buffer;
	int buffer_size;

#ifdef COPY_BUFFER
	unsigned char * copy_buffer;
	int copy_buffer_size;
#endif
};

#endif
