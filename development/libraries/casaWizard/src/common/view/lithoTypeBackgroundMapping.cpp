//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "lithoTypeBackgroundMapping.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QPainter>
#include <QSvgRenderer>

namespace casaWizard
{

LithoTypeBackgroundMapping* LithoTypeBackgroundMapping::instance_ = nullptr;

LithoTypeBackgroundMapping::LithoTypeBackgroundMapping() :
  mapping_{}
{
  initializeMapping();
}

void LithoTypeBackgroundMapping::initializeMapping()
{
  const QJsonArray lithologyArray = readJson();

  const QString prefix = ":/lithoTypeIcons/";
  for (const QJsonValue& lithology : lithologyArray)
  {
    const QJsonObject lithologyObject = lithology.toObject();
    const QString name = lithologyObject.value("name").toString();
    const QString pattern = lithologyObject.value("visualization").toObject().value("pattern").toString();
    const QString color = lithologyObject.value("visualization").toObject().value("bgColour").toString();
    mapping_.insert({name, createBrush(prefix + pattern, color)});
  }
}

QJsonArray LithoTypeBackgroundMapping::readJson() const
{
  QFile loadFile(":/lithotypes.json");
  loadFile.open(QIODevice::ReadOnly);
  const QByteArray jsonData = loadFile.readAll();
  loadFile.close();

  const QJsonDocument document = QJsonDocument::fromJson(jsonData);
  const QJsonObject object = document.object();
  const QJsonValue value = object.value("lithotypes");
  return value.toArray();
}

QBrush LithoTypeBackgroundMapping::createBrush(QString patternFileName, QColor backgroundColor) const
{
  QBrush brush;
  QSvgRenderer svg(patternFileName);
  QSize size = svg.defaultSize();

  QImage img(size, QImage::Format_ARGB32);
  img.fill(backgroundColor);
  QPainter painter(&img);
  svg.render(&painter);
  brush.setTextureImage(img);
  return brush;
}

void LithoTypeBackgroundMapping::getBackgroundBrush(const QString& lithotypeName, QBrush& brush)
{
  if (!instance_)
  {
    instance_ = new LithoTypeBackgroundMapping();
  }

  try
  {
    brush = instance_->mapping_.at(lithotypeName);
  }
  catch (std::out_of_range)
  {
    // if there is no background brush for the lithotype name, just use the default
  }
}

void LithoTypeBackgroundMapping::deleteInstance()
{
  if (instance_)
  {
    delete instance_;
  }
}

} // namespace casaWizard
