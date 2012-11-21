#ifndef DCS_EESIM_LOGGING_BASE_LOGGER_HPP
#define DCS_EESIM_LOGGING_BASE_LOGGER_HPP


#include <dcs/functional/bind.hpp>
#include <dcs/des/engine_traits.hpp>
#include <dcs/memory.hpp>
#include <iosfwd>
#include <string>


namespace dcs { namespace eesim { namespace logging {

namespace detail { namespace /*<unnamed>*/ {

struct empty_deleter
{
	typedef void result_type;
	void operator()(volatile void const*) const {}
};

}} // Namespace detail::<unnamed>


template <typename TraitsT>
class base_logger
{
	private: typedef base_logger<TraitsT> self_type;
	public: typedef TraitsT traits_type;
	public: typedef ::std::ostream ostream_type;
	private: typedef typename traits_type::des_engine_type des_engine_type;
	private: typedef typename ::dcs::des::engine_traits<des_engine_type>::event_type des_event_type;
	private: typedef typename ::dcs::des::engine_traits<des_engine_type>::engine_context_type des_engine_context_type;


	public: base_logger()
	: ptr_os_(&::std::clog, detail::empty_deleter())
	{
	}


	public: void sink(::std::ostream* ptr_os)
	{
		ptr_os_ = ::dcs::shared_ptr< ::std::ostream >(ptr_os, detail::empty_deleter());
	}


	public: void sink(::std::string const& fname)
	{
		ptr_os_ = ::dcs::make_shared< ::std::ofstream >(fname.c_str());
	}


	public: void attach(des_engine_type& eng)
	{
		eng.begin_of_sim_event_source().connect(
				::dcs::functional::bind(
					&self_type::process_begin_of_sim_event,
					this,
					::dcs::functional::placeholders::_1,
					::dcs::functional::placeholders::_2
				)
			);
		eng.end_of_sim_event_source().connect(
				::dcs::functional::bind(
					&self_type::process_end_of_sim_event,
					this,
					::dcs::functional::placeholders::_1,
					::dcs::functional::placeholders::_2
				)
			);
		eng.system_initialization_event_source().connect(
				::dcs::functional::bind(
					&self_type::process_sys_init_event,
					this,
					::dcs::functional::placeholders::_1,
					::dcs::functional::placeholders::_2
				)
			);
		eng.system_finalization_event_source().connect(
				::dcs::functional::bind(
					&self_type::process_sys_finit_event,
					this,
					::dcs::functional::placeholders::_1,
					::dcs::functional::placeholders::_2
				)
			);

		do_attach(eng);
	}


	public: void detach(des_engine_type& eng)
	{
		do_detach(eng);

		eng.begin_of_sim_event_source().disconnect(
				::dcs::functional::bind(
					&self_type::process_begin_of_sim_event,
					this,
					::dcs::functional::placeholders::_1,
					::dcs::functional::placeholders::_2
				)
			);
		eng.end_of_sim_event_source().disconnect(
				::dcs::functional::bind(
					&self_type::process_end_of_sim_event,
					this,
					::dcs::functional::placeholders::_1,
					::dcs::functional::placeholders::_2
				)
			);
		eng.system_initialization_event_source().disconnect(
				::dcs::functional::bind(
					&self_type::process_sys_init_event,
					this,
					::dcs::functional::placeholders::_1,
					::dcs::functional::placeholders::_2
				)
			);
		eng.system_finalization_event_source().disconnect(
				::dcs::functional::bind(
					&self_type::process_sys_finit_event,
					this,
					::dcs::functional::placeholders::_1,
					::dcs::functional::placeholders::_2
				)
			);
	}


	protected: ostream_type const& sink() const
	{
		return *ptr_os_;
	}


	protected: ostream_type& sink()
	{
		return *ptr_os_;
	}


	private: void process_begin_of_sim_event(des_event_type const& evt, des_engine_context_type& ctx)
	{
		do_process_begin_of_sim_event(evt, ctx);
	}


	private: void process_end_of_sim_event(des_event_type const& evt, des_engine_context_type& ctx)
	{
		do_process_end_of_sim_event(evt, ctx);
	}


	private: void process_sys_init_event(des_event_type const& evt, des_engine_context_type& ctx)
	{
		do_process_sys_init_event(evt, ctx);
	}


	private: void process_sys_finit_event(des_event_type const& evt, des_engine_context_type& ctx)
	{
		do_process_sys_finit_event(evt, ctx);
	}


	private: virtual void do_attach(des_engine_type& /*eng*/)
	{
		// empty
	}


	private: virtual void do_detach(des_engine_type& /*eng*/)
	{
		// empty
	}


	private: virtual void do_process_begin_of_sim_event(des_event_type const& evt, des_engine_context_type& ctx) = 0;


	private: virtual void do_process_end_of_sim_event(des_event_type const& evt, des_engine_context_type& ctx) = 0;


	private: virtual void do_process_sys_init_event(des_event_type const& evt, des_engine_context_type& ctx) = 0;


	private: virtual void do_process_sys_finit_event(des_event_type const& evt, des_engine_context_type& ctx) = 0;


	private: ::dcs::shared_ptr<ostream_type> ptr_os_;
};

}}} // Namespace dcs::eesim::logging


#endif // DCS_EESIM_LOGGING_BASE_LOGGER_HPP
