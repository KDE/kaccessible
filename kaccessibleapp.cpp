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
#include <QDBusInterface>
#include <QDBusPendingCall>
#include <QTimer>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <klocale.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kdebug.h>

#if defined(SPEECHD_FOUND)
#include <libspeechd.h>
#endif

class Adaptor::Private
{
    public:
        bool m_speechEnabled;
        QString m_sayed;
#if defined(SPEECHD_FOUND)
        SPDConnection *m_connection;
#endif

        explicit Private()
            : m_speechEnabled(false)
#if defined(SPEECHD_FOUND)
            , m_connection(0)
#endif
        {
            KConfig config("kaccessibleapp");
            KConfigGroup group = config.group("Main");
            m_speechEnabled = group.readEntry("SpeechEnabled", m_speechEnabled);
        }

        ~Private()
        {
            disconnect();
        }

        void disconnect()
        {
#if defined(SPEECHD_FOUND)
            if(m_connection) {
                spd_close(m_connection);
                m_connection = 0;
            }
#endif
        }
        void reconnect()
        {
            disconnect();
#if defined(SPEECHD_FOUND)
            m_connection = spd_open("kaccessible", "main", NULL, SPD_MODE_THREADED);
            if(!m_connection) kWarning() << "Failed to connect with speech-dispatcher";
#endif
        }
};

Adaptor::Adaptor(QObject *parent)
    : QObject(parent)
    , d(new Private)
{
}

Adaptor::~Adaptor()
{
    delete d;
}
        
void Adaptor::setFocusChanged(int px, int py, int rx, int ry, int rwidth, int rheight, const QString& text)
{
    emit focusChanged(px, py, rx, ry, rwidth, rheight);
    if(d->m_speechEnabled && !text.isEmpty() && text != d->m_sayed) {
        d->m_sayed = text;
        sayText(text);
    }
}

void Adaptor::sayText(const QString& text)
{
    kDebug() << text;
#if defined(SPEECHD_FOUND)
    if(!d->m_connection) {
        d->reconnect();
        if(!d->m_connection)
            return;
    }
    SPDPriority spdpriority = SPD_TEXT;
    spd_say(d->m_connection, spdpriority, text.toUtf8().data());
#endif
#if 0
    QDBusInterface iface("org.kde.jovie","/KSpeech");
    iface.asyncCall("say", text, 0);
#endif
}

bool Adaptor::speechEnabled() const
{
    return d->m_speechEnabled;
}

void Adaptor::setSpeechEnabled(bool enabled)
{
    KConfig config("kaccessibleapp");
    KConfigGroup group = config.group("Main");
    d->m_speechEnabled = enabled;
    group.writeEntry("SpeechEnabled", d->m_speechEnabled);
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
    KAboutData aboutData("kaccessibleapp", "", ki18n("KDE Accessibility Application"), "0.2",
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
