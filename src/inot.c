#define PY_SSIZE_T_CLEAN

#include <Python.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/inotify.h>
#include <sys/stat.h>
#include <sys/types.h>
// Python class members.
#include <structmember.h>


typedef struct {
    PyObject_HEAD
    int inotifier;
    PyListObject * watches;
} inotObject;

static PyMemberDef inot_members[] = {
    {
        "inotifier", T_INT, offsetof(inotObject, inotifier), 0,
        "inotify file descriptor, as returned by the kernel"
    },
    {
        "watches", T_OBJECT_EX, offsetof(inotObject, watches), 0,
        "active watches currently attached to this inotifier object"
    },
    {NULL}
};


static PyObject * inot_add_watch(inotObject * self, PyObject * args);


static PyObject * inot_new(PyTypeObject * type, PyObject * args, PyObject * kwargs) {
    inotObject * self = (inotObject *) type->tp_alloc(type, 0);
    if (self == NULL)
    {
        PyErr_SetString(PyExc_RuntimeError, "failed to allocate a new inot object.");
        return NULL;
    }
    self->inotifier = 0;
    self->watches = (PyListObject *) PyList_New(0);
    if (self->watches == NULL)
    {
        PyErr_SetString(PyExc_RuntimeError, "failed to allocate a new list.");
        Py_DECREF(self);
        return NULL;
    }
#ifdef DEBUG
    fprintf(stderr, "New inot object created.\n", self);
#endif
    return (PyObject *) self;
}

static void inot_dealloc(inotObject * self) {
    int index;
    int watch;
    int ret;
    const Py_ssize_t sz = PyList_Size((PyObject *) self->watches);
#ifdef DEBUG
    fprintf(stderr, "%ld watches to remove before de-allocation.\n", sz);
#endif
    for (index = 0; index++; index < sz)
    {
        watch = (int) PyLong_AsLong(PyList_GetItem((PyObject *) self->watches, index));
        ret = inotify_rm_watch(self->inotifier, watch);
#ifdef DEBUG
    if (ret != 0)
    {
        fprintf(stderr, "Failed to remove watch for fd %d.\n", watch);
    }
#endif
    }
    ret = close(self->inotifier);
#ifdef DEBUG
    if (ret != 0)
    {
        fprintf(stderr, "Failed to close the inotify file descriptor.\n");
    }
#endif
    Py_XDECREF((PyObject *) self->watches);
    Py_TYPE(self)->tp_free((PyObject *) self);
#ifdef DEBUG
    fprintf(stderr, "inot object successfully de-allocated.\n");
#endif
}

static int inot_init(inotObject * self, PyObject * args, PyObject * kwargs) {
    static char * kwds[] = {"path", "mask", NULL};
    PyObject * path = NULL;
    PyObject * mask = NULL;
    self->inotifier = inotify_init();
    if (self->inotifier == -1) {
        PyErr_SetString(PyExc_OSError, "failed to create an new inotify fd.");
        return -1;
    }
#ifdef DEBUG
        fprintf(stderr, "inotify file descriptor successfully created.\n");
#endif
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|OO", kwds, &path, &mask))
    {
        PyErr_SetString(PyExc_RuntimeError, "failed to parse arguemnts in __init__.");
        return -1;
    }
    if (path != NULL)
    {
        PyObject * inot_args = Py_BuildValue("OO", path, mask);
        inot_add_watch(self, inot_args);
    }
#ifdef DEBUG
    fprintf(stderr, "End of function %s.\n", __FUNCTION__);
#endif
    return 0;
}

static PyObject * inot_add_watch(inotObject * self, PyObject * args) {
    struct stat stat_buffer;
    const char * path;
    unsigned int mask;
    int watch;
    PyObject * py_watch;
    if (!PyArg_ParseTuple(args, "sI", &path, &mask))
    {
        PyErr_SetString(PyExc_RuntimeError, "failed to parse arguemnts in add_watch.");
        return NULL;
    }
    if (stat((const char *) path, &stat_buffer) != 0) {
        PyErr_Format(PyExc_FileNotFoundError, "path %s not found.", path);
        return NULL;
    }
    watch = inotify_add_watch(self->inotifier, path, mask);
    if (watch <= 0)
    {
        PyErr_SetString(PyExc_OSError, "could not add path to the inotify instance.");
        return NULL;
    }
    py_watch = PyLong_FromLong(watch);
    PyList_Append((PyObject *) self->watches, py_watch);
#ifdef DEBUG
    fprintf(stderr, "Successfully added %s to the watched paths.\n", path);
#endif
    return py_watch;
}

static PyObject * inot_rm_watch(inotObject * self, PyObject * py_watch) {
    bool found = false;
    int index;
    int watch;
    Py_ssize_t sz = PyList_Size((PyObject *) self->watches);
    PyListObject * new_watches = (PyListObject *) PyList_New(sz - 1);
    PyObject * tmp;
    if (!PyArg_ParseTuple(py_watch, "i", &watch))
    {
        PyErr_SetString(PyExc_RuntimeError, "failed to parse arguemnts in rm_watch.");
        return NULL;
    }
#ifdef DEBUG
    fprintf(stderr, "Looking for fd %d.\n", watch);
#endif
    for (index = 0; index < sz; index++)
    {
        if ((int) PyLong_AsLong(PyList_GetItem((PyObject *) self->watches, index)) == watch)
        {
            found = true;
            break;
        }
    }
    if (!found)
    {
        PyErr_SetString(PyExc_FileNotFoundError, "fd given is not watched.");
        return NULL;
    }
    if (inotify_rm_watch(self->inotifier, watch) != 0)
    {
        PyErr_Format(PyExc_OSError, "could not remove fd %d from the watched paths.", watch);
        return NULL;
    }
    if (index > 0)
    { 
        PyList_SetSlice(
            (PyObject *) new_watches,
            0,
            index - 1,
            PyList_GetSlice((PyObject *) self->watches, 0, index - 1)
        );
    }
    if (index < (sz - 1))
    {
        PyList_SetSlice(
            (PyObject *) new_watches,
            index,
            sz - 2,
            PyList_GetSlice((PyObject *) self->watches, index + 1, sz - 1)
        );
    }
    tmp = (PyObject *) self->watches;
    self->watches = new_watches;
    Py_XDECREF(tmp);
#ifdef DEBUG
    fprintf(stderr, "Successfully removed fd %d from the watched paths.\n", watch);
#endif
    Py_RETURN_NONE;
}

static PyObject * inot_fd(inotObject * self, PyObject * Py_UNUSED(ignored)) {
    return PyLong_FromLong((long) self->inotifier);
}

static PyMethodDef inot_methods[] = {
    {
        "add_watch", (PyCFunction) inot_add_watch, METH_VARARGS,
        "Adds a new path to the inotify instance"
    },
    {
        "rm_watch", (PyCFunction) inot_rm_watch, METH_VARARGS,
        "Removes a watch instance from the inotifier instance"
    },
    {
        "fd", (PyCFunction) inot_fd, METH_NOARGS,
        "Returns the associated inotify object file descriptor (an int)"
    },
    {NULL}
};

static PyTypeObject inotType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "cinot.inot",
    .tp_doc = "Linux inotify Python interface object",
    .tp_basicsize = sizeof(inotObject),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_new = inot_new,
    .tp_init = (initproc) inot_init,
    .tp_dealloc = (destructor) inot_dealloc,
    .tp_members = inot_members,
    .tp_methods = inot_methods

};

static PyModuleDef cinot = {
    PyModuleDef_HEAD_INIT,
    .m_name = "cinot",
    .m_doc = "Linux inotify Python interface module",
    .m_size = -1
};

PyMODINIT_FUNC PyInit__cinot(void) {
    PyObject * module;
    if (PyType_Ready(&inotType) < 0)
    {
        return NULL;
    }
    module = PyModule_Create(&cinot);
    if (module == NULL)
    {
        return NULL;
    }

    Py_INCREF(&inotType);
    PyModule_AddObject(module, "inot", (PyObject *) &inotType);
    return module;
}
