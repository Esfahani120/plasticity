//base class for elliptic boundary value problem implementation
#ifndef ELLIPTICBVP_H
#define ELLIPTICBVP_H

//dealii headers
#include "dealIIheaders.h"
#include "userInputParameters.h"

using namespace dealii;

//compiler directives to handle warnings
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wextra"
#pragma GCC diagnostic pop

//define data types
typedef PETScWrappers::MPI::Vector vectorType;
typedef PETScWrappers::MPI::SparseMatrix matrixType;
//LA::MPI::SparseMatrix
//LA::MPI::Vector

//
//base class for elliptic PDE's
//
template <int dim>
class ellipticBVP : public Subscriptor
{
public:
  ellipticBVP(userInputParameters _userInputs);
  ~ellipticBVP();
  void run   ();

protected:

  //parallel objects
  MPI_Comm   mpi_communicator;
  IndexSet   locally_owned_dofs;
  IndexSet   locally_owned_dofs_Scalar;
  IndexSet   locally_relevant_dofs;
  IndexSet   locally_relevant_dofs_Scalar;

  //User input parameters object
  userInputParameters userInputs;

  //FE data structres
  parallel::distributed::Triangulation<dim> triangulation;
  FESystem<dim>      FE;
  FESystem<dim>      FE_Scalar;
  DoFHandler<dim>    dofHandler;
  DoFHandler<dim>    dofHandler_Scalar;

  //methods
  virtual void mesh();
  void init();
  void assemble();
  void assemble2();
  #if ((DEAL_II_VERSION_MAJOR < 9)||(DEAL_II_VERSION_MINOR < 1))
  ConstraintMatrix   constraints;
  ConstraintMatrix   constraintsMassMatrix;
  void solveLinearSystem(ConstraintMatrix& constraintmatrix, matrixType& A, vectorType& b, vectorType& x, vectorType& xGhosts, vectorType& dxGhosts);
  void solveLinearSystem2(ConstraintMatrix& constraintmatrix, matrixType& A, vectorType& b, vectorType& x, vectorType& xGhosts, vectorType& dxGhosts);
  #else
  AffineConstraints<double>   constraints;
  AffineConstraints<double>   constraintsMassMatrix;
  void solveLinearSystem(AffineConstraints<double>& constraintmatrix, matrixType& A, vectorType& b, vectorType& x, vectorType& xGhosts, vectorType& dxGhosts);
  void solveLinearSystem2(AffineConstraints<double>& constraintmatrix, matrixType& A, vectorType& b, vectorType& x, vectorType& xGhosts, vectorType& dxGhosts);
  #endif




  bool solveNonLinearSystem();
  void solve();
  void output();
  void initProjection();
  void projection();
  void markBoundaries();

  //virtual methods to be implemented in derived class
  //method to calculate elemental Jacobian and Residual,
  //which should be implemented in the derived material model class

  virtual void getElementalValues(FEValues<dim>& fe_values,
    unsigned int dofs_per_cell,
    unsigned int num_quad_points,
    FullMatrix<double>& elementalJacobian,
    Vector<double>&     elementalResidual) = 0;

    virtual void getElementalValues2(FEValues<dim>& fe_values,
      unsigned int dofs_per_cell,
      unsigned int num_quad_points,
      Vector<double>&     elementalResidual) = 0;
      //methods to allow for pre/post iteration updates
      virtual void updateBeforeIteration();
      virtual void updateAfterIteration();
      virtual bool testConvergenceAfterIteration();
      //methods to allow for pre/post increment updates
      virtual void updateBeforeIncrement();
      virtual void updateAfterIncrement();

      //methods to apply dirichlet BC's and initial conditions
      void applyDirichletBCs();
      void applyInitialConditions();
      void setBoundaryValues(const Point<dim>& node, const unsigned int dof, bool& flag, double& value);

      ///////These functions are for DIC BCs evaluation
      void bcFunction1(double _yval, double &value_x, double &value_y, double _currentIncr);
      void bcFunction2(double _yval, double &value_x, double &value_y, double _currentIncr);
      void bcFunction3(double _yval, double &value_x, double &value_y, double _currentIncr);
      void bcFunction4(double _yval, double &value_x, double &value_y, double _currentIncr);


      std::map<types::global_dof_index, Point<dim> > supportPoints;

      //parallel data structures
      vectorType solution, oldSolution, residual;
      vectorType solutionWithGhosts, solutionIncWithGhosts;
      matrixType jacobian;

      // Boundary condition variables
      std::vector<std::vector<bool>> faceDOFConstrained;
      std::vector<std::vector<double>> deluConstraint;

      FullMatrix<double> tabularDisplacements;
      unsigned int timeCounter;
      double currentTime;
      /////DIC bc names
      FullMatrix<double> bc_new1,bc_new2,bc_new3,bc_new4;

      FullMatrix<double> Fprev=IdentityMatrix(dim);
      FullMatrix<double> F,deltaF;
      FullMatrix<double> targetVelGrad;
      //misc variables
      double delT,totalT,cycleTime;
      unsigned int currentIteration, currentIncrement;
      unsigned int totalIncrements;
      bool resetIncrement;
      double loadFactorSetByModel;
      double totalLoadFactor;

      //parallel message stream
      ConditionalOStream  pcout;

      //compute-time logger
      TimerOutput computing_timer;

      //output variables
      //solution name array
      std::vector<std::string> nodal_solution_names;
      std::vector<DataComponentInterpretation::DataComponentInterpretation> nodal_data_component_interpretation;

      //post processing
      unsigned int numPostProcessedFields;
      unsigned int numPostProcessedFieldsAtCellCenters;
      //postprocessed scalar variable name array (only scalar variables supported currently, will be extended later to vectors and tensors, if required.)
      std::vector<std::string> postprocessed_solution_names;
      //postprocessing data structures
      std::vector<vectorType*> postFields, postFieldsWithGhosts, postResidual;
      matrixType massMatrix;
      Table<4,double> postprocessValues;
      Table<2,double> postprocessValuesAtCellCenters;

      //user model related variables and methods
      #ifdef enableUserModel
      unsigned int numQuadHistoryVariables;
      Table<3, double> quadHistory;
      virtual void initQuadHistory();
      #endif
    };

    #endif
