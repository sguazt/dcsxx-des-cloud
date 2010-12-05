#ifndef DCS_EESIM_APPLICATION_SIMULATION_MODEL_ADAPTOR_HPP
#define DCS_EESIM_APPLICATION_SIMULATION_MODEL_ADAPTOR_HPP


#include <dcs/eesim/base_application_simulation_model.hpp>
#include <dcs/eesim/application_simulation_model_traits.hpp>


namespace dcs { namespace eesim {

template <
	typename TraitsT,
	typename ModelT,
	typename ModelTraitsT = application_simulation_model_traits<TraitsT,ModelT>
>
class application_simulation_model_adaptor: public base_application_simulation_model<TraitsT>
{
	private: typedef base_application_simulation_model<TraitsT> base_type;
	public: typedef TraitsT traits_type;
	public: typedef ModelT model_type;
	public: typedef ModelTraitsT model_traits_type;
	public: typedef typename base_type::des_event_source_type des_event_source_type;


	public: explicit application_simulation_model_adaptor(model_type const& model)
	: base_type(),
	  model_(model)
	{
	}


	private: void do_enable()
	{
		model_traits_type::enable(model_);
	}


	private: void do_disable()
	{
		model_traits_type::disable(model_);
	}


	private: des_event_source_type& do_request_arrival_event_source()
	{
		return model_traits_type::request_arrival_event_source(model_);
	}


	private: des_event_source_type const& do_request_arrival_event_source() const
	{
		return model_traits_type::request_arrival_event_source(model_);
	}


	private: des_event_source_type& do_request_departure_event_source()
	{
		return model_traits_type::request_departure_event_source(model_);
	}


	private: des_event_source_type const& do_request_departure_event_source() const
	{
		return model_traits_type::request_departure_event_source(model_);
	}


	private: model_type model_;
};

}} // Namespace dcs::eesim


#endif // DCS_EESIM_APPLICATION_SIMULATION_MODEL_ADAPTOR_HPP
