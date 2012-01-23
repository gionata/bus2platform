/*! \file ConstraintProgrammingListGraphColoring.h
 *  \brief
 *  \author Gionata Massi <massi@diiga.univpm.it>
 */

#ifndef ConstraintProgrammingListGraphColoring_H_
#define ConstraintProgrammingListGraphColoring_H_

#include <vector>
#include <algorithms>


#include <gecode/driver.hh>
#include <gecode/int.hh>
using namespace Gecode;

class GraphColorSpec {
public:
  int  n_v; ///< Number of nodes
//  int* e;   ///< Edges
  int* c;   ///< Cliques
  GraphColorSpec(int n_v0, /*int* e0,*/ int* c0): n_v(n_v0),/* e(e0),*/ c(c0) {}
  bool solution(int *&color)
};

//GraphColorSpec g1;

/**
 * \brief %Example: Clique-based graph coloring
 *
 * \ingroup Example
 *
 */
class GraphColor : public MinimizeScript {
private:
  const GraphColorSpec& g;
  /// Color of nodes
  IntVarArray v;
  /// Number of colors
  IntVar m;
public:
  /// Model variants
  enum {
    MODEL_NONE,  ///< No lower bound
    MODEL_CLIQUE ///< Use maximal clique size as lower bound
  };
  /// Branching to use for model
  enum {
    BRANCH_DEGREE,      ///< Choose variable with largest degree
    BRANCH_SIZE,        ///< Choose variable with smallest size
    BRANCH_SIZE_DEGREE, ///< Choose variable with smallest size/degree
    BRANCH_SIZE_AFC,    ///< Choose variable with smallest size/degree
  };
  /// The actual model
  GraphColor(const SizeOptions& opt, GraphColorSpec &gcs): g(gcs),  v(*this,g.n_v,0,g.n_v), m(*this,0,g.n_v)  {
    rel(*this, v, IRT_LQ, m);
    //for (int i = 0; g.e[i] != -1; i += 2)
    //  rel(*this, v[g.e[i]], IRT_NQ, v[g.e[i+1]]);

    const int* c = g.c;
    //for (int i = *c++; i--; c++)
    //  rel(*this, v[*c], IRT_EQ, i);
    while (*c != -1) {
      int n = *c;
      IntVarArgs x(n); c++;
      for (int i = n; i--; c++)
        x[i] = v[*c];
      distinct(*this, x, opt.icl());
      if (opt.model() == MODEL_CLIQUE)
        rel(*this, m, IRT_GQ, n-1);
    }
    branch(*this, m, INT_VAL_MIN);
    switch (opt.branching()) {
    case BRANCH_SIZE:
      branch(*this, v, INT_VAR_SIZE_MIN, INT_VAL_MIN);
      break;
    case BRANCH_DEGREE:
      branch(*this, v, tiebreak(INT_VAR_DEGREE_MAX,INT_VAR_SIZE_MIN),
             INT_VAL_MIN);
      break;
    case BRANCH_SIZE_DEGREE:
      branch(*this, v, INT_VAR_SIZE_DEGREE_MIN, INT_VAL_MIN);
      break;
    case BRANCH_SIZE_AFC:
      branch(*this, v, INT_VAR_SIZE_AFC_MIN, INT_VAL_MIN);
      break;
    default:
      break;
    }
  }
  /// Cost function
  virtual IntVar cost(void) const {
    return m;
  }
  /// Constructor for cloning \a s
  GraphColor(bool share, GraphColor& s) : MinimizeScript(share,s), g(s.g) {
    v.update(*this, share, s.v);
    m.update(*this, share, s.m);
  }
  /// Copying during cloning
  virtual Space*
  copy(bool share) {
    return new GraphColor(share,*this);
  }
  /// Print the solution
  virtual void
  print(std::ostream& os) const {
    os << "\tm = " << m << std::endl
       << "\tv[] = {";
    for (int i = 0; i < v.size(); i++) {
      os << v[i] << ", ";
      if ((i+1) % 15 == 0)
        os << std::endl << "\t       ";
    }
    os << "};" << std::endl;
  }
  
  bool solution(int *&color) {
	for (int i = 0; i < v.size(); i++)
		color[i] = v[i];
  }
};

#endif /* ConstraintProgrammingListGraphColoring_H */