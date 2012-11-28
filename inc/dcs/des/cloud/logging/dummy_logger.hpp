#ifndef DCS_DES_CLOUD_LOGGING_DUMMY_LOGGER_HPP
#define DCS_DES_CLOUD_LOGGING_DUMMY_LOGGER_HPP


#include <dcs/des/engine_traits.hpp>
#include <dcs/des/cloud/logging/base_logger.hpp>
#include <dcs/macro.hpp>
#include <iostream>


namespace dcs { namespace des { namespace cloud { namespace logging {

template <typename TraitsT>
class dummy_logger: public base_logger<TraitsT>
{
	private: typedef base_logger<TraitsT> base_type;
	private: typedef dummy_logger<TraitsT> self_type;
	public: typedef TraitsT traits_type;
	private: typedef typename traits_type::des_engine_type des_engine_type;
	private:  typedef typename ::dcs::des::engine_traits<des_engine_type>::event_type des_event_type;
	private:  typedef typename ::dcs::des::engine_traits<des_engine_type>::engine_context_type des_engine_context_type;


	private: void do_process_begin_of_sim_event(des_event_type const& evt, des_engine_context_type& ctx)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );
	}


	private: void do_process_end_of_sim_event(des_event_type const& evt, des_engine_context_type& ctx)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );
	}


	private: void do_process_sys_init_event(des_event_type const& evt, des_engine_context_type& ctx)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );
	}


	private: void do_process_sys_finit_event(des_event_type const& evt, des_engine_context_type& ctx)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );
	}
};

}}}} // Namespace dcs::des::cloud::logging


#endif // DCS_DES_CLOUD_LOGGING_DUMMY_LOGGER_HPP
