/**
 * \file dcs/perfeval/energy/fan2007_model.hpp
 *
 * \brief Energy model proposed by Fan et al in the paper <em>Power provisioning
 * for a warehouse-sized computer<em> (2007).
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

#ifndef DCS_PERFEVAL_ENERGY_FAN2007_MODEL_HPP
#define DCS_PERFEVAL_ENERGY_FAN2007_MODEL_HPP


#include <cmath>


namespace dcs { namespace perfeval { namespace energy {

/**
 * \brief Energy model proposed by Fan et al in the paper <em>Power provisioning
 * for a warehouse-sized computer<em> (2007).
 *
 * \tparam RealT The type for real numbers.
 *
 * Represent the energy model:
 * \f[
 *   P(u) = P_0 + P_1 u + P_2 u^r
 * \f]
 * where \f$P_0\f$ is the idle power consumption, \f$P_1\f$ and \f$P_2\f$ are
 * power coefficients, \f$u\f$ is the system utilization and \f$r\f$ is an
 * empirical exponent.
 * The model is a generalized version of the one proposed by Fan et al in the
 * work:
 * \par
 *  X. Fan and W.D. Weber and L.A. Barroso.<br/>
 *  Power provisioning for a warehouse-sized computer.<br/>
 *  In Proc. of the International Symposium on Computer Architecture
 *  (ISCA'2007), 2007.
 *
 * \note
 *  In the original work, the authors considered two models:
 *  - A linear model: \f$P_{\text{idle}} + (P_{\text{busy}}-P_{\text{idle}})u\f$
 *  - An empirical model: \f$P_{\text{idle}} + (P_{\text{busy}}-P_{\text{idle}})
 *    (2u-u^r)\f$
 *  .
 *
 * \author Marco Guazzone, &lt;marco.guazzone@mfn.unipmn.it&gt;
 */
template <typename RealT>
class fan2007_model //: public base_model<RealT>
{
	public: typedef RealT real_type;


	public: fan2007_model()
	{
	}


	public: fan2007_model(real_type p0, real_type p1, real_type p2, real_type r)
		: p0_(p0),
		  p1_(p1),
		  p2_(p2),
		  r_(r)
	{
		// empty
	}


	public: void coefficients(real_type p0, real_type p1, real_type p2, real_type r)
	{
		p0_ = p0;
		p1_ = p1;
		p2_ = p2;
		r_ = r;
	}


	public: real_type consumed_energy(real_type u) const
	{
		return p0_+p1_*u+p2_*::std::pow(u,r_);
	}


	private: real_type p0_;
	private: real_type p1_;
	private: real_type p2_;
	private: real_type r_;
};

}}} // Namespace dcs::perfeval::energy


#endif // DCS_PERFEVAL_ENERGY_FAN2007_MODEL_HPP
