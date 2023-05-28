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

#include "myapplication.h"
#include "smplayer.h"

#include <QDir>
#ifdef USE_GL_WINDOW
#include <QSurfaceFormat>
#endif

#ifdef HDPI_SUPPORT
#include "paths.h"
#include "hdpisupport.h"

#ifdef PORTABLE_APP
#ifdef Q_OS_WIN
QString applicationPath() {
	wchar_t my_path[_MAX_PATH+1];
	GetModuleFileName(NULL, my_path,_MAX_PATH);
	QString app_path = QString::fromWCharArray(my_path);
	if (app_path.isEmpty()) return "";
	QFileInfo fi(app_path);
	return fi.absolutePath();
}
#else
QString applicationPath() {
	QString exe_file = QFile::symLinkTarget(QString("/proc/%1/exe").arg(QCoreApplication::applicationPid()));
	return QFileInfo(exe_file).absolutePath();
}
#endif
#endif // PORTABLE_APP

QString hdpiConfig() {
#ifdef PORTABLE_APP
	// We can't use QCoreApplication::applicationDirPath() here
	return applicationPath();
#else
	return Paths::configPath();
#endif // PORTABLE_APP
}
#endif // HDPI_SUPPORT

int main( int argc, char ** argv )
{
#ifdef HDPI_SUPPORT
	QString hdpi_config_path = hdpiConfig();
	HDPISupport * hdpi = 0;
	if (!hdpi_config_path.isEmpty()) {
		hdpi = new HDPISupport(hdpi_config_path);
	}
#endif

	MyApplication a( "smplayer", argc, argv );

#ifdef USE_GL_WINDOW
	QSurfaceFormat fmt;
	fmt.setVersion( 3, 2 );
	fmt.setProfile( QSurfaceFormat::CoreProfile );
	//fmt.setRenderableType(QSurfaceFormat::OpenGLES);
	QSurfaceFormat::setDefaultFormat(fmt);
#endif

	/*
	if (a.isRunning()) { 
		qDebug("Another instance is running. Exiting.");
		return 0;
	}
	*/

	a.setQuitOnLastWindowClosed(false);
	
#ifdef Q_OS_WIN
	// Change the working directory to the application path
	QDir::setCurrent(a.applicationDirPath());
#endif

#if QT_VERSION >= 0x040400
	// Enable icons in menus
	QCoreApplication::setAttribute(Qt::AA_DontShowIconsInMenus, false);
	//QCoreApplication::setAttribute(Qt::AA_DontUseNativeMenuBar, true);
#endif

	// Sets the config path
	QString config_path;

#ifdef PORTABLE_APP
	config_path = a.applicationDirPath();
#else
	// If a smplayer.ini exists in the app path, will use that path
	// for the config file by default
	if (QFile::exists( a.applicationDirPath() + "/smplayer.ini" ) ) {
		config_path = a.applicationDirPath();
		qDebug("main: using existing %s", QString(config_path + "/smplayer.ini").toUtf8().data());
	}
#endif

#ifdef Q_OS_WIN
	QStringList args = a.winArguments();
#else
	QStringList args = a.arguments();
#endif
	int pos = args.indexOf("-config-path");
	if ( pos != -1) {
		if (pos+1 < args.count()) {
			pos++;
			config_path = args[pos];
			// Delete from list
			args.removeAt(pos);
			args.removeAt(pos-1);
		} else {
			printf("Error: expected parameter for -config-path\r\n");
			return SMPlayer::ErrorArgument;
		}
	}

	SMPlayer * smplayer = new SMPlayer(config_path);
	#ifdef HDPI_SUPPORT
	qDebug() << "main: hdpi_config_path:" << hdpi_config_path;
	#endif
	SMPlayer::ExitCode c = smplayer->processArgs( args );
	if (c != SMPlayer::NoExit) {
		return c;
	}
	smplayer->start();

	int r = a.exec();

	delete smplayer;

#ifdef HDPI_SUPPORT
	if (hdpi != 0) delete hdpi;
#endif

	return r;
}

