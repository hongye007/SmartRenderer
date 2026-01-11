#ifndef __khrplatform_h_
#define __khrplatform_h_

/*
** Khronos platform-specific types and definitions.
*/

#if defined(_WIN32) && !defined(__SCITECH_SNAP__)
#   define KHRONOS_APIENTRY __stdcall
#else
#   define KHRONOS_APIENTRY
#endif

#ifndef KHRONOS_APICALL
#   if defined(_WIN32) && !defined(__SCITECH_SNAP__)
#       define KHRONOS_APICALL __declspec(dllimport)
#   else
#       define KHRONOS_APICALL
#   endif
#endif

typedef signed   char          khronos_int8_t;
typedef unsigned char          khronos_uint8_t;
typedef signed   short int     khronos_int16_t;
typedef unsigned short int     khronos_uint16_t;
typedef signed   int           khronos_int32_t;
typedef unsigned int           khronos_uint32_t;
typedef signed   long long     khronos_int64_t;
typedef unsigned long long     khronos_uint64_t;

typedef float                  khronos_float_t;
typedef khronos_uint64_t       khronos_utime_nanoseconds_t;
typedef khronos_int64_t        khronos_stime_nanoseconds_t;

#ifdef _WIN64
typedef signed   long long int khronos_intptr_t;
typedef unsigned long long int khronos_uintptr_t;
typedef signed   long long int khronos_ssize_t;
typedef unsigned long long int khronos_usize_t;
#else
typedef signed   long  int     khronos_intptr_t;
typedef unsigned long  int     khronos_uintptr_t;
typedef signed   long  int     khronos_ssize_t;
typedef unsigned long  int     khronos_usize_t;
#endif

#define KHRONOS_SUPPORT_INT64   1
#define KHRONOS_SUPPORT_FLOAT   1

#ifndef KHRONOS_MAX_ENUM
#define KHRONOS_MAX_ENUM 0x7FFFFFFF
#endif

#endif /* __khrplatform_h_ */
