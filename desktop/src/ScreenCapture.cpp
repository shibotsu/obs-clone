#include "incl/ScreenCapture.h"
#include <QDebug>
#include <sstream>
#include <QPainter>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

ScreenCapture::ScreenCapture()
{
}

ScreenCapture::~ScreenCapture()
{
    cleanup();
}

bool ScreenCapture::initialize()
{
    if (!initDirectX()) {
        qDebug() << "Failed to initialize DirectX";
        return false;
    }

    if (!initDuplication()) {
        qDebug() << "Failed to initialize DXGI Duplication";
        return false;
    }

    return true;
}

bool ScreenCapture::initDirectX()
{
    // Create D3D11 device
    UINT createDeviceFlags = 0;
#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
        D3D_FEATURE_LEVEL_9_3,
    };
    UINT numFeatureLevels = ARRAYSIZE(featureLevels);
    D3D_FEATURE_LEVEL featureLevel;

    HRESULT hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags,
        featureLevels, numFeatureLevels, D3D11_SDK_VERSION, &m_d3dDevice, &featureLevel, &m_d3dContext);

    if (FAILED(hr)) {
        qDebug() << "Failed to create D3D11 device:" << hr;
        return false;
    }

    return true;
}

bool ScreenCapture::initDuplication()
{
    // Get DXGI Device
    IDXGIDevice* dxgiDevice = nullptr;
    HRESULT hr = m_d3dDevice->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgiDevice);
    if (FAILED(hr)) {
        qDebug() << "Failed to get DXGI Device";
        return false;
    }

    // Get DXGI Adapter
    IDXGIAdapter* dxgiAdapter = nullptr;
    hr = dxgiDevice->GetParent(__uuidof(IDXGIAdapter), (void**)&dxgiAdapter);
    dxgiDevice->Release();
    if (FAILED(hr)) {
        qDebug() << "Failed to get DXGI Adapter";
        return false;
    }

    // Get output
    IDXGIOutput* dxgiOutput = nullptr;
    hr = dxgiAdapter->EnumOutputs(0, &dxgiOutput);
    dxgiAdapter->Release();
    if (FAILED(hr)) {
        qDebug() << "Failed to get DXGI Output";
        return false;
    }

    // Get output description (for screen dimensions)
    DXGI_OUTPUT_DESC outputDesc;
    hr = dxgiOutput->GetDesc(&outputDesc);
    if (SUCCEEDED(hr)) {
        m_screenWidth = outputDesc.DesktopCoordinates.right - outputDesc.DesktopCoordinates.left;
        m_screenHeight = outputDesc.DesktopCoordinates.bottom - outputDesc.DesktopCoordinates.top;
        qDebug() << "Screen dimensions:" << m_screenWidth << "x" << m_screenHeight;
    }

    // QI for Output 1
    IDXGIOutput1* dxgiOutput1 = nullptr;
    hr = dxgiOutput->QueryInterface(__uuidof(IDXGIOutput1), (void**)&dxgiOutput1);
    dxgiOutput->Release();
    if (FAILED(hr)) {
        qDebug() << "Failed to get IDXGIOutput1";
        return false;
    }

    // Create desktop duplication
    hr = dxgiOutput1->DuplicateOutput(m_d3dDevice, &m_deskDupl);
    dxgiOutput1->Release();
    if (FAILED(hr)) {
        qDebug() << "Failed to duplicate output. HRESULT:" << hr;
        return false;
    }

    // Create staging texture for CPU access
    D3D11_TEXTURE2D_DESC stagingDesc = {};
    stagingDesc.Width = m_screenWidth;
    stagingDesc.Height = m_screenHeight;
    stagingDesc.MipLevels = 1;
    stagingDesc.ArraySize = 1;
    stagingDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    stagingDesc.SampleDesc.Count = 1;
    stagingDesc.Usage = D3D11_USAGE_STAGING;
    stagingDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    stagingDesc.BindFlags = 0;

    hr = m_d3dDevice->CreateTexture2D(&stagingDesc, nullptr, &m_stagingTexture);
    if (FAILED(hr)) {
        qDebug() << "Failed to create staging texture:" << hr;
        return false;
    }

    // Initialize QImage with the correct size
    m_latestFrame = QImage(m_screenWidth, m_screenHeight, QImage::Format_ARGB32);

    return true;
}

bool ScreenCapture::captureFrame()
{
    if (!m_deskDupl) {
        qDebug() << "No desktop duplicator available";
        return false;
    }

    // Release any previous frame
    if (m_acquiredDesktopImage) {
        m_acquiredDesktopImage->Release();
        m_acquiredDesktopImage = nullptr;
    }

    // Get next frame
    IDXGIResource* desktopResource = nullptr;
    DXGI_OUTDUPL_FRAME_INFO frameInfo;
    HRESULT hr = m_deskDupl->AcquireNextFrame(0, &frameInfo, &desktopResource);

    if (hr == DXGI_ERROR_WAIT_TIMEOUT) {
        // No new frame available
        return false;
    }

    if (FAILED(hr)) {
        if (hr == DXGI_ERROR_ACCESS_LOST) {
            qDebug() << "Access lost to desktop duplication";
            // Try to reinitialize
            if (m_deskDupl) {
                m_deskDupl->Release();
                m_deskDupl = nullptr;
            }
            initDuplication();
        }
        else {
            qDebug() << "Failed to acquire frame. HRESULT:" << hr;
        }
        return false;
    }

    // Get mouse info
    int offsetX = m_outputDesc.DesktopCoordinates.left;
    int offsetY = m_outputDesc.DesktopCoordinates.top;
    getMouse(&m_ptrInfo, &frameInfo, offsetX, offsetY);

    // QI for ID3D11Texture2D
    hr = desktopResource->QueryInterface(__uuidof(ID3D11Texture2D), (void**)&m_acquiredDesktopImage);
    desktopResource->Release();
    if (FAILED(hr)) {
        qDebug() << "Failed to QI for ID3D11Texture2D";
        m_deskDupl->ReleaseFrame();
        return false;
    }

    // Copy to staging texture
    m_d3dContext->CopyResource(m_stagingTexture, m_acquiredDesktopImage);

    // Map staging texture to read pixels
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    hr = m_d3dContext->Map(m_stagingTexture, 0, D3D11_MAP_READ, 0, &mappedResource);
    if (SUCCEEDED(hr)) {
        // Lock the frame mutex while we update the frame
        QMutexLocker locker(&m_frameMutex);

        // Copy from staging texture to QImage
        uchar* dest = m_latestFrame.bits();
        uchar* src = (uchar*)mappedResource.pData;
        const int bytesPerLine = m_screenWidth * 4;

        for (int y = 0; y < m_screenHeight; y++) {
            memcpy(dest, src, bytesPerLine);
            dest += m_latestFrame.bytesPerLine();
            src += mappedResource.RowPitch;
        }

        m_d3dContext->Unmap(m_stagingTexture, 0);

        // Draw mouse cursor on top of the frame
        drawMouse(m_latestFrame, &m_ptrInfo);
    }
    else {
        qDebug() << "Failed to map staging texture:" << hr;
    }

    // Release frame
    m_deskDupl->ReleaseFrame();
    return true;
}

QImage ScreenCapture::getLatestFrame()
{
    QMutexLocker locker(&m_frameMutex);
    return m_latestFrame;
}

void ScreenCapture::cleanup()
{
    if (m_deskDupl) {
        if (m_acquiredDesktopImage) {
            m_deskDupl->ReleaseFrame();
            m_acquiredDesktopImage->Release();
            m_acquiredDesktopImage = nullptr;
        }
        m_deskDupl->Release();
        m_deskDupl = nullptr;
    }

    if (m_stagingTexture) {
        m_stagingTexture->Release();
        m_stagingTexture = nullptr;
    }

    if (m_d3dContext) {
        m_d3dContext->Release();
        m_d3dContext = nullptr;
    }

    if (m_d3dDevice) {
        m_d3dDevice->Release();
        m_d3dDevice = nullptr;
    }
}

HRESULT ScreenCapture::getMouse(PTR_INFO* ptrInfo, DXGI_OUTDUPL_FRAME_INFO* frameInfo, int offsetX, int offsetY)
{
    HRESULT hr = S_OK;

    // A non-zero mouse update timestamp indicates that there is a mouse position update and optionally a shape change
    if (frameInfo->LastMouseUpdateTime.QuadPart == 0)
    {
        return hr;
    }

    bool updatePosition = true;

    // Make sure we don't update pointer position wrongly
    // If pointer is invisible, make sure we did not get an update from another output that the last time that said pointer
    // was visible, if so, don't set it to invisible or update.
    if (!frameInfo->PointerPosition.Visible && (ptrInfo->WhoUpdatedPositionLast != m_outputNumber))
    {
        updatePosition = false;
    }

    // If two outputs both say they have a visible, only update if new update has newer timestamp
    if (frameInfo->PointerPosition.Visible && ptrInfo->Visible &&
        (ptrInfo->WhoUpdatedPositionLast != m_outputNumber) &&
        (ptrInfo->LastTimeStamp.QuadPart > frameInfo->LastMouseUpdateTime.QuadPart))
    {
        updatePosition = false;
    }

    // Update position
    if (updatePosition)
    {
        ptrInfo->Position.x = frameInfo->PointerPosition.Position.x + m_outputDesc.DesktopCoordinates.left - offsetX;
        ptrInfo->Position.y = frameInfo->PointerPosition.Position.y + m_outputDesc.DesktopCoordinates.top - offsetY;
        ptrInfo->WhoUpdatedPositionLast = m_outputNumber;
        ptrInfo->LastTimeStamp = frameInfo->LastMouseUpdateTime;
        ptrInfo->Visible = frameInfo->PointerPosition.Visible != 0;
    }

    // No new shape
    if (frameInfo->PointerShapeBufferSize == 0)
    {
        return hr;
    }

    // Old buffer too small
    if (frameInfo->PointerShapeBufferSize > ptrInfo->BufferSize)
    {
        if (ptrInfo->PtrShapeBuffer)
        {
            delete[] ptrInfo->PtrShapeBuffer;
            ptrInfo->PtrShapeBuffer = nullptr;
        }
        ptrInfo->PtrShapeBuffer = new (std::nothrow) BYTE[frameInfo->PointerShapeBufferSize];
        if (!ptrInfo->PtrShapeBuffer)
        {
            qDebug() << "Failed to allocate memory for pointer shape";
            ptrInfo->BufferSize = 0;
            return E_OUTOFMEMORY;
        }
        // Update buffer size
        ptrInfo->BufferSize = frameInfo->PointerShapeBufferSize;
    }

    UINT bufferSizeRequired;
    // Get shape
    hr = m_deskDupl->GetFramePointerShape(
        frameInfo->PointerShapeBufferSize,
        reinterpret_cast<VOID*>(ptrInfo->PtrShapeBuffer),
        &bufferSizeRequired,
        &(ptrInfo->ShapeInfo));

    if (FAILED(hr))
    {
        if (hr != DXGI_ERROR_ACCESS_LOST)
        {
            qDebug() << "Failed to get frame pointer shape. HRESULT:" << hr;
        }
        delete[] ptrInfo->PtrShapeBuffer;
        ptrInfo->PtrShapeBuffer = nullptr;
        ptrInfo->BufferSize = 0;
        return hr;
    }

    return hr;
}

void ScreenCapture::drawMouse(QImage& image, PTR_INFO* ptrInfo)
{
    // If pointer is not visible or there's no shape data, nothing to draw
    if (!ptrInfo->Visible || !ptrInfo->PtrShapeBuffer) {
        return;
    }

    // Get the cursor position
    int x = ptrInfo->Position.x;
    int y = ptrInfo->Position.y;

    // Get pointer shape information
    DXGI_OUTDUPL_POINTER_SHAPE_INFO shapeInfo = ptrInfo->ShapeInfo;
    UINT width = shapeInfo.Width;
    UINT height = shapeInfo.Height;
    UINT pitch = shapeInfo.Pitch;
    BYTE* buffer = ptrInfo->PtrShapeBuffer;

    // Ensure the mouse cursor is within the image boundaries
    if (x < 0 || y < 0 || x >= image.width() || y >= image.height()) {
        return;
    }

    QPainter painter(&image);

    // Handle different cursor types
    switch (shapeInfo.Type) {
    case DXGI_OUTDUPL_POINTER_SHAPE_TYPE_MONOCHROME:
    {
        // Monochrome cursor (AND/XOR masks)
        QImage cursorImage(width, height, QImage::Format_ARGB32);
        cursorImage.fill(Qt::transparent);

        // XOR mask is in the first half, AND mask in the second half
        int andMaskOffset = pitch * (height / 2);

        for (UINT row = 0; row < height / 2; row++) {
            for (UINT col = 0; col < width; col++) {
                // Get AND and XOR mask values for this pixel
                UINT byteIndex = row * pitch + col / 8;
                UINT bitIndex = 7 - (col % 8); // Most significant bit first

                bool andMask = (buffer[andMaskOffset + byteIndex] & (1 << bitIndex)) != 0;
                bool xorMask = (buffer[byteIndex] & (1 << bitIndex)) != 0;

                QRgb color;
                if (andMask) {
                    if (xorMask) {
                        // Inverted screen without transparency
                        color = qRgba(255, 255, 255, 255); // White
                    }
                    else {
                        // Transparent
                        color = qRgba(0, 0, 0, 0);
                    }
                }
                else {
                    if (xorMask) {
                        // Inverted screen
                        color = qRgba(0, 0, 0, 255); // Black
                    }
                    else {
                        // Opaque
                        color = qRgba(0, 0, 0, 255); // Black
                    }
                }

                cursorImage.setPixel(col, row, color);
            }
        }

        // Draw the cursor onto the image
        painter.drawImage(x, y, cursorImage);
        break;
    }
    case DXGI_OUTDUPL_POINTER_SHAPE_TYPE_COLOR:
    {
        // Color cursor (BGRA format)
        QImage cursorImage(buffer, width, height, pitch, QImage::Format_ARGB32);
        painter.drawImage(x, y, cursorImage);
        break;
    }
    case DXGI_OUTDUPL_POINTER_SHAPE_TYPE_MASKED_COLOR:
    {
        // Masked color cursor
        QImage cursorImage(width, height, QImage::Format_ARGB32);
        cursorImage.fill(Qt::transparent);

        for (UINT row = 0; row < height; row++) {
            BYTE* srcRow = buffer + (row * pitch);
            for (UINT col = 0; col < width; col++) {
                // BGRA format
                BYTE b = srcRow[col * 4];
                BYTE g = srcRow[col * 4 + 1];
                BYTE r = srcRow[col * 4 + 2];
                BYTE a = srcRow[col * 4 + 3];

                // If the alpha channel is 0, this is a masked area
                if (a == 0) {
                    // The color data contains the mask
                    if (b == 0 && g == 0 && r == 0) {
                        // Transparent area
                        cursorImage.setPixel(col, row, qRgba(0, 0, 0, 0));
                    }
                    else {
                        // Inverted area (XOR)
                        // For simplicity, just use black
                        cursorImage.setPixel(col, row, qRgba(0, 0, 0, 255));
                    }
                }
                else {
                    // Normal color
                    cursorImage.setPixel(col, row, qRgba(r, g, b, a));
                }
            }
        }

        painter.drawImage(x, y, cursorImage);
        break;
    }
    default:
        qDebug() << "Unknown cursor type:" << shapeInfo.Type;
        break;
    }
}