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

void CauldronIO::MapNative::retrieve()
{
    throw CauldronIOException("Not implemented");
}

void CauldronIO::MapNative::setDataStore(const std::string& filename, bool compressed, size_t offset, size_t size)
{
    m_filename = filename;
    m_compressed = compressed;
    m_offset = offset;
    m_size = size;
}

CauldronIO::VolumeNative::VolumeNative(bool cellCentered, SubsurfaceKind kind, boost::shared_ptr<const Property> property) 
    : Volume(cellCentered, kind, property)
{
}

void CauldronIO::VolumeNative::retrieve()
{
    throw CauldronIOException("Not implemented");
}

void CauldronIO::VolumeNative::setDataStore(const std::string& filename, bool compressed, size_t offset, size_t size)
{
    m_filename = filename;
    m_compressed = compressed;
    m_offset = offset;
    m_size = size;
}
