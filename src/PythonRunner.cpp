#include "PythonBridge.h"
#include <QMetaType>
#include <QDebug>

#define PrintPythonError if (nullptr != PyErr_Occurred()) PyErr_Print()

namespace RstPad {

    PythonBridge::PythonBridge(char *programName, char* libPath)
    {
        Py_NoSiteFlag = 1;

        Py_SetProgramName(programName);
        Py_Initialize();

        // override sys.path
        QVariantMap setSysPathArgs;
        setSysPathArgs.insert("libPath", libPath);

        eval("import sys\nsys.path = [libPath]\n", &setSysPathArgs);
    }

    PythonBridge::~PythonBridge()
    {
        Py_Finalize();
    }

    QVariant PythonBridge::callModuleFunction(const char *moduleName, const char *functionName, QVariantList *args)
    {
        // import module
        auto pModuleName = PyString_FromString(moduleName);
        auto module = PyImport_Import(pModuleName);
        Py_DECREF(pModuleName);

        if (nullptr != module) {
            // get function
            auto pFunc = PyObject_GetAttrString(module, functionName);

            // check function
            if (nullptr != pFunc && PyCallable_Check(pFunc)) {
                // call function
                auto pArgs = nullptr != args ? listToPythonTuple(*args) : PyTuple_New(0);

                auto pResult = PyObject_Call(pFunc, pArgs, nullptr);

                Py_DECREF(pArgs);

                // handle result
                if (nullptr != pResult) {
                    auto result = pythonObjectToVariant(pResult);

                    Py_DECREF(pResult);

                    return result;
                } else {
                    PrintPythonError;
                }
            }
        } else {
            PrintPythonError;
        }

        return QVariant();
    }

    void PythonBridge::eval(const char *code, QVariantMap *locals, const char *filename)
    {
        auto pBlock = (PyCodeObject*) Py_CompileString(code, filename, Py_file_input);

        if (nullptr != pBlock) {
            auto pGlobals = PyModule_GetDict(PyImport_AddModule("__main__"));
            auto pLocals = nullptr == locals ? PyDict_New() : mapToPythonDict(*locals);

            auto pResult = PyEval_EvalCode(pBlock, pGlobals, pLocals);

            if (nullptr != pResult) {
                Py_DECREF(pResult);
            } else {
                PrintPythonError;
            }

            Py_DECREF(pLocals);
            Py_DECREF(pBlock);
        } else {
            PrintPythonError;
        }
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
                auto str = value.toString();

                return PyUnicode_FromStringAndSize(str.toUtf8(), str.length());
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
                return listToPythonTuple(value.toStringList());
            case QMetaType::QVariantList:
                return listToPythonTuple(value.toList());

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
            return QVariant(PyInt_AsSsize_t(object));
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
            auto str = QString::fromUtf8(PyString_AsString(pStr), PyString_Size(pStr));

            Py_DECREF(pStr);

            return QVariant(str);
        } else if (type == &PyByteArray_Type) {
            // byte array
            return QVariant(QByteArray(PyByteArray_AsString(object), PyByteArray_Size(object)));
        } else if (type == &PyTuple_Type) {
            // tuple
            auto size = PyTuple_Size(object);
            auto list = new QVariantList();

            for (int i = 0; i < size; ++i) {
                list->insert(i, pythonObjectToVariant(PyTuple_GetItem(object, i)));
            }

            return QVariant(*list);
        } else if (type == &PyList_Type) {
            // list
            auto size = PyList_Size(object);
            auto list = new QVariantList();

            for (int i = 0; i < size; ++i) {
                list->insert(i, pythonObjectToVariant(PyList_GetItem(object, i)));
            }

            return QVariant();
        } else if (type == &PyDict_Type) {
            // dictionary
            auto map = new QVariantMap();
            auto pKeys = PyDict_Keys(object);
            auto size = PyList_Size(pKeys);

            for (int i = 0; i < size; ++i) {
                auto pKey = PyList_GetItem(pKeys, i);

                map->insert(
                    pythonObjectToVariant(pKey).toString(),
                    pythonObjectToVariant(PyDict_GetItem(object, pKey))
                );
            }

            Py_DECREF(pKeys);

            return QVariant(*map);
        } else {
            // none or unsupported
            return QVariant();
        }
    }

}
