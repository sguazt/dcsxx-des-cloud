#ifndef DCS_EESIM_QN_APPLICATION_SIMULATION_MODEL_HPP
#define DCS_EESIM_QN_APPLICATION_SIMULATION_MODEL_HPP


#include <dcs/debug.hpp>
#include <dcs/des/engine_traits.hpp>
#include <dcs/des/base_statistic.hpp>
#include <dcs/des/mean_estimator.hpp>
#include <dcs/des/model/qn/network_node_category.hpp>//FIXME: see resource_share method
#include <dcs/des/model/qn/service_station_node.hpp>//FIXME: see do_resource_share method
#include <dcs/des/model/qn/queueing_network.hpp>
#include <dcs/des/model/qn/queueing_network_traits.hpp>
#include <dcs/des/model/qn/output_statistic_category.hpp>
#include <dcs/eesim/application_simulation_model_traits.hpp>
#include <dcs/eesim/base_application_simulation_model.hpp>
#include <dcs/eesim/performance_measure_category.hpp>
#include <dcs/eesim/physical_resource_category.hpp>
#include <dcs/eesim/registry.hpp>
#include <dcs/eesim/user_request.hpp>
#include <dcs/eesim/utility.hpp>
#include <dcs/functional/bind.hpp>
#include <dcs/macro.hpp>
#include <dcs/memory.hpp>
#include <stdexcept>
#include <vector>


//TODO:
// - Output statistic category is currently fixed to mean_estimator, but
//   maybe we want to handles some other kind of stats.


namespace dcs { namespace eesim {

namespace detail { namespace /*<unnamed>*/ {

//bool has_network_category(performance_measure_category category)
//{
//	switch (category)
//	{
//		case response_time_performance_measure:
//			return true;
//		case throughput_performance_measure:
//			return true;
//		default:
//			break;
//	}
//
//	return false;
//}


inline
::dcs::des::model::qn::network_output_statistic_category network_category(performance_measure_category category)
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

	throw ::std::runtime_error("[dcs::eesim::detail::network_category] Unable to associate a suitable network output statistic category for the given performance measure category.");
}


//bool has_node_category(performance_measure_category category)
//{
//	switch (category)
//	{
//		case response_time_performance_measure:
//			return true;
//		case throughput_performance_measure:
//			return true;
//		default:
//			break;
//	}
//
//	return false;
//}


inline
::dcs::des::model::qn::node_output_statistic_category node_category(performance_measure_category category)
{
	switch (category)
	{
		case busy_time_performance_measure:
			return ::dcs::des::model::qn::busy_time_statistic_category;
		case response_time_performance_measure:
			return ::dcs::des::model::qn::response_time_statistic_category;
		case throughput_performance_measure:
			return ::dcs::des::model::qn::throughput_statistic_category;
		case utilization_performance_measure:
			return ::dcs::des::model::qn::utilization_statistic_category;
		default:
			break;
	}

	throw ::std::runtime_error("[dcs::eesim::detail::node_category] Unable to associate a suitable node output statistic category for the given performance measure category.");
}


inline
bool has_performance_measure(performance_measure_category category)
{
	switch (category)
	{
		case busy_time_performance_measure:
			return true;
		case response_time_performance_measure:
			return true;
		case throughput_performance_measure:
			return true;
		case utilization_performance_measure:
			return true;
		default:
			break;
	}

	return false;
}

}} // Namespace detail::<unnamed>


template <
	typename TraitsT,
	typename UIntT,
	typename RealT,
	typename UniformRandomGeneratorT,
	typename DesEngineT
>
class qn_application_simulation_model: public base_application_simulation_model<TraitsT>
{
	private: typedef base_application_simulation_model<TraitsT> base_type;
	private: typedef qn_application_simulation_model<TraitsT,UIntT,RealT,UniformRandomGeneratorT,DesEngineT> self_type;
	public: typedef TraitsT traits_type;
	public: typedef ::dcs::des::model::qn::queueing_network<UIntT,RealT,UniformRandomGeneratorT,DesEngineT> qn_model_type;
	private: typedef ::dcs::des::model::qn::queueing_network_traits<qn_model_type> qn_model_traits_type;
	private: typedef typename base_type::des_event_source_type des_event_source_type;
	private: typedef typename base_type::des_event_type des_event_type;
	private: typedef typename base_type::real_type real_type;
	private: typedef typename base_type::uint_type uint_type;
	private: typedef typename base_type::output_statistic_type output_statistic_type;
	private: typedef typename base_type::output_statistic_pointer output_statistic_pointer;
	private: typedef typename qn_model_type::node_identifier_type node_identifier_type;
	private: typedef ::std::map<uint_type,node_identifier_type> tier_mapping_container;
	private: typedef ::std::map<node_identifier_type,uint_type> node_mapping_container;
	private: typedef typename base_type::user_request_type user_request_type;
	private: typedef registry<traits_type> registry_type;
	private: typedef typename traits_type::des_engine_type des_engine_type;
	private: typedef typename ::dcs::des::engine_traits<des_engine_type>::engine_context_type des_engine_context_type;
	private: typedef ::dcs::des::mean_estimator<real_type,uint_type> mean_estimator_statistic_type;
	private: typedef ::std::vector<output_statistic_pointer> output_statistic_container;
	private: typedef typename base_type::virtual_machine_type virtual_machine_type;


	/// A constructor.
	public: explicit qn_application_simulation_model(qn_model_type const& model)
	: base_type(),
	  model_(model),
	  tier_node_map_(),
	  num_sla_viols_(0),
	  ptr_num_arrs_stat_(new mean_estimator_statistic_type()),//FIXME: this should be forwarded to the adaptee object
	  ptr_num_deps_stat_(new mean_estimator_statistic_type()),//FIXME: this should be forwarded to the adaptee object
	  ptr_num_sla_viols_stat_(new mean_estimator_statistic_type())
	{
		init();
	}


	/// The destructor.
	public: ~qn_application_simulation_model()
	{
		disconnect_from_event_sources();
	}


	public: void tier_node_mapping(uint_type tier_id, node_identifier_type node_id)
	{
		tier_node_map_[tier_id] = node_id;
		node_tier_map_[node_id] = tier_id;

		if (tier_num_arrs_stats_.size() <= tier_id)
		{
			tier_num_arrs_stats_.resize(tier_id+1);
			tier_num_arrs_stats_[tier_id] = ::dcs::make_shared<mean_estimator_statistic_type>();
		}
		if (tier_num_deps_stats_.size() <= tier_id)
		{
			tier_num_deps_stats_.resize(tier_id+1);
			tier_num_deps_stats_[tier_id] = ::dcs::make_shared<mean_estimator_statistic_type>();
		}
	}


	private: void init()
	{
		connect_to_event_sources();
	}


	private: void connect_to_event_sources()
	{
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
		model_.departure_event_source().connect(
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
		model_.departure_event_source().disconnect(
			::dcs::functional::bind(
				&self_type::process_request_departure,
				this,
				::dcs::functional::placeholders::_1,
				::dcs::functional::placeholders::_2
			)
		);
	}


	private: bool tier_mapped(uint_type tier_id) const
	{
		return tier_node_map_.count(tier_id) > 0;
	}


	private: bool node_mapped(node_identifier_type node_id) const
	{
		return node_tier_map_.count(node_id) > 0;
	}


	private: node_identifier_type node_from_tier(uint_type tier_id) const
	{
		typedef typename tier_mapping_container::const_iterator iterator;

		iterator it(tier_node_map_.find(tier_id));

		if (it == tier_node_map_.end())
		{
			throw ::std::logic_error("[dcs::eesim::qn_application_simulation_model::node_from_tier] Tier not mapped.");
		}

		return it->second;
	}


	private: uint_type tier_from_node(node_identifier_type node_id) const
	{
		typedef typename node_mapping_container::const_iterator iterator;

		iterator it(node_tier_map_.find(node_id));

		if (it == node_tier_map_.end())
		{
			throw ::std::logic_error("[dcs::eesim::qn_application_simulation_model::tier_from_node] Node not mapped.");
		}

		return it->second;
	}


	//@{ Interface Member Functions

	private: void do_enable(bool flag)
	{
		model_.enable(flag);

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
		return model_.enabled();
	}


	private: uint_type do_actual_num_arrivals() const
	{
		return model_.num_arrivals();
	}


	private: uint_type do_actual_num_departures() const
	{
		return model_.num_departures();
	}


	private: uint_type do_actual_num_sla_violations() const
	{
		return num_sla_viols_;
	}


//	private: real_type do_actual_busy_time() const
//	{
//	}


	private: uint_type do_actual_tier_num_arrivals(uint_type tier_id) const
	{
		return model_.get_node(node_from_tier(tier_id)).num_arrivals();
	}


	private: uint_type do_actual_tier_num_departures(uint_type tier_id) const
	{
		return model_.get_node(node_from_tier(tier_id)).num_departures();
	}


	private: real_type do_actual_tier_busy_time(uint_type tier_id) const
	{
		return model_.get_node(node_from_tier(tier_id)).busy_time();
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


	private: output_statistic_type const& do_tier_num_arrivals(uint_type tier_id) const
	{
		// pre: tier_id is a valid tier identifier.
		DCS_ASSERT(
			tier_id < tier_num_arrs_stats_.size(),
			throw ::std::invalid_argument("[dcs::eesim::application_simulation_model_adaptor::do_tier_num_arrivals] Invalid tier identifier.")
		);

		return *tier_num_arrs_stats_[tier_id];
	}


	private: output_statistic_type const& do_tier_num_departures(uint_type tier_id) const
	{
		// pre: tier_id is a valid tier identifier.
		DCS_ASSERT(
			tier_id < tier_num_arrs_stats_.size(),
			throw ::std::invalid_argument("[dcs::eesim::application_simulation_model_adaptor::do_tier_num_departures] Invalid tier identifier.")
		);

		return *tier_num_deps_stats_[tier_id];
	}


	private: des_event_source_type& do_request_arrival_event_source()
	{
		return model_.arrival_event_source();
	}


	private: des_event_source_type const& do_request_arrival_event_source() const
	{
		return model_.arrival_event_source();
	}


	private: des_event_source_type& do_request_departure_event_source()
	{
		return model_.departure_event_source();
	}


	private: des_event_source_type const& do_request_departure_event_source() const
	{
		return model_.departure_event_source();
	}


	private: des_event_source_type& do_request_tier_arrival_event_source(uint_type tier_id)
	{
		return model_.get_node(node_from_tier(tier_id)).arrival_event_source();
	}


	private: des_event_source_type const& do_request_tier_arrival_event_source(uint_type tier_id) const
	{
		return model_.get_node(node_from_tier(tier_id)).arrival_event_source();
	}


	private: des_event_source_type& do_request_tier_departure_event_source(uint_type tier_id)
	{
		return model_.get_node(node_from_tier(tier_id)).departure_event_source();
	}


	private: des_event_source_type const& do_request_tier_departure_event_source(uint_type tier_id) const
	{
		return model_.get_node(node_from_tier(tier_id)).departure_event_source();
	}


	private: void do_statistic(performance_measure_category category, output_statistic_pointer const& ptr_stat)
	{
		model_.statistic(detail::network_category(category), ptr_stat);
	}


	private: ::std::vector<output_statistic_pointer> do_statistic(performance_measure_category category) const
	{
		return model_.statistic(detail::network_category(category));
	}


	private: void do_tier_statistic(uint_type tier_id, performance_measure_category category, output_statistic_pointer const& ptr_stat)
	{
		model_.get_node(node_from_tier(tier_id)).statistic(detail::node_category(category), ptr_stat);
	}


	private: ::std::vector<output_statistic_pointer> do_tier_statistic(uint_type tier_id, performance_measure_category category) const
	{
		return model_.get_node(node_from_tier(tier_id)).statistic(detail::node_category(category));
	}


	private: user_request_type do_request_state(des_event_type const& evt) const
	{
		return make_request(evt);
	}


	private: void do_resource_share(uint_type tier_id, physical_resource_category category, real_type share)
	{
//FIXME
// * What should I do with delay nodes?
// * What should I do if dynamic_cast returns a null pointer

		typedef typename qn_model_type::node_type node_type;

		node_type& node = model_.get_node(node_from_tier(tier_id));

		DCS_ASSERT(
				node.category() == ::dcs::des::model::qn::service_station_node_category,
				throw ::std::runtime_error("[dcs::eesim::qn_application_simulation_model::do_resource_share] Expected a service station node. Got another kind of node.")
			);

		typedef ::dcs::des::model::qn::service_station_node<qn_model_traits_type> service_node_type;

		service_node_type* ptr_svc_node(dynamic_cast<service_node_type*>(&node));

		DCS_ASSERT(
				ptr_svc_node,
				throw ::std::runtime_error("[dcs::eesim::qn_application_simulation_model::do_resource_share] Unable to get a service station node.")
			);

DCS_DEBUG_TRACE("New resource share for tier: " << tier_id);//XXX
DCS_DEBUG_TRACE("Current share: " << share);//XXX
DCS_DEBUG_TRACE("Actual machine capacity: " << this->tier_virtual_machine(tier_id)->vmm().hosting_machine().resource(category)->capacity());//XXX
DCS_DEBUG_TRACE("Actual machine threshold: " << this->tier_virtual_machine(tier_id)->vmm().hosting_machine().resource(category)->utilization_threshold());//XXX
		real_type multiplier;
		multiplier = ::dcs::eesim::resource_scaling_factor(
				this->application().reference_resource(category).capacity(),
				this->application().reference_resource(category).utilization_threshold(),
				this->tier_virtual_machine(tier_id)->vmm().hosting_machine().resource(category)->capacity(),
				this->tier_virtual_machine(tier_id)->vmm().hosting_machine().resource(category)->utilization_threshold()
			);
		multiplier *= share;
DCS_DEBUG_TRACE("New scaled share: " << multiplier);///XXX
		ptr_svc_node->service_strategy().capacity_multiplier(multiplier);
	}

	//@} Interface Member Functions


	//@{ Event Handlers

	private: void process_begin_of_sim(des_event_type const& evt, des_engine_context_type& ctx)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );

		DCS_DEBUG_TRACE("(" << this << ") BEGIN Processing BEGIN-OF-SIMULATION (Clock: " << ctx.simulated_time() << ")");

		// Reset stats
		ptr_num_sla_viols_stat_->reset();
		ptr_num_arrs_stat_->reset();
		ptr_num_deps_stat_->reset();

		DCS_DEBUG_TRACE("(" << this << ") END Processing BEGIN-OF-SIMULATION (Clock: " << ctx.simulated_time() << ")");
	}


	private: void process_sys_init(des_event_type const& evt, des_engine_context_type& ctx)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );

		DCS_DEBUG_TRACE("(" << this << ") BEGIN Processing SYSTEM-INITIALIZATION (Clock: " << ctx.simulated_time() << ")");

		num_sla_viols_ = uint_type/*zero*/();
::std::cerr << ">>>>>" << ::std::endl;//XXX

		DCS_DEBUG_TRACE("(" << this << ") END Processing SYSTEM-INITIALIZATION (Clock: " << ctx.simulated_time() << ")");
	}


	private: void process_sys_finit(des_event_type const& evt, des_engine_context_type& ctx)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );

		DCS_DEBUG_TRACE("(" << this << ") BEGIN Processing SYSTEM-FINALIZATION (Clock: " << ctx.simulated_time() << ")");

		// Update stats

		// - System-level stats
		(*ptr_num_sla_viols_stat_)(this->actual_num_sla_violations());
		(*ptr_num_arrs_stat_)(this->actual_num_arrivals());
		(*ptr_num_deps_stat_)(this->actual_num_departures());

		// - Per-tier stats
		typedef typename tier_mapping_container::const_iterator tier_node_map_iterator;
		tier_node_map_iterator tier_node_map_end_it = tier_node_map_.end();
		for (tier_node_map_iterator it = tier_node_map_.begin(); it != tier_node_map_end_it; ++it)
		{
			uint_type tier_id(it->first);
//			uint_type node_id(it->second);
::std::cerr << "<<<<<" << ::std::endl;//XXX

			(*tier_num_arrs_stats_[tier_id])(this->actual_tier_num_arrivals(tier_id));
			(*tier_num_deps_stats_[tier_id])(this->actual_tier_num_departures(tier_id));
		}

		DCS_DEBUG_TRACE("(" << this << ") END Processing SYSTEM-FINALIZATION (Clock: " << ctx.simulated_time() << ")");
	}


	private: void process_request_departure(des_event_type const& evt, des_engine_context_type& ctx)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );

		DCS_DEBUG_TRACE("(" << this << ") BEGIN Processing REQUEST-DEPARTURE (Clock: " << ctx.simulated_time() << ")");

		typedef ::std::vector<performance_measure_category> category_container;
		typedef typename category_container::const_iterator category_iterator;
		typedef ::std::vector<real_type> measure_container;

		user_request_type req = make_request(evt);

		category_container categories(this->application().sla_cost_model().slo_categories());

		measure_container measures;
		category_iterator end_it = categories.end();
		for (category_iterator it = categories.begin(); it != end_it; ++it)
		{
			performance_measure_category category(*it);

			switch (category)
			{
				case busy_time_performance_measure:
					throw ::std::runtime_error("[dcs::eesim::qn_application_simulation_model::process_request_departure] Busy time as SLO category has not been implemented yet.");//FIXME
				case response_time_performance_measure:
					{
						real_type rt = req.departure_time()-req.arrival_time();
::std::cerr << rt << ::std::endl;//XXX
						measures.push_back(rt);
					}
					break;
				case throughput_performance_measure:
					// Nothing to do since throughput is already an aggregate metric
					////real_type tp = (num_arrivals_-num_departies)/simulated_time;
					////measures.push_back(tp);
					break;
				case utilization_performance_measure:
					throw ::std::runtime_error("[dcs::eesim::qn_application_simulation_model::process_request_departure] Utilization as SLO category has not been implemented yet.");//FIXME
			}
		}

		if (!this->application().sla_cost_model().satisfied(categories.begin(), categories.end(), measures.begin()))
		{
			DCS_DEBUG_TRACE("Found SLA violation for measures: " << measures[0]);

			++num_sla_viols_;
		}

		DCS_DEBUG_TRACE("(" << this << ") END Processing REQUEST-DEPARTURE (Clock: " << ctx.simulated_time() << ")");
	}

	//@} Event Handlers


	//@{ Class members

	private: user_request_type make_request(des_event_type const& evt) const
	{
		typedef typename qn_model_type::customer_type customer_type;
		typedef ::dcs::shared_ptr<customer_type> customer_pointer;
		typedef typename tier_mapping_container::const_iterator tier_node_iterator;
		typedef ::std::vector<real_type> time_container;
		typedef typename time_container::const_iterator time_iterator;

		user_request_type req;

		customer_pointer ptr_customer = evt.template unfolded_state<customer_pointer>();

		req.id(ptr_customer->id());
		req.arrival_time(ptr_customer->arrival_time());
		req.departure_time(ptr_customer->departure_time());
		//TODO: introduce an invalid_tier_id and add an else branch to set req.current_tier(invalid_tier_id)
		if (node_mapped(ptr_customer->current_node()))
		{
			req.current_tier(tier_from_node(ptr_customer->current_node()));
		}
//		req.current_class(ptr_customer->current_class());
		tier_node_iterator tier_node_end_it = tier_node_map_.end();
		for (tier_node_iterator it = tier_node_map_.begin(); it != tier_node_end_it; ++it)
		{
			uint_type tier_id(it->first);
			node_identifier_type node_id(it->second);

			::std::vector<real_type> arr_times;
			::std::vector<real_type> dep_times;
			::std::size_t nt;
			arr_times = ptr_customer->node_arrival_times(node_id);
			dep_times = ptr_customer->node_departure_times(node_id);
			nt = arr_times.size();
			for (::std::size_t t = 0; t < nt; ++t)
			{
				req.tier_arrival_time(tier_id, arr_times[t]);
				req.tier_departure_time(tier_id, dep_times[t]);
			}
		}

		return req;
	}

	//@} Class members


	//@{ Data Members

	private: qn_model_type model_;
	private: tier_mapping_container tier_node_map_;
	private: node_mapping_container node_tier_map_;
//	private: output_statistic_category_container stats_;
	private: uint_type num_sla_viols_;
	private: output_statistic_pointer ptr_num_arrs_stat_;
	private: output_statistic_pointer ptr_num_deps_stat_;
	private: output_statistic_pointer ptr_num_sla_viols_stat_;
	private: output_statistic_container tier_num_arrs_stats_;
	private: output_statistic_container tier_num_deps_stats_;

	//@} Data Members
};

}} // Namespace dcs::eesim


#endif // DCS_EESIM_QN_APPLICATION_SIMULATION_MODEL_HPP
