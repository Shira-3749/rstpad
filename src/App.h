#ifndef RSTPAD_APP_H
#define RSTPAD_APP_H

#include <QApplication>
#include <QWebEnginePage>
#include <QString>
#include <QByteArray>
#include "Config.h"
#include "PythonBridge.h"
#include "FileManager.h"
#include "PreviewManager.h"
#include "RstRenderer.h"
#include "UI/MainWindow.h"

#define APP (static_cast<App *>(QCoreApplication::instance()))

namespace RstPad {

    class PreviewManager;

    enum class EditorOrientation {
        Horizontal = 0,
        Vertical = 1,
    };

    enum class AutoscrollMode {
        Disabled = 0,
        FirstLine = 1,
        CurrentLine = 2,
    };

    class App : public QApplication
    {
        public:
            App(int &argc, char **argv);
            ~App();
            Config *config();
            FileManager *fileManager();
            PreviewManager *previewManager();
            RstRenderer *rstRenderer();
            MainWindow *mainWindow();
            QByteArray currentRstOutput();
            void showError(const QString &message);
            void exportToPdf();
            static int exec();

        private:
            int argc;
            char **argv;
            Config *m_config;
            FileManager *m_fileManager;
            PreviewManager* m_previewManager;
            RstRenderer *m_rstRenderer;
            PythonBridge *pythonBridge;
            MainWindow *m_mainWindow;
            QByteArray m_currentRstOutput = QByteArray();
            void initialize();
            void loadDocutilsExtensions();

        private slots:
            void setConfigDefaults(QVariantMap &defaults);
            void applyConfig(Config &config);
            void rstRendered(const QByteArray &result);
            void beforeQuit();
    };

}

#endif // RSTPAD_APP_H
