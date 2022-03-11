/**
 * This code is based on:
 * - The DragImg sample in the Windows SDK for Windows Server 2008 and .NET Framework 3.5
 * - https://msdn.microsoft.com/en-gb/library/ms997502.aspx [http://archive.is/uvdvf]
 *
 * The licence terms for the relevant Windows SDK are contained in windows-sdk-6.1-licence.html.
 */

/**************************************************************************
   THIS CODE AND INFORMATION IS PROVIDED 'AS IS' WITHOUT WARRANTY OF
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
   PARTICULAR PURPOSE.

   Copyright 1999 - 2000 Microsoft Corporation.  All Rights Reserved.
**************************************************************************/

#pragma once

#include <windows.h>
#include <ole2.h>

#define MAX_NUM_FORMAT 5

class CDataObject : public IDataObject {
public:
    CDataObject();
    ~CDataObject();

    // IUnknown members that delegate to m_pUnkOuter.
    STDMETHOD(QueryInterface)(REFIID, LPVOID*);
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();

    /* IDataObject methods */
    STDMETHOD(GetData)(LPFORMATETC, LPSTGMEDIUM);
    STDMETHOD(GetDataHere)(LPFORMATETC, LPSTGMEDIUM);
    STDMETHOD(QueryGetData)(LPFORMATETC);
    STDMETHOD(GetCanonicalFormatEtc)(LPFORMATETC, LPFORMATETC);
    STDMETHOD(SetData)(LPFORMATETC, LPSTGMEDIUM, BOOL);
    STDMETHOD(EnumFormatEtc)(DWORD, LPENUMFORMATETC*);
    STDMETHOD(DAdvise)(FORMATETC*, DWORD, LPADVISESINK, LPDWORD);
    STDMETHOD(DUnadvise)(DWORD);
    STDMETHOD(EnumDAdvise)(LPENUMSTATDATA*);

private:
    HRESULT _FindFormatEtc(LPFORMATETC lpfe, t_size& index, bool b_checkTymed);
    HRESULT _GetStgMediumAddRef(t_size index, STGMEDIUM* pstgmOut);

    class t_data_entry {
    public:
        FORMATETC fe;
        STGMEDIUM sm;

        static int g_compare_formatetc(const t_data_entry& v1, const t_data_entry& v2)
        {
            int rv = pfc::compare_t(v1.fe.cfFormat, v2.fe.cfFormat);
            if (!rv)
                rv = pfc::compare_t(v1.fe.dwAspect, v2.fe.dwAspect);
            if (!rv)
                rv = pfc::compare_t(v1.fe.lindex, v2.fe.lindex);
            return rv;
        }

        static int g_compare_formatetc_value(const t_data_entry& v1, LPFORMATETC v2)
        {
            int rv = pfc::compare_t(v1.fe.cfFormat, v2->cfFormat);
            if (!rv)
                rv = pfc::compare_t(v1.fe.dwAspect, v2->dwAspect);
            if (!rv)
                rv = pfc::compare_t(v1.fe.lindex, v2->lindex);
            return rv;
        }

        t_data_entry()
        {
            memset(&fe, 0, sizeof(fe));
            memset(&sm, 0, sizeof(sm));
        }
    };

    pfc::list_t<t_data_entry> m_data_entries;

    DWORD m_cRefCount;
};
