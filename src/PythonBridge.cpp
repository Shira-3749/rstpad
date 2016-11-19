#include "PythonBridge.h"
#include <QMetaType>

#define PYTHON_THREAD_BEGIN PyGILState_STATE gstate; gstate = PyGILState_Ensure();
#define PYTHON_THREAD_END PyGILState_Release(gstate);

namespace RstPad {

    PythonBridge::PythonBridge(const char *programName, const QStringList &libPaths)
    {
        Py_NoSiteFlag = 1;

        Py_SetProgramName(const_cast<char*>(programName));
        PyEval_InitThreads();
        Py_Initialize();

        globalThreadState = PyEval_SaveThread();

        initializePythonEnvironment(libPaths);
    }

    PythonBridge::~PythonBridge()
    {
        if (m_currentException) {
            delete m_currentException;
        }

        PyEval_RestoreThread(globalThreadState);
        Py_Finalize();
    }

    void PythonBridge::initializePythonEnvironment(const QStringList &libPaths)
    {
        QVariantMap args;
        args.insert("libPaths", libPaths);

        auto code = R"code(
import sys

sys.path = libPaths
sys.dont_write_bytecode = True
)code";

        eval(code, args);
    }

    QVariant PythonBridge::callModuleFunction(const char *moduleName, const char *functionName, const QVariantList &args, ExceptionHandlingMode exMode)
    {
        PYTHON_THREAD_BEGIN

        // clear exception
        clearException();

        // prepare argument list
        auto pArgs = args.isEmpty() ? PyTuple_New(0) : listToPythonTuple(args);

        // call the function
        auto result = doCallModuleFunction(moduleName, functionName, pArgs, exMode);

        // free argument list
        Py_DECREF(pArgs);

        PYTHON_THREAD_END

        return result;
    }

    void PythonBridge::eval(const char *code, const QVariantMap &locals, const char *filename, ExceptionHandlingMode exMode)
    {
        PYTHON_THREAD_BEGIN

        // clear exception
        clearException();

        // compile and eval the code
        auto pBlock = (PyCodeObject*) Py_CompileString(code, filename, Py_file_input);

        if (pBlock) {
            auto pGlobals = PyModule_GetDict(PyImport_AddModule("__main__"));
            auto pLocals = locals.isEmpty() ? PyDict_New() : mapToPythonDict(locals);

            auto pResult = PyEval_EvalCode(pBlock, pGlobals, pLocals);

            if (pResult) {
                Py_DECREF(pResult);
            } else {
                handleException(exMode);
            }

            Py_DECREF(pLocals);
            Py_DECREF(pBlock);
        } else {
            handleException(exMode);
        }

        PYTHON_THREAD_END
    }

    bool PythonBridge::hasException()
    {
        return nullptr != m_currentException;
    }

    PythonException* PythonBridge::currentException()
    {
        return m_currentException;
    }

    QVariant PythonBridge::doCallModuleFunction(const char *moduleName, const char *functionName, PyObject *pArgs, ExceptionHandlingMode exMode)
    {
        // import module
        auto pModuleName = PyString_FromString(moduleName);
        auto pModule = PyImport_Import(pModuleName);
        Py_DECREF(pModuleName);

        if (pModule) {
            // get and call function
            auto pFunc = PyObject_GetAttrString(pModule, functionName);
            auto result = doCallFunction(pFunc, pArgs, exMode);

            if (pFunc) {
                Py_DECREF(pFunc);
            }
            Py_DECREF(pModule);

            return result;
        } else {
            handleException(exMode);
        }

        return QVariant();
    }

    QVariant PythonBridge::doCallFunction(PyObject *pFunc, PyObject *pArgs, ExceptionHandlingMode exMode)
    {
        // check function
        if (pFunc && PyCallable_Check(pFunc)) {
            // call function
            auto pResult = PyObject_Call(pFunc, pArgs, (PyObject*) NULL);

            // handle result
            if (pResult) {
                auto result = pythonObjectToVariant(pResult);

                Py_DECREF(pResult);

                return result;
            } else {
                handleException(exMode);
            }
        }

        return QVariant();
    }

    PyObject* PythonBridge::variantToPythonObject(const QVariant &value)
    {
        switch ((int) value.type()) {
            // boolean
            case QMetaType::Bool:
                if (value.toBool()) {
                    Py_RETURN_TRUE;
                } else {
                    Py_RETURN_FALSE;
                }

            // whole numbers
            case QMetaType::Char:
            case QMetaType::UChar:
            case QMetaType::Short:
            case QMetaType::UShort:
            case QMetaType::Int:
            case QMetaType::UInt:
            case QMetaType::Long:
            case QMetaType::ULong:
            case QMetaType::LongLong:
            case QMetaType::ULongLong:
                return PyInt_FromSsize_t(value.toLongLong());

            // float / double
            case QMetaType::Float:
            case QMetaType::Double:
                return PyFloat_FromDouble(value.toDouble());

            // char
            case QMetaType::QChar: {
                char *str = (char *) malloc(2);

                str[0] = value.toChar().toLatin1();
                str[1] = '\0';

                return PyString_FromString((const char*) str);
            }

            // string
            case QMetaType::QString: {
                auto data = value.toString().toUtf8();

                return PyUnicode_FromStringAndSize(data.constData(), data.length());
            }

            // byte array
            case QMetaType::QByteArray: {
                auto data = value.toByteArray();

                return PyByteArray_FromStringAndSize(data.data(), data.length());
            }

            // void
            case QMetaType::Void:
            case QMetaType::VoidStar:
                Py_RETURN_NONE;

            // lists
            case QMetaType::QStringList:
                return listToPythonList(value.toStringList());
            case QMetaType::QVariantList:
                return listToPythonList(value.toList());

            // maps
            case QMetaType::QVariantMap:
                return mapToPythonDict(value.toMap());
            case QMetaType::QVariantHash:
                return mapToPythonDict(value.toHash());

            // nested variants
            case QMetaType::QVariant:
                return variantToPythonObject(value);

            // unsupported type
            default:
                Py_RETURN_NONE;
        }
    }

    template<typename T> PyObject* PythonBridge::listToPythonTuple(T &&list)
    {
        int length = list.length();
        PyObject* pTuple = PyTuple_New(length);

        for (int i = 0; i < length; ++i) {
            PyTuple_SetItem(pTuple, i, variantToPythonObject(list.value(i)));
        }

        return pTuple;
    }

    template<typename T> PyObject* PythonBridge::listToPythonList(T &&list)
    {
        int length = list.length();
        PyObject* pList = PyList_New(length);

        for (int i = 0; i < length; ++i) {
            PyList_SetItem(pList, i, variantToPythonObject(list.value(i)));
        }

        return pList;
    }

    template<typename T> PyObject* PythonBridge::mapToPythonDict(T &&map)
    {
        auto pDict = PyDict_New();

        foreach (auto key, map.keys()) {
            PyDict_SetItem(
                pDict,
                PyUnicode_FromStringAndSize(key.toUtf8(), key.length()),
                variantToPythonObject(map.value(key))
            );
        }

        return pDict;
    }

    QVariant PythonBridge::pythonObjectToVariant(PyObject *object)
    {
        if (nullptr == object) {
            return QVariant();
        }

        auto type = Py_TYPE(object);

        if (type == &PyBool_Type) {
            // bool
            return object == Py_True ? QVariant(true) : QVariant(false);
        } else if (type == &PyInt_Type) {
            // int
            return QVariant((qint64) PyInt_AsSsize_t(object));
        } else if (type == &PyLong_Type) {
            // long
            return QVariant((qlonglong) PyInt_AsLong(object));
        } else if (type == &PyFloat_Type) {
            // float
            return QVariant(PyFloat_AsDouble(object));
        } else if (type == &PyString_Type) {
            // string
            return QVariant(QString::fromLatin1(PyString_AsString(object), PyString_Size(object)));
        } else if (type == &PyUnicode_Type) {
            // unicode string
            auto pStr = PyUnicode_AsUTF8String(object);

            QString str;

            if (pStr) {
                str = QString::fromUtf8(PyString_AsString(pStr), PyString_Size(pStr));
                Py_DECREF(pStr);
            } else {
                str = QString();
            }

            return QVariant(str);
        } else if (type == &PyByteArray_Type) {
            // byte array
            return QVariant(QByteArray(PyByteArray_AsString(object), PyByteArray_Size(object)));
        } else if (type == &PyTuple_Type) {
            // tuple
            auto size = PyTuple_Size(object);
            auto list = QVariantList();

            for (int i = 0; i < size; ++i) {
                list.insert(i, pythonObjectToVariant(PyTuple_GetItem(object, i)));
            }

            return QVariant(list);
        } else if (type == &PyList_Type) {
            // list
            auto size = PyList_Size(object);
            auto list = QVariantList();

            for (int i = 0; i < size; ++i) {
                list.insert(i, pythonObjectToVariant(PyList_GetItem(object, i)));
            }

            return QVariant(list);
        } else if (type == &PyDict_Type) {
            // dictionary
            auto map = QVariantMap();
            auto pKeys = PyDict_Keys(object);
            auto size = PyList_Size(pKeys);

            for (int i = 0; i < size; ++i) {
                auto pKey = PyList_GetItem(pKeys, i);

                map.insert(
                    pythonObjectToVariant(pKey).toString(),
                    pythonObjectToVariant(PyDict_GetItem(object, pKey))
                );
            }

            Py_DECREF(pKeys);

            return QVariant(map);
        } else {
            // none or unsupported
            return QVariant();
        }
    }

    QString PythonBridge::pythonObjectToString(PyObject *object)
    {
        auto pString = PyObject_Str(object);

        if (pString) {
            auto result = pythonObjectToVariant(pString).toString();

            Py_DECREF(pString);

            return result;
        } else {
            return QString();
        }
    }

    void PythonBridge::clearException()
    {
        if (m_currentException) {
            delete m_currentException;
        }
        m_currentException = nullptr;
    }

    bool PythonBridge::handleException(ExceptionHandlingMode mode)
    {
        if (PyErr_Occurred()) {
            switch (mode) {
                case Store:
                    storeException();
                    PyErr_Clear();
                    break;
                case Print:
                    PyErr_Print();
                    break;
                case Ignore:
                    PyErr_Clear();
                    break;
            }

            return true;
        }

        return false;
    }

    void PythonBridge::storeException()
    {
        PyObject *pType, *pValue, *pTraceback;

        // fetch python exception
        PyErr_Fetch(&pType, &pValue, &pTraceback);
        PyErr_NormalizeException(&pType, &pValue, &pTraceback);

        // get type name
        QString type;
        auto pTypeName = PyObject_GetAttrString(pType, "__name__");
        if (pTypeName) {
            type = pythonObjectToString(pTypeName);
        } else {
            type = QString();
        }
        Py_DECREF(pType);

        // get message
        QString message;
        if (pValue) {
            message = pythonObjectToString(pValue);
            Py_DECREF(pValue);
        } else {
            message = QString();
        }

        // get trace
        QString trace;
        if (pTraceback) {
            auto pFormatTbArgs = PyTuple_New(1);
            PyTuple_SetItem(pFormatTbArgs, 0, pTraceback); // steals reference to pTraceback
            trace = doCallModuleFunction("traceback", "format_tb", pFormatTbArgs, Print).toStringList().join("");
            Py_DECREF(pFormatTbArgs);
        } else {
            trace = QString();
        }

        // set current exception
        m_currentException = new PythonException(type, message, trace);
    }

}
