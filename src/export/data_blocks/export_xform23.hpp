/*
 *     Copyright (c) 2013 Battelle Memorial Institute
 *     Licensed under modified BSD License. A copy of this license can be found
 *     in the LICENSE file in the top level directory of this distribution.
 */
/*
 * export_xform23.hpp
 *
 * This class exports transformer data in PSS/E v23 format
 *
 *  Created on: July 9, 2021
 *      Author: Bruce Palmer
 */

#ifndef EXPORTXFORM23_HPP_
#define EXPORTXFORM23_HPP_

#include <iostream>
#include <string>
#include <vector>
#include <map>

#include "gridpack/parallel/communicator.hpp"
#include "gridpack/component/data_collection.hpp"
#include "gridpack/parser/dictionary.hpp"
#include "gridpack/network/base_network.hpp"
#include "gridpack/export/base_export.hpp"

namespace gridpack {
namespace expnet {

template <class _network>
class ExportXform23
{
  public:

    /**
     * Constructor
     */
    explicit ExportXform23(boost::shared_ptr<_network> network) :
      p_network(network), p_comm(network->communicator())
    {
    }

    /**
     * Destructor
     */
    virtual ~ExportXform23(){}

    /**
     * export text to fstream
     * @param fout stream object to export data
     * @param text_data vector of text strings that should be written out
     *                  consecutively, based on values in text_line data
     *                  data structures.
     */
    void writeXformBlock(std::ofstream &fout)
    {
      BaseExport<_network> exprt(p_comm);
      int me = p_comm.rank();
      // Form vector of text strings based from data collection objects
      int nbranch = p_network->numBranches();
      gridpack::component::DataCollection *data;
      int i, j;
      char buf[MAX_STRING_SIZE];
      std::vector<text_line> text_data;
      for (i=0; i<nbranch; i++) {
        if (p_network->getActiveBranch(i)) {
          data = p_network->getBranchData(i).get();
          double rval;
          int ival;
          int idx1, idx2;
          std::string sval;
          DoubleComplex zval;
          char *ptr;
          int nline = 0;
          double tap_ratio;
          data->getValue(BRANCH_NUM_ELEMENTS,&nline);
          for (j=0; j<nline; j++) {
            ptr = buf;
            data->getValue(BRANCH_TAP,&tap_ratio,j);
            bool is_xform = data->getValue(TRANSFORMER_WINDV1,&rval,j);
            is_xform = is_xform && data->getValue(TRANSFORMER_WINDV2,&rval,j);
            if (is_xform || (tap_ratio != 0.0 && tap_ratio != 1.0)) {
              data->getValue(BRANCH_FROMBUS,&idx1);
              data->getValue(BRANCH_TOBUS,&idx2);
              ival = 1;
              data->getValue(BRANCH_SWITCHED,&ival,j);
              // First line
              if (ival == 1) {
                sprintf(ptr,"%d, %d, 0,",idx1,idx2);
              } else {
                sprintf(ptr,"%d, %d, 0,",idx2,idx1);
              }
              ptr += strlen(ptr);
              sval = " 1";
              data->getValue(BRANCH_CKT,&sval,j);
              sprintf(ptr," \'%s\',",sval.c_str());
              ptr += strlen(ptr);
              ival = idx1;
              if (!data->getValue(TRANSFORMER_CONTROL,&ival,j)) {
                data->getValue(TRANSFORMER_CONT1,&ival,j);
              }
              sprintf(ptr," %d,",ival);
              ptr += strlen(ptr);
              rval = 1.1;
              data->getValue(TRANSFORMER_RMA,&rval,j);
              sprintf(ptr," %f,",rval);
              ptr += strlen(ptr);
              rval = 0.9;
              data->getValue(TRANSFORMER_RMI,&rval,j);
              sprintf(ptr," %f,",rval);
              ptr += strlen(ptr);
              rval = 1.1;
              data->getValue(TRANSFORMER_VMA,&rval,j);
              sprintf(ptr," %f,",rval);
              ptr += strlen(ptr);
              rval = 0.9;
              data->getValue(TRANSFORMER_VMI,&rval,j);
              sprintf(ptr," %f,",rval);
              ptr += strlen(ptr);
              rval = 0.0;
              data->getValue(TRANSFORMER_STEP,&rval,j);
              sprintf(ptr," %f,",rval);
              ptr += strlen(ptr);
              ival = 0;
              data->getValue(TRANSFORMER_TAB,&ival,j);
              sprintf(ptr," %d\n",ival);
              text_line text;
              strcpy(text.text,buf);
              text.global_idx = p_network->getGlobalBranchIndex(i);
              text.device_idx = j;
              text_data.push_back(text);
            }
          }
        }
      }
      if (me == 0) {
        fout << "0 / END LINE DATA, BEGIN TRANSFORMER ADJUSTMENT DATA" << std::endl;
      }
      exprt.writeDataBlock(fout, text_data);
    }

  private:
    boost::shared_ptr<_network>      p_network;

    gridpack::parallel::Communicator p_comm;
};

} /* namespace export */
} /* namespace gridpack */

#endif /* EXPORTXFORM23_HPP_ */
