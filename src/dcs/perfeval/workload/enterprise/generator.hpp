/**
 * \file dcs/perfeval/workload/enterprise/generator.hpp
 *
 * \brief Workload generator for any workload model.
 *
 * Copyright (C) 2009-2010  Distributed Computing System (DCS) Group, Computer
 * Science Department - University of Piemonte Orientale, Alessandria (Italy).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * \author Marco Guazzone, &lt;marco.guazzone@mfn.unipmn.it&gt;
 */

#ifndef DCS_PERFEVAL_WORKLOAD_ENTERPRISE_GENERATOR_HPP
#define DCS_PERFEVAL_WORKLOAD_ENTERPRISE_GENERATOR_HPP


#include <utility>


namespace dcs { namespace perfeval { namespace workload { namespace enterprise {

/**
 * \brief Application workload model based on request mixes.
 *
 * A workload based on user interaction mixes can be modeled as a tuple:
 * \f[
 *   W = \left\langle
 *         \{R_j\},
 *         \{\mathcal{D}_j^a\},
 *         \{M_i\},
 *         \mathbf{w}_W,
 *       \right\rangle
 * \f]
 * where:
 * - \f$R_j\f$, for \f$1 \le j \le N_R\f$ is a user request category;
 * - \f$\mathcal{D}_j^a\f$, for \f$1 \le j \le N_R\f$, is the probability
 *   distribution of request \f$R_j\f$'s interarrival times.
 * - \f$M_i\f$, for \f$1 \le i \le N_M\f$ is a request mix model;
 * - \f$\mathbf{w}_W\f$ is the vector of request mix weights, such that
 *   each element \f$w_W(i)\f$ is the weight of observing a request of
 *   request mix \f$M_i\f$, for \f$1 \le i \le N_M\f$.
 * .
 * Each request mix model \f$M_i\f$ is a tuple:
 * \f[
 *   M_i = \left\langle
 *           \{R_j\},
 *           \mathbf{w}_{M_i}
 *         \right\rangle
 * \f]
 * where:
 * - \f$R_j\f$, for \f$1 \le j \le N_R\f$ is a user request category;
 * - \f$\mathbf{w}_{M_i}\f$ is the vector of request weights for the mix
 *   \f$M_i\f$, such that each element \f$w_{M_i}(j)\f$ is the weight of
 *   observing a request category \f$R_j\f$ when the mix \f$M_i\f$ is selected.
 * .
 *
 * According to this model, the probability to draw a specific request category
 * is given by the Bayes theorem:
 * \f{align}{
 *   \Pr(R_j) &= \sum_{i=1}^{N_M} \Pr(M_i)\Pr(R_j|M_i) \\
 *            &= \sum_{i=1}^{N_M} \frac{w_W(i)}{\sum_{k=1}^{N_M}w_W(k)}\frac{w_{M_i}(j)}{\sum_{k=1}^{N_R}w_{M_i}(k)}
 * \f}
 *
 * \author Cosimo Anglano, &lt;cosimo.anglano@mfn.unipmn.it&gt;
 * \author Marco Guazzone, &lt;marco.guazzone@mfn.unipmn.it&gt;
 */
template <
	typename WorkloadModelT
>
class generator
{
	public: typedef WorkloadModelT workload_model_type;
	public: typedef typename workload_model_type::request_category_type request_category_type;
	public: typedef typename workload_model_type::real_type real_type;


	public: generator(workload_model_type const& model)
		: model_(model)
	{
		// empty
	}


	public: template <typename UniformRandomGeneratorT>
		::std::pair<request_category_type,real_type> operator()(UniformRandomGeneratorT& rng) const
	{
		return model_.generate(rng);
	}


	private: workload_model_type model_;
};

}}}} // Namespace dcs::perfeval::workload::enterprise


#endif // DCS_PERFEVAL_WORKLOAD_ENTERPRISE_GENERATOR_HPP
