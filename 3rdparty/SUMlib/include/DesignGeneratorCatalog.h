// Copyright 2012, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#ifndef SUMLIB_DESIGNGENERATORCATALOG_H
#define SUMLIB_DESIGNGENERATORCATALOG_H

#include <vector>

#include "DesignGenerator.h"
#include "SUMlib.h"

using SUMlib::DesignGenerator;

namespace SUMlib {

class INTERFACE_SUMLIB_DEBUG DesignGeneratorCatalog
{
   public:

      /**
       * Fetch the DesignGeneratorCatalog singleton instance
       * @return the DesignGeneratorCatalog instance. Constructed if necessary
       */
      static const DesignGeneratorCatalog* GetInstance();

      /**
       * Get factorial designs for specified number of parameters, runs and
       * resolution
       * @param [in] nbPar          the number of parameters
       * @param [in] minNbRuns      the minimum number of runs needed for the design
       * @param [in] minResolution  the minimum resolution of the design
       * @param [out] dgvec         the design generators
       */
      void getFactorialDesigns(
            const unsigned int nbPar,
            const unsigned int minNbRuns,
            const unsigned int minResolution,
            std::vector<DesignGenerator> &dgvec
            ) const;

   private:

      /**
       * Constructor
       */
      DesignGeneratorCatalog();

      /**
       * The singleton instance
       */
      static DesignGeneratorCatalog *theCatalog;

      /**
       * The contents of the catalog as a series of strings
       */
      static const char *theFactorialDesigns[];

      /**
       * The data structure holding the catalog
       */
      std::vector<DesignGenerator>m_catalog;
};

} // namespace SUMlib


#endif // SUMLIB_DESIGNGENERATORCATALOG_H
