#ifndef PYTHON_BRIDGE
#define PYTHON_BRIDGE

#include <QVariant>
#include <QVariantList>
#include <QVariantMap>
#include <QStringList>
#include "PythonException.h"

#ifdef _DEBUG
  #undef _DEBUG
  #include <Python.h>
  #define _DEBUG
#else
  #include <Python.h>
#endif

namespace RstPad {

    class PythonBridge
    {
        public:
            enum ExceptionHandlingMode {Store, Print, Ignore};
            PythonBridge(const char *programName, const QStringList &libPaths);
            ~PythonBridge();
            QVariant callModuleFunction(const char *moduleName, const char *functionName, const QVariantList &args = QVariantList(), ExceptionHandlingMode exMode = Store);
            void eval(const char *code, const QVariantMap &locals = QVariantMap(), const char* filename = "eval", ExceptionHandlingMode exMode = Store);
            bool hasException();
            PythonException* currentException();

        private:
            PythonException *m_currentException = nullptr;
            PyThreadState *globalThreadState = nullptr;
            void initializePythonEnvironment(const QStringList &libPaths);
            QVariant doCallModuleFunction(const char *moduleName, const char *functionName, PyObject *pArgs, ExceptionHandlingMode exMode);
            QVariant doCallFunction(PyObject *pFunc, PyObject *pArgs, ExceptionHandlingMode exMode);
            PyObject* variantToPythonObject(const QVariant &value);
            template<typename T> PyObject* listToPythonList(T &&list);
            template<typename T> PyObject* listToPythonTuple(T &&list);
            template<typename T> PyObject* mapToPythonDict(T &&map);
            QVariant pythonObjectToVariant(PyObject *object);
            QString pythonObjectToString(PyObject *object);
            void clearException();
            bool handleException(ExceptionHandlingMode mode);
            void storeException();
    };

}

#endif // PYTHON_BRIDGE
