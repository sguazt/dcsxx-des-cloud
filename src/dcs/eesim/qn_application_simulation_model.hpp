#ifndef QN_APPLICATION_SIMULATION_MODEL_HPP
#define QN_APPLICATION_SIMULATION_MODEL_HPP


//#include <boost/any.hpp>
#include <dcs/des/engine_traits.hpp>
#include <dcs/des/base_statistic.hpp>
#include <dcs/des/model/qn/output_statistic_category.hpp>
#include <dcs/des/model/qn/queueing_network.hpp>
#include <dcs/eesim/application_simulation_model_traits.hpp>
#include <dcs/eesim/performance_measure_category.hpp>
#include <dcs/eesim/user_request.hpp>
#include <dcs/memory.hpp>
#include <stdexcept>
#include <vector>


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
	public: typedef typename ::dcs::des::engine_traits<des_engine_type>::event_type des_event_type;
	public: typedef typename traits_type::uint_type uint_type;
	public: typedef typename traits_type::real_type real_type;
	public: typedef ::dcs::des::base_statistic<real_type,uint_type> output_statistic_type;
	public: typedef ::dcs::shared_ptr<output_statistic_type> output_statistic_pointer;
	//public: typedef ::boost::any foreign_identifier_type;
	public: typedef uint_type foreign_identifier_type;
	public: typedef user_request<real_type> user_request_type;
	private: typedef typename model_type::customer_type customer_type;
	private: typedef ::dcs::shared_ptr<customer_type> customer_pointer;


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


	public: static void statistic(model_type& model, performance_measure_category category, output_statistic_pointer const& ptr_stat)
	{
		model.statistic(
				foreign_network_category(category),
				ptr_stat
			);
	}


	public: static ::std::vector<output_statistic_pointer> statistic(model_type const& model, performance_measure_category category)
	{
		return model.statistic(foreign_network_category(category));
	}


	public: static void tier_statistic(model_type& model, foreign_identifier_type foreign_id, performance_measure_category category, output_statistic_pointer const& ptr_stat)
	{
		model.get_node(foreign_id).statistic(
				foreign_node_category(category),
				ptr_stat
			);
	}


	public: static ::std::vector<output_statistic_pointer> tier_statistic(model_type const& model, foreign_identifier_type foreign_id, performance_measure_category category)
	{
		return model.get_node(foreign_id).statistic(foreign_node_category(category));
	}


	public: static user_request_type request_state(model_type const& model, des_event_type const& evt)
	{
		user_request_type req;

		customer_pointer ptr_customer = evt.template unfolded_state<customer_pointer>();

		req.arrival_time(ptr_customer->arrival_time());
		req.departure_time(ptr_customer->departure_time());

		return req;
	}


	public: static bool has_performance_measure(model_type const& model, performance_measure_category category)
	{
		switch (category)
		{
			case response_time_performance_measure:
				return true;
			case throughput_performance_measure:
				return true;
			default:
				break;
		}

		return false;
	}


//	private: static bool has_foreign_network_category(performance_measure_category category)
//	{
//		switch (category)
//		{
//			case response_time_performance_measure:
//				return true;
//			case throughput_performance_measure:
//				return true;
//			default:
//				break;
//		}
//
//		return false;
//	}


	private: static ::dcs::des::model::qn::network_output_statistic_category foreign_network_category(performance_measure_category category)
	{
		switch (category)
		{
			case response_time_performance_measure:
				return ::dcs::des::model::qn::net_response_time_statistic_category;
			case throughput_performance_measure:
				return ::dcs::des::model::qn::net_throughput_statistic_category;
			default:
				break;
		}

		throw ::std::runtime_error("[dcs::eesim::application_simulation_model_traits::foreign_network_category] Unable to associate a suitable network output statistic category for the given performance measure category.");
	}


//	private: static bool has_foreign_node_category(performance_measure_category category)
//	{
//		switch (category)
//		{
//			case response_time_performance_measure:
//				return true;
//			case throughput_performance_measure:
//				return true;
//			default:
//				break;
//		}
//
//		return false;
//	}


	private: static ::dcs::des::model::qn::node_output_statistic_category foreign_node_category(performance_measure_category category)
	{
		switch (category)
		{
			case response_time_performance_measure:
				return ::dcs::des::model::qn::response_time_statistic_category;
			case throughput_performance_measure:
				return ::dcs::des::model::qn::throughput_statistic_category;
			default:
				break;
		}

		throw ::std::runtime_error("[dcs::eesim::application_simulation_model_traits::foreign_network_category] Unable to associate a suitable network output statistic category for the given performance measure category.");
	}


//	private: static performance_measure_category native_category(::dcs::des::model::qn::network_output_statistic_category category)
//	{
//		switch (category)
//		{
//			case ::dcs::des::model::qn::net_response_time_statistic_category:
//				return response_time_performance_measure;
//			default:
//		}
//	}
};

}} // Namespace dcs::eesim


#endif // QN_APPLICATION_SIMULATION_MODEL_HPP
