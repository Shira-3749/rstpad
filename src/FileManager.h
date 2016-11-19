#ifndef RSTPAD_FILEMANAGER_H
#define RSTPAD_FILEMANAGER_H

#include <QObject>
#include <QString>
#include <QFileInfo>
#include <QFileDialog>
#include <QByteArray>

namespace RstPad {

    class FileManager : public QObject
    {
        Q_OBJECT

        public:
            bool newFile();
            bool openFile();
            bool openFile(const QString &path);
            QByteArray readFile();
            bool saveFile(const QByteArray &data = QByteArray());
            bool saveFile(const QString &path, const QByteArray &data = QByteArray());
            bool saveFileAs(const QByteArray &data = QByteArray());
            bool isFileOpen();
            QFileInfo file();
            QString presentableFileName();
            bool isDirty();
            void makeDirty();
            bool isClean();
            void makeClean();
            bool ensureChangesNotLost();
            QString lastDirectory();
            void setLastDirectory(const QString &path);

        signals:
            void fileChanged();
            void fileSaveRequested(const QString &path, QByteArray &data);
            void statusChanged();

        private:
            QFileInfo m_file = QFileInfo();
            QString m_lastDirectory = QString();
            bool dirty = false;
            bool requestFileSave(const QString &path);
            void setFile(const QFileInfo &file);
            QString getOpenPath();
            QString getSavePath();
            QString showFileDialog(QFileDialog::AcceptMode acceptMode, QFileDialog::FileMode fileMode);
    };

}

#endif // RSTPAD_FILEMANAGER_H
