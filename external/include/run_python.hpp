// https://docs.python.org/3/extending/embedding.html

#define PY_SSIZE_T_CLEAN
#include <Python.h>

PyStatus status;
PyConfig config;

int init_python(int argc, char* argv[])
{
    char* libpathcharp = getenv("LD_LIBRARY_PATH");
    std::string libpath;
    if (libpathcharp != NULL)
    {
        libpath = libpathcharp;
    }

    if ((libpathcharp == NULL) or (std::string::npos == libpath.find("diff3df")))
    {
        // std::cout << "BRO" << std::endl;
        std::cout << "please run `export LD_LIBRARY_PATH=$CONDA_PREFIX/lib:$LD_LIBRARY_PATH` before the program starts" << std::endl;
        return 0;
    }

    (void)argc; // avoid unused param;
    PyConfig_InitPythonConfig(&config);

    // Recommended: set program name
    status = PyConfig_SetBytesString(&config, &config.program_name, argv[0]);
    if (PyStatus_Exception(status))
    {
        PyConfig_Clear(&config);
        Py_ExitStatusException(status);
        return 0;
    }

    const char* conda_prefix = getenv("CONDA_PREFIX");
    
    if (conda_prefix == NULL)
    {
        std::cout << "Please set up conda environment diff3df." << std::endl;
    }
    
    
    if (conda_prefix && *conda_prefix)
        PyConfig_SetBytesString(&config, &config.home, conda_prefix);
    if (PyStatus_Exception(status))
    {
        PyConfig_Clear(&config);
        Py_ExitStatusException(status);
        return 0;
    }

    status = Py_InitializeFromConfig(&config);
    if (PyStatus_Exception(status))
    {
        PyConfig_Clear(&config);
        Py_ExitStatusException(status);
        return 0;
    }

    PyConfig_Clear(&config);
    return 1;
}



int run_python(std::string path)
{
    const char* python_path = path.c_str();
    FILE* file = fopen(python_path, "r");


    if (file != nullptr) {
        PyRun_SimpleFile(file, python_path);
        fclose(file);
    }
    else
    {
        std::cout << "Failed to open file: " << python_path << std::endl;
        return 0;
    }

    return 1;
}

int finish_python()
{
    if (Py_FinalizeEx() < 0) {
        std::cout << "PyFinalizeEx failed" << std::endl;
        // exit(120);
        return 0;
    }
    return 1;
}