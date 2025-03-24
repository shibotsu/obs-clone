#pragma once

#include <d3d11.h>
#include <dxgi1_2.h>
#include <QImage>
#include <QMutex>

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
};