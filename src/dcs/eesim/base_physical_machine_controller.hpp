#ifndef DCS_EESIM_BASE_PHYSICAL_MACHINE_CONTROLLER_HPP
#define DCS_EESIM_BASE_PHYSICAL_MACHINE_CONTROLLER_HPP


#include <dcs/assert.hpp>
#include <dcs/debug.hpp>
#include <dcs/des/engine_traits.hpp>
#include <dcs/eesim/physical_machine.hpp>
#include <dcs/eesim/registry.hpp>
#include <dcs/functional/bind.hpp>
#include <dcs/macro.hpp>
#include <dcs/memory.hpp>


namespace dcs { namespace eesim {

template <typename TraitsT>
class base_physical_machine_controller
{
	private: typedef base_physical_machine_controller<TraitsT> self_type;
	public: typedef TraitsT traits_type;
	public: typedef typename traits_type::real_type real_type;
	public: typedef physical_machine<traits_type> physical_machine_type;
	//public: typedef physical_machine_type* physical_machine_pointer;
	public: typedef ::dcs::shared_ptr<physical_machine_type> physical_machine_pointer;
	private: typedef typename traits_type::des_engine_type des_engine_type;
	private: typedef typename ::dcs::des::engine_traits<des_engine_type>::engine_context_type des_engine_context_type;
	private: typedef typename ::dcs::des::engine_traits<des_engine_type>::event_type des_event_type;
	private: typedef typename ::dcs::des::engine_traits<des_engine_type>::event_source_type des_event_source_type;
	public: typedef ::dcs::shared_ptr<des_event_source_type> des_event_source_pointer;
	private: typedef registry<traits_type> registry_type;


	/// A constructor.
	protected: base_physical_machine_controller()
	: ptr_mach_(),
	  ts_(0),
	  ptr_control_evt_src_(/*new des_event_source_type()*/)
	{
		init();
	}


	/// A constructor.
	protected: explicit base_physical_machine_controller(physical_machine_pointer const& ptr_mach, real_type ts)
	: ptr_mach_(ptr_mach),
	  ts_(ts),
	  ptr_control_evt_src_(/*new des_event_source_type()*/)
	{
		init();
	}


	/// Copy constructor.
	public: base_physical_machine_controller(base_physical_machine_controller const& that)
	: ptr_mach_(that.ptr_mach_),
	  ts_(that.ts_),
	  ptr_control_evt_src_(that.ptr_control_evt_src_ ? new des_event_source_type(*that.ptr_control_evt_src_) : des_event_source_type())
	{
		init();
	}


	/// Copy assignment.
	public: base_physical_machine_controller& operator=(base_physical_machine_controller const& rhs)
	{
		if (this != &rhs)
		{
			this->disconnect_from_event_sources();

			ptr_mach_ = rhs.ptr_mach_;
			ts_ = rhs.ts_;
			if (rhs.ptr_control_evt_src_)
			{
				ptr_control_evt_src_ = ::dcs::make_shared<des_event_source_type>(*(rhs.ptr_control_evt_src_));
			}
			else
			{
				ptr_control_evt_src_ = des_event_source_type();
			}

			init();
		}

		return *this;
	}


	/// The destructor.
	public: virtual ~base_physical_machine_controller()
	{
		this->disconnect_from_event_sources();
	}


	public: void machine(physical_machine_pointer const& ptr_mach)
	{
		do_machine(ptr_mach);

		ptr_mach_ = ptr_mach;
	}


	public: physical_machine_type& machine()
	{
		// pre: ptr_mach_ must be a valid physical machine pointer.
		DCS_DEBUG_ASSERT( ptr_mach_ );

		return *ptr_mach_;
	}


	public: physical_machine_type const& machine() const
	{
		// pre: ptr_mach_ must be a valid physical machine pointer.
		DCS_DEBUG_ASSERT( ptr_mach_ );

		return *ptr_mach_;
	}


	public: des_event_source_type const& control_event_source() const
	{
		return *ptr_control_evt_src_;
	}


	public: void sampling_time(real_type ts)
	{
		this->disconnect_from_event_sources();

		ts_ = ts;

		this->connect_to_event_sources();
	}


	public: real_type sampling_time() const
	{
		return ts_;
	}


	protected: physical_machine_pointer machine_ptr() const
	{
		return ptr_mach_;
	}


	protected: physical_machine_pointer machine_ptr()
	{
		return ptr_mach_;
	}


	private: void init()
	{
		this->connect_to_event_sources();
	}


	private: void connect_to_event_sources()
	{
DCS_DEBUG_TRACE("BEGIN connect to event sources: sampling time: " << ts_);//XXX
		if (ts_ > 0)
		{
			if (!ptr_control_evt_src_)
			{
				ptr_control_evt_src_ = ::dcs::make_shared<des_event_source_type>();
			}

DCS_DEBUG_TRACE("Connecting to contrl event sources");//XXX
			ptr_control_evt_src_->connect(
				::dcs::functional::bind(
					&self_type::process_control,
					this,
					::dcs::functional::placeholders::_1,
					::dcs::functional::placeholders::_2
				)
			);

			registry_type& reg(registry_type::instance());

DCS_DEBUG_TRACE("Connecting to sys inti event sources");//XXX
			reg.des_engine_ptr()->system_initialization_event_source().connect(
				::dcs::functional::bind(
					&self_type::process_sys_init,
					this,
					::dcs::functional::placeholders::_1,
					::dcs::functional::placeholders::_2
				)
			);
		}
DCS_DEBUG_TRACE("END connect to event sources");//XXX
	}


	private: void disconnect_from_event_sources()
	{
		if (ts_ > 0)
		{
DCS_DEBUG_TRACE("END connect to event sources");//XXX
			if (ptr_control_evt_src_)
			{
				ptr_control_evt_src_->disconnect(
					::dcs::functional::bind(
						&self_type::process_control,
						this,
						::dcs::functional::placeholders::_1,
						::dcs::functional::placeholders::_2
					)
				);
	DCS_DEBUG_TRACE("END disconnect to event sources");//XXX
			}

			registry_type& reg(registry_type::instance());

			reg.des_engine_ptr()->system_initialization_event_source().disconnect(
				::dcs::functional::bind(
					&self_type::process_sys_init,
					this,
					::dcs::functional::placeholders::_1,
					::dcs::functional::placeholders::_2
				)
			);
		}
	}


	//@{ Event Triggers

	private: void schedule_control()
	{
		if (ts_ > 0)
		{
			registry_type& reg(registry_type::instance());

			reg.des_engine_ptr()->schedule_event(
				ptr_control_evt_src_,
				reg.des_engine_ptr()->simulated_time() + ts_
			);
		}

		do_schedule_control();
	}

	//@} Event Triggers


	//@{ Event Handlers


	private: void process_sys_init(des_event_type const& evt, des_engine_context_type& ctx)
	{
		schedule_control();

		do_process_sys_init(evt, ctx);
	}


	private: void process_control(des_event_type const& evt, des_engine_context_type& ctx)
	{
		schedule_control();

		do_process_control(evt, ctx);
	}


	//@} Event Handlers

	//@{ Interface Member Functions

	protected: virtual void do_machine(physical_machine_pointer const& ptr_mach)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ptr_mach );

		// empty
	}


	protected: virtual void do_process_sys_init(des_event_type const& evt, des_engine_context_type& ctx)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );

		// empty
	}


	private: virtual void do_process_control(des_event_type const& evt, des_engine_context_type& ctx) = 0;


	protected: virtual void do_schedule_control()
	{
		// empty
	}

	//@} Interface Member Functions


	private: physical_machine_pointer ptr_mach_;
	private: real_type ts_;
	private: des_event_source_pointer ptr_control_evt_src_;
};

}} // Namespace dcs::eesim


#endif // DCS_EESIM_BASE_PHYSICAL_MACHINE_CONTROLLER_HPP
