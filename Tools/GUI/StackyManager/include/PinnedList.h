#pragma once
#include <shobjidl.h>

// Undocumented interface for pinning items to the taskbar
MIDL_INTERFACE("fb77b1e4-7492-498c-8594-5d97f26771d9")
IPinnedList3 : public IUnknown
{
public:
    virtual HRESULT STDMETHODCALLTYPE EnumObjects(IEnumIDList * *ppidl) = 0;
    virtual HRESULT STDMETHODCALLTYPE Modify(PCIDLIST_ABSOLUTE pidlOld, PCIDLIST_ABSOLUTE pidlNew, int nFlags) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetSlot(PCIDLIST_ABSOLUTE pidl, int *pnSlot) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetSlot(PCIDLIST_ABSOLUTE pidl, int nSlot) = 0;
    virtual HRESULT STDMETHODCALLTYPE IsPinnable(IDataObject * pdtobj, int nFlags, PIDLIST_ABSOLUTE *ppidl) = 0;
};

const CLSID CLSID_PinnedList = {0xa4344958, 0xd41, 0x474a, {0x8d, 0x3d, 0x5, 0x2f, 0x6b, 0x81, 0x11, 0xc1}};
