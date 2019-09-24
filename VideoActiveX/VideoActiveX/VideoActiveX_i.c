

/* this ALWAYS GENERATED file contains the IIDs and CLSIDs */

/* link this file in with the server and any clients */


 /* File created by MIDL compiler version 8.01.0622 */
/* at Tue Jan 19 11:14:07 2038
 */
/* Compiler settings for VideoActiveX.idl:
    Oicf, W1, Zp8, env=Win64 (32b run), target_arch=AMD64 8.01.0622 
    protocol : all , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
/* @@MIDL_FILE_HEADING(  ) */



#ifdef __cplusplus
extern "C"{
#endif 


#include <rpc.h>
#include <rpcndr.h>

#ifdef _MIDL_USE_GUIDDEF_

#ifndef INITGUID
#define INITGUID
#include <guiddef.h>
#undef INITGUID
#else
#include <guiddef.h>
#endif

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        DEFINE_GUID(name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8)

#else // !_MIDL_USE_GUIDDEF_

#ifndef __IID_DEFINED__
#define __IID_DEFINED__

typedef struct _IID
{
    unsigned long x;
    unsigned short s1;
    unsigned short s2;
    unsigned char  c[8];
} IID;

#endif // __IID_DEFINED__

#ifndef CLSID_DEFINED
#define CLSID_DEFINED
typedef IID CLSID;
#endif // CLSID_DEFINED

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        EXTERN_C __declspec(selectany) const type name = {l,w1,w2,{b1,b2,b3,b4,b5,b6,b7,b8}}

#endif // !_MIDL_USE_GUIDDEF_

MIDL_DEFINE_GUID(IID, LIBID_VideoActiveXLib,0x150b4ea9,0x2752,0x4c00,0x8c,0xdd,0x96,0xc1,0x89,0x46,0xd2,0xa8);


MIDL_DEFINE_GUID(IID, DIID__DVideoActiveX,0x7f39cbcf,0xb1c8,0x48c3,0xa6,0xa5,0xf6,0x81,0x3b,0xe8,0x88,0x98);


MIDL_DEFINE_GUID(IID, DIID__DVideoActiveXEvents,0xa6295121,0x0a3a,0x485a,0xa4,0xf5,0x25,0xb4,0x4f,0xe5,0xee,0xf2);


MIDL_DEFINE_GUID(CLSID, CLSID_VideoActiveX,0x84ede05c,0xc758,0x4ef4,0xa2,0x77,0x0e,0xe9,0x94,0xfb,0xba,0x21);

#undef MIDL_DEFINE_GUID

#ifdef __cplusplus
}
#endif



