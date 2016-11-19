#include "FileManager.h"
#include "App.h"
#include <QDir>
#include <QMessageBox>

namespace RstPad {

    bool FileManager::newFile()
    {
        if (ensureChangesNotLost()) {
            m_file.setFile("");

            makeClean();
            emit fileChanged();
        }

        return false;
    }

    bool FileManager::openFile()
    {
        if (ensureChangesNotLost()) {
            auto path = getOpenPath();

            if (!path.isEmpty()) {
                return openFile(path);
            }
        }

        return false;
    }

    bool FileManager::openFile(const QString &path)
    {
        QFileInfo file(path);

        if (file.exists() && file.isReadable()) {
            setFile(file);

            return true;
        }

        return false;
    }

    QByteArray FileManager::readFile()
    {
        if (isFileOpen()) {
            QFile handle(m_file.filePath());

            if (handle.open(QFile::ReadOnly)) {
                return handle.readAll();
            }
        }

        return QByteArray();
    }

    bool FileManager::saveFile(const QByteArray &data)
    {
        if (dirty) {
            auto path = isFileOpen() ? m_file.filePath() : getSavePath();

            if (!path.isEmpty() && saveFile(path, data)) {
                makeClean();

                return true;
            }
        }

        return false;
    }

    bool FileManager::saveFile(const QString &path, const QByteArray &data)
    {
        QFileInfo file(path);

        if (!file.exists() || file.isWritable()) {
            QFile handle(path);

            if (data.isNull()) {
                // no data given, handle through signals
                return requestFileSave(path);
            }

            if (handle.open(QFile::WriteOnly) && data.length() == handle.write(data)) {
                if (m_file.filePath() != file.filePath()) {
                    handle.close();
                    setFile(file);
                } else {
                    makeClean();
                }

                return true;
            }
        }

        return false;
    }

    bool FileManager::saveFileAs(const QByteArray &data)
    {
        auto path = getSavePath();

        if (!path.isEmpty() && saveFile(path, data)) {
            makeClean();

            return true;
        }

        return false;
    }

    bool FileManager::isFileOpen()
    {
        return !m_file.filePath().isEmpty();
    }

    QFileInfo FileManager::file()
    {
        return m_file;
    }

    QString FileManager::presentableFileName()
    {
        return isFileOpen() ? m_file.fileName() : tr("New file");
    }

    bool FileManager::isDirty()
    {
        return dirty;
    }

    void FileManager::makeDirty()
    {
        if (!dirty) {
            dirty = true;
            emit statusChanged();
        }
    }

    bool FileManager::isClean()
    {
        return !dirty;
    }

    void FileManager::makeClean()
    {
        if (dirty) {
            dirty = false;
            emit statusChanged();
        }
    }

    bool FileManager::ensureChangesNotLost()
    {
        if (dirty) {
            // there are changes, prompt the user
            QMessageBox prompt(
                QMessageBox::Warning,
                tr("Save changes"),
                tr("Save changes to \"%1\"?").arg(presentableFileName()),
                QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel
            );

            prompt.exec();

            switch (prompt.result()) {
                case QMessageBox::Yes:
                    return saveFile();

                case QMessageBox::No:
                    return true;

                case QMessageBox::Cancel:
                default:
                    return false;
            }
        }

        // no changes
        return true;
    }

    QString FileManager::lastDirectory()
    {
        return !m_lastDirectory.isEmpty() ? m_lastDirectory : QDir::homePath();
    }

    void FileManager::setLastDirectory(const QString &path)
    {
        m_lastDirectory = path;
    }

    bool FileManager::requestFileSave(const QString &path)
    {
        QByteArray data;

        emit fileSaveRequested(path, data);

        if (!data.isNull()) {
            return saveFile(path, data);
        }

        return false;
    }

    void FileManager::setFile(const QFileInfo &file)
    {
        m_file = file;
        m_lastDirectory = file.path();

        makeClean();
        emit fileChanged();
    }

    QString FileManager::getOpenPath()
    {
        return showFileDialog(QFileDialog::AcceptOpen, QFileDialog::ExistingFile);
    }

    QString FileManager::getSavePath()
    {
        return showFileDialog(QFileDialog::AcceptSave, QFileDialog::AnyFile);
    }

    QString FileManager::showFileDialog(QFileDialog::AcceptMode acceptMode, QFileDialog::FileMode fileMode)
    {
        auto dlgParent = dynamic_cast<QWidget*>(parent());

        QFileDialog dlg(
            dlgParent,
            tr(acceptMode == QFileDialog::AcceptOpen ? "Open file" : "Save file"),
            lastDirectory(),
            "(*.rst);;(*)"
        );

        dlg.setAcceptMode(acceptMode);
        dlg.setFileMode(fileMode);
        dlg.setDefaultSuffix("rst");

        #ifdef Q_OS_WIN
            // don't use native file dialog on windows platform since it always shows hidden files
            dlg.setOption(QFileDialog::DontUseNativeDialog);
        #endif

        if (QFileDialog::Accepted == dlg.exec()) {
            return dlg.selectedFiles().value(0);
        } else {
            return QString();
        }
    }

}
