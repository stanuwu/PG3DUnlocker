#pragma once

namespace il2cpp
{
    struct image
    {
        const char* m_pName;
        const char* m_pNameNoExt;
    };

    struct assemblyName
    {
        const char* m_pName;
        const char* m_pCulture;
        const char* m_pHash;
        const char* m_pPublicKey;
        unsigned int m_uHash;
        int m_iHashLength;
        unsigned int m_uFlags;
        int m_iMajor;
        int m_iMinor;
        int m_iBuild;
        int m_bRevision;
        unsigned char m_uPublicKeyToken[8];
    };

    struct assembly
    {
        image* m_pImage;
        unsigned int m_uToken;
        int m_ReferencedAssemblyStart;
        int m_ReferencedAssemblyCount;
        assemblyName m_aName;
    };

    struct clazz
    {
        void* m_pImage;
        void* m_pGC;
        const char* m_pName;
        const char* m_pNamespace;
        void* m_pValue;
        void* m_pArgs;
        clazz* m_pElementClass;
        clazz* m_pCastClass;
        clazz* m_pDeclareClass;
        clazz* m_pParentClass;
        void* m_pGenericClass;
        void* m_pTypeDefinition;
        void* m_pInteropData;
        void* m_pFields;
        void* m_pEvents;
        void* m_pProperties;
        void** m_pMethods;
        clazz** m_pNestedTypes;
        clazz** m_ImplementedInterfaces;
        void* m_pInterfaceOffsets;
        void* m_pStaticFields;
        void* m_pRGCTX;
    };

    struct object
    {
        clazz* type;
        void* monitor;
    };
}
