// NOTE(hugo): function hotloading in DLL
#if 0
    typedef RETURN_TYPE FUNCTION_NAME(FUNCTION_PARAMETERS);
    static FUNCTION_NAME* FUNCTION_NAME_DYNAMIC = FUNCTION_NAME_DUMMY;
    #define FUNCTION_NAME FUNCTION_NAME_DYNAMIC

    HMODULE ... = LoadLibrary(".dll");
    FUNCTION_NAME_DYNAMIC = (FUNCTION_NAME*)GetProcAdress(..., "FUNCTION_NAME");
    FreeLibrary(...);

    linux -> dlopen, dlclose, dlsym
#endif

