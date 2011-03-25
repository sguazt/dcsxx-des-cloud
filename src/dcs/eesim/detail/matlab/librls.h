//
// MATLAB Compiler: 4.14 (R2010b)
// Date: Sun Feb 27 08:57:39 2011
// Arguments: "-B" "macro_default" "-W" "cpplib:librls" "-T" "link:lib" "-v"
// "rls_miso.m" "rls_miso_init.m" 
//

#ifndef __librls_h
#define __librls_h 1

#if defined(__cplusplus) && !defined(mclmcrrt_h) && defined(__linux__)
#  pragma implementation "mclmcrrt.h"
#endif
#include "mclmcrrt.h"
#include "mclcppclass.h"
#ifdef __cplusplus
extern "C" {
#endif

#if defined(__SUNPRO_CC)
/* Solaris shared libraries use __global, rather than mapfiles
 * to define the API exported from a shared library. __global is
 * only necessary when building the library -- files including
 * this header file to use the library do not need the __global
 * declaration; hence the EXPORTING_<library> logic.
 */

#ifdef EXPORTING_librls
#define PUBLIC_librls_C_API __global
#else
#define PUBLIC_librls_C_API /* No import statement needed. */
#endif

#define LIB_librls_C_API PUBLIC_librls_C_API

#elif defined(_HPUX_SOURCE)

#ifdef EXPORTING_librls
#define PUBLIC_librls_C_API __declspec(dllexport)
#else
#define PUBLIC_librls_C_API __declspec(dllimport)
#endif

#define LIB_librls_C_API PUBLIC_librls_C_API


#else

#define LIB_librls_C_API

#endif

/* This symbol is defined in shared libraries. Define it here
 * (to nothing) in case this isn't a shared library. 
 */
#ifndef LIB_librls_C_API 
#define LIB_librls_C_API /* No special import/export declaration */
#endif

extern LIB_librls_C_API 
bool MW_CALL_CONV librlsInitializeWithHandlers(
       mclOutputHandlerFcn error_handler, 
       mclOutputHandlerFcn print_handler);

extern LIB_librls_C_API 
bool MW_CALL_CONV librlsInitialize(void);

extern LIB_librls_C_API 
void MW_CALL_CONV librlsTerminate(void);



extern LIB_librls_C_API 
void MW_CALL_CONV librlsPrintStackTrace(void);

extern LIB_librls_C_API 
bool MW_CALL_CONV mlxRls_miso(int nlhs, mxArray *plhs[], int nrhs, mxArray *prhs[]);

extern LIB_librls_C_API 
bool MW_CALL_CONV mlxRls_miso_init(int nlhs, mxArray *plhs[], int nrhs, mxArray *prhs[]);

extern LIB_librls_C_API 
long MW_CALL_CONV librlsGetMcrID();


#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

/* On Windows, use __declspec to control the exported API */
#if defined(_MSC_VER) || defined(__BORLANDC__)

#ifdef EXPORTING_librls
#define PUBLIC_librls_CPP_API __declspec(dllexport)
#else
#define PUBLIC_librls_CPP_API __declspec(dllimport)
#endif

#define LIB_librls_CPP_API PUBLIC_librls_CPP_API

#else

#if !defined(LIB_librls_CPP_API)
#if defined(LIB_librls_C_API)
#define LIB_librls_CPP_API LIB_librls_C_API
#else
#define LIB_librls_CPP_API /* empty! */ 
#endif
#endif

#endif

extern LIB_librls_CPP_API void MW_CALL_CONV rls_miso(int nargout, mwArray& thm, mwArray& yhat, mwArray& p, mwArray& phi, const mwArray& z, const mwArray& nn, const mwArray& ff, const mwArray& th0, const mwArray& p0, const mwArray& phi_in1);

extern LIB_librls_CPP_API void MW_CALL_CONV rls_miso_init(int nargout, mwArray& th0, mwArray& p0, mwArray& phi0, const mwArray& nu, const mwArray& nn);

#endif
#endif
