/**
 * \file dcs/perfeval/workload/enterprise/user_interaction_mix_model.hpp
 *
 * \brief Workload model based on user request mixes.
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
 * \author Cosimo Anglano, &lt;cosimo.anglano@mfn.unipmn.it&gt;
 * \author Marco Guazzone, &lt;marco.guazzone@mfn.unipmn.it&gt;
 */

#ifndef DCS_PERFEVAL_WORKLOAD_ENTERPRISE_USER_INTERACTION_MIX_MODEL_HPP
#define DCS_PERFEVAL_WORKLOAD_ENTERPRISE_USER_INTERACTION_MIX_MODEL_HPP


#include <cstddef>
#include <dcs/perfeval/workload/enterprise/user_interaction_mix.hpp>
#include <dcs/perfeval/workload/enterprise/user_request.hpp>
#include <dcs/perfeval/workload/enterprise/user_request_spec.hpp>
#include <dcs/container/unordered_map.hpp>
#include <dcs/math/stats/distribution/discrete.hpp>
#include <dcs/math/stats/function/rand.hpp>
#include <utility>
#include <vector>


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
	//typename RequestCategoryT,
	typename RandomNumberDistributionT,
	typename IntT,
	typename RealT
>
class user_interaction_mix_model
{
	public: typedef RandomNumberDistributionT random_distribution_type;
	public: typedef IntT int_type;
	public: typedef RealT real_type;
	public: typedef user_interaction_mix<int_type,real_type> interaction_mix_type;
	public: typedef user_request_spec<random_distribution_type,int_type> request_spec_type;
	public: typedef user_request<int_type,real_type> request_type;
	private: typedef ::std::vector<interaction_mix_type> interaction_mix_container;
	private: typedef ::dcs::container::unordered_map<int_type,request_spec_type> request_container;
	private: typedef ::dcs::math::stats::discrete_distribution<real_type> discrete_distribution_type;


	public: user_interaction_mix_model()
	{
	}


	public: template <
				typename ForwardIterator1T,
				typename ForwardIterator2T,
				typename ForwardIterator3T
			>
		user_interaction_mix_model(ForwardIterator1T first_request, ForwardIterator1T last_request, ForwardIterator2T first_mix, ForwardIterator2T last_mix, ForwardIterator3T first_mix_weight, ForwardIterator3T last_mix_weight)
	{
		for (
			ForwardIterator1T it = first_request;
			it != last_request;
			++it
		) {
			request_specs_[it->category()] = *it;
		}

		mixes_ = interaction_mix_container(first_mix, last_mix);
		mixes_dist_ = discrete_distribution_type(first_mix_weight, last_mix_weight);
	}


	public: template <typename ForwardIteratorT>
		void request_specifications(ForwardIteratorT first, ForwardIteratorT last)
	{
		request_specs_.clear();

		for (
			ForwardIteratorT it = first;
			it != last;
			++it
		) {
			request_specs_[it->category()] = *it;
		}
	}


	public: template <typename ForwardIterator1T, typename ForwardIterator2T>
		void interaction_mixes(ForwardIterator1T first_mix, ForwardIterator1T last_mix, ForwardIterator2T first_weight, ForwardIterator2T last_weight)
	{
		::std::vector<real_type> weights;
		mixes_.clear();

		for (
			ForwardIterator1T it = first_mix;
			it != last_mix;
			++it, ++first_weight
		) {
			mixes_.push_back(*it);
			if (first_weight != last_weight)
			{
				weights.push_back(*first_weight);
			}
			else
			{
				weights.push_back(0);
			}
		}
		mixes_dist_ = discrete_distribution_type(weights.being(), weights.end());
	}


//	public: template <typename UniformRandomGeneratorT>
//		::std::pair<request_category_type,real_type> generate(UniformRandomGeneratorT& rng) const
	public: template <typename UniformRandomGeneratorT>
		user_request<int_type,real_type> generate(UniformRandomGeneratorT& rng) const
	{
		::std::size_t mix = ::dcs::math::stats::rand(
				mixes_dist_,
				rng
		);

		int_type req_category = mixes_[mix].generate(rng);

//		real_type iatime = ::dcs::math::stats::rand(
//				request_specs_.at(req_category).iatime_distribution(),
//				rng
//		);
//
//		return ::std::make_pair(req_category, iatime);
		return request_specs_.at(req_category).generate(rng);
	}


	/// The mixes container.
	private: interaction_mix_container mixes_;
	/// The requests container.
	private: request_container request_specs_;
	/// The session initial probability distribution.
	private: discrete_distribution_type mixes_dist_;
};

}}}} // Namespace dcs::perfeval::workload::enterprise


#endif // DCS_PERFEVAL_WORKLOAD_ENTERPRISE_USER_INTERACTION_MIX_MODEL_HPP
