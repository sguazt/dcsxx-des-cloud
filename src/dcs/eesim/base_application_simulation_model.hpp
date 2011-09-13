#ifndef DCS_EESIM_BASE_APPLICATION_SIMULATION_MODEL_HPP
#define DCS_EESIM_BASE_APPLICATION_SIMULATION_MODEL_HPP


#include <dcs/des/base_statistic.hpp>
#include <dcs/des/engine_traits.hpp>
#include <dcs/des/entity.hpp>
#include <dcs/eesim/multi_tier_application.hpp>
#include <dcs/eesim/physical_resource_category.hpp>
#include <dcs/eesim/user_request.hpp>
#include <dcs/memory.hpp>
#include <map>
#include <stdexcept>
#include <vector>


namespace dcs { namespace eesim {

template <typename TraitsT>
class base_application_simulation_model: public ::dcs::des::entity
{
	public: typedef TraitsT traits_type;
	public: typedef typename traits_type::real_type real_type;
	public: typedef typename traits_type::uint_type uint_type;
	private: typedef typename traits_type::des_engine_type des_engine_type;
	public: typedef typename ::dcs::des::engine_traits<des_engine_type>::event_source_type des_event_source_type;
	public: typedef typename ::dcs::des::engine_traits<des_engine_type>::event_type des_event_type;
//	public: typedef typename engine_traits<des_engine_type>::engine_context_type des_engine_context_type;
	public: typedef ::dcs::des::base_statistic<real_type,uint_type> output_statistic_type;
	public: typedef ::dcs::shared_ptr<output_statistic_type> output_statistic_pointer;
	public: typedef user_request<traits_type> user_request_type;
	public: typedef multi_tier_application<traits_type> application_type;
	public: typedef application_type* application_pointer;
	public: typedef virtual_machine<traits_type> virtual_machine_type;
	public: typedef ::dcs::shared_ptr<virtual_machine_type> virtual_machine_pointer;
	//public: typedef ::std::vector<virtual_machine_pointer> vm_container;
	public: typedef typename application_type::application_tier_type application_tier_type;
	public: typedef typename application_tier_type::identifier_type tier_identifier_type;
	private: typedef ::std::map<tier_identifier_type,virtual_machine_pointer> tier_vm_mapping_container;


	public: void application(application_pointer const& ptr_app)
	{
		ptr_app_ = ptr_app;
	}


	public: application_type& application()
	{
		return *ptr_app_;
	}


	public: application_type const& application() const
	{
		return *ptr_app_;
	}


	public: void tier_virtual_machine(virtual_machine_pointer const& ptr_vm)
	{
		typedef typename virtual_machine_type::resource_share_container share_container;

		uint_type tier_id(ptr_vm->guest_system().id());

		tier_vm_map_[ptr_vm->guest_system().id()] = ptr_vm;

		share_container shares(ptr_vm->resource_shares());
		this->resource_shares(tier_id, shares.begin(), shares.end());
	}


	public: template <typename VMForwardIterT>
		void tier_virtual_machines(VMForwardIterT first, VMForwardIterT last)
	{
		while (first != last)
		{
			this->tier_virtual_machine(*first);

			++first;
		}
	}


	public: virtual_machine_pointer tier_virtual_machine(tier_identifier_type tier_id) const
	{
		typedef typename tier_vm_mapping_container::const_iterator iterator;

		iterator it(tier_vm_map_.find(tier_id));

		// pre: tier_id must have an associated VM
		DCS_ASSERT(
			it != tier_vm_map_.end(),
			throw ::std::invalid_argument("[dcs::eesim::base_application_simulation_model::tier_virtual_machine] Tier with no associated VM.")
		);

		// check: double check on tier identifier
		DCS_DEBUG_ASSERT( tier_id == it->second->guest_system().id() );

		return it->second;
	}


	public: void resource_share(uint_type tier_id, physical_resource_category category, real_type share)
	{
		do_resource_share(tier_id, category, share);
	}


	public: template <typename ForwardIterT>
		void resource_shares(uint_type tier_id, ForwardIterT first_share, ForwardIterT last_share)
	{
		while (first_share != last_share)
		{
			this->resource_share(tier_id, first_share->first, first_share->second);
			++first_share;
		}
	}


	public: uint_type actual_num_arrivals() const
	{
		return do_actual_num_arrivals();
	}


	public: uint_type actual_num_departures() const
	{
		return do_actual_num_departures();
	}


	public: uint_type actual_num_sla_violations() const
	{
		return do_actual_num_sla_violations();
	}


//	public: real_type actual_busy_time() const
//	{
//		return do_actual_busy_time();
//	}


	public: uint_type actual_tier_num_arrivals(uint_type tier_id) const
	{
		return do_actual_tier_num_arrivals(tier_id);
	}


	public: uint_type actual_tier_num_departures(uint_type tier_id) const
	{
		return do_actual_tier_num_departures(tier_id);
	}


	public: real_type actual_tier_busy_time(uint_type tier_id) const
	{
		return do_actual_tier_busy_time(tier_id);
	}


	public: real_type actual_tier_utilization(uint_type tier_id) const
	{
		return actual_tier_busy_time(tier_id)/registry<traits_type>::instance().des_engine_ptr()->simulated_time();
	}


	public: ::std::vector<user_request_type> tier_in_service_requests(uint_type tier_id) const//EXP
	{
		return do_tier_in_service_requests(tier_id);
	}


	public: output_statistic_type const& num_arrivals() const
	{
		return do_num_arrivals();
	}


	public: output_statistic_type const& num_departures() const
	{
		return do_num_departures();
	}


	public: output_statistic_type const& num_sla_violations() const
	{
		return do_num_sla_violations();
	}


	public: output_statistic_type const& tier_num_arrivals(uint_type tier_id) const
	{
		return do_tier_num_arrivals(tier_id);
	}


	public: output_statistic_type const& tier_num_departures(uint_type tier_id) const
	{
		return do_tier_num_departures(tier_id);
	}


	public: des_event_source_type& request_arrival_event_source()
	{
		return do_request_arrival_event_source();
	}


	public: des_event_source_type const& request_arrival_event_source() const
	{
		return do_request_arrival_event_source();
	}


	public: des_event_source_type& request_departure_event_source()
	{
		return do_request_departure_event_source();
	}


	public: des_event_source_type const& request_departure_event_source() const
	{
		return do_request_departure_event_source();
	}


	public: des_event_source_type& request_tier_arrival_event_source(uint_type tier_id)
	{
		return do_request_tier_arrival_event_source(tier_id);
	}


	public: des_event_source_type const& request_tier_arrival_event_source(uint_type tier_id) const
	{
		return do_request_tier_arrival_event_source(tier_id);
	}


	public: des_event_source_type& request_tier_service_event_source(uint_type tier_id)
	{
		return do_request_tier_service_event_source(tier_id);
	}


	public: des_event_source_type const& request_tier_service_event_source(uint_type tier_id) const
	{
		return do_request_tier_service_event_source(tier_id);
	}


	public: des_event_source_type& request_tier_departure_event_source(uint_type tier_id)
	{
		return do_request_tier_departure_event_source(tier_id);
	}


	public: des_event_source_type const& request_tier_departure_event_source(uint_type tier_id) const
	{
		return do_request_tier_departure_event_source(tier_id);
	}


	public: void statistic(performance_measure_category category, output_statistic_pointer const& ptr_stat)
	{
		do_statistic(category, ptr_stat);
	}


	public: ::std::vector<output_statistic_pointer> statistic(performance_measure_category category) const
	{
		return do_statistic(category);
	}


	public: void tier_statistic(uint_type tier_id, performance_measure_category category, output_statistic_pointer const& ptr_stat)
	{
		do_tier_statistic(tier_id, category, ptr_stat);
	}


	public: ::std::vector<output_statistic_pointer> tier_statistic(uint_type tier_id, performance_measure_category category) const
	{
		return do_tier_statistic(tier_id, category);
	}


	public: user_request_type request_state(des_event_type const& evt) const
	{
		return do_request_state(evt);
	}


	protected: application_pointer application_ptr() const
	{
		return ptr_app_;
	}


	protected: application_pointer application_ptr()
	{
		return ptr_app_;
	}


	private: virtual uint_type do_actual_num_arrivals() const = 0;


	private: virtual uint_type do_actual_num_departures() const = 0;


	private: virtual uint_type do_actual_num_sla_violations() const = 0;


//	private: virtual real_type do_actual_busy_time() const = 0;


	private: virtual uint_type do_actual_tier_num_arrivals(uint_type tier_id) const = 0;


	private: virtual uint_type do_actual_tier_num_departures(uint_type tier_id) const = 0;


	private: virtual real_type do_actual_tier_busy_time(uint_type tier_id) const = 0;


	private: virtual ::std::vector<user_request_type> do_tier_in_service_requests(uint_type tier_id) const = 0;


	private: virtual output_statistic_type const& do_num_arrivals() const = 0;


	private: virtual output_statistic_type const& do_num_departures() const = 0;


	private: virtual output_statistic_type const& do_num_sla_violations() const = 0;


	private: virtual output_statistic_type const& do_tier_num_arrivals(uint_type tier_id) const = 0;


	private: virtual output_statistic_type const& do_tier_num_departures(uint_type tier_id) const = 0;


	private: virtual des_event_source_type& do_request_arrival_event_source() = 0;


	private: virtual des_event_source_type const& do_request_arrival_event_source() const = 0;


	private: virtual des_event_source_type& do_request_departure_event_source() = 0;


	private: virtual des_event_source_type const& do_request_departure_event_source() const = 0;


	private: virtual des_event_source_type& do_request_tier_arrival_event_source(uint_type tier_id) = 0;


	private: virtual des_event_source_type const& do_request_tier_arrival_event_source(uint_type tier_id) const = 0;


	private: virtual des_event_source_type& do_request_tier_service_event_source(uint_type tier_id) = 0;


	private: virtual des_event_source_type const& do_request_tier_service_event_source(uint_type tier_id) const = 0;


	private: virtual des_event_source_type& do_request_tier_departure_event_source(uint_type tier_id) = 0;


	private: virtual des_event_source_type const& do_request_tier_departure_event_source(uint_type tier_id) const = 0;


	private: virtual void do_statistic(performance_measure_category category, output_statistic_pointer const& ptr_stat) = 0;


	private: virtual ::std::vector<output_statistic_pointer> do_statistic(performance_measure_category category) const = 0;


	private: virtual void do_tier_statistic(uint_type tier_id, performance_measure_category category, output_statistic_pointer const& ptr_stat) = 0;


	private: virtual ::std::vector<output_statistic_pointer> do_tier_statistic(uint_type tier_id, performance_measure_category category) const = 0;


	private: virtual user_request_type do_request_state(des_event_type const& evt) const = 0;


	private: virtual void do_resource_share(uint_type tier_id, physical_resource_category category, real_type share) = 0;


	private: application_pointer ptr_app_;
	private: tier_vm_mapping_container tier_vm_map_;
};

}} // Namespace dcs::eesim


#endif // DCS_EESIM_BASE_APPLICATION_SIMULATION_MODEL_HPP
