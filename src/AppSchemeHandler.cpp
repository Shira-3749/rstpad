#include "App.h"
#include "AppSchemeHandler.h"
#include <QBuffer>
#include <QFile>
#include <QString>
#include <QImageReader>

namespace RstPad {

    AppSchemeHandler::AppSchemeHandler()
    {
    }

    void AppSchemeHandler::requestStarted(QWebEngineUrlRequestJob *request)
    {
        auto requestUrl = request->requestUrl();

        if ("local" == requestUrl.host()) {
            if (requestUrl.path() == "/current-rst-output") {
                // current RST output
                handleCurrentRstOutputRequest(request);
            } else {
                // local file
                handleLocalFileRequest(request);
            }
        } else {
            // invalid host
            request->fail(QWebEngineUrlRequestJob::UrlInvalid);
        }
    }

    void AppSchemeHandler::handleCurrentRstOutputRequest(QWebEngineUrlRequestJob *request)
    {
        auto currentRstOutput = APP->currentRstOutput();

        if (!currentRstOutput.isEmpty()) {
            auto buffer = new QBuffer();
            buffer->open(QBuffer::WriteOnly);
            buffer->write(APP->currentRstOutput());
            buffer->close();

            connect(request, &QWebEngineUrlRequestJob::destroyed, buffer, &QBuffer::deleteLater);

            request->reply("text/html", buffer);
        } else {
            request->fail(QWebEngineUrlRequestJob::RequestFailed);
        }
    }

    void AppSchemeHandler::handleLocalFileRequest(QWebEngineUrlRequestJob *request)
    {
        // resolve files relative to the currently open file's directory
        if (APP->fileManager()->isFileOpen()) {
            QString filePath;

            filePath.append(APP->fileManager()->file().dir().path());
            filePath.append(request->requestUrl().path());

            QFile handle(filePath);

            // check and open file
            if (handle.exists() && handle.open(QFile::ReadOnly)) {
                // serve only valid images
                QImageReader imageReader(&handle);

                auto imageFormat = imageReader.format();

                if (!imageFormat.isNull()) {
                    auto buffer = new QBuffer();
                    buffer->open(QBuffer::WriteOnly);
                    buffer->write(handle.readAll());
                    buffer->close();

                    connect(request, &QWebEngineUrlRequestJob::destroyed, buffer, &QBuffer::deleteLater);

                    request->reply(imageFormat, buffer);

                    // success
                    return;
                } else {
                    // not a valid image
                    request->fail(QWebEngineUrlRequestJob::RequestDenied);
                }
            }
        }

        request->fail(QWebEngineUrlRequestJob::UrlNotFound);
    }

}
