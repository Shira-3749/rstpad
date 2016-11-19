#ifndef RSTPAD_PYTHONEXCEPTION_H
#define RSTPAD_PYTHONEXCEPTION_H

#include <QObject>
#include <QString>

namespace RstPad {

    class PythonException : public QObject
    {
        Q_OBJECT

        public:
            explicit PythonException();
            explicit PythonException(const QString &type, const QString &message, const QString &trace);
            bool isNull() const;
            QString type() const;
            QString message() const;
            QString trace() const;

        private:
            QString m_type;
            QString m_message;
            QString m_trace;
            bool m_isNull = true;
    };

}

#endif // RSTPAD_PYTHONEXCEPTION_H
