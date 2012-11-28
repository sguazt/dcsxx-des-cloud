/**
 * \file dcs/des/cloud/application_controller.hpp
 *
 * \brief Class modeling the application controller component.
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
 * \author Marco Guazzone (marco.guazzone@gmail.com)
 */

#ifndef DCS_DES_CLOUD_PID_APPLICATION_CONTROLLER_HPP
#define DCS_DES_CLOUD_PID_APPLICATION_CONTROLLER_HPP


#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/vector_expression.hpp>
#include <boost/numeric/ublas/matrix_expression.hpp>
#include <dcs/control/design/pid_controller.hpp>
#include <dcs/des/cloud/base_application_statistics_retriever.hpp>
#include <dcs/des/cloud/multi_tier_application.hpp>
#include <dcs/des/cloud/registry.hpp>
#include <dcs/exception.hpp>
#include <dcs/functional/bind.hpp>
#include <dcs/macro.hpp>
#include <dcs/memory.hpp>
#include <stdexcept>
#include <vector>


namespace dcs { namespace des { namespace cloud {

template <typename TraitsT>
class pid_application_controller
{
	public: typedef TraitsT traits_type;
	public: typedef typename traits_type::real_type real_type;
	public: typedef typename traits_type::uint_type uint_type;
	public: typedef multi_tier_application<traits_type> application_type;
	public: typedef ::dcs::shared_ptr<application_type> application_pointer;
	public: typedef base_application_statistics_retriever<traits_type> statistics_retriever_type;
	public: typedef ::dcs::shared_ptr<statistics_retriever_type> statistics_retriever_pointer;
	public: typedef ::boost::numeric::ublas::vector<real_type> vector_type;
	private: typedef ::boost::numeric::ublas::matrix<real_type> matrix_type;
	private: typedef typename traits_type::des_engine_type des_engine_type;
	private: typedef typename des_engine_type::engine_context_type des_engine_context_type;
	private: typedef typename des_engine_type::event_type des_event_type;
	private: typedef typename des_engine_type::event_source_type des_event_source_type;
	private: typedef ::dcs::shared_ptr<des_event_source_type> des_event_source_pointer;
	private: typedef registry<traits_type> registry_type;
	private: typedef application_controller<traits_type> self_type;


	private: static const real_type default_Kp;
	private: static const real_type default_Ki;
	private: static const real_type default_Kd;
	private: static const real_type default_sample_time;


	public: pid_application_controller()
		: controller_(
			matrix_type(0, 0, default_Kp),
			matrix_type(0, 0, default_Ki),
			matrix_type(0, 0, default_Kd),
			default_sample_time) //FIXME
	{
		init();
	}


	public: pid_application_controller(application_pointer const& ptr_app)
		: controller_(
			matrix_type(ptr_app->num_tiers(), ptr_app->num_tiers(), default_Kp),
			matrix_type(ptr_app->num_tiers(), ptr_app->num_tiers(), default_Ki),
			matrix_type(ptr_app->num_tiers(), ptr_app->num_tiers(), default_Kd),
			default_sample_time), //FIXME
		  ptr_app_(ptr_app)
	{
		init();
	}


	/// Copy constructor.
	private: pid_application_controller(pid_application_controller const& that)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(that);

		//TODO
		DCS_EXCEPTION_THROW( ::std::runtime_error, "Copy-constructor not yet implemented." );
	}


	/// Copy assignment.
	private: pid_application_controller& operator=(pid_application_controller const& rhs)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(rhs);

		//TODO
		DCS_EXCEPTION_THROW( ::std::runtime_error, "Copy-assigment not yet implemented." );
	}


	public: void controlled_application(application_pointer const& ptr_app)
	{
		ptr_app_ = ptr_app;
	}


	public: real_type sampling_time() const
	{
		return controller_.sampling_time();
	}


	public: void start()
	{
		prepare_run();
	}


	public: void stop()
	{
		//TODO
	}


	private: void init()
	{
		ptr_control_evt_src_->connect(
			::dcs::functional::bind(
				&self_type::process_control,
				this,
				::dcs::functional::placeholders::_1,
				::dcs::functional::placeholders::_2
			)
		);

//		registry_type& reg(registry_type::instance());
//
//		reg.des_engine().system_initialization_event_source().connect(
//			::dcs::functional::bind(
//				&self_type::process_system_initialization,
//				this,
//				::dcs::functional::placeholders::_1,
//				::dcs::functional::placeholders::_2
//			)
//		);
	}


	private: void prepare_run()
	{
		schedule_control();
	}


	private: void schedule_control()
	{
		registry_type& ref_reg(registry_type::instance());

		ref_reg.des_engine_ptr()->schedule_event(
			ptr_control_evt_src_,
			reg.des_engine().simulated_time() + controller_.sampling_time()
		);
	}


	private: void process_control(des_event_type const& evt, des_engine_context_type& ctx)
	{
		::std::vector<real_type> measures;
		::std::vector<real_type> ref_measures;

		//measures = ptr_app_->statistic(response_time_application_statistic)->retrieve();
		measures = ptr_app_->performance_measures()->retrieve();
		ref_measures = ptr_app_->reference_performance_measures();

		vector_type meas(
			measures.begin(),
			measures.end()
		);
		vector_type ref_meas(
			ref_measures.begin(),
			ref_measures.end()
		);

		vector_type shares = controller_.control(meas-ref_meas);
	}


	private: ::dcs::control::dlqr_controller<real_type> controller_;
	private: ::dcs::control::mimo_pid_controller<vector_type, matrix_type, real_type> controller_;
	private: application_pointer ptr_app_;
	private: des_event_source_pointer ptr_control_evt_src_;
}; // pid_application_controller

template <typename TraitsT>
const typename TraitsT::real_type pid_application_controller<TraitsT>::default_Kp = 5;

template <typename TraitsT>
const typename TraitsT::real_type pid_application_controller<TraitsT>::default_Ki = 10;

template <typename TraitsT>
const typename TraitsT::real_type pid_application_controller<TraitsT>::default_Kd = 0.05;

template <typename TraitsT>
const typename TraitsT::real_type pid_application_controller<TraitsT>::default_sample_time = 1;

}}} // Namespace dcs::des::cloud


#endif // DCS_DES_CLOUD_PID_APPLICATION_CONTROLLER_HPP
