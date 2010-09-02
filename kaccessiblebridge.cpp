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

#include <QAccessibleInterface>
#include <QWidget>
#include <QFile>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusInterface>
#include <kdebug.h>

Q_EXPORT_PLUGIN(BridgePlugin)

class Bridge::Private
{
    public:
        BridgePlugin *m_plugin;
        const QString m_key;
        QAccessibleInterface *m_root;
        QObject *m_currentPopupMenu;
        
        Private(BridgePlugin *plugin, const QString& key)
            : m_plugin(plugin)
            , m_key(key)
            , m_root(0)
            , m_currentPopupMenu(0)
        {
        }
};

Bridge::Bridge(BridgePlugin *plugin, const QString& key)
    : QObject(plugin)
    , QAccessibleBridge()
    , d(new Private(plugin, key))
{
}

Bridge::~Bridge()
{
    delete d;
}

QString reasonToString(int reason)
{
    switch(reason) {
        case QAccessible::Focus: return "QAccessible::Focus";
        case QAccessible::MenuCommand: return "QAccessible::MenuCommand";
        case QAccessible::MenuStart: return "QAccessible::MenuStart";
        case QAccessible::MenuEnd: return "QAccessible::MenuEnd";
        case QAccessible::PopupMenuEnd: return "QAccessible::PopupMenuEnd";
        case QAccessible::PopupMenuStart: return "QAccessible::PopupMenuStart";
        case QAccessible::ScrollingEnd: return "QAccessible::ScrollingEnd";
        case QAccessible::ScrollingStart: return "QAccessible::ScrollingStart";
        case QAccessible::Selection: return "QAccessible::Selection";
        case QAccessible::StateChanged: return "QAccessible::StateChanged";
        case QAccessible::ValueChanged: return "QAccessible::ValueChanged";
        case QAccessible::NameChanged: return "QAccessible::NameChanged";
        case QAccessible::ObjectCreated: return "QAccessible::ObjectCreated";
        case QAccessible::ObjectDestroyed: return "QAccessible::ObjectDestroyed";
        //case QAccessible::ObjectHide: return "QAccessible::ObjectHide";
        case QAccessible::ObjectReorder: return "QAccessible::ObjectReorder";
        //case QAccessible::ObjectShow: return "QAccessible::ObjectShow";
        case QAccessible::ParentChanged: return "QAccessible::ParentChanged";
        case QAccessible::Alert: return "QAccessible::Alert";
        case QAccessible::DefaultActionChanged: return "QAccessible::DefaultActionChanged";
        case QAccessible::DialogEnd: return "QAccessible::DialogEnd";
        case QAccessible::DialogStart: return "QAccessible::DialogStart";
        case QAccessible::DragDropEnd: return "QAccessible::DragDropEnd";
        case QAccessible::DragDropStart: return "QAccessible::DragDropStart";
        case QAccessible::ForegroundChanged: return "QAccessible::ForegroundChanged";
        case QAccessible::LocationChanged: return "QAccessible::LocationChanged";
        case QAccessible::SelectionAdd: return "QAccessible::SelectionAdd";
        case QAccessible::SelectionRemove: return "QAccessible::SelectionRemove";
        case QAccessible::SelectionWithin: return "QAccessible::SelectionWithin";
        default: break;
    }
    return QString::number(reason);
}

void Bridge::notifyAccessibilityUpdate(int reason, QAccessibleInterface *interface, int child)
{
    if(!d->m_root) {
        return;
    }

    if(reason == QAccessible::ObjectShow || reason == QAccessible::ObjectHide) {
        return;
    }

    QAccessibleInterface *childInterface = 0;
    if(child > 0) {
        interface->navigate(QAccessible::Child, child, &childInterface);
    }

    kDebug() << "reason=" << reasonToString(reason) << "child=" << child << "childrect=" << interface->rect(child) << "state=" << interface->state(child) << "object=" << (interface->object() ? QString("%1 (%2)").arg(interface->object()->objectName()).arg(interface->object()->metaObject()->className()) : "NULL") << "childInterface=" << (childInterface && childInterface->object() ? QString("%1 (%2)").arg(childInterface->object()->objectName()).arg(childInterface->object()->metaObject()->className()) : "NULL");

    switch(reason) {
        case QAccessible::PopupMenuStart:
            d->m_currentPopupMenu = interface->object();
            break;
        case QAccessible::PopupMenuEnd:
            d->m_currentPopupMenu = 0;
            break;
        case QAccessible::Focus: {
            // abort if the focus would interrupt a popupmenu
            if(d->m_currentPopupMenu && !childInterface) {
                return;
            }

            // the rectangle that has focus
            QRect r = interface->rect(child);

            // an optional exact point that has the focus
            QPoint p(-1,-1);            
            if(r.width() == 0 && r.height() == 0 && childInterface) {
                p = r.topLeft();
                r = childInterface->rect(0);
            }

            // here we could add hacks to special case applications/widgets :)
            //
            // QWidget *w = childInterface ? dynamic_cast<QWidget*>(childInterface->object()) : 0;
            // if(!w) w = dynamic_cast<QWidget*>(interface->object());
            // if(w) r = QRect(w->mapToGlobal(QPoint(w->x(), w->y())), w->size());

            QDBusInterface iface("org.kde.kaccessibleapp","/Adaptor");
            iface.asyncCall("setFocusChanged", p.x(), p.y(), r.x(), r.y(), r.width(), r.height());
        } break;
        default:
            break;
    }

    delete childInterface;
}

void Bridge::focusChanged(int px, int py, int rx, int ry, int rwidth, int rheight)
{
    kDebug()<<"KAccessibleBridge: focusChanged px=" << px << "py=" << py << "rx=" << rx << "ry=" << ry << "rwidth=" << rwidth << "rheight=" << rheight;
}

void Bridge::setRootObject(QAccessibleInterface *interface)
{
    d->m_root = interface;

    kDebug()<<"KAccessibleBridge: setRootObject object=" << (interface->object() ? QString("%1 (%2)").arg(interface->object()->objectName()).arg(interface->object()->metaObject()->className()) : "NULL");

    if( ! QDBusConnection::sessionBus().isConnected()) {
        kWarning()<<"KAccessibleBridge: Failed to connect to session bus";
        d->m_root = 0;
        return;
    }

    if( ! QDBusConnection::sessionBus().interface()->isServiceRegistered("org.kde.kaccessibleapp")) {
        kDebug()<<"KAccessibleBridge: Starting kaccessibleapp dbus service";
        QDBusConnection::sessionBus().interface()->startService("org.kde.kaccessibleapp");
        if( ! QDBusConnection::sessionBus().interface()->isServiceRegistered("org.kde.kaccessibleapp")) {
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
    return QStringList() << "KAccessibleBridge";
}
