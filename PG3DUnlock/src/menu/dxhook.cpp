#include "dxhook.h"

#include <iostream>

#include "menu.h"
#include <detours/detours.h>

// method based on kiero hook

int PRESENT_INDEX_11 = 8;
int PRESENT_INDEX_12 = 140;
int LIST_INDEX = 54;

ID3D11Device* pDevice11 = nullptr;
ID3D11DeviceContext* pContext11 = nullptr;
ID3D11RenderTargetView* mainRenderTargetView11 = nullptr;

ID3D12Device* pDevice12 = nullptr;
ID3D12DescriptorHeap* pDescriptorHeap = nullptr;
ID3D12DescriptorHeap* g_pd3dRtvDescHeap = nullptr;
ID3D12DescriptorHeap* g_pd3dSrvDescHeap = nullptr;
ID3D12GraphicsCommandList* pCommandList = nullptr;
ID3D12Resource** backBuffers = nullptr;
D3D12_CPU_DESCRIPTOR_HANDLE* descriptorHandles = nullptr;
ID3D12CommandAllocator* pAllocator = nullptr;
ID3D12CommandQueue* pCommandQueue = nullptr;
D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle;
UINT bufferCount = 0;

bool isDx12 = false;
UINT g_ResizeHeight = 0;
UINT g_ResizeWidth = 0;
static UINT64* g_methodsTable = nullptr;
HWND imguiWindow = nullptr;
bool isForeground = false;

// mouse cursor utility functions
void ShowMouseCursor(bool show)
{
    if (show)
    {
        while (ShowCursor(TRUE) < 0);
    }
    else
    {
        while (ShowCursor(FALSE) >= 0);
    }
}

void HandleMouseInputs(HWND hWnd, ImGuiIO& io)
{
    if (!menu::is_active())
        return;

    POINT mousePos;
    GetCursorPos(&mousePos);
    ScreenToClient(hWnd, &mousePos);
    io.MousePos = ImVec2(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));

    io.MouseDown[0] = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
    io.MouseDown[1] = (GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0;
    io.MouseDown[2] = (GetAsyncKeyState(VK_MBUTTON) & 0x8000) != 0;
}

// https://stackoverflow.com/a/15977613
WPARAM MapLeftRightKeys(WPARAM wParam, LPARAM lParam)
{
    WPARAM new_vk;
    const UINT scancode = (lParam & 0x00ff0000) >> 16;
    const int extended = (lParam & 0x01000000) != 0;

    switch (wParam)
    {
    case VK_SHIFT:
        new_vk = MapVirtualKey(scancode, MAPVK_VSC_TO_VK_EX);
        break;
    case VK_CONTROL:
        new_vk = extended ? VK_RCONTROL : VK_LCONTROL;
        break;
    default:
        // not a key we map from generic to left/right specialized
        // just return it.
        new_vk = wParam;
        break;
    }

    return new_vk;
}

// yeah imgui is missing this for some reason
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT WINAPI dxhook::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    ImGuiIO& io = ImGui::GetIO();

    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam)) return true;

    switch (msg)
    {
    case WM_ACTIVATE:
        {
            if (LOWORD(wParam) == 0)
            {
                ClipCursor(nullptr);
                isForeground = false;
                return CallWindowProcW(oWndProc, GetActiveWindow(), msg, wParam, lParam);
            }
            isForeground = true;
            break;
        }
    case WM_MOUSEMOVE:
        HandleMouseInputs(hWnd, io);
        break;
    case WM_LBUTTONDOWN:
        if (menu::is_active())
            io.MouseDown[0] = true;
        break;
    case WM_LBUTTONUP:
        if (menu::is_active())
            io.MouseDown[0] = false;
        break;
    case WM_RBUTTONDOWN:
        if (menu::is_active())
            io.MouseDown[1] = true;
        break;
    case WM_RBUTTONUP:
        if (menu::is_active())
            io.MouseDown[1] = false;
        break;
    case WM_MBUTTONDOWN:
        if (menu::is_active())
            io.MouseDown[2] = true;
        break;
    case WM_MBUTTONUP:
        if (menu::is_active())
            io.MouseDown[2] = false;
        break;
    case WM_KEYDOWN:
        if (MapLeftRightKeys(wParam, lParam) == VK_RSHIFT || menu::is_active() && wParam
            == VK_ESCAPE)
        {
            ShowMouseCursor(!menu::is_active());
        }
        break;
    case WM_SIZE:
        if (wParam == SIZE_MINIMIZED)
            return 0;
        g_ResizeWidth = static_cast<UINT>(LOWORD(lParam));
        g_ResizeHeight = static_cast<UINT>(HIWORD(lParam));
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    default:
        break;
    }

    if (!menu::is_active())
    {
        RECT rect;
        GetClientRect(hWnd, &rect);
        POINT center = {(rect.right - rect.left) / 2, (rect.bottom - rect.top) / 2};
        ClientToScreen(hWnd, &center);
        RECT confineRect;
        confineRect.left = center.x - 10;
        confineRect.right = center.x + 10;
        confineRect.top = center.y - 10;
        confineRect.bottom = center.y + 10;
        if (center.x >= 0 && center.y >= 0 && isForeground) ClipCursor(&confineRect);
        return CallWindowProcW(oWndProc, GetActiveWindow(), msg, wParam, lParam);
    }
    ClipCursor(nullptr);
    return DefWindowProcW(GetActiveWindow(), msg, wParam, lParam);
}

bool dxhook::init()
{
    // create a window
    WNDCLASSEX windowClass;
    windowClass.cbSize = sizeof(WNDCLASSEX);
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = DefWindowProc;
    windowClass.cbClsExtra = 0;
    windowClass.cbWndExtra = 0;
    windowClass.hInstance = GetModuleHandle(nullptr);
    windowClass.hIcon = nullptr;
    windowClass.hCursor = nullptr;
    windowClass.hbrBackground = nullptr;
    windowClass.lpszMenuName = nullptr;
    windowClass.lpszClassName = L"DXWindow";
    windowClass.hIconSm = nullptr;
    ::RegisterClassEx(&windowClass);
    HWND window = ::CreateWindow(windowClass.lpszClassName, L"DXWindow", WS_OVERLAPPEDWINDOW, 0, 0, 100, 100, NULL,
                                 NULL, windowClass.hInstance, NULL);

    return dx12setup(windowClass, window) || dx11setup(windowClass, window);
}


bool dxhook::dx12setup(WNDCLASSEX windowClass, HWND window)
{
    // dx 12 setup
    // get all needed instances and api
    HMODULE libDXGI;
    HMODULE libD3D12;
    if ((libDXGI = ::GetModuleHandle(L"dxgi.dll")) == nullptr || (libD3D12 = ::GetModuleHandle(L"d3d12.dll")) ==
        nullptr)
    {
        DestroyWindow(window);
        ::UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
        return false;
    }

    void* CreateDXGIFactory;
    if ((CreateDXGIFactory = GetProcAddress(libDXGI, "CreateDXGIFactory")) == nullptr)
    {
        DestroyWindow(window);
        ::UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
        return false;
    }

    IDXGIFactory* factory;
    if (reinterpret_cast<long(__stdcall*)(const IID&, void**)>(CreateDXGIFactory)(
        __uuidof(IDXGIFactory), reinterpret_cast<void**>(&factory)) < 0)
    {
        DestroyWindow(window);
        ::UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
        return false;
    }

    IDXGIAdapter* adapter;
    if (factory->EnumAdapters(0, &adapter) == DXGI_ERROR_NOT_FOUND)
    {
        DestroyWindow(window);
        ::UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
        return false;
    }

    void* D3D12CreateDevice;
    if ((D3D12CreateDevice = GetProcAddress(libD3D12, "D3D12CreateDevice")) == nullptr)
    {
        DestroyWindow(window);
        ::UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
        return false;
    }

    ID3D12Device* device;
    if (reinterpret_cast<long(__stdcall*)(IUnknown*, D3D_FEATURE_LEVEL, const IID&, void**)>(D3D12CreateDevice)(
        adapter, D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), reinterpret_cast<void**>(&device)) < 0)
    {
        DestroyWindow(window);
        ::UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
        return false;
    }

    D3D12_COMMAND_QUEUE_DESC queueDesc;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    queueDesc.Priority = 0;
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.NodeMask = 0;

    ID3D12CommandQueue* commandQueue;
    if (device->CreateCommandQueue(&queueDesc, __uuidof(ID3D12CommandQueue), reinterpret_cast<void**>(&commandQueue)) <
        0)
    {
        DestroyWindow(window);
        ::UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
        return false;
    }

    ID3D12CommandAllocator* commandAllocator;
    if (device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, __uuidof(ID3D12CommandAllocator),
                                       reinterpret_cast<void**>(&commandAllocator)) < 0)
    {
        DestroyWindow(window);
        ::UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
        return false;
    }

    ID3D12GraphicsCommandList* commandList;
    if (device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator, nullptr,
                                  __uuidof(ID3D12GraphicsCommandList), reinterpret_cast<void**>(&commandList)) < 0)
    {
        DestroyWindow(window);
        ::UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
        return false;
    }

    // set up our swapchain
    DXGI_RATIONAL refreshRate;
    refreshRate.Numerator = 60;
    refreshRate.Denominator = 1;

    DXGI_MODE_DESC bufferDesc;
    bufferDesc.Width = 100;
    bufferDesc.Height = 100;
    bufferDesc.RefreshRate = refreshRate;
    bufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    bufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    bufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

    DXGI_SAMPLE_DESC sampleDesc;
    sampleDesc.Count = 1;
    sampleDesc.Quality = 0;

    DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
    swapChainDesc.BufferDesc = bufferDesc;
    swapChainDesc.SampleDesc = sampleDesc;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = 2;
    swapChainDesc.OutputWindow = window;
    swapChainDesc.Windowed = 1;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

    IDXGISwapChain* swapChain;
    if (factory->CreateSwapChain(commandQueue, &swapChainDesc, &swapChain) < 0)
    {
        DestroyWindow(window);
        ::UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
        return false;
    }

    // yoink the method table
    g_methodsTable = static_cast<UINT64*>(calloc(150, sizeof(UINT64)));
    memcpy(g_methodsTable, *reinterpret_cast<UINT64**>(device), 44 * sizeof(UINT64));
    memcpy(g_methodsTable + 44, *reinterpret_cast<UINT64**>(commandQueue), 19 * sizeof(UINT64));
    memcpy(g_methodsTable + 44 + 19, *reinterpret_cast<UINT64**>(commandAllocator), 9 * sizeof(UINT64));
    memcpy(g_methodsTable + 44 + 19 + 9, *reinterpret_cast<UINT64**>(commandList), 60 * sizeof(UINT64));
    memcpy(g_methodsTable + 44 + 19 + 9 + 60, *reinterpret_cast<UINT64**>(swapChain), 18 * sizeof(UINT64));

    // we dont need this stuff now
    device->Release();
    device = nullptr;

    commandQueue->Release();
    commandQueue = nullptr;

    commandAllocator->Release();
    commandAllocator = nullptr;

    commandList->Release();
    commandList = nullptr;

    swapChain->Release();
    swapChain = nullptr;

    DestroyWindow(window);
    ::UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);

    auto targetPresent = reinterpret_cast<PVOID*>(g_methodsTable[PRESENT_INDEX_12]);
    auto targetExecuteCommandLists = reinterpret_cast<PVOID*>(g_methodsTable[LIST_INDEX]);

    oPresent12 = reinterpret_cast<HRESULT (*)(void*, UINT, UINT)>(targetPresent);
    oExecuteCommandLists = reinterpret_cast<void (*)(ID3D12CommandQueue*, UINT, ID3D12CommandList* const*)>(
        targetExecuteCommandLists);

    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    DetourAttach(&(PVOID&)oPresent12, dxPresent12);
    DetourAttach(&(PVOID&)oExecuteCommandLists, executeCommandLists12);

    if (DetourTransactionCommit() != NO_ERROR)
    {
        free(g_methodsTable);
        g_methodsTable = nullptr;
        return false;
    }

    // init done
    return true;
}

bool dxhook::dx11setup(WNDCLASSEX windowClass, HWND window)
{
    // dx 11 setup
    // get all needed instances and api
    HMODULE libD3D11;
    if ((libD3D11 = ::GetModuleHandle(L"d3d11.dll")) == nullptr)
    {
        DestroyWindow(window);
        ::UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
        return false;
    }

    void* D3D11CreateDeviceAndSwapChain;
    if ((D3D11CreateDeviceAndSwapChain = reinterpret_cast<void*>(GetProcAddress(
        libD3D11, "D3D11CreateDeviceAndSwapChain"))) == nullptr)
    {
        DestroyWindow(window);
        ::UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
        return false;
    }

    // set up our swapchain
    D3D_FEATURE_LEVEL featureLevel;
    constexpr D3D_FEATURE_LEVEL featureLevels[] = {D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_11_0};

    DXGI_RATIONAL refreshRate11;
    refreshRate11.Numerator = 60;
    refreshRate11.Denominator = 1;

    DXGI_MODE_DESC bufferDesc11;
    bufferDesc11.Width = 100;
    bufferDesc11.Height = 100;
    bufferDesc11.RefreshRate = refreshRate11;
    bufferDesc11.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    bufferDesc11.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    bufferDesc11.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

    DXGI_SAMPLE_DESC sampleDesc11;
    sampleDesc11.Count = 1;
    sampleDesc11.Quality = 0;

    DXGI_SWAP_CHAIN_DESC swapChainDesc11;
    swapChainDesc11.BufferDesc = bufferDesc11;
    swapChainDesc11.SampleDesc = sampleDesc11;
    swapChainDesc11.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc11.BufferCount = 1;
    swapChainDesc11.OutputWindow = window;
    swapChainDesc11.Windowed = 1;
    swapChainDesc11.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    swapChainDesc11.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

    IDXGISwapChain* swapChain11;
    ID3D11Device* device11;
    ID3D11DeviceContext* context11;

    if (reinterpret_cast<long(__stdcall*)(
        IDXGIAdapter*,
        D3D_DRIVER_TYPE,
        HMODULE,
        UINT,
        const D3D_FEATURE_LEVEL*,
        UINT,
        UINT,
        const DXGI_SWAP_CHAIN_DESC*,
        IDXGISwapChain**,
        ID3D11Device**,
        D3D_FEATURE_LEVEL*,
        ID3D11DeviceContext**)>(D3D11CreateDeviceAndSwapChain)(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
                                                               featureLevels, 2, D3D11_SDK_VERSION, &swapChainDesc11,
                                                               &swapChain11, &device11, &featureLevel, &context11) < 0)
    {
        DestroyWindow(window);
        ::UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
        return false;
    }

    // yoink the method table
    g_methodsTable = static_cast<UINT64*>(calloc(205, sizeof(UINT64)));
    memcpy(g_methodsTable, *reinterpret_cast<UINT64**>(swapChain11), 18 * sizeof(UINT64));
    memcpy(g_methodsTable + 18, *reinterpret_cast<UINT64**>(device11), 43 * sizeof(UINT64));
    memcpy(g_methodsTable + 18 + 43, *reinterpret_cast<UINT64**>(context11), 144 * sizeof(UINT64));

    // we dont need this stuff now
    swapChain11->Release();
    swapChain11 = nullptr;

    device11->Release();
    device11 = nullptr;

    context11->Release();
    context11 = nullptr;

    DestroyWindow(window);
    ::UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);

    // hook present func
    auto target = reinterpret_cast<PVOID*>(g_methodsTable[PRESENT_INDEX_11]);
    oPresent11 = reinterpret_cast<HRESULT (*)(void*, UINT, UINT)>(target);

    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    DetourAttach(&(PVOID&)oPresent11, dxPresent11);

    if (DetourTransactionCommit() != NO_ERROR)
    {
        return false;
    }

    // init done
    return true;
}

void dxhook::shutdown()
{
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    DetourDetach(&(PVOID&)oPresent11, (PVOID)dxPresent11);
    DetourDetach(&(PVOID&)oPresent12, (PVOID)dxPresent12);
    DetourDetach(&(PVOID&)oExecuteCommandLists, (PVOID)executeCommandLists12);

    DetourTransactionCommit();
    free(g_methodsTable);
    g_methodsTable = nullptr;
}

void __fastcall dxhook::executeCommandLists12(ID3D12CommandQueue* pCommandQueue1, UINT NumCommandLists,
                                              ID3D12CommandList* const * ppCommandLists)
{
    if (!pCommandQueue)
    {
        pCommandQueue = pCommandQueue1;
    }
    oExecuteCommandLists(pCommandQueue1, NumCommandLists, ppCommandLists);
}

HRESULT __fastcall dxhook::dxPresent11(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
{
    // init imgui first time
    if (!initDone)
    {
        if (SUCCEEDED(pSwapChain->GetDevice(__uuidof(ID3D11Device), (void**)& pDevice11)))
        {
            isDx12 = false;
            pDevice11->GetImmediateContext(&pContext11);
            DXGI_SWAP_CHAIN_DESC sd;
            pSwapChain->GetDesc(&sd);
            imguiWindow = sd.OutputWindow;
            ID3D11Texture2D* pBackBuffer;
            pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
            pDevice11->CreateRenderTargetView(pBackBuffer, nullptr, &mainRenderTargetView11);
            pBackBuffer->Release();
            oWndProc = reinterpret_cast<LRESULT(*)(HWND, UINT, WPARAM, LPARAM)>(SetWindowLongPtr(
                imguiWindow, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(WndProc)));

            // Init Imgui
            initImgui();

            initDone = true;
        }
        else
        {
            isDx12 = true;
        }
    }

    // draw imgui ui
    if (!isDx12) drawImgui(nullptr);

    return oPresent11(pSwapChain, SyncInterval, Flags);
}

HRESULT __fastcall dxhook::dxPresent12(IDXGISwapChain3* pSwapChain, UINT SyncInterval, UINT Flags)
{
    // init imgui first time
    if (!initDone)
    {
        if (SUCCEEDED(pSwapChain->GetDevice(__uuidof(ID3D12Device), (void**)& pDevice12)))
        {
            isDx12 = true;
            DXGI_SWAP_CHAIN_DESC sd;
            pSwapChain->GetDesc(&sd);
            imguiWindow = sd.OutputWindow;

            DXGI_SWAP_CHAIN_DESC swapChainDesc;
            bufferCount = pSwapChain->GetDesc(&swapChainDesc);
            bufferCount = swapChainDesc.BufferCount;

            pDevice12->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&pAllocator));
            pDevice12->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, pAllocator, nullptr,
                                         IID_PPV_ARGS(&pCommandList));
            pCommandList->Close();

            pDevice12->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
            D3D12_DESCRIPTOR_HEAP_DESC desc = {};
            desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
            desc.NumDescriptors = bufferCount;
            desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
            desc.NodeMask = 1;
            pDevice12->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&g_pd3dRtvDescHeap));
            SIZE_T rtvDescriptorSize = pDevice12->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
            rtvHandle = g_pd3dRtvDescHeap->GetCPUDescriptorHandleForHeapStart();
            backBuffers = new ID3D12Resource*[bufferCount];
            descriptorHandles = new D3D12_CPU_DESCRIPTOR_HANDLE[bufferCount];
            for (UINT i = 0; i < bufferCount; i++)
            {
                ID3D12Resource* pBackBuffer = nullptr;
                pSwapChain->GetBuffer(i, IID_PPV_ARGS(&pBackBuffer));
                pDevice12->CreateRenderTargetView(pBackBuffer, nullptr, rtvHandle);
                backBuffers[i] = pBackBuffer;
                descriptorHandles[i] = rtvHandle;
                rtvHandle.ptr += rtvDescriptorSize;
            }

            oWndProc = reinterpret_cast<LRESULT(*)(HWND, UINT, WPARAM, LPARAM)>(SetWindowLongPtr(
                imguiWindow, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(WndProc)));

            // Init Imgui
            initImgui();

            initDone = true;
        }
        else
        {
            isDx12 = false;
        }
    }

    // draw imgui ui
    if (isDx12) drawImgui(pSwapChain);

    return oPresent12(pSwapChain, SyncInterval, Flags);
}

void dxhook::initImgui()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGui_ImplWin32_Init(imguiWindow);
    if (isDx12)
    {
        D3D12_DESCRIPTOR_HEAP_DESC descriptor = {};
        descriptor.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        descriptor.NumDescriptors = bufferCount;
        descriptor.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

        pDevice12->CreateDescriptorHeap(&descriptor, IID_PPV_ARGS(&pDescriptorHeap));
        ImGui_ImplDX12_Init(pDevice12, 2, DXGI_FORMAT_R8G8B8A8_UNORM, pDescriptorHeap,
                            pDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
                            pDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
    }
    else
    {
        ImGui_ImplDX11_Init(pDevice11, pContext11);
    }

    initDone = true;
}

void dxhook::shutdownImgui()
{
    initDone = false;

    if (isDx12)
    {
        ImGui_ImplDX12_Shutdown();
        pDescriptorHeap->Release();
        g_pd3dSrvDescHeap->Release();
        pAllocator->Release();
        pCommandList->Release();
        for (size_t i = 0; i < bufferCount; ++i)
        {
            backBuffers[i]->Release();
        }
        delete[] backBuffers;
        delete[] descriptorHandles;
    }
    else
    {
        ImGui_ImplDX11_Shutdown();
        mainRenderTargetView11->Release();
    }
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}

void dxhook::drawImgui(IDXGISwapChain3* pSwapChain)
{
    if (isDx12)
    {
        ImGui_ImplDX12_NewFrame();
    }
    else
    {
        ImGui_ImplDX11_NewFrame();
    }
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
    {
        menu::draw();
    }
    ImGui::EndFrame();
    ImGui::Render();

    if (isDx12)
    {
        size_t index = pSwapChain->GetCurrentBackBufferIndex();
        ID3D12Resource* pBackBuffer = backBuffers[index];
        pAllocator->Reset();
        D3D12_RESOURCE_BARRIER barrier;
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier.Transition.pResource = pBackBuffer;
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
        pCommandList->Reset(pAllocator, nullptr);
        pCommandList->ResourceBarrier(1, &barrier);
        pCommandList->OMSetRenderTargets(1, &descriptorHandles[index], FALSE, nullptr);
        pCommandList->SetDescriptorHeaps(1, &pDescriptorHeap);
        ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), pCommandList);
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
        pCommandList->ResourceBarrier(1, &barrier);
        pCommandList->Close();

        // skip if we cont have the queue right now
        if (pCommandQueue == nullptr) return;
        pCommandQueue->ExecuteCommandLists(1, reinterpret_cast<ID3D12CommandList* const*>(&pCommandList));
    }
    else
    {
        pContext11->OMSetRenderTargets(1, &mainRenderTargetView11, nullptr);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    }
}
