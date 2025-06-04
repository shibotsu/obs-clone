#pragma once

#include <d3d11.h>
#include <dxgi1_2.h>
#include <windows.h>
#include <vector>

struct PTR_INFO
{
    std::vector<BYTE> PtrShapeBuffer; // Change from BYTE* to std::vector<BYTE>
    UINT BufferSize = 0;             // Can potentially be replaced by PtrShapeBuffer.size()
    DXGI_OUTDUPL_POINTER_SHAPE_INFO ShapeInfo = {};
    POINT Position = {};
    bool Visible = false;
    LARGE_INTEGER LastTimeStamp = {};
    int WhoUpdatedPositionLast = 0;
};