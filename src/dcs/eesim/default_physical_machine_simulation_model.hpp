#ifndef DCS_EESIM_DEFAULT_PHYSICAL_MACHINE_SIMULATION_MODEL_HPP
#define DCS_EESIM_DEFAULT_PHYSICAL_MACHINE_SIMULATION_MODEL_HPP


#include <dcs/debug.hpp>
#include <dcs/des/engine_traits.hpp>
#include <dcs/des/mean_estimator.hpp>
#include <dcs/eesim/base_physical_machine_simulation_model.hpp>
#include <dcs/eesim/physical_resource_category.hpp>
#include <dcs/eesim/power_status.hpp>
#include <dcs/eesim/registry.hpp>
#include <dcs/eesim/resource_utilization_profile.hpp>
#include <dcs/eesim/user_request.hpp>
#include <dcs/functional/bind.hpp>
#include <dcs/macro.hpp>
#include <string>


//FIXME:
// - statistic type (mean estimator) is hard-coded
//


namespace dcs { namespace eesim {

template <typename TraitsT>
class default_physical_machine_simulation_model: public base_physical_machine_simulation_model<TraitsT>
{
	private: typedef base_physical_machine_simulation_model<TraitsT> base_type;
	private: typedef default_physical_machine_simulation_model<TraitsT> self_type;
	public: typedef TraitsT traits_type;
	private: typedef typename traits_type::real_type real_type;
	private: typedef typename traits_type::uint_type uint_type;
	private: typedef typename base_type::output_statistic_type output_statistic_type;
	private: typedef typename base_type::output_statistic_pointer output_statistic_pointer;
	private: typedef typename base_type::des_event_source_type des_event_source_type;
	private: typedef ::dcs::shared_ptr<des_event_source_type> des_event_source_pointer;
	private: typedef typename base_type::des_event_type des_event_type;
	private: typedef typename traits_type::des_engine_type des_engine_type;
	private: typedef typename ::dcs::des::engine_traits<des_engine_type>::engine_context_type des_engine_context_type;
	private: typedef ::dcs::des::mean_estimator<real_type,uint_type> mean_estimator_statistic_type;
	private: typedef registry<traits_type> registry_type;
	private: typedef typename base_type::physical_machine_type physical_machine_type;
	private: typedef typename base_type::virtual_machine_pointer virtual_machine_pointer;
	private: typedef user_request<traits_type> user_request_type;
	private: typedef resource_utilization_profile<traits_type> utilization_profile_type;


	private: static const ::std::string poweron_event_source_name;
	private: static const ::std::string poweroff_event_source_name;


	public: default_physical_machine_simulation_model()
	: base_type(),
	  pwr_state_(powered_off_power_status),
//	  energy_(0),
	  uptime_(0),
	  last_pwron_time_(0),
	  ptr_pwron_evt_src_(new des_event_source_type(poweron_event_source_name)),
	  ptr_pwroff_evt_src_(new des_event_source_type(poweroff_event_source_name)),
	  ptr_vm_pwron_evt_src_(new des_event_source_type(poweron_event_source_name)),
	  ptr_vm_pwroff_evt_src_(new des_event_source_type(poweroff_event_source_name)),
	  ptr_vm_migr_evt_src_(new des_event_source_type(poweroff_event_source_name)),
	  ptr_energy_stat_(new mean_estimator_statistic_type()), //FIXME: statistic type (mean) is hard-coded
	  ptr_uptime_stat_(new mean_estimator_statistic_type()), //FIXME: statistic type (mean) is hard-coded
	  ptr_util_stat_(new mean_estimator_statistic_type()) //FIXME: statistic type (mean) is hard-coded
	{
		init();
	}


	/// The destructor
	public: ~default_physical_machine_simulation_model()
	{
		disconnect_from_event_sources();
	}


	private: void init()
	{
		connect_to_event_sources();
	}


	private: void connect_to_event_sources()
	{
		// Connect to foreign event sources
		registry_type& ref_reg = registry_type::instance();

		ref_reg.des_engine_ptr()->begin_of_sim_event_source().connect(
			::dcs::functional::bind(
				&self_type::process_begin_of_sim,
				this,
				::dcs::functional::placeholders::_1,
				::dcs::functional::placeholders::_2
			)
		);
		ref_reg.des_engine_ptr()->system_initialization_event_source().connect(
			::dcs::functional::bind(
				&self_type::process_sys_init,
				this,
				::dcs::functional::placeholders::_1,
				::dcs::functional::placeholders::_2
			)
		);
		ref_reg.des_engine_ptr()->system_finalization_event_source().connect(
			::dcs::functional::bind(
				&self_type::process_sys_finit,
				this,
				::dcs::functional::placeholders::_1,
				::dcs::functional::placeholders::_2
			)
		);

		// Connect to local event sources
		ptr_pwron_evt_src_->connect(
			::dcs::functional::bind(
				&self_type::process_power_on,
				this,
				::dcs::functional::placeholders::_1,
				::dcs::functional::placeholders::_2
			)
		);
		ptr_pwroff_evt_src_->connect(
			::dcs::functional::bind(
				&self_type::process_power_off,
				this,
				::dcs::functional::placeholders::_1,
				::dcs::functional::placeholders::_2
			)
		);
	}


	private: void disconnect_from_event_sources()
	{
		// Disconnect from foreign event sources
		registry_type& ref_reg = registry_type::instance();

		ref_reg.des_engine_ptr()->begin_of_sim_event_source().disconnect(
			::dcs::functional::bind(
				&self_type::process_begin_of_sim,
				this,
				::dcs::functional::placeholders::_1,
				::dcs::functional::placeholders::_2
			)
		);
		ref_reg.des_engine_ptr()->system_initialization_event_source().disconnect(
			::dcs::functional::bind(
				&self_type::process_sys_init,
				this,
				::dcs::functional::placeholders::_1,
				::dcs::functional::placeholders::_2
			)
		);
		ref_reg.des_engine_ptr()->system_finalization_event_source().disconnect(
			::dcs::functional::bind(
				&self_type::process_sys_finit,
				this,
				::dcs::functional::placeholders::_1,
				::dcs::functional::placeholders::_2
			)
		);
	}


//FIXME: this may be wrong...review
	private: void update_experiment_stats(des_engine_context_type& ctx)
	{
//		typedef typename physical_machine_type::vmm_type vmm_type;
//		typedef typename vmm_type::virtual_machine_container vm_container;
//		typedef typename vm_container::const_iterator vm_iterator;
//		typedef typename vmm_type::virtual_machine_type vm_type;
//		typedef typename vm_container::value_type vm_pointer;
//		typedef typename vm_type::application_tier_type application_tier_type;
//		typedef typename vm_type::resource_share_container resource_share_container;
//		typedef typename resource_share_container::const_iterator resource_share_iterator;

//		// Update consumed energy: compute the energy consumed by each VM
//		// running on this machine without considering the energy consumed when
//		// the machine is idle; this last energy is first subtracted from the
//		// energy returned by the model and then added at the end of the
//		// computation.
//		//real_type energy(0);
//		//real_type idle_energy(this->machine().consumed_energy(0));
//		vm_container vms(this->machine().vmm().virtual_machines());
//		vm_iterator vm_end_it(vms.end());
//		real_type busy_capacity(0);
//		for (vm_iterator vm_it = vms.begin(); vm_it != vm_end_it; ++vm_it)
//		{
//			vm_pointer ptr_vm(*vm_it);
//
//			application_tier_type const& ref_tier(ptr_vm->guest_system());
//
//			busy_capacity += ref_tier.application().simulation_model().tier_busy_capacity(ref_tier.id());
////			real_type busy_time;
////			busy_time = ref_tier.application().simulation_model().actual_tier_busy_time(ref_tier.id());
////
////			if (busy_time > 0)
////			{
////				resource_share_container shares(ptr_vm->resource_shares());
////				resource_share_iterator res_end_it(shares.end());
////				for (resource_share_iterator res_it = shares.begin(); res_it != res_end_it; ++res_it)
////				{
////					physical_resource_category category(res_it->first);
////					real_type share(res_it->second);
////
////					energy += busy_time * (this->machine().resource(category)->consumed_energy(share) - idle_energy);
////				}
////			}
//		}
////		energy_ += energy + idle_energy;
//		if (busy_capacity > 0)
//		{
//			typedef typename physical_machine_type::resource_pointer resource_pointer;
//			typedef ::std::vector<resource_pointer> resource_container;
//			typedef typename resource_container::const_iterator resource_iterator;
//
//			resource_container resources(this->machine().resources());
//			resource_iterator res_end_it(resources.end());
//			for (resource_iterator it = resources.begin(); it != res_end_it; ++it)
//			{
//				resource_pointer ptr_res(*it);
//
//				energy_ += ptr_res->consumed_energy(busy_capacity)-ptr_res->consumed_energy(0)
//						+  ptr_res->consumed_energy(0)*static_cast<real_type>(ctx.simulated_time() - last_pwron_time_);
//			}
//		}

		// Update uptime
		uptime_ += ctx.simulated_time() - last_pwron_time_;
	}


	//@{ Interface Member Functions

	private: power_status do_power_state() const
	{
		return pwr_state_;
	}


	private: void do_power_on()
	{
		pwr_state_ = powered_on_power_status;

		registry_type& ref_reg = registry_type::instance();

		ref_reg.des_engine_ptr()->schedule_event(
				ptr_pwron_evt_src_,
				ref_reg.des_engine_ptr()->simulated_time()
			);
	}


	private: void do_power_off()
	{
		pwr_state_ = powered_off_power_status;

		registry_type& ref_reg = registry_type::instance();

		ref_reg.des_engine_ptr()->schedule_event(
				ptr_pwroff_evt_src_,
				ref_reg.des_engine_ptr()->simulated_time()
			);
	}


	private: power_status do_vm_power_state(virtual_machine_pointer const& ptr_vm) const
	{
		DCS_DEBUG_ASSERT( ptr_vm );

		return ptr_vm->power_state();
	}


	private: void do_vm_power_on(virtual_machine_pointer const& ptr_vm)
	{
		DCS_DEBUG_ASSERT( ptr_vm );

		ptr_vm->power_on();

		registry_type& ref_reg = registry_type::instance();

		ref_reg.des_engine_ptr()->schedule_event(
				ptr_vm_pwron_evt_src_,
				ref_reg.des_engine_ptr()->simulated_time(),
				ptr_vm
			);

		ptr_vm->guest_system().application().simulation_model().request_tier_service_event_source(ptr_vm->guest_system().id()).connect(
				::dcs::functional::bind(
						&self_type::process_vm_request_service,
						this,
						::dcs::functional::placeholders::_1,
						::dcs::functional::placeholders::_2,
						ptr_vm
					)
			);
	}


	private: void do_vm_power_off(virtual_machine_pointer const& ptr_vm)
	{
		DCS_DEBUG_ASSERT( ptr_vm );

		ptr_vm->power_off();

		registry_type& ref_reg = registry_type::instance();

		ref_reg.des_engine_ptr()->schedule_event(
				ptr_vm_pwroff_evt_src_,
				ref_reg.des_engine_ptr()->simulated_time(),
				ptr_vm
			);

		ptr_vm->guest_system().application().simulation_model().request_tier_service_event_source(ptr_vm->guest_system().id()).disconnect(
				::dcs::functional::bind(
						&self_type::process_vm_request_service,
						this,
						::dcs::functional::placeholders::_1,
						::dcs::functional::placeholders::_2,
						ptr_vm
					)
			);
	}


	private: void do_vm_migrate(virtual_machine_pointer const& ptr_vm, physical_machine_type& pm, bool pm_is_source)
	{
		DCS_DEBUG_ASSERT( ptr_vm );

		registry_type& ref_reg = registry_type::instance();

		ref_reg.des_engine_ptr()->schedule_event(
				ptr_vm_migr_evt_src_,
				ref_reg.des_engine_ptr()->simulated_time(),
				ptr_vm//,FIXME
//				dst_pm
			);

		if (pm_is_source)
		{
			ptr_vm->guest_system().application().simulation_model().request_tier_service_event_source(ptr_vm->guest_system().id()).disconnect(
					::dcs::functional::bind(
							&self_type::process_vm_request_service,
							this,
							::dcs::functional::placeholders::_1,
							::dcs::functional::placeholders::_2,
							ptr_vm
						)
				);
		}
		else
		{
			ptr_vm->guest_system().application().simulation_model().request_tier_service_event_source(ptr_vm->guest_system().id()).connect(
					::dcs::functional::bind(
							&self_type::process_vm_request_service,
							this,
							::dcs::functional::placeholders::_1,
							::dcs::functional::placeholders::_2,
							ptr_vm
						)
				);
		}
	}


	private: des_event_source_type& do_power_on_event_source()
	{
		return *ptr_pwron_evt_src_;
	}


	private: des_event_source_type const& do_power_on_event_source() const
	{
		return *ptr_pwron_evt_src_;
	}


	private: des_event_source_type& do_power_off_event_source()
	{
		return *ptr_pwroff_evt_src_;
	}


	private: des_event_source_type const& do_power_off_event_source() const
	{
		return *ptr_pwroff_evt_src_;
	}


	private: des_event_source_type& do_vm_power_on_event_source()
	{
		return *ptr_vm_pwron_evt_src_;
	}


	private: des_event_source_type const& do_vm_power_on_event_source() const
	{
		return *ptr_vm_pwron_evt_src_;
	}


	private: des_event_source_type& do_vm_power_off_event_source()
	{
		return *ptr_vm_pwroff_evt_src_;
	}


	private: des_event_source_type const& do_vm_power_off_event_source() const
	{
		return *ptr_vm_pwroff_evt_src_;
	}


	private: des_event_source_type& do_vm_migrate_event_source()
	{
		return *ptr_vm_migr_evt_src_;
	}


	private: des_event_source_type const& do_vm_migrate_event_source() const
	{
		return *ptr_vm_migr_evt_src_;
	}


	private: output_statistic_type const& do_consumed_energy() const
	{
		return *ptr_energy_stat_;
	}


	private: output_statistic_type const& do_uptime() const
	{
		return *ptr_uptime_stat_;
	}


	private: output_statistic_type const& do_utilization() const
	{
		return *ptr_util_stat_;
	}

	//@} Interface Member Functions


	//@{ Event Handlers

	private: void process_begin_of_sim(des_event_type const& evt, des_engine_context_type& ctx)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );

		DCS_DEBUG_TRACE("(" << this << ") BEGIN Processing BEGIN-OF-SIMULATION (Clock: " << ctx.simulated_time() << ")");

		// Reset stats
		ptr_energy_stat_->reset();
		ptr_uptime_stat_->reset();
		ptr_util_stat_->reset();

		DCS_DEBUG_TRACE("(" << this << ") END Processing BEGIN-OF-SIMULATION (Clock: " << ctx.simulated_time() << ")");
	}


	private: void process_sys_init(des_event_type const& evt, des_engine_context_type& ctx)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );

		DCS_DEBUG_TRACE("(" << this << ") BEGIN Processing SYSTEM-INITIALIZATION (Clock: " << ctx.simulated_time() << ")");

		// Reset per-experiment stats
//		energy_ = uptime_
		uptime_ = real_type/*zero*/();

		pwr_state_ = powered_off_power_status;

		res_profile_map_.clear();

		DCS_DEBUG_TRACE("(" << this << ") END Processing SYSTEM-INITIALIZATION (Clock: " << ctx.simulated_time() << ")");
	}


	private: void process_sys_finit(des_event_type const& evt, des_engine_context_type& ctx)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );

		DCS_DEBUG_TRACE("(" << this << ") BEGIN Processing SYSTEM-FINALIZATION (Clock: " << ctx.simulated_time() << ")");

		// Update experiment-level stats only if the machine is currently
		// powered-on (because, in the other case, stats should have already
		// been updated in the power-change event handler).
		if (this->power_state() == powered_on_power_status)
		{
			update_experiment_stats(ctx);
		}

		// Compute energy & utilization
		typedef ::std::vector<virtual_machine_pointer> virtual_machine_container;
		typedef typename virtual_machine_container::const_iterator vm_iterator;
		typedef ::std::vector<user_request_type> request_container;
		typedef typename request_container::const_iterator request_iterator;
		typedef typename utilization_profile_type::const_iterator profile_iterator;
		// - Update energy profile with utilizations due to not-yet-terminated requests running on powered-on VMs
		virtual_machine_container active_vms(this->machine().vmm().virtual_machines(powered_on_power_status));
		vm_iterator vm_end_it(active_vms.end());
		for (vm_iterator vm_it = active_vms.begin(); vm_it != vm_end_it; ++vm_it)
		{
			virtual_machine_pointer ptr_vm(*vm_it);

			// check: valid VM pointer (paranoid)
			DCS_DEBUG_ASSERT( ptr_vm );

			request_container reqs(ptr_vm->guest_system().application().simulation_model().tier_in_service_requests(ptr_vm->guest_system().id()));
			request_iterator req_end_it(reqs.end());
			for (request_iterator req_it = reqs.begin(); req_it != req_end_it; ++req_it)
			{
				user_request_type const& req(*req_it);
				//FIXME: CPU resource category is hard-coded
				physical_resource_category category(cpu_resource_category);

				::std::vector<utilization_profile_type> profiles;
				profiles = req.tier_utilization_profiles(ptr_vm->guest_system().id(), category);
				if (profiles.size() > 0)
				{
					// The last profiles refers to the last visit (at this tier)
					utilization_profile_type const& profile(profiles.back());
					profile_iterator profile_end_it(profile.end());
					for (profile_iterator it = profile.begin(); it != profile_end_it; ++it)
					{
						res_profile_map_[category](*it);
					}
				}
			}
		}
		// - Compute total energy as the sum of the energy consumed during each
		//   active time interval and machine utilization as the ratio between
		//   busy and up time.
		real_type idle_energy(this->machine().consumed_energy(0));
		real_type energy(0);
		real_type busy_time(0);
		if (res_profile_map_.size() > 0)
		{
			//FIXME: CPU resource category is hard-coded
			physical_resource_category category(cpu_resource_category);
			utilization_profile_type const& profile(res_profile_map_.at(category));
			profile_iterator profile_end_it(profile.end());
			for (profile_iterator it = profile.begin(); it != profile_end_it; ++it)
			{
				typename utilization_profile_type::profile_item_type const& item(*it);

				//FIXME: replace boost::icl::length with a wrapper function
				energy += (this->machine().consumed_energy(item.second)-idle_energy)*::boost::icl::length(item.first);
				busy_time += ::boost::icl::length(item.first);
			}
		}
		energy += idle_energy*uptime_;

		// check: machine cannot be busy more than is up (paranoid check)
		DCS_DEBUG_ASSERT( busy_time <= uptime_ );

		// Update simulation-level stats
		(*ptr_energy_stat_)(energy);
		(*ptr_uptime_stat_)(uptime_);
		if (uptime_ > 0)
		{
			(*ptr_util_stat_)(busy_time/uptime_);
		}
		else
		{
			(*ptr_util_stat_)(0);
		}

		DCS_DEBUG_TRACE("(" << this << ") END Processing SYSTEM-FINALIZATION (Clock: " << ctx.simulated_time() << ")");
	}


	private: void process_power_on(des_event_type const& evt, des_engine_context_type& ctx)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );

		DCS_DEBUG_TRACE("(" << this << ") BEGIN Processing POWER-ON (Clock: " << ctx.simulated_time() << ")");

		last_pwron_time_ = ctx.simulated_time();

		DCS_DEBUG_TRACE("(" << this << ") END Processing POWER-ON (Clock: " << ctx.simulated_time() << ")");
	}


	private: void process_power_off(des_event_type const& evt, des_engine_context_type& ctx)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );

		DCS_DEBUG_TRACE("(" << this << ") BEGIN Processing POWER-OFF (Clock: " << ctx.simulated_time() << ")");

		// pre: make sure a power-off follows in time a power-on
		DCS_DEBUG_ASSERT( ctx.simulated_time() >= last_pwron_time_ );

		update_experiment_stats(ctx);

		DCS_DEBUG_TRACE("(" << this << ") END Processing POWER-OFF (Clock: " << ctx.simulated_time() << ")");
	}


	private: void process_vm_request_service(des_event_type const& evt, des_engine_context_type& ctx, virtual_machine_pointer const& ptr_vm)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(ctx);

		// pre: virtual machine pointer must be a valid pointer
		DCS_DEBUG_ASSERT( ptr_vm );

		user_request_type req = ptr_vm->guest_system().application().simulation_model().request_state(evt);

		typedef typename utilization_profile_type::const_iterator profile_iterator;
		::std::vector<utilization_profile_type> profiles;
		//FIXME: CPU resource category is hard-coded
		physical_resource_category category(cpu_resource_category);
		profiles = req.tier_utilization_profiles(ptr_vm->guest_system().id(), category);
		if (profiles.size() > 0)
		{
			// The last profiles refers to the last visit (at this tier)
			utilization_profile_type const& profile(profiles.back());
			profile_iterator profile_end_it(profile.end());
			for (profile_iterator it = profile.begin(); it != profile_end_it; ++it)
			{
				res_profile_map_[category](*it);
			}
		}
	}


	//@} Event Handlers


	private: power_status pwr_state_;
//	private: real_type energy_;
	private: real_type uptime_;
	private: real_type last_pwron_time_;
	private: des_event_source_pointer ptr_pwron_evt_src_;
	private: des_event_source_pointer ptr_pwroff_evt_src_;
	private: des_event_source_pointer ptr_vm_pwron_evt_src_;
	private: des_event_source_pointer ptr_vm_pwroff_evt_src_;
	private: des_event_source_pointer ptr_vm_migr_evt_src_;
	private: output_statistic_pointer ptr_energy_stat_;
	private: output_statistic_pointer ptr_uptime_stat_;
	private: output_statistic_pointer ptr_util_stat_;
	private: ::std::map<physical_resource_category,utilization_profile_type> res_profile_map_;
};

template <typename TraitsT>
const ::std::string default_physical_machine_simulation_model<TraitsT>::poweron_event_source_name("Machine Power-on");

template <typename TraitsT>
const ::std::string default_physical_machine_simulation_model<TraitsT>::poweroff_event_source_name("Machine Power-off");

}} // Namespace dcs::eesim

#endif // DCS_EESIM_DEFAULT_PHYSICAL_MACHINE_SIMULATION_MODEL_HPP
