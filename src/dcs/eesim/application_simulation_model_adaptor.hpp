#ifndef DCS_EESIM_APPLICATION_SIMULATION_MODEL_ADAPTOR_HPP
#define DCS_EESIM_APPLICATION_SIMULATION_MODEL_ADAPTOR_HPP


#include <dcs/des/engine_traits.hpp>
#include <dcs/des/mean_estimator.hpp>
#include <dcs/eesim/application_simulation_model_traits.hpp>
#include <dcs/eesim/base_application_simulation_model.hpp>
#include <dcs/eesim/performance_measure_category.hpp>
#include <dcs/eesim/registry.hpp>
//#include <map>
#include <stdexcept>
#include <vector>


namespace dcs { namespace eesim {

template <
	typename TraitsT,
	typename ModelT,
	typename ModelTraitsT = application_simulation_model_traits<TraitsT,ModelT>
>
class application_simulation_model_adaptor: public base_application_simulation_model<TraitsT>
{
	private: typedef base_application_simulation_model<TraitsT> base_type;
	private: typedef application_simulation_model_adaptor<TraitsT,ModelT,ModelTraitsT> self_type;
	public: typedef TraitsT traits_type;
	public: typedef ModelT model_type;
	public: typedef ModelTraitsT model_traits_type;
	private: typedef typename base_type::des_event_source_type des_event_source_type;
	private: typedef typename base_type::des_event_type des_event_type;
	private: typedef typename base_type::real_type real_type;
	private: typedef typename base_type::uint_type uint_type;
	private: typedef typename base_type::output_statistic_type output_statistic_type;
	private: typedef typename base_type::output_statistic_pointer output_statistic_pointer;
	private: typedef typename model_traits_type::foreign_identifier_type foreign_identifier_type;
	private: typedef ::std::map<uint_type,foreign_identifier_type> tier_mapping_container;
	private: typedef typename base_type::user_request_type user_request_type;
	private: typedef registry<traits_type> registry_type;
//	private: typedef ::std::vector<output_statistic_pointer> output_statistic_container;
//	private: typedef ::std::map<performance_measure_category,output_statistic_container> output_statistic_category_container;
	private: typedef typename traits_type::des_engine_type des_engine_type;
	private: typedef typename ::dcs::des::engine_traits<des_engine_type>::engine_context_type des_engine_context_type;


	public: explicit application_simulation_model_adaptor(model_type const& model)
	: base_type(),
	  model_(model),
	  tier_map_(),
	  num_sla_viols_(0),
	  ptr_num_arrs_stat_(new ::dcs::des::mean_estimator<real_type,uint_type>()),
	  ptr_num_deps_stat_(new ::dcs::des::mean_estimator<real_type,uint_type>()),
	  ptr_num_sla_viols_stat_(new ::dcs::des::mean_estimator<real_type,uint_type>())
	{
		init();
	}


	public: ~application_simulation_model_adaptor()
	{
		disconnect_from_event_sources();
	}


	public: void tier_mapping(uint_type tier_id, foreign_identifier_type foreign_id)
	{
		tier_map_[tier_id] = foreign_id;
	}


	private: void init()
	{
		connect_to_event_sources();
	}


	private: void connect_to_event_sources()
	{
		registry_type& ref_reg = registry_type::instance();

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
		model_traits_type::request_departure_event_source(model_).connect(
			::dcs::functional::bind(
				&self_type::process_request_departure,
				this,
				::dcs::functional::placeholders::_1,
				::dcs::functional::placeholders::_2
			)
		);
	}


	private: void disconnect_from_event_sources()
	{
		registry_type& ref_reg = registry_type::instance();

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
		model_traits_type::request_departure_event_source(model_).disconnect(
			::dcs::functional::bind(
				&self_type::process_request_departure,
				this,
				::dcs::functional::placeholders::_1,
				::dcs::functional::placeholders::_2
			)
		);
	}


	private: void do_enable(bool flag)
	{
		model_traits_type::enable(model_, flag);

		if (flag)
		{
			if (!this->enabled())
			{
				connect_to_event_sources();
			}
		}
		else
		{
			if (this->enabled())
			{
				disconnect_from_event_sources();
			}
		}
	}


	private: bool do_enabled() const
	{
		return model_traits_type::enabled(model_);
	}


	private: output_statistic_type const& do_num_arrivals() const
	{
		return *ptr_num_arrs_stat_;
	}


	private: output_statistic_type const& do_num_departures() const
	{
		return *ptr_num_deps_stat_;
	}


	private: output_statistic_type const& do_num_sla_violations() const
	{
		return *ptr_num_sla_viols_stat_;
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


	private: des_event_source_type& do_request_tier_arrival_event_source(uint_type tier_id)
	{
		return model_traits_type::request_tier_arrival_event_source(model_, tier_id);
	}


	private: des_event_source_type const& do_request_tier_arrival_event_source(uint_type tier_id) const
	{
		return model_traits_type::request_tier_arrival_event_source(model_, tier_id);
	}


	private: des_event_source_type& do_request_tier_departure_event_source(uint_type tier_id)
	{
		return model_traits_type::request_tier_departure_event_source(model_, tier_id);
	}


	private: des_event_source_type const& do_request_tier_departure_event_source(uint_type tier_id) const
	{
		return model_traits_type::request_tier_departure_event_source(model_, tier_id);
	}


	private: void do_statistic(performance_measure_category category, output_statistic_pointer const& ptr_stat)
	{
//		if (model_traits_type::has_performance_measure(model_, category))
//		{
			model_traits_type::statistic(model_, category, ptr_stat);
//		}
//		else
//		{
//			stats_[category].push_back(ptr_stat);
//		}
	}


	private: ::std::vector<output_statistic_pointer> do_statistic(performance_measure_category category) const
	{
//		if (model_traits_type::has_performance_measure(model_, category))
//		{
			return model_traits_type::statistic(model_, category);
//		}
//
//		return stats_.at(category);
	}


	private: void do_tier_statistic(uint_type tier_id, performance_measure_category category, output_statistic_pointer const& ptr_stat)
	{
		model_traits_type::tier_statistic(model_, tier_map_[tier_id], category, ptr_stat);
	}


	private: ::std::vector<output_statistic_pointer> do_tier_statistic(uint_type tier_id, performance_measure_category category) const
	{
		if (!tier_map_.count(tier_id))
		{
			throw ::std::logic_error("[dcs::eesim::application_simulation_model_adaptor::do_tier_statistic] Tier not mapped.");
		}

		return model_traits_type::tier_statistic(model_, tier_map_.at(tier_id), category);
	}


	private: user_request_type do_request_state(des_event_type const& evt) const
	{
		return model_traits_type::request_state(model_, evt);
	}


	//@{ Event Handlers

	private: void process_sys_init(des_event_type const& evt, des_engine_context_type& ctx)
	{
		DCS_DEBUG_TRACE("(" << this << ") BEGIN Processing System Initialization (Clock: " << ctx.simulated_time() << ")");

::std::cerr << ">>>>" << ::std::endl;//XXX
		num_sla_viols_ = uint_type/*zero*/();

		DCS_DEBUG_TRACE("(" << this << ") END Processing System Initialization (Clock: " << ctx.simulated_time() << ")");
	}


	private: void process_sys_finit(des_event_type const& evt, des_engine_context_type& ctx)
	{
		DCS_DEBUG_TRACE("(" << this << ") BEGIN Processing System Finalization (Clock: " << ctx.simulated_time() << ")");

::std::cerr << "<<<<" << ::std::endl;//XXX
		(*ptr_num_sla_viols_stat_)(num_sla_viols_);
		(*ptr_num_arrs_stat_)(model_.num_arrivals());
		(*ptr_num_deps_stat_)(model_.num_departures());

		DCS_DEBUG_TRACE("(" << this << ") END Processing System Finalization (Clock: " << ctx.simulated_time() << ")");
	}


	private: void process_request_departure(des_event_type const& evt, des_engine_context_type& ctx)
	{
		DCS_DEBUG_TRACE("(" << this << ") BEGIN Processing Request Departure (Clock: " << ctx.simulated_time() << ")");

		typedef ::std::vector<performance_measure_category> category_container;
		typedef typename category_container::const_iterator category_iterator;
		typedef ::std::vector<real_type> measure_container;

		user_request_type req = model_traits_type::request_state(model_, evt);

		category_container categories(this->application().sla_cost_model().slo_categories());

		measure_container measures;
		category_iterator end_it = categories.end();
		for (category_iterator it = categories.begin(); it != end_it; ++it)
		{
			switch (*it)
			{
				case response_time_performance_measure:
					{
						real_type rt = req.departure_time()-req.arrival_time();
						measures.push_back(rt);
::std::cerr << rt << ::std::endl;//XXX
					}
					break;
//				case throughput_performance_measure:
//					{
//						real_type tp = (num_arrivals_-num_departies)/simulated_time;
//						measures.push_back(rt);
//					}
//					break;
			}
		}

		if (!this->application().sla_cost_model().satisfied(categories.begin(), categories.end(), measures.begin()))
		{
			DCS_DEBUG_TRACE("Found SLA violation for measures: " << measures[0]);

			++num_sla_viols_;
		}

		DCS_DEBUG_TRACE("(" << this << ") END Processing Request Departure (Clock: " << ctx.simulated_time() << ")");
	}

	//@} Event Handlers


	private: model_type model_;
	private: tier_mapping_container tier_map_;
//	private: output_statistic_category_container stats_;
	private: uint_type num_sla_viols_;
	private: output_statistic_pointer ptr_num_arrs_stat_;
	private: output_statistic_pointer ptr_num_deps_stat_;
	private: output_statistic_pointer ptr_num_sla_viols_stat_;
};

}} // Namespace dcs::eesim


#endif // DCS_EESIM_APPLICATION_SIMULATION_MODEL_ADAPTOR_HPP
