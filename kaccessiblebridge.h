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
#ifndef KACCESSIBLEBRIDGE_H
#define KACCESSIBLEBRIDGE_H

#include <QDBusAbstractAdaptor>
#include <QAccessibleBridgePlugin>

class Bridge;
class BridgePlugin;

/**
 * This class implements a QAccessibleBridge that will be created
 * by the \a BridgePlugin factory.
 */
class Bridge : public QObject, public QAccessibleBridge
{
        Q_OBJECT
    public:
        Bridge(BridgePlugin *plugin, const QString& key);
        virtual ~Bridge();

        /**
         * This function is called by Qt to notify the bridge about a change in the accessibility
         * information for object wrapped by the given interface.
         *
         * \param reason specifies the cause of the change. It can take values of type QAccessible::Event.
         * \param child is the (1-based) index of the child element that has changed. When child is 0,
         * the object itself has changed.
         */
        virtual void notifyAccessibilityUpdate(int reason, QAccessibleInterface *interface, int child);

        /**
         * This function is called by Qt at application startup to set the root accessible object
         * of the application to object. All other accessible objects in the application can be
         * reached by the client using object navigation.
         */
        virtual void setRootObject(QAccessibleInterface *interface);

    private Q_SLOTS:

        /**
         * \internal slot for testing. See in the \a setRootObject method the commented out code
         * that connects the KAccessibleApp's focusChanged dbus signal to this method and prints
         * the arguments.
         *
         * \code
         * //for testing;
         * //QDBusConnection::sessionBus().connect("org.kde.kaccessibleapp", "/Adaptor", "org.kde.kaccessibleapp.Adaptor", "focusChanged", this, SLOT(focusChanged(int,int,int,int,int,int)));
         * \endcode
         */
        void focusChanged(int px, int py, int rx, int ry, int rwidth, int rheight);

    private:
        class Private;
        Private *const d;
};

/**
 * This class implements a QAccessibleBridgePlugin which will be loaded
 * by the QAccessible framework at runtime. The plugin hooks then into
 * the application, evaluates accessibility updates and calls the talks
 * the KAccessibleApp's over dbus to broadcast information around.
 */
class BridgePlugin : public QAccessibleBridgePlugin
{
    public:
        explicit BridgePlugin(QObject *parent = 0);
        virtual ~BridgePlugin();

        /**
         * Creates and returns the QAccessibleBridge object corresponding to the given key. Keys
         * are case sensitive.
         */
        virtual QAccessibleBridge* create(const QString &key);

        /**
         * Returns the list of keys this plugins supports. These keys must be the names of the
         * bridges that this plugin provides.
         */
        virtual QStringList keys() const;
};

#endif
