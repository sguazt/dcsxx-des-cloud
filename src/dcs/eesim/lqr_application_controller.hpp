/**
 * \file dcs/eesim/lqr_application_controller.hpp
 *
 * \brief Class modeling the application controller component using an LQR
 *  controller.
 *
 * Copyright (C) 2009-2011  Distributed Computing System (DCS) Group, Computer
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

#ifndef DCS_EESIM_LQR_APPLICATION_CONTROLLER_HPP
#define DCS_EESIM_LQR_APPLICATION_CONTROLLER_HPP


#include <algorithm>
#ifdef DCS_DEBUG
#	include <boost/numeric/ublas/io.hpp>
#endif // DCS_DEBUG
#include <boost/numeric/ublas/matrix.hpp>
//#include <boost/numeric/ublas/matrix_expression.hpp>
#include <boost/numeric/ublas/matrix_proxy.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/vector_proxy.hpp>
//#include <boost/numeric/ublas/vector_expression.hpp>
//#include <dcs/control/design/dlqr.hpp>
#include <dcs/control/design/dlqry.hpp>
#include <dcs/debug.hpp>
#include <dcs/des/base_statistic.hpp>
#include <dcs/des/engine_traits.hpp>
#include <dcs/des/mean_estimator.hpp>
#include <dcs/eesim/base_application_controller.hpp>
#include <dcs/eesim/multi_tier_application.hpp>
#include <dcs/eesim/performance_measure_category.hpp>
#include <dcs/eesim/physical_machine.hpp>
#include <dcs/eesim/physical_resource_category.hpp>
#include <dcs/eesim/registry.hpp>
#include <dcs/eesim/utility.hpp>
#include <dcs/functional/bind.hpp>
#include <dcs/macro.hpp>
#include <dcs/memory.hpp>
#include <dcs/sysid/algorithm/rls.hpp>
#include <exception>
#include <iostream>
#include <map>
#include <stdexcept>
#include <vector>


//TODO:
// - Currently the code in this class assumes the single resource (CPU) case.
//


namespace dcs { namespace eesim {

namespace detail { namespace /*<unnamed>*/ {

template <typename VectorExprT>
void rotate(::boost::numeric::ublas::vector_expression<VectorExprT>& v, ::std::size_t num_rot_grp, ::std::size_t rot_grp_size)
{
	namespace ublas = ::boost::numeric::ublas;

	// Alternative #1
	//
	//::std::rotate(v().begin(), v().begin()+rot_grp_size, v().end());

	// Alternative #2 (perhaps faster than solution #1 since performs less swaps))

	DCS_DEBUG_ASSERT( v().size() == (num_rot_grp+1)*rot_grp_size );

	for (::std::size_t i = 1; i <= num_rot_grp; ++i)
	{
		::std::size_t j1((i-1)*rot_grp_size);
		::std::size_t j2(i*rot_grp_size);
		::std::size_t j3((i+1)*rot_grp_size);
		ublas::subrange(v(), j1, j2) = ublas::subrange(v(), j2, j3);
	}
}

}} // Namespace detail::<unnamed>


template <typename TraitsT>
class lqr_application_controller: public base_application_controller<TraitsT>
{
	private: typedef base_application_controller<TraitsT> base_type;
	private: typedef lqr_application_controller<TraitsT> self_type;
	public: typedef TraitsT traits_type;
	public: typedef typename traits_type::real_type real_type;
	public: typedef typename traits_type::uint_type uint_type;
	public: typedef ::dcs::control::dlqry_controller<real_type> controller_type;
	public: typedef ::dcs::shared_ptr<controller_type> controller_pointer;
	public: typedef typename base_type::application_pointer application_pointer;
	private: typedef ::std::size_t size_type;
	private: typedef registry<traits_type> registry_type;
	private: typedef typename traits_type::des_engine_type des_engine_type;
	private: typedef typename ::dcs::des::engine_traits<des_engine_type>::event_type des_event_type;
	private: typedef typename ::dcs::des::engine_traits<des_engine_type>::engine_context_type des_engine_context_type;
//	private: typedef performance_measure<traits_type> performance_measure_type;
	private: typedef ::boost::numeric::ublas::matrix<real_type> matrix_type;
	private: typedef ::boost::numeric::ublas::vector<real_type> vector_type;
	private: typedef typename base_type::application_type application_type;
	private: typedef typename application_type::simulation_model_type application_simulation_model_type;
	private: typedef typename application_simulation_model_type::user_request_type user_request_type;
	private: typedef typename application_simulation_model_type::virtual_machine_type virtual_machine_type;
	private: typedef ::dcs::shared_ptr<virtual_machine_type> virtual_machine_pointer;
	private: typedef typename application_type::application_tier_type application_tier_type;
	private: typedef ::dcs::shared_ptr<application_tier_type> application_tier_pointer;
	private: typedef ::std::vector<performance_measure_category> category_container;
//	private: typedef ::std::vector<real_type> measure_container;
	private: typedef ::std::map<performance_measure_category,real_type> category_measure_container;
	private: typedef ::dcs::des::base_statistic<real_type,uint_type> statistic_type;
	private: typedef ::dcs::shared_ptr<statistic_type> statistic_pointer;
	private: typedef ::std::map<performance_measure_category,statistic_pointer> category_statistic_container;
	private: typedef ::std::vector<category_statistic_container> category_statistic_container_container;
	private: typedef ::std::map<performance_measure_category,real_type> category_value_container;
	private: typedef ::std::vector<category_value_container> category_value_container_container;
	private: typedef physical_machine<traits_type> physical_machine_type;


	private: static const size_type default_input_order_;
	private: static const size_type default_output_order_;
	private: static const size_type default_input_delay_;
	private: static const real_type default_rls_forgetting_factor_;
	private: static const real_type default_ewma_smoothing_factor_;
//	private: static const uint_type default_ss_state_size_;
//	private: static const uint_type default_ss_input_size_;
//	private: static const uint_type default_ss_output_size_;


	public: lqr_application_controller()
	: base_type(),
	  controller_(),
//	  Theta_hat_(),
//	  P_(),
//	  phi_()
	  n_a_(default_output_order_),
	  n_b_(default_input_order_),
	  d_(default_input_delay_),
	  n_p_(0),
	  n_s_(0),
	  n_x_(0),
	  n_y_(0),
	  n_u_(0),
	  rls_ff_(default_rls_forgetting_factor_),
	  ewma_smooth_(default_ewma_smoothing_factor_),
	  x_offset_(0),
	  u_offset_(0),
	  count_(0)
	{
		init();
	}


	public: lqr_application_controller(application_pointer const& ptr_app, real_type ts)
	: base_type(ptr_app, ts),
	  controller_(),
	  n_a_(default_output_order_),
	  n_b_(default_input_order_),
	  d_(default_input_delay_),
	  n_p_(0),
	  n_s_(0),
	  n_x_(0),
	  n_y_(0),
	  n_u_(0),
	  rls_ff_(default_rls_forgetting_factor_),
	  ewma_smooth_(default_ewma_smoothing_factor_),
	  x_offset_(0),
	  u_offset_(0),
	  count_(0)
	{
		init();
	}


	public: template <typename QMatrixExprT, typename RMatrixExprT>
		lqr_application_controller(::boost::numeric::ublas::matrix_expression<QMatrixExprT> const& Q,
								   ::boost::numeric::ublas::matrix_expression<RMatrixExprT> const& R,
								   application_pointer const& ptr_app,
								   real_type ts)
	: base_type(ptr_app, ts),
	  controller_(Q, R),
	  n_a_(default_output_order_),
	  n_b_(default_input_order_),
	  d_(default_input_delay_),
	  n_p_(0),
	  n_s_(0),
	  n_x_(0),
	  n_y_(0),
	  n_u_(0),
	  rls_ff_(default_rls_forgetting_factor_),
	  ewma_smooth_(default_ewma_smoothing_factor_),
	  x_offset_(0),
	  u_offset_(0),
	  count_(0)
	{
		init();
	}


	public: template <typename QMatrixExprT, typename RMatrixExprT, typename NMatrixExprT>
		lqr_application_controller(::boost::numeric::ublas::matrix_expression<QMatrixExprT> const& Q,
								   ::boost::numeric::ublas::matrix_expression<RMatrixExprT> const& R,
								   ::boost::numeric::ublas::matrix_expression<NMatrixExprT> const& N,
								   application_pointer const& ptr_app,
								   real_type ts)
	: base_type(ptr_app, ts),
	  controller_(Q, R, N),
	  n_a_(default_output_order_),
	  n_b_(default_input_order_),
	  d_(default_input_delay_),
	  n_p_(0),
	  n_s_(0),
	  n_x_(0),
	  n_y_(0),
	  n_u_(0),
	  rls_ff_(default_rls_forgetting_factor_),
	  ewma_smooth_(default_ewma_smoothing_factor_),
	  x_offset_(0),
	  u_offset_(0),
	  count_(0)
	{
		init();
	}


	public: ~lqr_application_controller()
	{
		this->disconnect_from_event_sources();
	}


	private: void init()
	{
		init_measures();

		this->connect_to_event_sources();
	}


	private: void connect_to_event_sources()
	{
		typedef ::std::vector<application_tier_pointer> tier_container;
		typedef typename tier_container::const_iterator tier_iterator;

		if (this->application_ptr())
		{
			// Connect to application-level request departure event source
			this->application_ptr()->simulation_model().request_departure_event_source().connect(
					::dcs::functional::bind(
							&self_type::process_request_departure,
							this,
							::dcs::functional::placeholders::_1,
							::dcs::functional::placeholders::_2
						)
				);

//			// Connect to tier-level request departure event sources
//			tier_container tiers = this->application().tiers();
//			tier_iterator tier_end_it(tiers.end());
//			for (tier_iterator it = tiers.begin(); it != tier_end_it; ++it)
//			{
//				application_tier_pointer ptr_tier(*it);
//
//				this->application_ptr()->simulation_model().request_tier_departure_event_source(ptr_tier->id()).connect(
//						::dcs::functional::bind(
//								&self_type::process_request_tier_departure,
//								this,
//								::dcs::functional::placeholders::_1,
//								::dcs::functional::placeholders::_2,
//								ptr_tier->id()
//							)
//					);
//			}
		}
	}


	private: void disconnect_from_event_sources()
	{
		typedef ::std::vector<application_tier_pointer> tier_container;
		typedef typename tier_container::const_iterator tier_iterator;

		if (this->application_ptr())
		{
			// Connect to application-level request departure event source
			this->application_ptr()->simulation_model().request_departure_event_source().disconnect(
					::dcs::functional::bind(
							&self_type::process_request_departure,
							this,
							::dcs::functional::placeholders::_1,
							::dcs::functional::placeholders::_2
						)
				);

//			// Connect to tier-level request departure event sources
//			tier_container tiers = this->application().tiers();
//			tier_iterator tier_end_it(tiers.end());
//			for (tier_iterator it = tiers.begin(); it != tier_end_it; ++it)
//			{
//				application_tier_pointer ptr_tier(*it);
//
//				this->application_ptr()->simulation_model().request_tier_departure_event_source(ptr_tier->id()).disconnect(
//						::dcs::functional::bind(
//								&self_type::process_request_tier_departure,
//								this,
//								::dcs::functional::placeholders::_1,
//								::dcs::functional::placeholders::_2,
//								ptr_tier->id()
//							)
//					);
//			}
		}
	}


	private: void init_measures()
	{
		typedef ::dcs::des::mean_estimator<real_type,uint_type> statistic_impl_type;

		if (this->application_ptr())
		{
			//FIXME: statistic category (e.g., mean) is hard-coded

			uint_type num_tiers(this->application().num_tiers());

			tier_measures_.resize(num_tiers);

			typedef typename category_container::const_iterator category_iterator;

			category_container categories(this->application().sla_cost_model().slo_categories());
			category_iterator end_it = categories.end();
			for (category_iterator it = categories.begin(); it != end_it; ++it)
			{
				performance_measure_category category(*it);

				// Initialize app-level measure
				measures_[category] = ::dcs::make_shared<statistic_impl_type>();

				// Initialize tier-level measure
				for (size_type tier_id = 0; tier_id < num_tiers; ++tier_id)
				{
					tier_measures_[tier_id][category] = ::dcs::make_shared<statistic_impl_type>();
//					ewma_tier_s_[tier_id][category] = real_type/*zero*/();
				}
			}

			n_p_ = n_s_
				 = num_tiers;
			n_x_ = n_p_*(n_a_+1);
			n_u_ = n_s_*(n_b_+1);
			n_y_ = uint_type(1);
			x_offset_ = n_x_-n_p_;
			u_offset_ = n_u_-n_p_;
		}
	}


	private: void reset_measures()
	{
		// Instead of making a full reset, keep some history of the
		// past in order to solve these issues:
		// - too few observation in the last control period
		// - not so much representative observations in the last control period.

//FIXME: try to use this below to reset stats
//		::std::for_each(
//				measures_.begin(),
//				measures_.end(),
//				::dcs::functional::bind(
//						&statistic_type::reset,
//						(::dcs::functional::placeholders::_1)->second
//					)
//			);
		typedef typename category_statistic_container::iterator measure_iterator;

		measure_iterator measure_end_it(measures_.end());
		for (measure_iterator it = measures_.begin(); it != measure_end_it; ++it)
		{
			performance_measure_category category(it->first);
			statistic_pointer ptr_stat(it->second);

			// Apply the EWMA filter to previously observed measurements
			//real_type ewma_old_s(ewma_s_.at(category));
			ewma_s_[category] = ewma_smooth_*ptr_stat->estimate() + (1-ewma_smooth_)*ewma_s_.at(category);

			// Reset stat and set as the first observation a memory of the past
			ptr_stat->reset();
			(*ptr_stat)(ewma_s_.at(category));
		}

		size_type num_tiers(tier_measures_.size());
		for (size_type tier_id = 0; tier_id < num_tiers; ++tier_id)
		{
			measure_end_it = tier_measures_[tier_id].end();
			for (measure_iterator inner_it = tier_measures_[tier_id].begin(); inner_it != measure_end_it; ++inner_it)
			{
				performance_measure_category category(inner_it->first);
				statistic_pointer ptr_stat(inner_it->second);

				// Apply the EWMA filter to previously observed measurements
				//real_type ewma_old_s(ewma_tier_s_[tier_id].at(category));
				ewma_tier_s_[tier_id][category] = ewma_smooth_*ptr_stat->estimate() + (1-ewma_smooth_)*ewma_tier_s_[tier_id].at(category);

				// Reset stat and set as the first observation a memory of the past
				ptr_stat->reset();
				(*ptr_stat)(ewma_tier_s_[tier_id].at(category));
			}
		}
	}


	private: void full_reset_measures()
	{
		typedef typename category_statistic_container::iterator measure_iterator;
		typedef typename category_statistic_container_container::iterator tier_measure_iterator;

		measure_iterator measure_end_it(measures_.end());
		for (measure_iterator it = measures_.begin(); it != measure_end_it; ++it)
		{
			it->second->reset();
			ewma_s_[it->first] = real_type/*zero*/();
		}

//		tier_measure_iterator tier_measure_end_it(tier_measures_.end());
//		for (tier_measure_iterator outer_it = tier_measures_.begin(); outer_it != tier_measure_end_it; ++outer_it)
		size_type num_tiers(tier_measures_.size());
		if (ewma_tier_s_.size() != num_tiers)
		{
			ewma_tier_s_.resize(num_tiers);
		}
		for (size_type tier_id = 0; tier_id < num_tiers; ++tier_id)
		{
//			measure_end_it = outer_it->end();
			measure_end_it = tier_measures_[tier_id].end();
//			for (measure_iterator inner_it = outer_it->begin(); inner_it != measure_end_it; ++inner_it)
			for (measure_iterator inner_it = tier_measures_[tier_id].begin(); inner_it != measure_end_it; ++inner_it)
			{
				inner_it->second->reset();
				ewma_tier_s_[tier_id][inner_it->first] = real_type/*zero*/();
			}
		}

		count_ = size_type/*zero*/();
		x_ = vector_type(n_x_, 0);
		u_ = vector_type(n_u_, 0);
	}


	//@{ Event Handlers

//	private: void process_request_tier_arrival(des_event_type const& evt, des_engine_context_type& ctx)
//	{
//		DCS_DEBUG_TRACE("(" << this << ") BEGIN Processing REQUEST-TIER-ARRIVAL (Clock: " << ctx.simulated_time() << ")");
//
//		typedef typename category_statistic_container_container::iterator tier_measure_iterator;
//
//		// Collect system output measures
//
//		user_request_type req = this->application().simulation_model().request_state(evt);
//
//		tier_measure_iterator end_it = tier_measures_.end();
//		for (measure_iterator it = measures_.begin(); it != end_it; ++it)
//		{
//			performance_measure_category category(it->first);
//
//			switch (category)
//			{
//				case response_time_performance_measure:
//					{
//						real_type rt(req.departure_time()-req.arrival_time());
//						(*measures_.at(category))(rt);
//					}
//					break;
//				default:
//					throw ::std::runtime_error("[dcs::eesim::lqr_application_controller::process_request_departure] LQR application controller currently handles only the response-time category.");
//			}
//		}
//		DCS_DEBUG_TRACE("(" << this << ") END Processing REQUEST-TIER-ARRIVAL (Clock: " << ctx.simulated_time() << ")");
//	}


//	private: void process_request_tier_departure(des_event_type const& evt, des_engine_context_type& ctx, uint_type tier_id)
//	{
//		DCS_DEBUG_TRACE("(" << this << ") BEGIN Processing REQUEST-TIER-DEPARTURE (Clock: " << ctx.simulated_time() << ")");
//
//		typedef typename category_statistic_container::iterator measure_iterator;
//
//		// Collect system output measures
//
//		user_request_type req = this->application().simulation_model().request_state(evt);
//
//		// check: double check on tier identifier.
//		DCS_DEBUG_ASSERT( tier_id == req.current_tier() );
//
////TODO: measure should be scaled w.r.t. the reference machine
//		measure_iterator end_it = tier_measures_[tier_id].end();
//		for (measure_iterator it = tier_measures_[tier_id].begin(); it != end_it; ++it)
//		{
//			performance_measure_category category(it->first);
//			statistic_pointer ptr_stat(it->second);
//
//			switch (category)
//			{
//				case response_time_performance_measure:
//					{
//						real_type rt(req.tier_departure_time(tier_id)-req.tier_arrival_time(tier_id));
//						(*ptr_stat)(rt);
//					}
//					break;
//				default:
//					throw ::std::runtime_error("[dcs::eesim::lqr_application_controller::process_tier_request_departure] LQR application controller currently handles only the response-time category.");
//			}
//		}
//
//		DCS_DEBUG_TRACE("(" << this << ") END Processing REQUEST-TIER-DEPARTURE (Clock: " << ctx.simulated_time() << ")");
//	}


	private: void process_request_departure(des_event_type const& evt, des_engine_context_type& ctx)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );

		DCS_DEBUG_TRACE("(" << this << ") BEGIN Processing REQUEST-DEPARTURE (Clock: " << ctx.simulated_time() << ")");

		typedef typename category_statistic_container::iterator measure_iterator;

		// Collect tiers and system output measures

		user_request_type req = this->application().simulation_model().request_state(evt);

		real_type app_rt(0);

		size_type num_tiers(tier_measures_.size());
		for (size_type tier_id = 0; tier_id < num_tiers; ++tier_id)
		{
			virtual_machine_pointer ptr_vm(this->application_ptr()->simulation_model().tier_virtual_machine(tier_id));
			physical_machine_type const& actual_pm(ptr_vm->vmm().hosting_machine());
			physical_resource_category res_category(cpu_resource_category);//FIXME

			// Actual-to-reference scaling factor
			real_type scale_factor;
			scale_factor = ::dcs::eesim::resource_scaling_factor(
					// Actual resource capacity and threshold
					actual_pm.resource(res_category)->capacity(),
					actual_pm.resource(res_category)->utilization_threshold(),
					// Reference resource capacity and threshold
					this->application().reference_resource(res_category).capacity(),
					this->application().reference_resource(res_category).utilization_threshold()
				);

			measure_iterator end_it = tier_measures_[tier_id].end();
			for (measure_iterator it = tier_measures_[tier_id].begin(); it != end_it; ++it)
			{
				performance_measure_category stat_category(it->first);
				statistic_pointer ptr_stat(it->second);

				switch (stat_category)
				{
					case response_time_performance_measure:
						{
							//NOTE: the relation between response time and
							//      resource capacity is inversely proportional:
							//      the greater is the capacity, the less is the
							//      response time.

							// Compute the residence time for this tier
							::std::vector<real_type> arr_times(req.tier_arrival_times(tier_id));
							::std::vector<real_type> dep_times(req.tier_departure_times(tier_id));
							size_type nt(arr_times.size());
							if (nt > 0)
							{
								real_type rt(0);
								for (size_type t = 0; t < nt; ++t)
								{
									rt += dep_times[t]-arr_times[t];
								}
								rt *= scale_factor;
								(*ptr_stat)(rt);
								app_rt += rt;
							}
						}
						break;
					default:
						throw ::std::runtime_error("[dcs::eesim::lqr_application_controller::process_request_departure] LQR application controller currently handles only the response-time category.");
				}
			}
		}

		//FIXME: measure should be scaled w.r.t. the reference machine but at
		//       application-level we are unable to do this since each tier
		//       might have been run on a machine with very different
		//       characteristics. So instead of computing the performance
		//       measure (e.g., the response time) from the request object
		//       (e.g., from its arrival and departure times), compute it as the
		//       sum of tier performance measures (e.g., residence times).
		//       It should be equivalent!
        measure_iterator end_it = measures_.end();
        for (measure_iterator it = measures_.begin(); it != end_it; ++it)
        {
			performance_measure_category category(it->first);
			statistic_pointer ptr_stat(it->second);

            switch (category)
            {
                case response_time_performance_measure:
					{
						//real_type rt(req.departure_time()-req.arrival_time());
						//rt *= scale_factor;
						//(*ptr_stat)(rt);
						(*ptr_stat)(app_rt);
					}
					break;
				default:
					throw ::std::runtime_error("[dcs::eesim::lqr_application_controller::process_request_departure] LQR application controller currently handles only the response-time category.");
			}
		}

		DCS_DEBUG_TRACE("(" << this << ") END Processing REQUEST-DEPARTURE (Clock: " << ctx.simulated_time() << ")");
	}

	//@} Event Handlers


	//@{ Inteface Member Functions

	protected: void do_application(application_pointer const& ptr_app)
	{
		if (this->application_ptr())
		{
			// Disconnect from "old" app event sources

			this->application_ptr()->simulation_model().request_departure_event_source().disconnect(
					::dcs::functional::bind(
							&self_type::process_request_departure,
							this,
							::dcs::functional::placeholders::_1,
							::dcs::functional::placeholders::_2
						)
				);
		}

		// Connect to the event sources of the "new" app
		ptr_app->simulation_model().request_departure_event_source().connect(
				::dcs::functional::bind(
						&self_type::process_request_departure,
						this,
						::dcs::functional::placeholders::_1,
						::dcs::functional::placeholders::_2
					)
			);

		init_measures();
	}


	protected: void do_process_sys_init(des_event_type const& evt, des_engine_context_type& ctx)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );

		DCS_DEBUG_TRACE("(" << this << ") BEGIN Do Process SYSTEM-INITIALIZATION event (Clock: " << ctx.simulated_time() << ")");

		// Prepare the data structures for the RLS algorithm 
		::dcs::sysid::rls_arx_mimo_init(n_a_,
										n_b_,
										d_,
										n_p_,
										n_s_,
										rls_Theta_hat_,
										rls_P_,
										rls_phi_);

		// Completely reset all measures
		full_reset_measures();

		DCS_DEBUG_TRACE("(" << this << ") END Do Process SYSTEM-INITIALIZATION event (Clock: " << ctx.simulated_time() << ")");
	}


	private: void do_process_control(des_event_type const& evt, des_engine_context_type& ctx)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );

		namespace ublas = ::boost::numeric::ublas;

		typedef typename category_container::const_iterator category_iterator;
//		typedef typename category_statistic_container_container::const_iterator tier_iterator;

		DCS_DEBUG_TRACE("(" << this << ") BEGIN Do Process CONTROL event (Clock: " << ctx.simulated_time() << " - Count: " << count_ << ")");

		++count_;

		size_type num_tiers(tier_measures_.size());

		vector_type s(n_s_,0); // control input (relative error of tier resource shares)
		vector_type p(n_p_,0); // control state (relative error of tier peformance measures)
		vector_type y(n_y_,0); // control output (relative error of app peformance measures)


		// Check if a measure rotation is needed (always but the first time)
		if (count_ > 1)
		{
			// throw away old observations from x and make space for new ones.
			detail::rotate(x_, n_a_, n_p_);
			// throw away old observations from u and make space for new ones.
			detail::rotate(u_, n_b_, n_s_);
		}


		// Collect data for creating control input and state
		//
		// NOTE: the application controller always work w.r.t. the reference
		//       machine.
		//       Anyway, at this time, both reference and actual measures have
		//       already been scaled according to the reference machine.
		//
		// NOTE: instead of using past control inputs computed by the controller
		//       we use the actual value used by VM since it can be different
		//       from the one computed by the controller, due to the
		//       "interference" of other components (like the physical machine
		//       controller).
		//
		// NOTE: one can think that this step can be done only once; however, in
		//       general, reference measures are not constant since they can
		//       change over time.
		//

		category_measure_container ref_measures;
		category_container categories(this->application().sla_cost_model().slo_categories());
		category_iterator end_it = categories.end();
//		tier_iterator tier_end_it = tier_measures_.end();
		for (category_iterator it = categories.begin(); it != end_it; ++it)
		{
			performance_measure_category category(*it);

			real_type ref_measure;
			real_type actual_measure;

			ref_measure = this->application().sla_cost_model().slo_value(category);
			actual_measure = measures_.at(category)->estimate();
			y(0) = actual_measure/ref_measure - real_type(1);

			for (size_type tier_id = 0; tier_id < num_tiers; ++tier_id)
			{
				switch (category)
				{
					case response_time_performance_measure:
						{
							statistic_pointer ptr_stat(tier_measures_[tier_id].at(category));

							ref_measure = this->application_ptr()->performance_model().tier_measure(tier_id, category);
							if (ptr_stat->num_observations() > 0)
							{
								actual_measure = ptr_stat->estimate();
							}
							else
							{
								actual_measure = ref_measure;
							}
DCS_DEBUG_TRACE("TIER OBSERVATION: ref: " << ref_measure << " - actual: " << actual_measure);//XXX
							x_(x_offset_+tier_id) = p(tier_id)
												  = actual_measure/ref_measure - real_type(1);
						}
						break;
					default:
						throw ::std::runtime_error("[dcs::eesim::lqr_application_controller::do_process_control] LQR application controller currently handles only the response-time category.");
				}
			}
		}

//FIXME: resource category is actually hard-coded to CPU
		for (size_type tier_id = 0; tier_id < num_tiers; ++tier_id)
		{
			virtual_machine_pointer ptr_vm = this->application_ptr()->simulation_model().tier_virtual_machine(tier_id);
			physical_machine_type const& actual_pm(ptr_vm->vmm().hosting_machine());
			physical_resource_category res_category(cpu_resource_category);//FIXME

			// Get the reference resource share for the tier from the application specs
//			//real_type ref_share(ptr_vm->wanted_resource_share(ptr_vm->vmm().hosting_machine(), category));
//			//real_type ref_share(ptr_vm->wanted_resource_share(res_category));
			real_type ref_share(ptr_vm->guest_system().resource_share(res_category));

			// Get the actual resource share from the VM and scale w.r.t. the reference machine
			real_type actual_share;
			actual_share = ::dcs::eesim::scale_resource_share(actual_pm.resource(res_category)->capacity(),
															  actual_pm.resource(res_category)->utilization_threshold(),
															  this->application().reference_resource(res_category).capacity(),
															  this->application().reference_resource(res_category).utilization_threshold(),
															  ptr_vm->resource_share(res_category));

			//FIXME: should u contain relative errore w.r.t. resource share given from performance model?
			u_(u_offset_+tier_id) = s(tier_id)
								  = actual_share/ref_share - real_type(1);
		}

		// Estimate system parameters
DCS_DEBUG_TRACE("p(k): " << p);//XXX
DCS_DEBUG_TRACE("s(k): " << s);//XXX
DCS_DEBUG_TRACE("Theta_hat(k): " << rls_Theta_hat_);//XXX
DCS_DEBUG_TRACE("P(k): " << rls_P_);//XXX
DCS_DEBUG_TRACE("phi(k): " << rls_phi_);//XXX
		::dcs::sysid::rls_ff_arx_mimo(p,
									  s,
									  rls_ff_,
									  n_a_,
									  n_b_,
									  d_,
									  rls_Theta_hat_,
									  rls_P_,
									  rls_phi_);

DCS_DEBUG_TRACE("Theta_hat(k+1): " << rls_Theta_hat_);//XXX
DCS_DEBUG_TRACE("P(k+1): " << rls_P_);//XXX
DCS_DEBUG_TRACE("phi(k+1): " << rls_phi_);//XXX
		// Check if RLS (and LQR) can be applied.
		// If not, then no control is performed.
DCS_DEBUG_TRACE("Count: " << count_ << " - Na: " << n_a_ << " - Nb: " << n_b_ << " - Max: " << ::std::max(n_a_,n_b_));//XXX
//		if (count_ >= ::std::max(n_a_,n_b_))
		if (count_ > ::std::min(n_a_,n_b_))
		{
			matrix_type A(n_x_,n_x_,0);
			matrix_type B(n_x_,n_u_,0);
			matrix_type C(n_y_,n_x_,0);
			matrix_type D(n_y_,n_u_,0);

			// A=[0 I 0 ... 0;
			// 	  0 0 I ... 0;
			// 	  ...
			// 	  0 0 0 ... I;
			// 	  A_{n_a} ... A_1]]
			// NOTE: rls_Theta_hat_ contains parameters in reverse order: [A_1 ... A_{n_a} B_1 ... B_{n_b}]
			size_type k;
			//k = (n_p_-1)*n_a_;
			k = n_x_-n_p_;
			ublas::subrange(A, 0, k, n_p_, n_x_) = ublas::identity_matrix<real_type>(k,k);
			//ublas::subrange(A, k, n_x_, 0, n_x_) = ublas::subrange(rls_Theta_hat_, 0, n_p_, 0, n_x_);
			for (size_type i = 0; i < n_a_; ++i)
			{
				size_type itnp(i*n_p_);
				size_type ip1tnp((i+1)*n_p_);

				ublas::subrange(A, k, n_x_, itnp, ip1tnp) = ublas::subrange(rls_Theta_hat_, 0, n_p_, n_x_-ip1tnp, n_x_-itnp);
			}

			// B=[0 ... 0;
			//    0 ... 0;
			//    B_{n_b} ... B_1]
			// NOTE: rls_Theta_hat_ contains parameters in reverse order: [A_1 ... A_{n_a} B_1 ... B_{n_b}]
			size_type theta_nc(rls_Theta_hat_.size2());
			k = n_u_-n_s_;
			//ublas::subrange(B, k, n_u_, 0, n_u_) = ublas::subrange(rls_Theta_hat_, 0, n_s_, 0, n_u_);
			for (size_type i = 0; i < n_b_; ++i)
			{
				size_type itns(i*n_s_);
				size_type ip1tns((i+1)*n_s_);

				ublas::subrange(B, k, n_u_, itns, ip1tns) = ublas::subrange(rls_Theta_hat_, 0, n_p_, theta_nc-ip1tns, theta_nc-itns);
			}

			// C[0:n_y_,(n_x_-n_p):n_x_] = 1
			//ublas::subrange(C, 0, n_y_, 0, n_p_) = ublas::scalar_matrix<real_type>(n_y_, n_p_, real_type(1));
			ublas::subrange(C, 0, n_y_, (n_x_-n_p_), n_x_) = ublas::scalar_matrix<real_type>(n_y_, n_p_, real_type(1));

			// D=[0 ... 0]

			// Compute the optimal control
DCS_DEBUG_TRACE("Solving LQR with");//XXX
DCS_DEBUG_TRACE("A=" << A);//XXX
DCS_DEBUG_TRACE("B=" << B);//XXX
DCS_DEBUG_TRACE("C=" << C);//XXX
DCS_DEBUG_TRACE("D=" << D);//XXX
			bool ok(true);
			try
			{
				controller_.solve(A, B, C, D);
			}
			catch (::std::exception const& e)
			{
				::std::clog << "[Warning] Unable to compute control input." << ::std::endl;
				DCS_DEBUG_TRACE( "Caught exception: " << e.what() );

				ok = false;
			}
			if (ok)
			{
DCS_DEBUG_TRACE("Solved!");//XXX
DCS_DEBUG_TRACE("Getting control for x= " << x_);//XXX
				vector_type opt_u;
				opt_u = ublas::real(controller_.control(x_));
DCS_DEBUG_TRACE("Optima Control => " << opt_u);//XXX

DCS_DEBUG_TRACE("Applying optimal control");//XXX
				for (size_type tier_id = 0; tier_id < num_tiers; ++tier_id)
				{
					physical_resource_category res_category(cpu_resource_category);//FIXME

					virtual_machine_pointer ptr_vm(this->application_ptr()->simulation_model().tier_virtual_machine(tier_id));
					physical_machine_type const& pm(ptr_vm->vmm().hosting_machine());

					real_type share;
					share = ::dcs::eesim::scale_resource_share(
							// Reference resource capacity and threshold
							this->application_ptr()->reference_resource(res_category).capacity(),
							this->application_ptr()->reference_resource(res_category).utilization_threshold(),
							// Actual resource capacity and threshold
							pm.resource(res_category)->capacity(),
							pm.resource(res_category)->utilization_threshold(),
							// Old resource + computed deviation
							ptr_vm->wanted_resource_share(res_category)+opt_u(tier_id)
						);
															
DCS_DEBUG_TRACE("Assigning new wanted share: VM: " << ptr_vm->name() << " (" << ptr_vm->id() << ") - Category: " << res_category << " ==> Share: " << share);//XXX
					ptr_vm->wanted_resource_share(res_category, share);
				}
DCS_DEBUG_TRACE("Optimal control applied");//XXX
			}
		}

		// Reset previously collected system measure in order to collect a new ones.
		reset_measures();

		DCS_DEBUG_TRACE("(" << this << ") END Do Process CONTROL event (Clock: " << ctx.simulated_time() << " - Count: " << count_ << ")");
	}

	//@} Inteface Member Functions


	/// The LQR controller.
	private: controller_type controller_;
	/// Matrix of system parameters estimated by RLS: [A_1 ... A_{n_a} B_1 ... B_{n_b}].
	private: matrix_type rls_Theta_hat_;
	/// The covariance matrix computed by RLS.
	private: matrix_type rls_P_;
	/// The regression vector used by RLS.
	private: vector_type rls_phi_;
	/// The memory for the control output.
	private: size_type n_a_;
	/// The memory for the control input.
	private: size_type n_b_;
	/// Input time delay used in the RLS algorithm
	private: size_type d_;
	/// The size of the control state vector.
	private: size_type n_p_;
	/// The size of the control input vector.
	private: size_type n_s_;
	/// The size of the augmented control state vector.
	private: size_type n_x_;
	/// The size of the control output vector.
	private: size_type n_y_;
	/// The size of the augmented control input vector.
	private: size_type n_u_;
	/// Forgetting factor used in the RLS algorithm.
	private: real_type rls_ff_;
	/// Smoothing factor for the EWMA filter
	private: real_type ewma_smooth_;
	private: size_type x_offset_;
	private: size_type u_offset_;
	private: size_type count_;
	private: vector_type x_;
	private: vector_type u_;
	/// System-level measures collected during the last control interval.
	private: category_statistic_container measures_;
	/// Tier-level measures collected during the last control interval.
	private: category_statistic_container_container tier_measures_;
	private: category_value_container ewma_s_;
	private: category_value_container_container ewma_tier_s_;
};


template <typename TraitsT>
const typename lqr_application_controller<TraitsT>::size_type lqr_application_controller<TraitsT>::default_input_order_ = 1;


template <typename TraitsT>
const typename lqr_application_controller<TraitsT>::size_type lqr_application_controller<TraitsT>::default_output_order_ = 2;


template <typename TraitsT>
const typename lqr_application_controller<TraitsT>::size_type lqr_application_controller<TraitsT>::default_input_delay_ = 0;


template <typename TraitsT>
const typename lqr_application_controller<TraitsT>::real_type lqr_application_controller<TraitsT>::default_rls_forgetting_factor_ = 0.98;


template <typename TraitsT>
const typename lqr_application_controller<TraitsT>::real_type lqr_application_controller<TraitsT>::default_ewma_smoothing_factor_ = 0.7;

//template <typename TraitsT>
//const typename lqr_application_controller<TraitsT>::uint_type lqr_application_controller<TraitsT>::default_ss_state_size_ = 3;


//template <typename TraitsT>
//const typename lqr_application_controller<TraitsT>::uint_type lqr_application_controller<TraitsT>::default_ss_input_size_ = 3;


//template <typename TraitsT>
//const typename lqr_application_controller<TraitsT>::uint_type lqr_application_controller<TraitsT>::default_ss_output_size_ = 1;


}} // Namespace dcs::eesim


#endif // DCS_EESIM_LQR_APPLICATION_CONTROLLER_HPP
