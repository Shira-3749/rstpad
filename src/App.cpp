#include "App.h"
#include "AppSchemeHandler.h"
#include "UI/Code/CodeEdit.h"
#include <QVariantMap>
#include <QMessageBox>
#include <QFileDialog>
#include <QWebEngineProfile>
#include <QFile>
#include <QUrl>
#include <QFont>
#include <QByteArray>
#include <QPixmap>
#include <QSplashScreen>
#include <QTextOption>
#include <QStringList>
#include <stdlib.h>

namespace RstPad {

    App::App(int &argc, char **argv) :
        QApplication(argc, argv),
        argc(argc),
        argv(argv)
    {
        setApplicationVersion(APP_VERSION);
    }

    App::~App()
    {
        delete m_fileManager;
        delete m_config;
        delete m_rstRenderer;
        delete pythonBridge;
        delete m_mainWindow;
        delete m_previewManager;
    }


    int App::exec()
    {
        APP->initialize();

        return QApplication::exec();
    }

    void App::initialize()
    {
        // splash screen
        #define SPLASH_MSG(msg) splash.showMessage(tr(msg), Qt::AlignRight | Qt::AlignBottom, Qt::white);
        QPixmap splashImage(":/splash.png");
        QSplashScreen splash(splashImage);
        splash.show();

        // init components
        SPLASH_MSG("Instantiating components")

        QStringList pythonLibs;
        pythonLibs.append(applicationDirPath().append("/libs"));
        pythonLibs.append(applicationDirPath().append("/libs/libs.zip"));

        m_fileManager = new FileManager();
        m_config = new Config(applicationDirPath() + "/config.json");
        pythonBridge = new PythonBridge(argv[0], pythonLibs);
        m_rstRenderer = new RstRenderer(pythonBridge);

        // create main window
        SPLASH_MSG("Loading UI")
        m_mainWindow = new MainWindow();
        m_previewManager = new PreviewManager(m_mainWindow->preview(), m_mainWindow->codeEditor());

        // configure web engine
        SPLASH_MSG("Configuring web engine")
        auto profile = QWebEngineProfile::defaultProfile();
        profile->setCachePath(applicationDirPath().append("/cache"));
        profile->setPersistentStoragePath(applicationDirPath().append("/cache"));
        profile->setHttpCacheType(QWebEngineProfile::MemoryHttpCache);
        profile->installUrlSchemeHandler("app", new AppSchemeHandler);

        // load docutil extensions
        SPLASH_MSG("Loading Docutils extensions")
        loadDocutilsExtensions();

        // events
        connect(this, &App::aboutToQuit, this, &App::beforeQuit);
        connect(m_fileManager, &FileManager::fileChanged, m_mainWindow, &MainWindow::fileChanged);
        connect(m_fileManager, &FileManager::fileSaveRequested, m_mainWindow, &MainWindow::fileSaveRequested);
        connect(m_fileManager, &FileManager::fileChanged, m_mainWindow, &MainWindow::updateFileUi);
        connect(m_fileManager, &FileManager::statusChanged, m_mainWindow, &MainWindow::updateFileUi);
        connect(m_config, &Config::setDefaults, this, &App::setConfigDefaults);
        connect(m_config, &Config::updated, this, &App::applyConfig);
        connect(m_rstRenderer, &RstRenderer::ready, this, &App::rstRendered);

        // load configuration
        SPLASH_MSG("Loading configuration")
        m_config->load();

        // initialize file manager
        SPLASH_MSG("Initializing file manager")
        m_fileManager->setLastDirectory(m_config->get("lastDirectory").toString());
        if (argc >= 2) {
            // try to open the passed file path
            m_fileManager->openFile(argv[1]);
        } else {
            // new file
            m_fileManager->newFile();
        }

        // finish splash and show main window
        SPLASH_MSG("Loaded!")
        splash.finish(m_mainWindow);
        m_mainWindow->show();

        #undef SPLASH_MSG
    }

    Config *App::config()
    {
        return m_config;
    }

    FileManager *App::fileManager()
    {
        return m_fileManager;
    }

    PreviewManager *App::previewManager()
    {
        return m_previewManager;
    }

    RstRenderer *App::rstRenderer()
    {
        return m_rstRenderer;
    }

    MainWindow *App::mainWindow()
    {
        return m_mainWindow;
    }

    QByteArray App::currentRstOutput()
    {
        return m_currentRstOutput;
    }

    void App::showError(const QString &message)
    {
        QMessageBox::critical(m_mainWindow, tr("Error"), message);
    }

    void App::exportToPdf()
    {
        QFileDialog dlg(m_mainWindow, tr("Export to PDF"), m_fileManager->lastDirectory(), "(*.pdf);;(*)");

        dlg.setAcceptMode(QFileDialog::AcceptSave);
        dlg.setFileMode(QFileDialog::AnyFile);
        dlg.setDefaultSuffix("pdf");

        #ifdef Q_OS_WIN
            // don't use native file dialog on windows platform since it always shows hidden files
            dlg.setOption(QFileDialog::DontUseNativeDialog);
        #endif

        if (QFileDialog::Accepted == dlg.exec()) {
            m_previewManager->printToPdf(dlg.selectedFiles().value(0));
        }
    }

    void App::loadDocutilsExtensions()
    {
        QFile handle("://preview/docutils_extensions.py");
        handle.open(QFile::ReadOnly);

        pythonBridge->eval(handle.readAll().constData());
    }

    void App::setConfigDefaults(QVariantMap &defaults)
    {
        defaults.insert("orientation", static_cast<int>(EditorOrientation::Horizontal));
        defaults.insert("autoscrollMode", static_cast<int>(AutoscrollMode::FirstLine));
        defaults.insert("autoscrollWaitDelay", 500);
        defaults.insert("rstRendererDelay", 500);
        defaults.insert("previewZoomFactor", 1.0);
        defaults.insert("wordWrapMode", static_cast<int>(QTextOption::WordWrap));
        defaults.insert("fontSize", 13);
        defaults.insert("searchWrapAround", true);
        defaults.insert("indentSize", 2);
        defaults.insert("ensureSingleEmptyLineAtEndOnSave", false);
        defaults.insert("trimTrailingWhitespaceOnSave", false);
        defaults.insert("headingSymbol1", "#");
        defaults.insert("headingOverline1", false);
        defaults.insert("headingSymbol2", "*");
        defaults.insert("headingOverline2", false);
        defaults.insert("headingSymbol3", "=");
        defaults.insert("headingOverline3", false);
        defaults.insert("headingSymbol4", "-");
        defaults.insert("headingOverline4", false);
        defaults.insert("headingSymbol5", "^");
        defaults.insert("headingOverline5", false);
        defaults.insert("headingSymbol6", "\"");
        defaults.insert("headingOverline6", false);
        defaults.insert("hrSymbol", "-");
        defaults.insert("hrWidth", 4);
        defaults.insert("lastDirectory", "");
        defaults.insert("mainWindowState", "");
        defaults.insert("mainWindowGeometry", "");
        defaults.insert("recentFiles", QStringList());
    }

    void App::applyConfig(Config &config)
    {
        // orientation
        m_mainWindow->setEditorOrientation(static_cast<EditorOrientation>(config.get("orientation").toInt()));

        // autoscroll mode and delay
        m_previewManager->setAutoscrollMode(static_cast<AutoscrollMode>(config.get("autoscrollMode").toInt()));
        m_previewManager->setAutoscrollWaitDelay(config.get("autoscrollWaitDelay").toInt());

        // rst renderer delay
        m_rstRenderer->setDelay(config.get("rstRendererDelay").toInt());
    }

    void App::rstRendered(const QByteArray &result)
    {
        m_currentRstOutput = result;
        m_previewManager->refresh();
        m_mainWindow->codeEditor()->setFocus(); // return focus to the editor
    }

    void App::beforeQuit()
    {
        // store main window state and geometry
        m_config->set("mainWindowState", m_mainWindow->saveState().toBase64());
        m_config->set("mainWindowGeometry", m_mainWindow->saveGeometry().toBase64());

        // store last directory
        m_config->set("lastDirectory", m_fileManager->lastDirectory());

        // save configuration
        m_config->save();

        // truncate useless debug.log file so it doesn't grow forever
        // https://bugreports.qt.io/browse/QTBUG-53879
        QFile debugLog(applicationDirPath().append("/debug.log"));

        if (debugLog.exists()) {
            debugLog.resize(0);
        }
    }

}
