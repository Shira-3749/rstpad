#include "Config.h"
#include <stdexcept>

namespace RstPad {

    Config::Config(const QString filePath) : file(filePath)
    {
    }

    bool Config::has(const QString &key)
    {
        return values.contains(key);
    }

    QVariant Config::get(const QString &key)
    {
        ensureInitialized();

        return values.value(key);
    }

    QVariantMap Config::all()
    {
        ensureInitialized();

        return values;
    }

    void Config::set(const QString &key, const QVariant &value)
    {
        ensureInitialized();

        values.insert(key, value);
        modified = true;
        emit updated(*this);
    }

    void Config::set(const QVariantMap &valuesToInsert)
    {
        ensureInitialized();

        if (values.count() > 0) {
            foreach (const QString &key, valuesToInsert.keys()) {
                values.insert(key, valuesToInsert.value(key));
            }
            modified = true;
            emit updated(*this);
        }
    }

    QVariantMap Config::defaultValues()
    {
        ensureInitialized();

        return defaults;
    }

    void Config::load()
    {
        ensureInitialized();
    }

    void Config::restoreDefaults()
    {
        if (!initialized) {
            ensureInitialized(false);
        } else {
            loadDefaults();
            emit updated(*this);
        }
    }

    bool Config::save()
    {
        if (initialized && modified) {
            QFile handle(file.filePath());

            auto data = serialize(values);

            if (handle.open(QIODevice::WriteOnly) && data.length() == handle.write(data)) {
                modified = false;

                return true;
            }
        }

        return false;
    }

    void Config::ensureInitialized(bool loadFile)
    {
        if (!initialized) {
            if (initializing) {
                throw std::logic_error("Cannot access configuration during the initialization process");
            }

            initializing = true;

            loadDefaults();
            if (loadFile) {
                loadFromFile();
            }

            initializing = false;
            initialized = true;

            emit updated(*this);
        }
    }

    void Config::loadDefaults()
    {
        defaults = QVariantMap();
        emit setDefaults(defaults);
        values = defaults;
    }

    bool Config::loadFromFile()
    {
        if (file.exists()) {
            QFile handle(file.filePath());

            if (handle.open(QIODevice::ReadOnly)) {
                auto fileObject = unserialize(handle.readAll());

                // override known defaults with values from the file
                foreach (const QString &fileKey, fileObject.keys()) {
                    if (values.contains(fileKey)) {
                        values.insert(fileKey, fileObject.value(fileKey));
                    }
                }

                return true;
            }
        }

        return false;
    }

    QByteArray Config::serialize(const QVariantMap &values)
    {
        QJsonDocument document(QJsonObject::fromVariantMap(values));

        return document.toJson();
    }

    QVariantMap Config::unserialize(const QByteArray &data)
    {
        auto document = QJsonDocument::fromJson(data);

        return document.object().toVariantMap();
    }
}
