// -*-c++-*-
/*! \file value.cu
\brief Functions for setting all elements to single value

\section LICENSE
Copyright (c) 2017 Sijme-Jan Paardekooper

This file is part of Astrix.

Astrix is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Astrix is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Astrix.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "./array.h"
#include "../Common/cudaLow.h"

namespace astrix {

//######################################################################
//! Kernel: Set all array elements to specific value
//######################################################################

template<class T>
__global__ void
devSetToValue(int N, T *array, T value,
              unsigned int startIndex, unsigned int endIndex,
              int realSize, int nDims)
{
  for (int n = 0; n < nDims; n++) {
    unsigned int i = blockIdx.x*blockDim.x + threadIdx.x + startIndex;

    while (i < endIndex) {
      array[i + n*realSize] = value;
      i += gridDim.x*blockDim.x;
    }
  }
}

//##########################################
// Set all array elements to specific value
//##########################################

template <class T>
void Array<T>::SetToValue(T value)
{
  if (cudaFlag == 1) {
    int nBlocks = 128;
    int nThreads = 128;

    // Base nThreads and nBlocks on maximum occupancy
    cudaOccupancyMaxPotentialBlockSize(&nBlocks, &nThreads,
                                       devSetToValue<T>,
                                       (size_t) 0, 0);

    devSetToValue<<<nBlocks, nThreads>>>(size, deviceVec, value,
                                         0, size,
                                         realSize, nDims);
    gpuErrchk( cudaPeekAtLastError() );
    gpuErrchk( cudaDeviceSynchronize() );
  }

  if (cudaFlag == 0) {
    for (unsigned int n = 0; n < nDims; n++)
      for (unsigned int i = 0; i < size; i++)
        hostVec[i + n*realSize] = value;
  }
}

//##########################################################
// Set array elements to specific value from certain offset
//##########################################################

template <class T>
void Array<T>::SetToValue(T value, unsigned int startIndex,
                          unsigned int endIndex)
{
  if (cudaFlag == 1) {
    int nBlocks = 128;
    int nThreads = 128;

    // Base nThreads and nBlocks on maximum occupancy
    cudaOccupancyMaxPotentialBlockSize(&nBlocks, &nThreads,
                                       devSetToValue<T>,
                                       (size_t) 0, 0);

    devSetToValue<<<nBlocks, nThreads>>>(size, deviceVec, value,
                                         startIndex, endIndex,
                                         realSize, nDims);
    gpuErrchk( cudaPeekAtLastError() );
    gpuErrchk( cudaDeviceSynchronize() );
  }

  if (cudaFlag == 0) {
    for (unsigned int n = 0; n < nDims; n++)
      for (unsigned int i = startIndex; i < endIndex; i++)
        hostVec[i + n*realSize] = value;
  }
}

//###################################################
// Instantiate
//###################################################

template void Array<float>::SetToValue(float value);
template void Array<float>::SetToValue(float value,
                                       unsigned int startIndex,
                                       unsigned int endIndex);

//###################################################

template void Array<double>::SetToValue(double value);
template void Array<double>::SetToValue(double value,
                                        unsigned int startIndex,
                                        unsigned int endIndex);

//###################################################

template void Array<int>::SetToValue(int value);
template void Array<int>::SetToValue(int value,
                                     unsigned int startIndex,
                                     unsigned int endIndex);

//###################################################

template void Array<unsigned int>::SetToValue(unsigned int value);
template void Array<unsigned int>::SetToValue(unsigned int value,
                                              unsigned int startIndex,
                                              unsigned int endIndex);
}  // namespace astrix
