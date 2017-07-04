/******************************************************************************
 * Copyright 2017 The Apollo Authors. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *****************************************************************************/
#include "modules/common/math/linear_quadratic_regulator.h"
#include "modules/common/log.h"

#include "Eigen/Dense"

namespace apollo {
namespace common {
namespace math {

using Matrix = Eigen::MatrixXd;

void SolveLQRProblem(const Matrix &A, const Matrix &B,
               const Matrix &Q, const Matrix &R,
               const double tolerance, const uint max_num_iteration,
               Matrix *ptr_K) {
  if (A.rows() != A.cols() ||
      B.rows() != A.rows() ||
      Q.rows() != Q.cols() ||
      Q.rows() != A.rows() ||
      R.rows() != R.cols() ||
      R.rows() != B.cols()) {
    AERROR << "One or more matrices have incompatible dimensions.";
    return;
  }

  Matrix AT = A.transpose();
  Matrix BT = B.transpose();

  // Solves a discrete-time Algebraic Riccati equation (DARE)
  // Calculate Matrix Difference Riccati Equation, initialize P and Q
  Matrix P = Q;
  uint num_iteration = 0;
  double diff = 0.0;
  while (num_iteration++ < max_num_iteration) {
    Matrix P_next = AT * P * A -
        AT * P * B * (R + BT * P * B).inverse() * BT * P * A +
        Q;
    // check the difference between P and P_next
    diff = fabs((P_next - P).maxCoeff());
    P = P_next;

    if (diff < tolerance) {
      break;
    }
  }

  if (num_iteration >= max_num_iteration) {
    AWARN << "lqr_not_convergence, last_diff_is:" << diff;
  } else {
    ADEBUG << "Number of iterations until convergence: " << num_iteration
           << ", max difference: " << diff;
  }
  *ptr_K = (R + BT * P * B).inverse() * BT * P * A;
}

}  // namespace math
}  // namespace common
}  // namespace apollo
