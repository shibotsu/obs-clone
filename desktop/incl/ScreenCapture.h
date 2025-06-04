#pragma once

#include <QObject>
#include <QImage>
#include <vector>
#include "PTR_INFO.h"

struct IDXGIOutputDuplication;
struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11Texture2D;
struct DXGI_OUTPUT_DESC;
struct DXGI_OUTDUPL_FRAME_INFO;

//#include <d3d11.h>
//#include <dxgi1_2.h>
//#include <QImage>
//#include <QMutex>
//#include <QCursor>
//#include "PTR_INFO.h"
//
//#include "FFmpegRecorder.h"

class ScreenCapture : public QObject
{
	Q_OBJECT

public:
    ScreenCapture();
    ~ScreenCapture();

    bool initialize();
    int getWidth() const { return m_screenWidth; }
    int getHeight() const { return m_screenHeight; }

    // This will be the only function that calls AcquireNextFrame
    // It will process the frame and then emit a signal
    bool captureFrame();

signals:
    void newFrameReady(const uchar* data, int width, int height, int bytesPerLine, const PTR_INFO& mousePtrInfo);

private:
    // DirectX objects
    ID3D11Device* m_d3dDevice = nullptr;
    ID3D11DeviceContext* m_d3dContext = nullptr;
    IDXGIOutputDuplication* m_deskDupl = nullptr;
    ID3D11Texture2D* m_acquiredDesktopImage = nullptr; // Used temporarily for frame acquisition
    ID3D11Texture2D* m_stagingTexture = nullptr;

    // Screen dimensions
    int m_screenWidth = 0;
    int m_screenHeight = 0;
    DXGI_OUTPUT_DESC m_outputDesc = {}; // Store this for mouse offset
    UINT m_outputNumber = 0;
    LARGE_INTEGER m_qpcFreq;

    PTR_INFO m_ptrInfo;

    // Internal buffer to hold the captured frame data before emitting
    std::vector<uchar> m_frameBuffer;
	int m_frameBytesPerLine = 0; // Will be width * 4 for BGRA

    bool initDirectX();
    bool initDuplication();
    void cleanup();

    HRESULT getMouse(PTR_INFO* ptrInfo, DXGI_OUTDUPL_FRAME_INFO* frameInfo, int offsetX, int offsetY);
    void drawMouse(uchar* buffer, int width, int height, int bytesPerLine, PTR_INFO* ptrInfo);
};