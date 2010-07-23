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

class Bridge : public QObject, public QAccessibleBridge
{
        Q_OBJECT
    public:
        Bridge(BridgePlugin *plugin, const QString& key);
        virtual ~Bridge();
        virtual void notifyAccessibilityUpdate(int reason, QAccessibleInterface *interface, int child);
        virtual void setRootObject(QAccessibleInterface *interface);
    private Q_SLOTS:
        void focusChanged(int x, int y, int width, int height);
    private:
        BridgePlugin *m_plugin;
        const QString m_key;
        QAccessibleInterface *m_root;
        QObject *m_currentPopupMenu;
        QList<QObject*> m_shownObjects;
};

class BridgePlugin : public QAccessibleBridgePlugin
{
    public:
        explicit BridgePlugin(QObject *parent = 0);
        virtual ~BridgePlugin();
        virtual QAccessibleBridge* create(const QString &key);
        virtual QStringList keys() const;
};

#endif
