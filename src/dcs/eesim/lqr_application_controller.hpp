/**
 * \file dcs/eesim/lqr_application_controller.hpp
 *
 * \brief Class modeling the application controller component using an LQR
 *  controller.
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

#ifndef DCS_EESIM_LQR_APPLICATION_CONTROLLER_HPP
#define DCS_EESIM_LQR_APPLICATION_CONTROLLER_HPP


//#include <boost/numeric/ublas/matrix.hpp>
//#include <boost/numeric/ublas/vector.hpp>
//#include <boost/numeric/ublas/vector_expression.hpp>
//#include <boost/numeric/ublas/matrix_expression.hpp>
#include <dcs/control/design/dlqr.hpp>
#include <dcs/eesim/base_application_controller.hpp>
#include <dcs/eesim/multi_tier_application.hpp>
#include <dcs/eesim/performance_measure.hpp>
#include <dcs/macro.hpp>
#include <dcs/memory.hpp>
#include <vector>


namespace dcs { namespace eesim {

template <typename TraitsT>
class lqr_application_controller: public base_application_controller<TraitsT>
{
	private: typedef base_application_controller<TraitsT> base_type;
	private: typedef lqr_application_controller<TraitsT> self_type;
	public: typedef TraitsT traits_type;
	public: typedef typename traits_type::real_type real_type;
	public: typedef typename traits_type::uint_type uint_type;
	public: typedef ::dcs::control::dlqr_controller<real_type> controller_type;
	public: typedef ::dcs::shared_ptr<controller_type> controller_pointer;
	public: typedef typename base_type::application_pointer application_pointer;
	private: typedef typename traits_type::des_engine_type des_engine_type;
	private: typedef typename ::dcs::des::engine_traits<des_engine_type>::event_type des_event_type;
	private: typedef typename ::dcs::des::engine_traits<des_engine_type>::engine_context_type des_engine_context_type;
	private: typedef performance_measure<traits_type> performance_measure_type;


	public: lqr_application_controller()
	: base_type(),
	  ptr_ctrl_()
	{
	}


	public: lqr_application_controller(controller_pointer const& ptr_control, application_pointer const& ptr_app, real_type ts)
	: base_type(ptr_app, ts),
	  ptr_ctrl_(ptr_control)
	{
	}


	public: void controller(controller_pointer const& ptr_control)
	{
		ptr_ctrl_ = ptr_control;
	}


	private: void do_process_control(des_event_type const& evt, des_engine_context_type& ctx)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );

		::std::vector<performance_measure_type> measures;
		::std::vector<performance_measure_type> ref_measures;

		//measures = ptr_app_->statistic(response_time_application_statistic)->retrieve();
//		measures = this->application_ptr()->performance_measures();
		ref_measures = this->application_ptr()->reference_performance_measures();

/*
		vector_type meas(
			measures.begin(),
			measures.end()
		);
		vector_type ref_meas(
			ref_measures.begin(),
			ref_measures.end()
		);

		vector_type shares = controller_.control(meas-ref_meas);
*/
	}


	private: controller_pointer ptr_ctrl_;
};

}} // Namespace dcs::eesim


#endif // DCS_EESIM_LQR_APPLICATION_CONTROLLER_HPP
