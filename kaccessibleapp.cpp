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
#include "kaccessibleinterface.h"

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusPendingCall>
#include <QDBusArgument>
#include <QDBusMetaType>
#include <QTimer>
#include <QStack>
#include <QFile>
#include <QTextStream>
#include <QMutex>
#include <QMutexLocker>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <klocale.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kdebug.h>

#if defined(SPEECHD_FOUND)
#include <libspeechd.h>
#endif

Q_GLOBAL_STATIC(Speaker, speaker);

class Speaker::Private
{
    public:
        bool m_isSpeaking;
        QStack< QPair<QString,Speaker::Priority> > m_sayStack;
        QMutex m_mutex;
#if defined(SPEECHD_FOUND)
        SPDConnection *m_connection;
#endif
        explicit Private()
            : m_isSpeaking(false)
#if defined(SPEECHD_FOUND)
            , m_connection(0)
#endif
        {
        }
#if defined(SPEECHD_FOUND)
        static void speechdCallback(size_t msg_id, size_t client_id, SPDNotificationType state)
        {
            Q_UNUSED(msg_id);
            Q_UNUSED(client_id);
            switch(state) {
                case SPD_EVENT_BEGIN:
                    Speaker::instance()->setSpeaking(true);
                    break;
                case SPD_EVENT_END:
                    Speaker::instance()->setSpeaking(false);
                    QTimer::singleShot(0, Speaker::instance(), SLOT(sayNext()));
                    break;
                case SPD_EVENT_CANCEL:
                    Speaker::instance()->setSpeaking(false);
                    Speaker::instance()->clearSayStack();
                    break;
                case SPD_EVENT_PAUSE:
                    break;
                case SPD_EVENT_RESUME:
                    break;
                case SPD_EVENT_INDEX_MARK:
                    break;
            }
        }
#endif
};

Speaker::Speaker()
    : d(new Private)
{
}

Speaker::~Speaker()
{
    disconnect();
    delete d;
}

Speaker* Speaker::instance()
{
    return speaker();
}

bool Speaker::isConnected() const
{
#if defined(SPEECHD_FOUND)
    return d->m_connection;
#else
    return true;
#endif
}

void Speaker::disconnect()
{
#if defined(SPEECHD_FOUND)
    if(d->m_connection) {
        spd_set_notification_off(d->m_connection, SPD_BEGIN);
        spd_set_notification_off(d->m_connection, SPD_END);
        spd_set_notification_off(d->m_connection, SPD_CANCEL);
        spd_set_notification_off(d->m_connection, SPD_PAUSE);
        spd_set_notification_off(d->m_connection, SPD_RESUME);
        d->m_connection->callback_begin = d->m_connection->callback_end = d->m_connection->callback_cancel = d->m_connection->callback_pause = d->m_connection->callback_resume = 0;
        spd_close(d->m_connection);
        d->m_connection = 0;
        d->m_isSpeaking = false;
        d->m_sayStack.clear();
    }
#endif
}

bool Speaker::reconnect()
{
    disconnect();

#if defined(SPEECHD_FOUND)
    d->m_connection = spd_open("kaccessible", "main", NULL, SPD_MODE_THREADED); //SPD_MODE_SINGLE);
    if( ! d->m_connection) {
        kWarning() << "Failed to connect with speech-dispatcher";
        return false;
    }

    d->m_connection->callback_begin = d->m_connection->callback_end = d->m_connection->callback_cancel = d->m_connection->callback_pause = d->m_connection->callback_resume = Private::speechdCallback;
    spd_set_notification_on(d->m_connection, SPD_BEGIN);
    spd_set_notification_on(d->m_connection, SPD_END);
    spd_set_notification_on(d->m_connection, SPD_CANCEL);
    spd_set_notification_on(d->m_connection, SPD_PAUSE);
    spd_set_notification_on(d->m_connection, SPD_RESUME);
#endif
    return true;
}

bool Speaker::isSpeaking() const
{
    return d->m_isSpeaking;
}

void Speaker::setSpeaking(bool speaking)
{
    d->m_isSpeaking = speaking;
}

void Speaker::cancel()
{
    QMutexLocker locker(&d->m_mutex);
    d->m_sayStack.clear();
#if defined(SPEECHD_FOUND)
    if(d->m_connection) {
        spd_cancel_all(d->m_connection);
    }
#endif
}

bool Speaker::say(const QString& text, Priority priority)
{
    QMutexLocker locker(&d->m_mutex);
    if(!isConnected())
        return false;
    d->m_sayStack.push( QPair<QString,Speaker::Priority>(text,priority) );
    if(!d->m_isSpeaking)
        QTimer::singleShot(0, this, SLOT(sayNext()));
    return true;
}

void Speaker::sayNext()
{
    QMutexLocker locker(&d->m_mutex);
    if(d->m_sayStack.isEmpty()) {
        return;
    }
    QPair<QString,Speaker::Priority> p = d->m_sayStack.pop();
#if defined(SPEECHD_FOUND)
    if(d->m_connection) {
        SPDPriority spdpriority = (SPDPriority) p.second;
        int msg_id  = spd_say(d->m_connection, spdpriority, p.first.toUtf8().data());
        if(msg_id == -1) {
            kWarning() << "Failed to say text=" << p.first;
        }
    }
#else
    //QDBusInterface iface("org.kde.jovie","/KSpeech");
    //iface.asyncCall("say", text, 0);
#endif
}

void Speaker::clearSayStack()
{
    d->m_sayStack.clear();
}

class Adaptor::Private
{
    public:
        bool m_speechEnabled;
        explicit Private()
            : m_speechEnabled(false)
        {
            KConfig config("kaccessibleapp");
            KConfigGroup group = config.group("Main");
            m_speechEnabled = group.readEntry("SpeechEnabled", m_speechEnabled);
        }
};

Adaptor::Adaptor(QObject *parent)
    : QObject(parent)
    , d(new Private)
{
    qDBusRegisterMetaType<KAccessibleDBusInterface>();
}

Adaptor::~Adaptor()
{
    delete d;
}
        
void Adaptor::setFocusChanged(const KAccessibleDBusInterface& iface)
{
    int px = -1;
    int py = -1;
    QRect r = iface.rect;
    emit focusChanged(px, py, r.x(), r.y(), r.width(), r.height());
    QString text = iface.name;
    if(!iface.accelerator.isEmpty()) {
        QString s = iface.accelerator;
        s = s.replace("+"," ");
        text += " " + s;
    }
    sayText(text);
}

void Adaptor::setValueChanged(const KAccessibleDBusInterface& iface)
{
    sayText(iface.value);
}

void Adaptor::setAlert(const KAccessibleDBusInterface& iface)
{
    Speaker::instance()->cancel();
    sayText(iface.name, int(Speaker::Message));
}

void Adaptor::sayText(const QString& text, int priority)
{
    if(d->m_speechEnabled && !text.isEmpty() && (Speaker::instance()->isConnected() || Speaker::instance()->reconnect())) {
        Speaker::instance()->say(text, Speaker::Priority(priority));
    }
}

bool Adaptor::speechEnabled() const
{
    return d->m_speechEnabled;
}

void Adaptor::setSpeechEnabled(bool enabled)
{
    d->m_speechEnabled = enabled;

    KConfig config("kaccessibleapp");
    KConfigGroup group = config.group("Main");
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
