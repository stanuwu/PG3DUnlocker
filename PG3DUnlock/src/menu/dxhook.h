#pragma once

// input, 11 and 12
#include <dxgi.h>
#include <dxgi1_4.h>
#include <d3d11.h>
#include <d3d12.h>

#include <Windows.h>

namespace dxhook
{
    inline bool initDone = false;
    inline LRESULT (*oWndProc)(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    LRESULT __stdcall WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    bool dx12setup(WNDCLASSEX windowClass, HWND window);
    bool dx11setup(WNDCLASSEX windowClass, HWND window);
    bool init();
    void shutdown();
    inline void (*oExecuteCommandLists)(ID3D12CommandQueue*, UINT, ID3D12CommandList* const*);
    inline HRESULT (*oPresent11)(void* pSwapChain, UINT SyncInterval, UINT Flags);
    inline HRESULT (*oPresent12)(void* pSwapChain, UINT SyncInterval, UINT Flags);
    void __fastcall executeCommandLists12(ID3D12CommandQueue* pCommandQueue1, UINT NumCommandLists,
                                          ID3D12CommandList* const * ppCommandLists);
    HRESULT __fastcall dxPresent11(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
    HRESULT __fastcall dxPresent12(IDXGISwapChain3* pSwapChain, UINT SyncInterval, UINT Flags);
    void initImgui();
    void shutdownImgui();
    void drawImgui(IDXGISwapChain3* pSwapChain);
}
