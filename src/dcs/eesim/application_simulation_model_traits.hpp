#ifndef DCS_EESIM_APPLICATION_SIMULATION_MODEL_TRAITS_HPP
#define DCS_EESIM_APPLICATION_SIMULATION_MODEL_TRAITS_HPP


#include <dcs/des/engine_traits.hpp>


namespace dcs { namespace eesim {

template <typename TraitsT, typename ModelT>
class application_simulation_model_traits
{
	public: typedef TraitsT traits_type;
	public: typedef ModelT model_type;
	private: typedef typename traits_type::engine_type des_engine_type;
	public: typedef typename ::dcs::des::engine_traits<des_engine_type>::event_source_type des_event_source_type;
	public: typedef typename traits_type::uint_type uint_type;


	public: static void enable(model_type& model, bool flag);


	public: static bool enabled(model_type const& model);


	public: static des_event_source_type& request_arrival_event_source(model_type& model);

	public: static des_event_source_type const& request_arrival_event_source(model_type const& model);

	public: static des_event_source_type& request_departure_event_source(model_type& model);

	public: static des_event_source_type const& request_departure_event_source(model_type const& model);


	public: static des_event_source_type& request_tier_arrival_event_source(model_type& model, uint_type tier_id);

	public: static des_event_source_type const& request_tier_arrival_event_source(model_type const& model, uint_type tier_id);

	public: static des_event_source_type& request_tier_departure_event_source(model_type& model, uint_type tier_id);

	public: static des_event_source_type const& request_tier_departure_event_source(model_type const& model, uint_type tier_id);
};


}} // Namespace dcs::eesim


#endif // DCS_EESIM_APPLICATION_SIMULATION_MODEL_TRAITS_HPP
