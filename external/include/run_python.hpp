// https://docs.python.org/3/extending/embedding.html

#define PY_SSIZE_T_CLEAN
#include <Python.h>


int run_python(int argc, char* argv[], std::string path)
{
    const char* python_path = path.c_str();

    FILE* file = fopen(python_path, "r");

    PyStatus status;
    PyConfig config;
    PyConfig_InitPythonConfig(&config);

    // Recommended: set program name
    status = PyConfig_SetBytesString(&config, &config.program_name, argv[0]);
    if (PyStatus_Exception(status)) goto exception;

    status = Py_InitializeFromConfig(&config);
    if (PyStatus_Exception(status)) goto exception;

    PyConfig_Clear(&config);

    if (file != nullptr) {
        PyRun_SimpleFile(file, python_path);
        fclose(file);
    } else {
        std::cerr << "Failed to open file: " << python_path << std::endl;
    }

    if (Py_FinalizeEx() < 0) {
        exit(120);
    }
    return 1;

exception:
    PyConfig_Clear(&config);
    Py_ExitStatusException(status);
    return 0;
}