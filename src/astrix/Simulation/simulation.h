/*! \file simulation.h
\brief Header file for Simulation class

\section LICENSE
Copyright (c) 2017 Sijme-Jan Paardekooper

This file is part of Astrix.

Astrix is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Astrix is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Astrix.  If not, see <http://www.gnu.org/licenses/>.*/
#ifndef ASTRIX_SIMULATION_H
#define ASTRIX_SIMULATION_H

#define BURGERS -1

namespace astrix {

class Mesh;
template <class T> class Array;
class Device;

//! Simulation: class containing simulation
/*! This is the basic class needed to run an Astrix simulation.  */

class Simulation
{
 public:
  //! Constructor for Simulation object.
  Simulation(int _verboseLevel, int _debugLevel,
             char *fileName, Device *_device, int restartNumber,
             int extraFlag);
  //! Destructor, releases all dynamically allocated
  ~Simulation();

  //! Run simulation
  void Run(real maxWallClockHours);

 private:
  //! GPU device available
  Device *device;

  //! Flag whether to use CUDA
  int cudaFlag;
  //! How much to output to screen
  int verboseLevel;
  //! Level of debugging
  int debugLevel;
  //! Extra user flag
  int extraFlag;

  //! Mesh on which to do simulation
  Mesh *mesh;

  //! Number of time steps taken
  int nTimeStep;

  //! Problem specification (see Common/definitions.h)
  /*! Read from input file: LIN: Linear wave,
      RT: Rayleigh-Taylor,
      KH: Kelvin-Helmholz,
      RIEMANN: 2D Riemann problem,
      SOD: Sod shock tube,
      VORTEX: Vortex advection,
      Converted to int using definitions in definitions.h.*/
  ProblemDefinition problemDef;

  //! Integration scheme (see Common/definitions.h)
  /*! Read from input file: N: N scheme, LDA: LDA scheme, B: blended scheme.*/
  IntegrationScheme intScheme;
  //! Order of accuracy in time (1 or 2)
  int integrationOrder;
  //! Mass matrix formulation to use (1, 2, 3 or 4)
  int massMatrix;
  //! Flag whether to use selective lumping
  int selectiveLumpFlag;
  //! Courant number
  real CFLnumber;
  //! Preference for using minimum/maximum value of blend parameter
  int preferMinMaxBlend;

  //! Number of space dimensions (fixed to 2)
  int nSpaceDim;

  //! Ratio of specific heats
  real specificHeatRatio;

  //! Time variable.
  real simulationTime;
  //! Maximum simulation time
  real maxSimulationTime;
  //! Time between 2D saves
  real saveIntervalTime;
  //! Time between 0D saves
  real saveIntervalTimeFine;
  //! Number of saves so far
  int nSave;
  //! Number of fine grain saves so far
  int nSaveFine;

  //! State vector at vertex
  Array <realNeq> *vertexState;
  //! Old state vector at vertex
  Array <realNeq> *vertexStateOld;
  //! Gravitational potential at vertex
  Array <real> *vertexPotential;
  //! Difference state vector at vertex
  Array <realNeq> *vertexStateDiff;
  //! Roe parameter vector
  Array <realNeq> *vertexParameterVector;

  //! Residual for N scheme
  Array <realNeq> *triangleResidueN;
  //! Residual for LDA scheme
  Array <realNeq> *triangleResidueLDA;
  //! Total residual
  Array <realNeq> *triangleResidueTotal;
  //! Shock sensor
  Array<real> *triangleShockSensor;
  //! Source contribution to residual
  Array <realNeq> *triangleResidueSource;

  //! Set up the simulation
  void Init(int restartNumber);
  //! Read input file
  void ReadInputFile(const char *fileName);

  //! Save current state
  void Save();
  //! Restore state from disc
  void Restore(int nRestore);
  //! Fine grain save
  void FineGrainSave();
  //! Make fine grain save file consistent when restoring
  void RestoreFine();
 //! Calculate Kelvin-Helmholtz diagnostics
  void KHDiagnostics(real& M, real& Ekin);
  //! Add eigenvector perturbation
  void KHAddEigenVector();

  //! Do one time step
  void DoTimeStep();

  //! Set initial conditions according to problemSpec.
  void SetInitial(real time);

  //! Calculate gravitational potential
  void CalcPotential();
  //! Calculate source term contribution to residual
  void CalcSource(Array<realNeq> *state);
  //! For every vertex, calculate the maximum allowed timestep.
  real CalcVertexTimeStep();

  //! Set reflecting boundary conditions
  void ReflectingBoundaries(real dt);
  //! Set boundary conditions using extrapolation
  void ExtrapolateBoundaries();
  //! Set non-reflecting boundaries
  void SetNonReflectingBoundaries();
  //! Set boundary conditions for 2D Riemann problem
  void SetRiemannBoundaries();
  //! Set boundary conditions for 2D Noh problem
  void SetNohBoundaries();

  //! Function to calculate Roe's parameter vector at all vertices.
  void CalculateParameterVector(int useOldFlag);
  //! Calculate space residual on triangles
  void CalcResidual();
  //! Calculate space-time residual N plus total
  void CalcTotalResNtot(real dt);
  //! Calculate space-time LDA residual
  void CalcTotalResLDA();
  //! Add selective lump contribution to residual
  void SelectLump(real dt, int massMatrix, int selectLumpFlag);
  //! Add contribution F3/F4 mass matrix to total residual
  void MassMatrixF34Tot(real dt, int massMatrix);
  //! Add contribution F3/F4 mass matrix to residual
  void MassMatrixF34(real dt, int massMatrix);

  //! Update state at nodes
  void UpdateState(real dt, int RKStep);
  //! Add residue to state at vertices
  void AddResidue(real dt);
  //! Find unphysical state and put in vertexUnphysicalFlag
  void FlagUnphysical(Array<int> *vertexUnphysicalFlag);
  //! Replace LDA with N wherever unphysical state
  void ReplaceLDA(Array<int> *vertexUnphysicalFlag, int RKStep);
  //! Calculate shock sensor for BX scheme
  void CalcShockSensor();
  //! Find minimum and maximum velocity in domain
  void FindMinMaxVelocity(real& minVel, real& maxVel);

  //! Refine mesh
  void Refine();
  //! Coarsen mesh
  void Coarsen(int maxCycle);

  //! In state vector, replace total energy with pressure
  void ReplaceEnergyWithPressure();
  //! In state vector, replace pressure with total energy
  void ReplacePressureWithEnergy();

  //! Calculate total mass in domain
  real TotalMass();
};

}  // namespace astrix
#endif  // ASTRIX_SIMULATION_H
