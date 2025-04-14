#pragma once

#include <d3d11.h>
#include <dxgi1_2.h>
#include <QImage>
#include <QMutex>
#include <QCursor>
#include "PTR_INFO.h"

class ScreenCapture
{
public:
    ScreenCapture();
    ~ScreenCapture();

    bool initialize();
    bool captureFrame();
    QImage getLatestFrame();

private:
    bool initDirectX();
    bool initDuplication();
    void cleanup();
    HRESULT getMouse(PTR_INFO* ptrInfo, DXGI_OUTDUPL_FRAME_INFO* frameInfo, int offsetX, int offsetY);
    void drawMouse(QImage& image, PTR_INFO* ptrInfo);

    // DirectX objects
    ID3D11Device* m_d3dDevice = nullptr;
    ID3D11DeviceContext* m_d3dContext = nullptr;
    IDXGIOutputDuplication* m_deskDupl = nullptr;
    ID3D11Texture2D* m_acquiredDesktopImage = nullptr;
    ID3D11Texture2D* m_stagingTexture = nullptr;

    // Frame data
    QImage m_latestFrame;
    QMutex m_frameMutex;

    // Screen dimensions
    int m_screenWidth = 0;
    int m_screenHeight = 0;

    // Output description 
    DXGI_OUTPUT_DESC m_outputDesc;

    // Pointer info
    PTR_INFO m_ptrInfo;

    // Output number
    UINT m_outputNumber = 0;

};