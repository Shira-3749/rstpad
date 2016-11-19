#ifndef CONFIG_H
#define CONFIG_H

#include <QObject>
#include <QString>
#include <QVariant>
#include <QVariantMap>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QFileInfo>
#include <QVariantMap>
#include <QByteArray>

namespace RstPad {

    class Config : public QObject
    {
        Q_OBJECT

        public:
            Config(const QString filePath);
            bool has(const QString &key);
            QVariant get(const QString &key);
            QVariantMap all();
            void set(const QString &key, const QVariant &value);
            void set(const QVariantMap &values);
            QVariantMap defaultValues();
            void load();
            void restoreDefaults();
            bool save();

        signals:
            void updated(Config &self);
            void setDefaults(QVariantMap &defaults);

        private:
            QFileInfo file;
            QVariantMap values;
            QVariantMap defaults;
            bool initialized = false;
            bool initializing = false;
            bool modified = false;
            void ensureInitialized(bool loadFile = true);
            void loadDefaults();
            bool loadFromFile();
            QByteArray serialize(const QVariantMap &values);
            QVariantMap unserialize(const QByteArray &data);
    };

}

#endif // CONFIG_H
