//
// Copyright (C) 2012-2015 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "VisualizationIO_native.h"

using namespace CauldronIO;

CauldronIO::MapNative::MapNative(bool cellCentered) : Map(cellCentered)
{
}

void CauldronIO::MapNative::Retrieve()
{
    throw CauldronIOException("Not implemented");
}

void CauldronIO::MapNative::SetDataStore(const std::string& filename, bool compressed, size_t offset, size_t size)
{
    _filename = filename;
    _compressed = compressed;
    _offset = offset;
    _size = size;
}

CauldronIO::VolumeNative::VolumeNative(bool cellCentered, SubsurfaceKind kind, boost::shared_ptr<const Property> property) 
    : Volume(cellCentered, kind, property)
{
}

void CauldronIO::VolumeNative::Retrieve()
{
    throw CauldronIOException("Not implemented");
}

void CauldronIO::VolumeNative::SetDataStore(const std::string& filename, bool compressed, size_t offset, size_t size)
{
    _filename = filename;
    _compressed = compressed;
    _offset = offset;
    _size = size;
}
