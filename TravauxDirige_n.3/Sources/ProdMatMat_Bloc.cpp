#include <algorithm>
#include <cassert>
#include <iostream>
#include <thread>
#if defined(_OPENMP)
#include <omp.h>
#endif
#include "ProdMatMat.hpp"

namespace {
void prodSubBlocks(int iRowBlkA, int iColBlkB, int iColBlkA, int szBlock,
                   const Matrix& A, const Matrix& B, Matrix& C) {
#pragma omp parallel for
  for(int jBl = 0; jBl<B.nbCols; jBl+=szBlock) // A 
    for(int kBl = 0; kBl<A.nbRows; kBl+=szBlock)
      for(int iBl = 0; iBl<A.nbCols; iBl+=szBlock)
        for (int j = jBl; j < std::min(jBl + szBlock,B.nbCols); j++)
          for (int k = kBl; k < std::min(kBl + szBlock,A.nbRows); k++)
            for (int i = iBl; i < std::min(iBl + szBlock,A.nbCols); ++i)
              C(i, j) += A(i, k) * B(k, j);
}
const int szBlock = 512;
}  // namespace

Matrix operator*(const Matrix& A, const Matrix& B) {
  Matrix C(A.nbRows, B.nbCols, 0.0);
  prodSubBlocks(0, 0, 0, szBlock, A, B, C);
  return C;
}
