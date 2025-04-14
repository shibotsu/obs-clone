#pragma once

#include <d3d11.h>
#include <dxgi1_2.h>
#include <windows.h>

struct PTR_INFO
{
    PTR_INFO() :
        PtrShapeBuffer(nullptr),
        BufferSize(0),
        WhoUpdatedPositionLast(0),
        LastTimeStamp{ 0 },
        Visible(false),
        Position{ 0, 0 },
        ShapeInfo{}
    {
    }

    ~PTR_INFO()
    {
        if (PtrShapeBuffer)
        {
            delete[] PtrShapeBuffer;
            PtrShapeBuffer = nullptr;
        }
    }

    BYTE* PtrShapeBuffer;
    UINT BufferSize;
    UINT WhoUpdatedPositionLast;
    LARGE_INTEGER LastTimeStamp;
    bool Visible;
    POINT Position;
    DXGI_OUTDUPL_POINTER_SHAPE_INFO ShapeInfo;
};