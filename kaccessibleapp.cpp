/* This file is part of the KDE project
 * Copyright (C) 2010 Sebastian Sauer <sebsauer@kdab.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#include "kaccessibleapp.h"

#include <QDBusConnection>
#include <QTimer>
#include <klocale.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kdebug.h>

void Adaptor::setFocusChanged(int px, int py, int rx, int ry, int rwidth, int rheight)
{
    emit focusChanged(px, py, rx, ry, rwidth, rheight);
}

KAccessibleApp::KAccessibleApp()
    : KUniqueApplication()
{
    if( ! QDBusConnection::sessionBus().registerObject("/Adaptor", new Adaptor(this), QDBusConnection::ExportAllContents)) {
        kWarning() << "Unable to register KAccessibleApp to dbus";
        QTimer::singleShot(0, this, SLOT(quit()));
    }
}

KAccessibleApp::~KAccessibleApp()
{
}

int main(int argc, char *argv[])
{
    KAboutData aboutData("kaccessibleapp", "", ki18n("KDE Accessibility Application"), "0.1",
                         ki18n("KDE Accessibility Application"), KAboutData::License_GPL,
                         ki18n("(c) 2010 Sebastian Sauer"));
    aboutData.addAuthor(ki18n("Sebastian Sauer"), ki18n("Maintainer"), "mail@dipe.org");
    KCmdLineArgs::init(argc, argv, &aboutData);
    KUniqueApplication::addCmdLineOptions();
    if (!KUniqueApplication::start()) {
       fprintf(stderr, "kaccessibleapp is already running!\n");
       return 0;
    }

    KAccessibleApp a;
    return a.exec();
}
