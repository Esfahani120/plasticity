//base class for elliptic boundary value problem implementation
#ifndef ELLIPTICPDE_H
#define ELLIPTICPDE_H

//general headers
#include <fstream>
#include <sstream>

//dealii headers
#include "dealIIheaders.h"

//define data types  
typedef PETScWrappers::MPI::Vector vectorType;

using namespace dealii;

//
//base class for elliptic PDE's
//
template <int dim>
class EllipticBVP:public Subscriptor
{
 public:
  EllipticBVP(); 
  ~EllipticBVP(); 
  void init  ();
  void solve ();

 protected:
  void assemble (bool firstIteration);
  void solve ();
  void outputResults  ();

  parallel::distributed::Triangulation<dim> triangulation;
  std::vector<FESystem<dim>*>          FESet;
  std::vector<const ConstraintMatrix*> constraintsSet;
  std::vector<const DoFHandler<dim>*>  dofHandlersSet;
  std::vector<const IndexSet*>         locally_relevant_dofsSet;
  std::vector<vectorType*>             solutionSet, residualSet;

  //matrix free objects
  MatrixFree<dim,double>               matrixFreeObject;
  vectorType                           invM;
  
  //matrix free methods
  unsigned int implicitFieldIndex;
  void computeInvM();
  void updateRHS();
  void computeRHS(const MatrixFree<dim,double> &data, 
		  std::vector<vectorType*> &dst, 
		  const std::vector<vectorType*> &src,
		  const std::pair<unsigned int,unsigned int> &cell_range) const;
  template <typename T>
    void computeLHS(const MatrixFree<dim,double> &data, 
		    vectorType &dst, 
		    const vectorType &src,
		    const std::pair<unsigned int,unsigned int> &cell_range) const;
  
  //virtual methods to be implemented in derived classe
  //methods to calculate RHS (implicit/explicit), LHS(implicit)
  virtual void getRHS(std::map<std::string, typeScalar*>  valsScalar,	\
		      std::map<std::string, typeVector*>  valsVector,	\
		      unsigned int q) const = 0;  
  virtual void getLHS(typeScalar& vals, unsigned int q) const;  
  virtual void getLHS(typeVector& vals, unsigned int q) const;  
  
  //methods to apply dirichlet BC's
  virtual void markBoundaries();
  virtual void applyDirichletBCs();
  virtual void applyInitialConditions();

  //utility functions
  //return index of given field name if exists, else throw error
  unsigned int getFieldIndex(std::string _name);

  //residual, matrix-vector computation variables
  std::map<std::string,bool> getValue, setValue;
  std::map<std::string,bool> getGradient, setGradient;

  //variables for time dependent problems 
  //isTimeDependentBVP flag is used to see if invM, time steppping in
  //run(), etc are necessary
  bool isTimeDependentBVP;
  double timeStep, currentTime, finalTime;
  unsigned int currentIncrement, totalIncrements;
  
  //parallel message stream
  ConditionalOStream  pcout;  
  //compute time log
  TimerOutput computing_timer;
};

//other matrixFree headers 
//(these are source files, which will are temporarily treated as
//header files till library packaging scheme is finalized)
#include "../src/matrixfree/matrixFreePDE.cc"
#include "../src/matrixfree/init.cc"
#include "../src/matrixfree/invM.cc"
#include "../src/matrixfree/computeLHS.cc"
#include "../src/matrixfree/computeRHS.cc"
#include "../src/matrixfree/solve.cc"
#include "../src/matrixfree/solveIncrement.cc"
#include "../src/matrixfree/outputResults.cc"
#include "../src/matrixfree/markBoundaries.cc"
#include "../src/matrixfree/boundaryConditions.cc"
#include "../src/matrixfree/initialConditions.cc"
#include "../src/matrixfree/utilities.cc"

#endif
