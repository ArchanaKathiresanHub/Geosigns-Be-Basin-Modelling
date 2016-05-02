//
// Copyright (C) 2012-2015 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "SegYConversionDialog.h"
#include "Seismic.h"

#include <VolumeViz/readers/SoVRSegyFileReader.h>
#include <VolumeViz/converters/SoVolumeConverter.h>
#include <LDM/converters/SoConverterParameters.h>

class SegyConverter : public SoVolumeConverter
{
  SoRef<SoVRSegyFileReader> m_reader;
  double m_minDepth;
  double m_maxDepth;

  int m_progress;
  QProgressBar* m_bar;

  static void xmlCallback(FILE* file, void* userData)
  {
    reinterpret_cast<SegyConverter*>(userData)->onXmlCallback(file);
  }

  void onXmlCallback(FILE* file)
  {
    CustomLDMInfo ldmInfo;
    m_reader->getP1P2P3Coordinates(ldmInfo.p1, ldmInfo.p2, ldmInfo.p3, ldmInfo.p4);
    ldmInfo.minDepth = m_minDepth;
    ldmInfo.maxDepth = m_maxDepth;

    ldmInfo.writeXML(file);
  }

protected:

  virtual SoVolumeReader* getReader(const SbString& filename, const SbString& fileExt)
  {
    return m_reader.ptr();
  }

  virtual SoVolumeConverter::Abort progress(int n, int total)
  {
    int progress = (100 * n) / total;
    if (m_bar && progress != m_progress)
    {
      m_progress = progress;
      m_bar->setValue(progress);
      //qApp->processEvents();
    }

    return SoConverter::CVT_CONTINUE;
  }

public:

  SegyConverter(SoRef<SoVRSegyFileReader> reader)
    : m_reader(reader)
    , m_minDepth(0.0)
    , m_maxDepth(0.0)
    , m_progress(0)
    , m_bar(nullptr)
  {
    m_maxDepth = 3722.5;
    m_minDepth = -1.49;

    setXmlCallback(xmlCallback, this);
  }

  void setDepthRange(double minDepth, double maxDepth)
  {
    m_minDepth = minDepth;
    m_maxDepth = maxDepth;
  }

  void setProgressBar(QProgressBar* bar)
  {
    m_bar = bar;
  }
};

void SegYConversionDialog::onCancel()
{
  reject();
}

void SegYConversionDialog::onConvert()
{
  double minDepth = -m_dlg.lineEditMaxDepth->text().toDouble();
  double maxDepth = -m_dlg.lineEditMinDepth->text().toDouble();

  SoConverterParameters params;
  params.setOutputDataFormat(SbDataType::UNSIGNED_BYTE);
  params.setInputFileName(m_reader->getFilename());

  SegyConverter converter(m_reader);
  converter.setDepthRange(minDepth, maxDepth);
  converter.setProgressBar(m_dlg.progressBar);
  converter.convert(&params);

  accept();
}

SegYConversionDialog::SegYConversionDialog(QWidget* parent, Qt::WindowFlags f)
	: QDialog(parent, f)
{
  m_dlg.setupUi(this);

  m_dlg.lineEditMinDepth->setValidator(new QDoubleValidator(this));
  m_dlg.lineEditMaxDepth->setValidator(new QDoubleValidator(this));

  connect(m_dlg.pushButtonCancel, SIGNAL(clicked()), this, SLOT(onCancel()));
  connect(m_dlg.pushButtonConvert, SIGNAL(clicked()), this, SLOT(onConvert()));
}

void SegYConversionDialog::setInputFileName(const QString& filename)
{
  m_filename = filename;

  m_reader = new SoVRSegyFileReader;
  auto ascii = filename.toAscii();
  m_reader->setFilename(ascii.data());

  SbString header = m_reader->getSegyTextHeader();
  m_dlg.textEditHeader->setLineWrapMode(QTextEdit::FixedColumnWidth);
  m_dlg.textEditHeader->setLineWrapColumnOrWidth(80);
  m_dlg.textEditHeader->insertPlainText(header.getString());
}

