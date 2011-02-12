#ifndef DCS_EESIM_LOGGING_COMPACT_LOGGER_HPP
#define DCS_EESIM_LOGGING_COMPACT_LOGGER_HPP


#include <cstddef>
#include <ctime>
#include <dcs/des/engine_traits.hpp>
#include <dcs/eesim/logging/minimal_logger.hpp>
#include <dcs/macro.hpp>
#include <iostream>
#include <stdexcept>
#include <string>


namespace dcs { namespace eesim { namespace logging {

namespace detail { namespace /*<unnamed>*/ {

::std::string date_now(bool utc = true)
{
	::std::time_t raw_time;
	::std::time(&raw_time);

	::std::tm time_info;
	if (utc)
	{
		::gmtime_r(&raw_time, &time_info);
	}
	else
	{
		::localtime_r(&raw_time, &time_info);
	}

	const ::std::size_t buf_size(10+1+8+1+5+1);
	char buf[buf_size];
	::std::size_t sz;
	sz = ::std::strftime(buf, buf_size, "%Y-%m-%d %H:%M:%S %z", &time_info);
	if (!sz)
	{
		throw ::std::runtime_error("[dcs::eesim::logging::detail::date_now] Unexepected error.");
	}

	return ::std::string(buf);
}

}} // Namespace detail::<unnamed>


template <typename TraitsT>
class compact_logger: public minimal_logger<TraitsT>
{
	private: typedef minimal_logger<TraitsT> base_type;
	private: typedef compact_logger<TraitsT> self_type;
	public: typedef TraitsT traits_type;
	private: typedef typename traits_type::des_engine_type des_engine_type;
	private:  typedef typename ::dcs::des::engine_traits<des_engine_type>::event_type des_event_type;
	private:  typedef typename ::dcs::des::engine_traits<des_engine_type>::engine_context_type des_engine_context_type;


	public: compact_logger()
	: base_type()
	{
	}


	public: void attach(des_engine_type& eng)
	{
		base_type::attach(eng);

		eng.after_of_event_firing_source().connect(
				::dcs::functional::bind(
					&self_type::process_after_firing_event,
					this,
					::dcs::functional::placeholders::_1,
					::dcs::functional::placeholders::_2
				)
			);
	}


	public: void detach(des_engine_type& eng)
	{
		eng.after_of_event_firing_source().disconnect(
				::dcs::functional::bind(
					&self_type::process_after_firing_event,
					this,
					::dcs::functional::placeholders::_1,
					::dcs::functional::placeholders::_2
				)
			);

		base_type::detach(eng);
	}


	private: void process_after_firing_event(des_event_type const& evt, des_engine_context_type& ctx)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );

		this->sink() << "." << ::std::flush;
	}


	private: void do_process_begin_of_sim_event(des_event_type const& evt, des_engine_context_type& ctx)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );

		this->sink() << "{"
					 << detail::date_now()
					 << ::std::endl;
	}


	private: void do_process_end_of_sim_event(des_event_type const& evt, des_engine_context_type& ctx)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );

		this->sink() << detail::date_now()
					 << "}" << ::std::endl;
	}


	private: void do_process_sys_init_event(des_event_type const& evt, des_engine_context_type& ctx)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );

		this->sink() << "[" << ::std::flush;
	}


	private: void do_process_sys_finit_event(des_event_type const& evt, des_engine_context_type& ctx)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );

		this->sink() << "]" << ::std::endl;
	}
};

}}} // Namespace dcs::eesim::logging


#endif // DCS_EESIM_LOGGING_COMPACT_LOGGER_HPP
