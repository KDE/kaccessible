#include "kaccessibleapp.h"

#include <QDebug>
#include <QDBusConnection>
#include <QTimer>
#include <klocale.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>

KAccessibleApp::KAccessibleApp()
    : KUniqueApplication()
{
    if( ! QDBusConnection::sessionBus().registerObject("/Adaptor", new Adaptor(this), QDBusConnection::ExportAllContents)) {
        qDebug() << "Unable to register KAccessibleApp to dbus";
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
