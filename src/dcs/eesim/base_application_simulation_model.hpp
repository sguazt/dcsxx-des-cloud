#ifndef DCS_EESIM_BASE_APPLICATION_SIMULATION_MODEL_HPP
#define DCS_EESIM_BASE_APPLICATION_SIMULATION_MODEL_HPP


#include <dcs/des/engine_traits.hpp>
#include <dcs/des/entity.hpp>


namespace dcs { namespace eesim {

template <typename TraitsT>
class base_application_simulation_model: public ::dcs::des::entity
{
	public: typedef TraitsT traits_type;
	private: typedef typename traits_type::des_engine_type des_engine_type;
	public: typedef typename ::dcs::des::engine_traits<des_engine_type>::event_source_type des_event_source_type;
	public: typedef typename traits_type::uint_type uint_type;
//	public: typedef typename engine_traits<des_engine_type>::event_type des_event_type;
//	public: typedef typename engine_traits<des_engine_type>::engine_context_type des_engine_context_type;


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


	private: virtual des_event_source_type& do_request_arrival_event_source() = 0;

	private: virtual des_event_source_type const& do_request_arrival_event_source() const = 0;

	private: virtual des_event_source_type& do_request_departure_event_source() = 0;

	private: virtual des_event_source_type const& do_request_departure_event_source() const = 0;


	private: virtual des_event_source_type& do_request_tier_arrival_event_source(uint_type tier_id) = 0;


	private: virtual des_event_source_type const& do_request_tier_arrival_event_source(uint_type tier_id) const = 0;


	private: virtual des_event_source_type& do_request_tier_departure_event_source(uint_type tier_id) = 0;


	private: virtual des_event_source_type const& do_request_tier_departure_event_source(uint_type tier_id) const = 0;
};

}} // Namespace dcs::eesim


#endif // DCS_EESIM_BASE_APPLICATION_SIMULATION_MODEL_HPP
