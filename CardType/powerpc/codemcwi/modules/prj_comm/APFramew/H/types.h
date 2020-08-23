#if 0

#ifndef _INC_TYPES
#define _INC_TYPES

#ifndef __NUCLEUS__

#ifdef UINT32
#undef UINT32
#endif
typedef unsigned int UINT32;


#ifdef UINT16
#undef UINT16
#endif
typedef unsigned short UINT16;

#ifdef UINT8
#undef UINT8
#endif
typedef unsigned char UINT8;

#else

typedef unsigned int size_t;

#endif

#ifdef SINT32
#undef SINT32
#endif
typedef int SINT32;

#ifdef SINT16
#undef SINT16
#endif
#ifdef COMIP
typedef signed short SINT16;
#else
typedef short SINT16;
#endif

#ifdef SINT8
#undef SINT8
#endif
typedef char SINT8;


#ifdef NULL
#undef NULL
#endif
#define NULL 0

#endif //_INC_TYPES
#endif //0
