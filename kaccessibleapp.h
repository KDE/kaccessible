#ifndef KACCESSIBLEAPP_H
#define KACCESSIBLEAPP_H

#include <QDBusAbstractAdaptor>
#include <QDebug>
#include <KUniqueApplication>

class Adaptor : public QObject
{
        Q_OBJECT
        Q_CLASSINFO("D-Bus Interface", "org.kde.kaccessibleapp.Adaptor")
    public:
        explicit Adaptor(QObject* parent = 0) : QObject(parent) {}
        virtual ~Adaptor() {}
    Q_SIGNALS:
        void focusChanged(/*QObject *obj,*/ int x, int y);
    public Q_SLOTS:
        void setFocusChanged(/*QObject *obj,*/ int x, int y) { emit focusChanged(/*obj,*/ x, y); }
};

class KAccessibleApp : public KUniqueApplication
{
        Q_OBJECT
        Q_CLASSINFO("D-Bus Interface", "org.kde.kaccessibleapp")
    public:
        KAccessibleApp();
        virtual ~KAccessibleApp();
};

#endif
