#include "pa22x.h"
#include "pa22xclient.h"
#include <QApplication>
#include <QTimer>
#include <QMessageBox>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <csignal>
#ifdef Q_OS_WIN
#include <windows.h>
#endif

namespace {

bool hasInternalToolsAccess()
{
    return QCoreApplication::arguments().contains(QStringLiteral("--internal-tools"))
            && qEnvironmentVariable("PA22X_INTERNAL_TOOLS") == QStringLiteral("pa22x_lab");
}

void crashLog(const QString& message)
{
    // 不使用 QCoreApplication::applicationDirPath(), 因为在 QApplication 销毁后调用会崩溃
    const QString logPath = QDir::currentPath()
        + QStringLiteral("/crash_%1.log")
              .arg(QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd")));
    QFile file(logPath);
    if (file.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream stream(&file);
        stream << QDateTime::currentDateTime().toString(Qt::ISODate)
               << " | " << message << "\n";
    }
}

#ifdef Q_OS_WIN
LONG WINAPI unhandledExceptionFilter(_EXCEPTION_POINTERS*)
{
    crashLog(QStringLiteral("SEH unhandled exception"));
    return EXCEPTION_EXECUTE_HANDLER;
}
#endif

void signalHandler(int sig)
{
    const char* name = "unknown";
    switch (sig) {
    case SIGSEGV: name = "SIGSEGV"; break;
    case SIGFPE:  name = "SIGFPE"; break;
    case SIGABRT: name = "SIGABRT"; break;
    case SIGILL:  name = "SIGILL"; break;
    default: break;
    }
    crashLog(QStringLiteral("Signal %1 (%2)").arg(sig).arg(name));
    signal(sig, SIG_DFL);
    raise(sig);
}

void installCrashHandlers()
{
#ifdef Q_OS_WIN
    SetUnhandledExceptionFilter(unhandledExceptionFilter);
#endif
    signal(SIGSEGV, signalHandler);
    signal(SIGFPE, signalHandler);
    signal(SIGABRT, signalHandler);
    signal(SIGILL, signalHandler);
}

}

#ifdef main
#undef main
#endif

int main(int argc, char *argv[])
{
    installCrashHandlers();

    //下面两种方法都可以,Qt默认采用的是 AA_UseDesktopOpenGL
    QCoreApplication::setAttribute(Qt::AA_UseOpenGLES);
    QApplication a(argc, argv);

    pa22x w;
    w.show();

    if (a.arguments().contains(QStringLiteral("--simulate-self-test")) && hasInternalToolsAccess()) {
        QTimer::singleShot(0, &w, [&w]() {
            // 通过 QMetaObject::invokeMethod 调用私有槽
            QMetaObject::invokeMethod(&w, "on_simulateDetectButton_clicked");
        });
        QTimer::singleShot(30000, &a, &QCoreApplication::quit);
    }


    return a.exec();

}




