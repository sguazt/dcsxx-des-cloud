#ifndef DCS_EESIM_APPLICATION_SIMULATION_MODEL_TRAITS_HPP
#define DCS_EESIM_APPLICATION_SIMULATION_MODEL_TRAITS_HPP


#include <boost/any.hpp>
#include <dcs/des/base_statistic.hpp>
#include <dcs/des/engine_traits.hpp>
#include <dcs/eesim/performance_measure_category.hpp>
#include <dcs/memory.hpp>


namespace dcs { namespace eesim {

template <typename TraitsT, typename ModelT>
class application_simulation_model_traits
{
	public: typedef TraitsT traits_type;
	public: typedef ModelT model_type;
	private: typedef typename traits_type::engine_type des_engine_type;
	public: typedef typename ::dcs::des::engine_traits<des_engine_type>::event_source_type des_event_source_type;
	public: typedef typename ::dcs::des::engine_traits<des_engine_type>::event_type des_event_type;
	public: typedef typename traits_type::uint_type uint_type;
	public: typedef typename traits_type::uint_type real_type;
	public: typedef ::dcs::des::base_statistic<real_type,uint_type> output_statistic_type;
	public: typedef ::dcs::shared_ptr<output_statistic_type> output_statistic_pointer;
	public: typedef ::boost::any foreign_identifier_type;
	public: typedef typename traits_type::user_request_type user_request_type;


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

    public: static void statistic(model_type& model, performance_measure_category category, output_statistic_pointer const& ptr_stat);

    public: static output_statistic_pointer statistic(model_type const& model, performance_measure_category category);

    public: static void tier_statistic(model_type& model, performance_measure_category category, foreign_identifier_type tier_id, output_statistic_pointer const& ptr_stat);

    public: static output_statistic_pointer tier_statistic(model_type const& model, foreign_identifier_type tier_id, performance_measure_category category);


	public: static bool has_performance_measure(model_type const& model, performance_measure_category category);


	public: static user_request_type request_state(model_type const& model, des_event_type const& evt);
};


}} // Namespace dcs::eesim


#endif // DCS_EESIM_APPLICATION_SIMULATION_MODEL_TRAITS_HPP
