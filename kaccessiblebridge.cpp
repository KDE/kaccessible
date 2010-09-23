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

#include "kaccessiblebridge.h"
#include "kaccessibleinterface.h"

#include <QAccessibleInterface>
#include <QWidget>
#include <QFile>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusInterface>
#include <QDBusPendingCall>
#include <QDBusArgument>
#include <QDBusMetaType>
#include <kdebug.h>

Q_EXPORT_PLUGIN(BridgePlugin)

class Bridge::Private
{
    public:
        BridgePlugin *m_plugin;
        const QString m_key;
        QAccessibleInterface *m_root;
        QDBusInterface *m_app;
        QList<QObject*> m_popupMenus;
        QRect m_lastFocusRect;
        QString m_lastFocusName;

        Private(BridgePlugin *plugin, const QString& key)
            : m_plugin(plugin)
            , m_key(key)
            , m_root(0)
            , m_app(0)
            , m_lastFocusRect(QRect(0,0,0,0))
        {
        }

        ~Private()
        {
            delete m_app;
        }
};

Bridge::Bridge(BridgePlugin *plugin, const QString& key)
    : QObject(plugin)
    , QAccessibleBridge()
    , d(new Private(plugin, key))
{
    qDBusRegisterMetaType<KAccessibleInterface>();
}

Bridge::~Bridge()
{
    delete d;
}

void Bridge::notifyAccessibilityUpdate(int reason, QAccessibleInterface *interface, int child)
{
    if(!d->m_root) {
        return;
    }

    if(reason == QAccessible::ObjectShow || reason == QAccessible::ObjectHide) {
        return;
    }

    if(!d->m_app) {
        d->m_app = new QDBusInterface(QLatin1String( "org.kde.kaccessibleapp" ),QLatin1String( "/Adaptor" ));
        if(d->m_app->isValid())
            kDebug() << "Connected with the org.kde.kaccessibleapp dbus-service";
    }

    if(d->m_app->lastError().isValid()) {
        kDebug() << "DBus error:" << d->m_app->lastError().name() << d->m_app->lastError().message();
        delete d->m_app;
        d->m_app = 0;
        return;
    }

    if(!d->m_app->isValid()) {
        kDebug() << "Failed to connect with the org.kde.kaccessibleapp dbus-service";
        delete d->m_app;
        d->m_app = 0;
        return;
    }

    QObject *obj = interface->object();
    const QString name = interface->text(QAccessible::Name, child);
    const QString description = interface->text(QAccessible::Description, child);

    KAccessibleInterface dbusIface;
    dbusIface.name = name;
    dbusIface.description = description.isEmpty() ? interface->text(QAccessible::Help, child) : description;
    dbusIface.value = interface->text(QAccessible::Value, child);
    dbusIface.accelerator = interface->text(QAccessible::Accelerator, child);
    dbusIface.rect = interface->rect(child);
    dbusIface.objectName = interface->object()->objectName();
    dbusIface.className = QLatin1String( interface->object()->metaObject()->className() );
    dbusIface.state = interface->state(child);

    if(obj->inherits("QMenu") /*|| (!d->m_popupMenus.isEmpty() && obj->inherits("QAction"))*/)
        dbusIface.type = KAccessibleInterface::Menu;
    else if(obj->inherits("QAbstractButton"))
        dbusIface.type = KAccessibleInterface::Button;
    else if(obj->inherits("QLineEdit") || obj->inherits("QAbstractSpinBox"))
        dbusIface.type = KAccessibleInterface::Edit;
    else if(obj->inherits("QComboBox"))
        dbusIface.type = KAccessibleInterface::Combobox;
    else if(obj->inherits("QCheckBox"))
        dbusIface.type = KAccessibleInterface::Checkbox;
    else if(obj->inherits("QRadioButton"))
        dbusIface.type = KAccessibleInterface::Radiobutton;
    else if(obj->inherits("QLabel"))
        dbusIface.type = KAccessibleInterface::Label;
    else if(obj->inherits("QAbstractItemView"))
        dbusIface.type = KAccessibleInterface::Listview;
    else if(obj->inherits("QDialog") || obj->inherits("QMainWindow"))
        dbusIface.type = KAccessibleInterface::Dialog;
    else if(obj->inherits("QWidget"))
        dbusIface.type = KAccessibleInterface::Widget;
    else
        dbusIface.type = KAccessibleInterface::Object;

    QAccessibleInterface *childInterface = 0;
    //if(child > 0) interface->navigate(QAccessible::Child, child, &childInterface);

    switch(reason) {
        case QAccessible::PopupMenuStart: {
            d->m_popupMenus.append(obj);
        } break;
        case QAccessible::PopupMenuEnd: {
            const int index = d->m_popupMenus.lastIndexOf(obj);
            if(index >= 0) d->m_popupMenus.removeAt(index);
        } break;

        case QAccessible::Alert: {
            //kDebug() << reasonToString(reason) << "object=" << (obj ? QString("%1 (%2)").arg(obj->objectName()).arg(obj->metaObject()->className()) : "NULL") << "name=" << name;
            d->m_app->asyncCall(QLatin1String( "setAlert" ), qVariantFromValue(dbusIface));
        } break;

        case QAccessible::DialogStart: {
            kDebug() << reasonToString(reason) << QLatin1String( "object=" ) << (obj ? QString(QLatin1String( "%1 (%2)" )).arg(obj->objectName()).arg(QLatin1String( obj->metaObject()->className() )) : QLatin1String( "NULL" )) << QLatin1String( "name=" ) << name;
            //d->m_app->asyncCall("sayText", name);
        } break;
        case QAccessible::DialogEnd: {
            kDebug() << reasonToString(reason) << QLatin1String( "object=" ) << (obj ? QString(QLatin1String( "%1 (%2)" )).arg(obj->objectName()).arg(QLatin1String( obj->metaObject()->className() )) : QLatin1String( "NULL" )) << QLatin1String( "name=" ) << name;
            //d->m_app->asyncCall("sayText", name);
        } break;

        case QAccessible::NameChanged: {
            kDebug() << reasonToString(reason) << QLatin1String( "object=" ) << (obj ? QString(QLatin1String( "%1 (%2)" )).arg(obj->objectName()).arg(QLatin1String( obj->metaObject()->className() )) : QLatin1String( "NULL" )) << QLatin1String( "name=" ) << name;
            //d->m_app->asyncCall("sayText", name);
        } break;
        case QAccessible::ValueChanged: {
            QString value = interface->text(QAccessible::Value, child);
            kDebug() << reasonToString(reason) << QLatin1String( "object=" ) << (obj ? QString(QLatin1String( "%1 (%2)" )).arg(obj->objectName() ).arg(QLatin1String( obj->metaObject()->className() )) : QLatin1String( "NULL" )) << QLatin1String( "name=" ) << name << QLatin1String( "value=" ) << value;
            d->m_app->asyncCall(QLatin1String( "setValueChanged" ), qVariantFromValue(dbusIface));
        } break;
        case QAccessible::StateChanged: {
            kDebug() << reasonToString(reason) << QLatin1String( "object=" ) << (obj ? QString(QLatin1String( "%1 (%2)" ) ).arg(obj->objectName()).arg(QLatin1String( obj->metaObject()->className() )) : QLatin1String( "NULL" )) << QLatin1String( "name=" )<< name << QLatin1String( "state=" ) << stateToString(dbusIface.state);
        } break;

        case QAccessible::Focus: {
            // abort if the focus would interrupt a popupmenu
            if(!d->m_popupMenus.isEmpty()) {
                bool ok = false;
                QObject* lastPopupMenu = d->m_popupMenus.last();
                for(QObject* o = obj; o; o = o->parent())
                    if(o == lastPopupMenu) { ok = true; break; }
                if(!ok)
                    return;
            }

            // don't emit the focus changed signal if the focus didn't really changed since last time
            if(dbusIface.rect == d->m_lastFocusRect && dbusIface.name == d->m_lastFocusName)
                return;
            d->m_lastFocusRect = dbusIface.rect;
            d->m_lastFocusName = dbusIface.name;

            // here we could add hacks to special case applications/widgets :)
            //
            // QWidget *w = childInterface ? dynamic_cast<QWidget*>(childobj) : 0;
            // if(!w) w = dynamic_cast<QWidget*>(obj);
            // if(w) r = QRect(w->mapToGlobal(QPoint(w->x(), w->y())), w->size());

            kDebug() << reasonToString(reason) << QLatin1String( "object=" ) << (obj ? QString(QLatin1String( "%1 (%2)" )).arg(obj->objectName()).arg(QLatin1String( obj->metaObject()->className() )) : QLatin1String( "NULL" )) << QLatin1String( "name=" ) << name << QLatin1String( "rect=" ) << dbusIface.rect;
            d->m_app->asyncCall(QLatin1String( "setFocusChanged" ), qVariantFromValue(dbusIface));
        } break;
        default:
            kDebug() << reasonToString(reason) << QLatin1String( "object=" ) << (obj ? QString(QLatin1String( "%1 (%2)" )).arg(obj->objectName()).arg(QLatin1String( obj->metaObject()->className() )) : QLatin1String( "NULL" )) << QLatin1String( "name=" ) << name;
            break;
    }

    delete childInterface;
    //delete d->m_app; d->m_app = 0;
}

void Bridge::focusChanged(int px, int py, int rx, int ry, int rwidth, int rheight)
{
    kDebug()<<"KAccessibleBridge: focusChanged px=" << px << "py=" << py << "rx=" << rx << "ry=" << ry << "rwidth=" << rwidth << "rheight=" << rheight;
}

void Bridge::setRootObject(QAccessibleInterface *interface)
{
    d->m_root = interface;

    kDebug()<<QLatin1String( "KAccessibleBridge: setRootObject object=" ) << (interface->object() ? QString(QLatin1String( "%1 (%2)" )).arg(interface->object()->objectName()).arg(QLatin1String( interface->object()->metaObject()->className() )) : QLatin1String( "NULL" ));

    if( ! QDBusConnection::sessionBus().isConnected()) {
        kWarning()<<"KAccessibleBridge: Failed to connect to session bus";
        d->m_root = 0;
        return;
    }

    if( ! QDBusConnection::sessionBus().interface()->isServiceRegistered(QLatin1String( "org.kde.kaccessibleapp" ))) {
        kDebug()<<"KAccessibleBridge: Starting kaccessibleapp dbus service";
        QDBusConnection::sessionBus().interface()->startService(QLatin1String( "org.kde.kaccessibleapp" ));
        if( ! QDBusConnection::sessionBus().interface()->isServiceRegistered(QLatin1String( "org.kde.kaccessibleapp" ))) {
            kWarning()<<"KAccessibleBridge: Failed to start kaccessibleapp dbus service";
            d->m_root = 0;
            return;
        }

        //for testing;
        //QDBusConnection::sessionBus().connect("org.kde.kaccessibleapp", "/Adaptor", "org.kde.kaccessibleapp.Adaptor", "focusChanged", this, SLOT(focusChanged(int,int,int,int,int,int)));
    }
}

BridgePlugin::BridgePlugin(QObject *parent)
    : QAccessibleBridgePlugin(parent)
{
}

BridgePlugin::~BridgePlugin()
{
}

QAccessibleBridge* BridgePlugin::create(const QString &key)
{
    return new Bridge(this, key);
}

QStringList BridgePlugin::keys() const
{
    return QStringList() << QLatin1String( "KAccessibleBridge" );
}
