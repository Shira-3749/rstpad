#include "PythonException.h"

namespace RstPad {

    PythonException::PythonException()
    {
    }

    PythonException::PythonException(const QString &type, const QString &message, const QString &trace) :
        m_type(type),
        m_message(message),
        m_trace(trace),
        m_isNull(false)
    {
    }

    bool PythonException::isNull() const
    {
        return m_isNull;
    }

    QString PythonException::type() const
    {
        return m_type;
    }

    QString PythonException::message() const
    {
        return m_message;
    }

    QString PythonException::trace() const
    {
        return m_trace;
    }

}
