#ifndef QN_APPLICATION_SIMULATION_MODEL_HPP
#define QN_APPLICATION_SIMULATION_MODEL_HPP


#include <dcs/des/engine_traits.hpp>
#include <dcs/des/model/qn/queueing_network.hpp>
#include <dcs/eesim/application_simulation_model_traits.hpp>


namespace dcs { namespace eesim {

template <
	typename TraitsT,
	typename UIntT,
	typename RealT,
	typename UniformRandomGeneratorT,
	typename DesEngineT
>
class application_simulation_model_traits<
			TraitsT,
			::dcs::des::model::qn::queueing_network<
				UIntT,
				RealT,
				UniformRandomGeneratorT,
				DesEngineT
			>
		>
{
	public: typedef TraitsT traits_type;
	public: typedef ::dcs::des::model::qn::queueing_network<UIntT,RealT,UniformRandomGeneratorT,DesEngineT> model_type;
	private: typedef typename traits_type::des_engine_type des_engine_type;
	public: typedef typename ::dcs::des::engine_traits<des_engine_type>::event_source_type des_event_source_type;
	public: typedef typename traits_type::uint_type uint_type;


	public: static void enable(model_type& model, bool flag)
	{
		model.enable(flag);
	}


	public: static bool enabled(model_type const& model)
	{
		return model.enabled();
	}


	public: static des_event_source_type& request_arrival_event_source(model_type& model)
	{
		return model.arrival_event_source();
	}


	public: static des_event_source_type const& request_arrival_event_source(model_type const& model)
	{
		return model.arrival_event_source();
	}


	public: static des_event_source_type& request_departure_event_source(model_type& model)
	{
		return model.departure_event_source();
	}


	public: static des_event_source_type const& request_departure_event_source(model_type const& model)
	{
		return model.departure_event_source();
	}


	public: static des_event_source_type& request_tier_arrival_event_source(model_type& model, uint_type tier_id)
	{
		return model.get_node(tier_id).arrival_event_source();
	}


	public: static des_event_source_type const& request_tier_arrival_event_source(model_type const& model, uint_type tier_id)
	{
		return model.get_node(tier_id).arrival_event_source();
	}


	public: static des_event_source_type& request_tier_departure_event_source(model_type& model, uint_type tier_id)
	{
		return model.get_node(tier_id).departure_event_source();
	}


	public: static des_event_source_type const& request_tier_departure_event_source(model_type const& model, uint_type tier_id)
	{
		return model.get_node(tier_id).departure_event_source();
	}
};

}} // Namespace dcs::eesim


#endif // QN_APPLICATION_SIMULATION_MODEL_HPP
