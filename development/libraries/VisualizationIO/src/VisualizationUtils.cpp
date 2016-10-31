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
#include "VisualizationUtils.h"
#include <assert.h>
#include "hdf5.h"
#include <boost/thread.hpp>

namespace CauldronIO
{

void CauldronIO::VisualizationUtils::retrieveAllData(const std::shared_ptr<SnapShot>& snapShot, size_t numThreads)
{
	// Collect all data to retrieve
	// TODO: check if this works correctly in "append" mode: in append mode it should not retrieve data that has not been added
	std::vector < VisualizationIOData* > allReadData = snapShot->getAllRetrievableData();

	// This is the queue of indices of data ready to be retrieved (and compressed?)
	boost::lockfree::queue<int> queue(128);
	boost::atomic<bool> done(false);

	// Retrieve in separate threads; the queue will be filled from the main thread
	boost::thread_group threads;
	for (int i = 0; i < numThreads - 1; ++i)
		threads.add_thread(new boost::thread(CauldronIO::VisualizationUtils::retrieveDataQueue, &allReadData, &queue, &done));

	// Read HDF data into memory on single main thread
	prefetchHDFdata(allReadData, &queue);
	done = true;

	// Single threaded: retrieve now
	if (numThreads == 1)
		retrieveDataQueue(&allReadData, &queue, &done);
	threads.join_all();
}

void CauldronIO::VisualizationUtils::prefetchHDFdata(std::vector< VisualizationIOData* > allReadData, boost::lockfree::queue<int>* queue)
{
	std::vector < std::vector < std::shared_ptr<HDFinfo> > > hdfInfoListList;
	std::vector < std::string > hdffileNames;

	// Collect all hdfInfo
	for (int i = 0; i < allReadData.size(); i++)
	{
		std::vector < std::shared_ptr<HDFinfo> > hdfInfo = allReadData[i]->getHDFinfo();
		for (int j = 0; j < hdfInfo.size(); j++)
		{
			std::string filepathname = hdfInfo[j]->filepathName;

			// Signal the index so we know when it is complete
			hdfInfo[j]->indexMain = i;

			// See if this file is known
			assert(filepathname.length() > 0);

			bool exists = false;
			for (std::string filename : hdffileNames)
				exists |= filename == filepathname;
			if (!exists)
			{
				hdffileNames.push_back(filepathname);
				std::vector < std::shared_ptr<HDFinfo> > hdfInfoList;
				hdfInfoListList.push_back(hdfInfoList);
			}

			// Add to correct list
			for (int k = 0; k < hdffileNames.size(); ++k)
			{
				if (hdffileNames[k] == filepathname)
				{
					hdfInfoListList[k].push_back(hdfInfo[j]);
					break;
				}
			}
		}
	}

	for (int k = 0; k < hdfInfoListList.size(); ++k)
		loadHDFdata(hdfInfoListList[k], queue);
}

void CauldronIO::VisualizationUtils::loadHDFdata(std::vector< std::shared_ptr<HDFinfo> > hdfInfoList, boost::lockfree::queue<int>* queue)
{
	if (hdfInfoList.size() == 0) return;

	std::string filename = hdfInfoList[0]->filepathName;

	hid_t fileId = H5Fopen(filename.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT);
	if (fileId <= -1)
		throw CauldronIOException(std::string("Could not open HDF file: ") + filename);

	for (int i = 0; i < hdfInfoList.size(); i++)
	{
		// Find the dataset
		std::string datasetname = hdfInfoList[i]->dataSetName;
		hid_t dataSetId = H5Dopen(fileId, datasetname.c_str(), H5P_DEFAULT);

		if (dataSetId <= -1)
			throw CauldronIOException(std::string("Could not open dataset: ") + datasetname);

		hid_t dataTypeId = H5Tcopy(H5T_NATIVE_FLOAT);
		H5T_class_t HDFclass = H5Tget_class(dataTypeId);
		assert(HDFclass == H5T_FLOAT);

		hid_t dataSpaceId = H5Dget_space(dataSetId);
		assert(dataSpaceId >= 0);

		hsize_t dimensions[3];
		int rank = H5Sget_simple_extent_dims(dataSpaceId, dimensions, 0);

		// Add a third dimension if needed
		if (rank == 2) dimensions[2] = 1;

		// Allocate memory
		hdfInfoList[i]->setData(new float[dimensions[0] * dimensions[1] * dimensions[2]]);

		// Read the data
		herr_t status = H5Dread(dataSetId, dataTypeId, H5S_ALL, H5S_ALL, H5P_DEFAULT, (void*)hdfInfoList[i]->getData());

		/// Signal the parent new data is there; 
		/// it should return true if all data is now loaded
		/// and then we should push a new (completed) index to the queue
		bool complete = hdfInfoList[i]->parent->signalNewHDFdata();
		if (complete)
			queue->push(hdfInfoList[i]->indexMain);

		if (status <= -1)
			throw CauldronIOException("Error during hdf file read");

		H5Tclose(dataTypeId);
		H5Dclose(dataSetId);
	}

	H5Fclose(fileId);
}

void VisualizationUtils::retrieveDataQueue(std::vector < VisualizationIOData* >* allData, boost::lockfree::queue<int>* queue, boost::atomic<bool>* done)
{
	int value;
	while (!*done)
	{
		while (queue->pop(value))
		{
			VisualizationIOData* data = allData->at(value);
			assert(!data->isRetrieved());
			data->retrieve();
		}
	}

	while (queue->pop(value))
	{
		VisualizationIOData* data = allData->at(value);
		assert(!data->isRetrieved());
		data->retrieve();
	}
}

void VisualizationUtils::cellCenterAllMaps(const std::shared_ptr<SnapShot>& snapShot, std::shared_ptr<Project>& project)
{
	for (std::shared_ptr<Surface> surface : snapShot->getSurfaceList())
	{
		for (size_t i = 0; i < surface->getPropertySurfaceDataList().size(); i++)
		{
			PropertySurfaceData propSurfaceData = surface->getPropertySurfaceDataList().at(i);
			const std::shared_ptr<SurfaceData>& surfaceData = propSurfaceData.second;
			const std::shared_ptr<const Property>& propertyIO = propSurfaceData.first;

			// Don't cell-center depth
			if (propertyIO->getName() == "Depth") continue;

			filterFunc filter, filterUndef;
			getFilterFunc(propertyIO, filter, filterUndef);

			std::shared_ptr<CauldronIO::SurfaceData> cellCenteredMap = VisualizationUtils::cellCenterMap(surfaceData, filter, filterUndef);
			project->addGeometry(cellCenteredMap->getGeometry());
			CauldronIO::PropertySurfaceData propSurfaceDataCentered(propertyIO, cellCenteredMap);

			// Replace with new map
			surface->replaceAt(i, propSurfaceDataCentered);
		}
	}
}

std::shared_ptr<CauldronIO::SurfaceData> VisualizationUtils::cellCenterMap(const std::shared_ptr<CauldronIO::SurfaceData>& map, filterFunc filter, filterFunc filterUndef)
{
	// construct a new map
	assert(map->isRetrieved());
	auto geometry = map->getGeometry();

	size_t numI = geometry->getNumI();
	size_t numJ = geometry->getNumJ();

	assert(!geometry->isCellCentered());
	std::shared_ptr<Geometry2D> newGeometry(new Geometry2D(numI - 1, numJ - 1, geometry->getDeltaI(), geometry->getDeltaJ(),
		geometry->getMinI(), geometry->getMinJ(), true));

	std::shared_ptr<CauldronIO::SurfaceData> newMap(new CauldronIO::MapNative(newGeometry));
	newMap->setFormation(map->getFormation());
	newMap->setReservoir(map->getReservoir());

	if (map->isConstant())
	{
		newMap->setConstantValue(map->getConstantValue());
		return newMap;
	}

	// average the values
	float* newData = new float[newGeometry->getNumI() * newGeometry->getNumJ()];

	// check if ordered by row
	assert(map->canGetRow());
	const float* inputRow1;
	const float* inputRow2;

	inputRow2 = map->getRowValues(0);
	size_t outputIndex = 0;

	for (int j = 0; j < numJ - 1; j++)
	{
		inputRow1 = inputRow2;
		inputRow2 = map->getRowValues(j + 1);

		for (int i = 0; i < numI - 1; i++)
		{
			float result;
			float v[4];
			v[0] = inputRow1[i];
			v[1] = inputRow1[i + 1];
			v[2] = inputRow2[i];
			v[3] = inputRow2[i + 1];
			if (v[0] == DefaultUndefinedValue || v[1] == DefaultUndefinedValue || v[2] == DefaultUndefinedValue || v[3] == DefaultUndefinedValue)
				result = filterUndef(v, 4);
			else
				result = filter(v, 4);

			newData[outputIndex++] = result;
		}
	}

	newMap->setData_IJ(newData);
	delete[] newData;

	return newMap;
}

float VisualizationUtils::getUndefAverage(float* v, size_t size)
{
	// Check if all undefined
	bool undef = true;
	for (int i = 0; i < size && undef; ++i)
	{
		undef &= v[i] == DefaultUndefinedValue;
	}
	if (undef) return DefaultUndefinedValue;

	// Compute average with all values that are defined
	float result = 0;
	size_t actualValues = 0;

	for (int i = 0; i < size; ++i)
	{
		if (v[i] != DefaultUndefinedValue)
		{
			actualValues++;
			result += v[i];
		}
	}

	if (actualValues > 0)
	{
		result /= (float)actualValues;
	}

	return result;
}


float VisualizationUtils::getAverage(float* v, size_t size)
{
	float result = 0;

	for (int i = 0; i < size; ++i)
	{
		result += v[i];
	}

	return result / (float)size;
}

float VisualizationUtils::getMedian(float* v, size_t size)
{
	assert(size > 0);
	if (size == 1) return v[0];

	std::vector<float> val;
	val.assign(v, v + size);
	std::sort(val.begin(), val.end());

	return val.at(size / 2);
}

float VisualizationUtils::getUndefMedian(float* v, size_t size)
{
	// Check if all undefined
	bool undef = true;
	for (int i = 0; i < size; ++i)
	{
		undef &= v[i] == DefaultUndefinedValue;
	}
	if (undef) return DefaultUndefinedValue;

	// Compute median all values that are defined
	size_t actualValues = 0;

	std::vector<float> val;
	for (int i = 0; i < size; ++i)
	{
		if (v[i] != DefaultUndefinedValue)
		{
			val.push_back(v[i]);
			actualValues++;
		}
	}

	if (actualValues == 1)
		return val.at(0);

	std::sort(val.begin(), val.end());
	return val.at(actualValues / 2);
}

void VisualizationUtils::cellCenterVolume(const std::shared_ptr<Volume>& volume, std::shared_ptr<Project>& project)
{
	if (!volume) return;

	for (int i = 0; i < volume->getPropertyVolumeDataList().size(); i++)
	{
		PropertyVolumeData propVolData = volume->getPropertyVolumeDataList().at(i);
		const std::shared_ptr<VolumeData>& volData = propVolData.second;
		const std::shared_ptr<const Property>& propertyIO = propVolData.first;

		// Don't cell-center depth
		if (propertyIO->getName() == "Depth") continue;

		filterFunc filter, filterUndef;
		getFilterFunc(propertyIO, filter, filterUndef);

		std::shared_ptr<CauldronIO::VolumeData> cellCenteredVol = VisualizationUtils::cellCenterVolumeData(volData, filter, filterUndef);

		if (cellCenteredVol)
		{
			project->addGeometry(cellCenteredVol->getGeometry());
			CauldronIO::PropertyVolumeData propVolDataCentered(propertyIO, cellCenteredVol);

			// Replace with new map
			volume->replaceAt(i, propVolDataCentered);
		}
	}
}

std::shared_ptr<VolumeData> VisualizationUtils::cellCenterVolumeData(const std::shared_ptr<VolumeData>& volData, filterFunc filter, filterFunc filterUndef)
{
	// construct a new volume
	assert(volData->isRetrieved());
	auto geometry = volData->getGeometry();

	size_t numI = geometry->getNumI();
	size_t numJ = geometry->getNumJ();
	size_t numK = geometry->getNumK();

	// The volume should be large enough for this operation
	if (numI < 2 || numJ < 2 || numK < 2)
		return std::shared_ptr<VolumeData>();

	assert(!geometry->isCellCentered());
	std::shared_ptr<Geometry3D> newGeometry(new Geometry3D(numI - 1, numJ - 1, numK - 1, geometry->getFirstK(), geometry->getDeltaI(), geometry->getDeltaJ(),
		geometry->getMinI(), geometry->getMinJ(), true));

	std::shared_ptr<CauldronIO::VolumeData> newVolume(new CauldronIO::VolumeDataNative(newGeometry));

	if (volData->isConstant())
	{
		newVolume->setConstantValue(volData->getConstantValue());
		return newVolume;
	}

	float* newData = new float[newGeometry->getNumI() * newGeometry->getNumJ() * newGeometry->getNumK()];

	// average the values
	computeVolumeData(volData, geometry, newData, 0, filter, filterUndef);

	newVolume->setData_IJK(newData);
	delete[] newData;

	return newVolume;

}

void VisualizationUtils::cellCenterFormationVolumes(const std::shared_ptr<SnapShot>& snapShot, std::shared_ptr<Project> project)
{
	auto& formationVolumeList = snapShot->getFormationVolumeList();
	PropertyList propMerged;

	// for each property, see if we can find the propertyvolumedata
	for (auto& prop : project->getProperties())
	{
		PropertyVolumeDataList propVolumeList;
		if (prop->getName() == "Depth") continue;

		// Loop over all formations
		for (auto& formationVol : formationVolumeList)
		{
			// Loop over all propertyvolumedata for this formation
			for (auto& propVol : formationVol.second->getPropertyVolumeDataList())
			{
				// Add if property matches
				if (propVol.first == prop)
				{
					propVolumeList.push_back(propVol);
				}
			}
		}

		if (propVolumeList.size() > 0)
		{
			// Merge if possible
			std::shared_ptr<VolumeData> volData = mergeVolumeDataList(propVolumeList, project);

			// Add to volume of snapshot
			if (volData)
			{
				// Create a volume if not existing				
				if (!snapShot->getVolume())
				{
					std::shared_ptr<Volume> volume(new Volume(None));
					snapShot->setVolume(volume);
				}

				snapShot->getVolume()->addPropertyVolumeData(PropertyVolumeData(prop, volData));
				propMerged.push_back(prop);
			}
		}
	}

	// Remove the formationVolumes of merged volumes
	for (auto& prop : propMerged)
	{
		// Loop over all formations
		for (auto& formationVol : formationVolumeList)
		{
			// Loop over all propertyvolumedata for this formation
			for (PropertyVolumeData& propVol : formationVol.second->getPropertyVolumeDataList())
			{
				// Add if property matches
				if (propVol.first == prop)
				{
					formationVol.second->removeVolumeData(propVol);
					break;
				}
			}
		}
	}
}

std::shared_ptr<VolumeData> VisualizationUtils::mergeVolumeDataList(PropertyVolumeDataList& propVolumeList, std::shared_ptr<Project> project)
{
	// Check order list
	std::vector<std::shared_ptr<VolumeData> > volDataList;

	// Find VolumeData with smallest start K
	size_t smallestStartK = 16384;
	size_t kIndex;

	for (size_t index = 0; index < propVolumeList.size(); index++)
	{
		if (propVolumeList.at(index).second->getGeometry()->getFirstK() < smallestStartK)
		{
			kIndex = index;
			smallestStartK = propVolumeList.at(index).second->getGeometry()->getFirstK();

			if (smallestStartK == 0) break;
		}
	}

	// Bail out if we found nothing
	if (smallestStartK == 16384)
		return std::shared_ptr<VolumeData>();

	volDataList.push_back(propVolumeList.at(kIndex).second);

	// Now find all the consecutive VolumeData's
	bool found;
	do
	{
		found = false;
		size_t nextK = volDataList.back()->getGeometry()->getLastK();

		for (auto& propVol : propVolumeList)
		{
			if (propVol.second->getGeometry()->getFirstK() == nextK)
			{
				found = true;
				volDataList.push_back(propVol.second);
				break;
			}
		}

	} while (found);

	if (volDataList.size() < propVolumeList.size())
		throw CauldronIOException("Non-continuous volume found - not merging");

	return mergeVolumes(propVolumeList[0].first, volDataList, project);
}


std::shared_ptr<VolumeData> VisualizationUtils::mergeVolumes(std::shared_ptr<const Property>& propertyIO, std::vector<std::shared_ptr<VolumeData> > volDataList, std::shared_ptr<Project> project)
{
	// Find k-range
	size_t startK = volDataList[0]->getGeometry()->getFirstK();
	size_t endK = volDataList.back()->getGeometry()->getLastK();

	auto geometry = volDataList[0]->getGeometry();

	size_t numI = geometry->getNumI();
	size_t numJ = geometry->getNumJ();
	size_t numK = 1 + endK - startK;

	// The volume should be large enough for this operation
	if (numI < 2 || numJ < 2 || numK < 2)
		return std::shared_ptr<VolumeData>();

	assert(!geometry->isCellCentered());
	std::shared_ptr<Geometry3D> newGeometry(new Geometry3D(numI - 1, numJ - 1, numK - 1, startK, geometry->getDeltaI(), geometry->getDeltaJ(),
		geometry->getMinI(), geometry->getMinJ(), true));

	std::shared_ptr<CauldronIO::VolumeData> newVolume(new CauldronIO::VolumeDataNative(newGeometry));

	// average the values
	float* newData = new float[newGeometry->getNumI() * newGeometry->getNumJ() * newGeometry->getNumK()];

	filterFunc filter, filterUndef;
	getFilterFunc(propertyIO, filter, filterUndef);

	for (size_t volIndex = 0; volIndex < volDataList.size(); volIndex++)
	{
		auto& volData = volDataList.at(volIndex);
		geometry = volData->getGeometry();
		size_t outputIndex = newVolume->computeIndex_IJK(0, 0, geometry->getFirstK());

		if (volData->isConstant())
		{
			size_t number = newGeometry->getNumI() * newGeometry->getNumJ() * (geometry->getNumK() - 1);
			std::fill(&newData[outputIndex], &newData[outputIndex + number - 1], volData->getConstantValue());
		}
		else
		{
			computeVolumeData(volData, geometry, newData, outputIndex, filter, filterUndef);
		}
	}

	newVolume->setData_IJK(newData);
	delete[] newData;

	return newVolume;
}

void VisualizationUtils::computeVolumeData(std::shared_ptr<VolumeData> volData, std::shared_ptr<Geometry3D>& geometry, float* newData, size_t outputIndex,
	filterFunc filter, filterFunc filterUndef)
{
	assert(volData->isRetrieved());
	assert(volData->hasDataIJK());

	size_t numI = geometry->getNumI();
	size_t numJ = geometry->getNumJ();

	const float* surface1;
	const float* surface2;

	surface2 = volData->getSurface_IJ(geometry->getFirstK());

	for (size_t k = geometry->getFirstK(); k < geometry->getLastK(); k++)
	{
		surface1 = surface2;
		surface2 = volData->getSurface_IJ(k + 1);
		const float* row12 = surface1;
		const float* row22 = surface2;

		for (int j = 0; j < numJ - 1; j++)
		{
			const float* row11 = row12;
			row12 = &surface1[numI*(j + 1)];
			const float* row21 = row22;
			row22 = &surface2[numI*(j + 1)];

			for (int i = 0; i < numI - 1; i++)
			{
				float result;

				// Sample all 8 neighboring nodes
				float v[8];
				v[0] = row11[i];
				v[1] = row11[i + 1];
				v[2] = row12[i];
				v[3] = row12[i + 1];
				v[4] = row21[i];
				v[5] = row21[i + 1];
				v[6] = row22[i];
				v[7] = row22[i + 1];

				if (v[0] == DefaultUndefinedValue || v[1] == DefaultUndefinedValue || v[2] == DefaultUndefinedValue || v[3] == DefaultUndefinedValue ||
					v[4] == DefaultUndefinedValue || v[5] == DefaultUndefinedValue || v[6] == DefaultUndefinedValue || v[7] == DefaultUndefinedValue)
					result = filterUndef(v, 8);
				else // for permeability we might take the median value
					result = filter(v, 8);

				newData[outputIndex++] = result;
			}
		}
	}
}

float VisualizationUtils::getFlowIJK(float* v, size_t size)
{
	return v[0];
}

void VisualizationUtils::getFilterFunc(const std::shared_ptr<const Property>& propertyIO, filterFunc& filter, filterFunc& filterUndef)
{
	filter = VisualizationUtils::getAverage;
	filterUndef = VisualizationUtils::getUndefAverage;

	if (propertyIO->getName() == "Permeability")
	{
		filter = VisualizationUtils::getMedian;
		filterUndef = VisualizationUtils::getUndefMedian;
	}
	else if (propertyIO->getName() == "FlowDirectionIJK")
	{
		filter = VisualizationUtils::getFlowIJK;
		filterUndef = VisualizationUtils::getFlowIJK;
	}
}

}
