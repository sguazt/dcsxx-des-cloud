#ifndef DCS_EESIM_BASE_APPLICATION_SIMULATION_MODEL_HPP
#define DCS_EESIM_BASE_APPLICATION_SIMULATION_MODEL_HPP


#include <dcs/des/base_statistic.hpp>
#include <dcs/des/engine_traits.hpp>
#include <dcs/des/entity.hpp>
#include <dcs/eesim/multi_tier_application.hpp>
#include <dcs/eesim/user_request.hpp>
#include <dcs/memory.hpp>
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
	public: typedef user_request<real_type> user_request_type;
	public: typedef multi_tier_application<traits_type> application_type;
	public: typedef application_type* application_pointer;


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


	private: virtual output_statistic_type const& do_num_arrivals() const = 0;


	private: virtual output_statistic_type const& do_num_departures() const = 0;


	private: virtual output_statistic_type const& do_num_sla_violations() const = 0;


	private: virtual des_event_source_type& do_request_arrival_event_source() = 0;


	private: virtual des_event_source_type const& do_request_arrival_event_source() const = 0;


	private: virtual des_event_source_type& do_request_departure_event_source() = 0;


	private: virtual des_event_source_type const& do_request_departure_event_source() const = 0;


	private: virtual des_event_source_type& do_request_tier_arrival_event_source(uint_type tier_id) = 0;


	private: virtual des_event_source_type const& do_request_tier_arrival_event_source(uint_type tier_id) const = 0;


	private: virtual des_event_source_type& do_request_tier_departure_event_source(uint_type tier_id) = 0;


	private: virtual des_event_source_type const& do_request_tier_departure_event_source(uint_type tier_id) const = 0;


	private: virtual void do_statistic(performance_measure_category category, output_statistic_pointer const& ptr_stat) = 0;


	private: virtual ::std::vector<output_statistic_pointer> do_statistic(performance_measure_category category) const = 0;


	private: virtual void do_tier_statistic(uint_type tier_id, performance_measure_category category, output_statistic_pointer const& ptr_stat) = 0;


	private: virtual ::std::vector<output_statistic_pointer> do_tier_statistic(uint_type tier_id, performance_measure_category category) const = 0;


	private: virtual user_request_type do_request_state(des_event_type const& evt) const = 0;


	private: application_pointer ptr_app_;
};

}} // Namespace dcs::eesim


#endif // DCS_EESIM_BASE_APPLICATION_SIMULATION_MODEL_HPP
