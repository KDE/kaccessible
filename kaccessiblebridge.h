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
        void focusChanged(int x, int y);
    private:
        BridgePlugin *m_plugin;
        const QString m_key;
        QAccessibleInterface *m_root;
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
