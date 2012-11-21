/**
 * \file dcs/eesim/qn_application_controller.hpp
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

#ifndef DCS_EESIM_QN_APPLICATION_CONTROLLER_HPP
#define DCS_EESIM_QN_APPLICATION_CONTROLLER_HPP


//#include <algorithm>
//#ifdef DCS_DEBUG
//#	include <boost/numeric/ublas/io.hpp>
//#endif // DCS_DEBUG
//#include <boost/numeric/ublas/matrix.hpp>
////#include <boost/numeric/ublas/matrix_expression.hpp>
//#include <boost/numeric/ublas/matrix_proxy.hpp>
//#include <boost/numeric/ublas/vector.hpp>
//#include <boost/numeric/ublas/vector_proxy.hpp>
#include <dcs/debug.hpp>
//#include <dcs/des/base_statistic.hpp>
#include <dcs/des/engine_traits.hpp>
//#include <dcs/des/mean_estimator.hpp>
#include <dcs/eesim/base_application_controller.hpp>
#include <dcs/eesim/multi_tier_application.hpp>
#include <dcs/eesim/performance_measure_category.hpp>
#include <dcs/eesim/physical_machine.hpp>
#include <dcs/eesim/physical_resource_category.hpp>
//#include <dcs/eesim/registry.hpp>
#include <dcs/eesim/utility.hpp>
//#include <dcs/functional/bind.hpp>
#include <dcs/macro.hpp>
#include <dcs/memory.hpp>
#include <stdexcept>
#include <vector>


//TODO:
// - Currently the code in this class assumes the single resource (CPU) case.
//


namespace dcs { namespace eesim {

template <typename TraitsT>
class qn_application_controller: public base_application_controller<TraitsT>
{
	private: typedef base_application_controller<TraitsT> base_type;
	private: typedef qn_application_controller<TraitsT> self_type;
	public: typedef TraitsT traits_type;
	public: typedef typename traits_type::real_type real_type;
	public: typedef typename traits_type::uint_type uint_type;
	public: typedef typename base_type::application_pointer application_pointer;
	private: typedef ::std::size_t size_type;
	private: typedef typename traits_type::des_engine_type des_engine_type;
	private: typedef typename ::dcs::des::engine_traits<des_engine_type>::event_type des_event_type;
	private: typedef typename ::dcs::des::engine_traits<des_engine_type>::engine_context_type des_engine_context_type;


	private: static const real_type default_min_share_;


	public: qn_application_controller()
	: base_type()
	{
	}


	public: qn_application_controller(application_pointer const& ptr_app, real_type ts)
	: base_type(ptr_app, ts)
	{
	}


	//@{ Inteface Member Functions

	private: void do_process_control(des_event_type const& evt, des_engine_context_type& ctx)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );

		DCS_DEBUG_TRACE("(" << this << ") BEGIN Do Process CONTROL event (Clock: " << ctx.simulated_time() << ")");

//		namespace ublas = ::boost::numeric::ublas;

//		typedef ::boost::numeric::ublas::matrix<real_type> matrix_type;
//		typedef ::boost::numeric::ublas::vector<real_type> vector_type;
		typedef typename base_type::application_type application_type;
		typedef typename application_type::simulation_model_type application_simulation_model_type;
		typedef typename application_type::performance_model_type application_performance_model_type;
		typedef typename application_simulation_model_type::virtual_machine_type virtual_machine_type;
		typedef ::dcs::shared_ptr<virtual_machine_type> virtual_machine_pointer;
		typedef physical_machine<traits_type> physical_machine_type;
		typedef ::std::vector<performance_measure_category> perf_category_container;
		typedef typename perf_category_container::const_iterator perf_category_iterator;

		application_type const& app(this->application());
		application_simulation_model_type const& app_sim_model(app.simulation_model());
		application_performance_model_type const& app_perf_model(app.performance_model());

		// Create a QN model to reflect current observations
		//FIXME:
		// - actually only single-class customers are handled here.
		// - actually only single-server stations are handled here.
		//

		// The number of customer classes
		size_type nc(1);
		// The number of stations (i.e., the number of tiers)
		size_type ns(app.num_tiers());

/*
		// The vector of external arrival rates
		vector_type lambda(ns, 0);
		// The visit ratios
		matrix_type V(nc, ns, 0);
		// The service times
		matrix_type S(nc, ns, 0);
		// The multi-programming level for each station
		vector_type m(ns, 1);//FIXME: assume one server per station

		// NOTE: actually we assume that external arrivals always enters the system from the first tier;
		lambda(0) = app_sim_model.actual_num_arrivals()/ctx.simulated_time();

		for (size_type c = 0; s < nc; ++c)
		{
			for (size_type s = 0; s < ns; ++s)
			{
				//FIXME: these operational laws assume single-class customers

				V(c,s) = app_sim_model.actual_tier_num_departures(s)/app_sim_model.actual_num_departures();
				S(c,s) = app_sim_model.actual_tier_busy_time(s)/app_sim_model.actual_tier_num_departures(s);
			}
		}

		qn_model_type qn(lambda, S, V, m);

		bool solved;

		solved = qn.solve();

		if (solved)
		{
			vector_type u(qn.station_utilizations());

DCS_DEBUG_TRACE("Applying control");//XXX
			for (size_type c = 0; s < nc; ++c)
			{
				for (size_type s = 0; s < ns; ++s)
				{
					//FIXME: these operational laws assume single-class customers

					physical_resource_category res_category(cpu_resource_category);//FIXME

					virtual_machine_pointer ptr_vm(app_sim_model.tier_virtual_machine(s));
					physical_machine_type const& pm(ptr_vm->vmm().hosting_machine());

					real_type share;
					share = ::dcs::eesim::scale_resource_share(
							// Reference resource capacity and threshold
							app.reference_resource(res_category).capacity(),
							app.reference_resource(res_category).utilization_threshold(),
							// Actual resource capacity and threshold
							pm.resource(res_category)->capacity(),
							pm.resource(res_category)->utilization_threshold(),
							// Old resource + computed deviation
							u(s)
						);
															
DCS_DEBUG_TRACE("Assigning new wanted share: VM: " << ptr_vm->name() << " (" << ptr_vm->id() << ") - Category: " << res_category << " ==> Share: " << share);//XXX
					ptr_vm->wanted_resource_share(res_category, share);
				}
			}
DCS_DEBUG_TRACE("Control applied");//XXX
		}
*/

		perf_category_container slo_categories(app.sla_cost_model().slo_categories());
		perf_category_iterator end_it = slo_categories.end();

		for (size_type c = 0; c < nc; ++c)
		{
			//FIXME: CPU resource category is the only category currently handled

			physical_resource_category res_category(cpu_resource_category);

			for (size_type s = 0; s < ns; ++s)
			{
				//FIXME: these operational laws assume single-class customers

				virtual_machine_pointer ptr_vm(app_sim_model.tier_virtual_machine(s));
				physical_machine_type const& pm(ptr_vm->vmm().hosting_machine());

				// Actual to Reference resource scaling factor
//				real_type scale_factor;
//				scale_factor = ::dcs::eesim::resource_scaling_factor(
//						pm.resource(res_category)->capacity(),
//						pm.resource(res_category)->utilization_threshold(),
//						app.reference_resource(res_category).capacity(),
//						app.reference_resource(res_category).utilization_threshold()
//					);

				real_type new_share(0);

				for (perf_category_iterator it = slo_categories.begin(); it != end_it; ++it)
				{
					performance_measure_category perf_category(*it);

					switch (perf_category)
					{
						case response_time_performance_measure:
							{
								// Compute the utilization needed to achieve the reference
								// response time w.r.t to the observed service demand (and
								// w.r.t. to the reference machine).

								real_type ref_demand;
DCS_DEBUG_TRACE("Tier: " << s);//XXX
//DCS_DEBUG_TRACE("Scale Factor: " << scale_factor);//XXX
DCS_DEBUG_TRACE("#Arrivals: " << app_sim_model.actual_num_arrivals());//XXX
DCS_DEBUG_TRACE("#Departures: " << app_sim_model.actual_num_departures());//XXX
DCS_DEBUG_TRACE("Tier #Arrivals: " << app_sim_model.actual_tier_num_arrivals(s));//XXX
DCS_DEBUG_TRACE("Tier #Departures: " << app_sim_model.actual_tier_num_departures(s));//XXX
DCS_DEBUG_TRACE("Tier Busy Time: " << app_sim_model.actual_tier_busy_time(s));//XXX
//								real_type busy_time(app_sim_model.actual_tier_busy_time(s));
//								uint_type num_sys_deps(app_sim_model.actual_num_departures());
//								if (busy_time > 0 && num_sys_deps > 0)
//								{
//									ref_demand = busy_time/num_sys_deps;
//DCS_DEBUG_TRACE("Actual Resource demand: " << busy_time/num_sys_deps);//XXX
//DCS_DEBUG_TRACE("Reference Resource demand: " << ref_demand);//XXX
//									real_type ref_rt;
//									ref_rt = app_perf_model.tier_measure(s, perf_category);
//DCS_DEBUG_TRACE("Actual Response time: " << (ref_demand/(1-app_sim_model.actual_tier_num_arrivals(s)*ref_demand)));//XXX
//DCS_DEBUG_TRACE("Reference Response time: " << ref_rt);//XXX
//									real_type want_u;
//									want_u = ref_demand*(real_type(1)/ref_rt + app_sim_model.actual_tier_num_arrivals(s));
//DCS_DEBUG_TRACE("Actual U: " << (app_sim_model.actual_num_arrivals()*ref_demand));//XXX
//DCS_DEBUG_TRACE("Wanted U: " << want_u);//XXX
//									if (want_u < 0)
//									{
//										// This means that there is a very high demand
//										want_u = 1;
//									}
//
//									// Compute the service share as the utilization needed in the actual machine.
//
//									new_share = ::dcs::eesim::scale_resource_share(
//													app.reference_resource(res_category).capacity(),
//													app.reference_resource(res_category).utilization_threshold(),
//													pm.resource(res_category)->capacity(),
//													pm.resource(res_category)->utilization_threshold(),
//													want_u
//											);
//								}
//								else
//								{
//									new_share = ptr_vm->resource_share(res_category);
//								}
								ref_demand = app_perf_model.tier_measure(s, utilization_performance_measure)/app_perf_model.tier_measure(s, throughput_performance_measure);
DCS_DEBUG_TRACE("Reference Resource demand: " << ref_demand);//XXX
								real_type ref_rt;
								ref_rt = app_perf_model.tier_measure(s, perf_category);
DCS_DEBUG_TRACE("Actual Response time: " << (ref_demand/(1-app_sim_model.actual_tier_num_arrivals(s)*ref_demand)));//XXX
DCS_DEBUG_TRACE("Reference Response time: " << ref_rt);//XXX
								real_type want_u;
//									want_u = 1 - ref_demand/ref_rt;
								want_u = ref_demand*(real_type(1)/ref_rt + app_sim_model.actual_tier_num_arrivals(s));
//									want_u = 1 + ref_rt*app_sim_model.actual_tier_num_arrivals(s);
DCS_DEBUG_TRACE("Actual U: " << (app_sim_model.actual_num_arrivals()*ref_demand));//XXX
DCS_DEBUG_TRACE("Wanted U: " << want_u);//XXX
								if (want_u < 0)
								{
									// This means that there is a very high demand
									want_u = 1;
								}

								// Compute the service share as the utilization needed in the actual machine.

								new_share = ::dcs::eesim::scale_resource_share(
												app.reference_resource(res_category).capacity(),
												//app.reference_resource(res_category).utilization_threshold(),
												pm.resource(res_category)->capacity(),
												//pm.resource(res_category)->utilization_threshold(),
												want_u
										);
DCS_DEBUG_TRACE("New Share: " << new_share);//XXX
							}
							break;
						default:
							throw ::std::runtime_error("[dcs::eesim::qn_application_controller::do_process_control] Currently only the response-time statistic is handled.");
					}

					new_share = ::std::max(new_share, default_min_share_);

DCS_DEBUG_TRACE("Assigning new wanted share: VM: " << ptr_vm->name() << " (" << ptr_vm->id() << ") - Category: " << res_category << " - Actual Share: " << ptr_vm->resource_share(res_category) << " ==> New Share: " << new_share);//XXX
					ptr_vm->wanted_resource_share(res_category, new_share);
				}
			}
		}

		DCS_DEBUG_TRACE("(" << this << ") END Do Process CONTROL event (Clock: " << ctx.simulated_time() << ")");
	}

	//@} Inteface Member Functions
};

template <typename TraitsT>
const typename qn_application_controller<TraitsT>::real_type qn_application_controller<TraitsT>::default_min_share_ = 0.01;

}} // Namespace dcs::eesim


#endif // DCS_EESIM_QN_APPLICATION_CONTROLLER_HPP
