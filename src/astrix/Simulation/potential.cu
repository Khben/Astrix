// -*-c++-*-
/*! \file potential.cu
\brief File containing function to calculate external gravitational potential at vertices.

\section LICENSE
Copyright (c) 2017 Sijme-Jan Paardekooper

This file is part of Astrix.

Astrix is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Astrix is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Astrix.  If not, see <http://www.gnu.org/licenses/>.*/
#include <iostream>

#include "../Common/definitions.h"
#include "../Array/array.h"
#include "../Mesh/mesh.h"
#include "./simulation.h"
#include "../Common/cudaLow.h"
#include "./Param/simulationparameter.h"

namespace astrix {

//######################################################################
/*! \brief Calculate external gravitational potential at vertex \a i

\param i Vertex to calculate gravitational potential at
\param problemDef Problem definition
\param *pVc Pointer to coordinates of vertices
\param *pVpot Pointer to gravitational potential (output)*/
//######################################################################

__host__ __device__
void CalcPotentialSingle(int i, ProblemDefinition problemDef,
                         const real2 *pVc, real *pVpot)
{
  real zero = (real) 0.0;
  real tenth = (real) 0.1;

  pVpot[i] = zero;

  if (problemDef == PROBLEM_SOURCE) pVpot[i] = tenth*pVc[i].y;
  /*
  if (problemDef == PROBLEM_SOURCE) {
    real x = pVc[i].x;
    real y = pVc[i].y;

    real xc = 5.0;
    real yc = 5.0;

    real beta = 5.0;

    real r = sqrt((x - xc)*(x - xc) +
                  (y - yc)*(y - yc)) + (real) 1.0e-10;

    pVpot[i] = -0.125*beta*beta*exp(1.0 - r*r)/(M_PI*M_PI);
  }
  */
}

//######################################################################
/*! \brief Kernel calculating external gravitational potential at vertices

\param nVertex Total number of vertices in Mesh
\param problemDef Problem definition
\param *pVc Pointer to coordinates of vertices
\param *pVpot Pointer to gravitational potential (output)*/
//######################################################################

__global__ void
devCalcPotential(int nVertex, ProblemDefinition problemDef,
                 const real2 *pVc, real *vertPot)
{
  // n = vertex number
  int n = blockIdx.x*blockDim.x + threadIdx.x;

  while (n < nVertex) {
    CalcPotentialSingle(n, problemDef, pVc, vertPot);

    n += blockDim.x*gridDim.x;
  }
}

//#########################################################################
/*! Calculate external gravitational potential at vertices, based on vertex
coordinates and problem definition.*/
//#########################################################################

template <class realNeq, ConservationLaw CL>
void Simulation<realNeq, CL>::CalcPotential()
{
  int nVertex = mesh->GetNVertex();
  real *vertPot = vertexPotential->GetPointer();
  const real2 *pVc = mesh->VertexCoordinatesData();
  ProblemDefinition p = simulationParameter->problemDef;

  if (cudaFlag == 1) {
    int nBlocks = 128;
    int nThreads = 128;

    // Base nThreads and nBlocks on maximum occupancy
    cudaOccupancyMaxPotentialBlockSize(&nBlocks, &nThreads,
                                       devCalcPotential,
                                       (size_t) 0, 0);

    devCalcPotential<<<nBlocks, nThreads>>>
      (nVertex, p, pVc, vertPot);

    gpuErrchk( cudaPeekAtLastError() );
    gpuErrchk( cudaDeviceSynchronize() );
  } else {
    for (int i = 0; i < nVertex; i++)
      CalcPotentialSingle(i, p, pVc, vertPot);
  }
}

//##############################################################################
// Instantiate
//##############################################################################

template void Simulation<real, CL_ADVECT>::CalcPotential();
template void Simulation<real, CL_BURGERS>::CalcPotential();
template void Simulation<real3, CL_CART_ISO>::CalcPotential();
template void Simulation<real4, CL_CART_EULER>::CalcPotential();

}  // namespace astrix
