#ifndef DCS_SYSID_ALGORITHM_RLS_HPP
#define DCS_SYSID_ALGORITHM_RLS_HPP


#include <dcs/math/la/expression/matrix_expression.hpp>
#include <dcs/math/la/expression/vector_expression.hpp>
#include <dcs/math/la/operation/matrix_basic_operations.hpp>
#include <dcs/math/la/operation/vector_basic_operations.hpp>


namespace dcs { namespace sysid {

/**
 * \brief The Recursive Least Square (RLS) algorithm.
 * \param y The current measurement (output) vector.
 * \param u The current regressor (input) vector.
 * \param lambda The forgetting factor.
 * \param P Covariance matrix.
 * \param theta_hat The current parameter estimate vector.
 * \return Nothing. However matrix \a P and vector \a theta_hat are changed in
 *  order to reflect the current RLS update step.
 *
 *  The RLS method is defined as follows:
 *  \f[
 *  \hat{y}(t) = \psi^T(t)\hat{\theta}(t-1)
 *  \epsilon(t) = y(t)-\hat{y}(t)
 *  Q(t) = \frac{P(t-1)}{\lambda(t)+\psi^T(t)P(t-1)\psi(t)}
 *  K(t) = Q(t)\psi(t) \quad \text{gain vector}
 *  \hat{\theta}(t) = \hat{\theta}(t-1)+K(t)\epsilon(t)
 *  P(t) = \frac{1}{\lambda(t)}\left(P(t-1)-\frac{P(t-1)\psi(t)\psi^T(t)P(t-1)}{\lmbda(t)+\psi^T(t)P(t-1)\psi(t)}\right)
 *  \f]
 */
//template <typename Expr1T, typename Expr2T, typename Expr3T, typename Expr4T, typename RealT>
//void rls_step_inplace(dcs::math::la::vector_expression<Expr1T> const& y, dcs::math::la::vector_expression<Expr2T> const& u, RealT lambda, dcs::math::la::vector_expression<Expr3T>& theta_hat, dcs::math::la::matrix_expression<Expr4T> &P)
template <typename VectorT, typename RealT, typename MatrixT>
void rls_step_inplace(VectorT const& y, VectorT const& u, RealT lambda, VectorT& theta_hat, MatrixT &P)
{
	//dcs::math::la::vector_expression<Expr2T> u_t;
	VectorT u_t;
	//dcs::math::la::vector_expression<Expr2T> Pu;
	VectorT Pu;
	RealT q;
	//dcs::math::la::vector_expression<Expr2T> K;
	VectorT K;

	u_t = dcs::math::la::trans(u);
	Pu = dcs::math::la::prod(P, u);
	q = dcs::math::la::inner_prod(u_t, Pu);

	// Compute the Gain
	K = Pu/(lambda+q);

	// Estimate update
	theta_hat = theta_hat + K*(y-dcs::math::la::inner_prod(u_t,theta_hat));

	// Covariance matrix update
	//P = (P-(dcs::math::la::prod(dcs::math::la::prod(Pu, u_t), P))/(lambda+q))/lambda;
	P = P - dcs::math::la::outer_prod(K, dcs::math::la::prod(u_t, P));
}

}} // Namespace dcs::sysid


#endif // DCS_SYSID_ALGORITHM_RLS_HPP
