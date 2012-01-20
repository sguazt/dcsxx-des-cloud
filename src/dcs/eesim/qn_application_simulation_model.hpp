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
#include <dcs/exception.hpp>
#include <dcs/functional/bind.hpp>
#include <dcs/macro.hpp>
#include <dcs/memory.hpp>
#include <stdexcept>
#include <vector>

//[XXX]
#ifdef DCS_EESIM_EXP_OUTPUT_VM_MEASURES
# include <cstdlib>
# include <iosfwd>
# include <fstream>
# include <sstream>
# include <string>
#endif // DCS_EESIM_EXP_OUTPUT_VM_MEASURES
//[/XXX]


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
		case queue_length_performance_measure:
			return ::dcs::des::model::qn::num_waiting_statistic_category;
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
		case queue_length_performance_measure:
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


#ifdef DCS_EESIM_EXP_OUTPUT_VM_MEASURES

template <typename TraitsT>
inline
void dump_app_measure(typename TraitsT::uint_type app_id, performance_measure_category category, typename TraitsT::real_type measure, typename TraitsT::real_type target_value)
{
	typedef TraitsT traits_type;
	typedef typename traits_type::uint_type uint_type;
	typedef typename traits_type::real_type real_type;

	uint_type nrep = dynamic_cast< ::dcs::des::replications::engine<real_type,uint_type>* >(registry<traits_type>::instance().des_engine_ptr().get())->num_replications();

	if (nrep == 1)
	{
		::std::ostringstream oss;
		oss << "vm_measures-" << ::std::string(::getenv("CONDOR_JOB_ID")) << ".dat";
		::std::ofstream ofs(oss.str().c_str(), ::std::ios_base::app);
		ofs << ::std::setprecision(16);
		ofs << registry<traits_type>::instance().des_engine_ptr()->simulated_time() << "," << app_id << "," << -1 << "," << category << "," << measure << "," << target_value << ::std::endl;
		ofs.close();
	}
}


template <typename TraitsT>
inline
void dump_tier_measure(typename TraitsT::uint_type app_id, typename TraitsT::uint_type tier_id, performance_measure_category category, typename TraitsT::real_type measure, typename TraitsT::real_type target_value)
{
	typedef TraitsT traits_type;
	typedef typename traits_type::uint_type uint_type;
	typedef typename traits_type::real_type real_type;

//	static bool first_call(true);

	uint_type nrep = dynamic_cast< ::dcs::des::replications::engine<real_type,uint_type>* >(registry<traits_type>::instance().des_engine_ptr().get())->num_replications();

	if (nrep == 1)
	{
		::std::ostringstream oss;
		oss << "vm_measures-" << ::std::string(::getenv("CONDOR_JOB_ID")) << ".dat";
		::std::ofstream ofs(oss.str().c_str(), ::std::ios_base::app);
		ofs << ::std::setprecision(16);
//		if (first_call)
//		{
//			// Dump header: "clock","aid","tid","category","measure","target"
//			ofs << "\"clock\",\"aid\",\"tid\",\"category\",\"measure\",\"target\"" << ::std::endl;
//			first_call = false;
//		}
		ofs << registry<traits_type>::instance().des_engine_ptr()->simulated_time() << "," << app_id << "," << tier_id << "," << category << "," << measure << "," << target_value << ::std::endl;
		ofs.close();
	}
}

#endif // DCS_EESIM_EXP_OUTPUT_VM_MEASURES

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
	public: typedef ::dcs::shared_ptr<qn_model_type> qn_model_pointer;
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
	private: typedef typename base_type::virtual_machine_pointer virtual_machine_pointer;
	private: typedef typename qn_model_type::customer_type customer_type;
	private: typedef ::dcs::shared_ptr<customer_type> customer_pointer;



	/// A constructor.
	public: explicit qn_application_simulation_model(qn_model_pointer const& ptr_model)
	: base_type(),
	  ptr_model_(ptr_model),
	  tier_node_map_(),
	  num_sla_viols_(0),
	  ptr_num_arrs_stat_(new mean_estimator_statistic_type()),//FIXME: this should be forwarded to the qn_model_type object
	  ptr_num_deps_stat_(new mean_estimator_statistic_type()),//FIXME: this should be forwarded to the qn_model_type object
	  ptr_num_sla_viols_stat_(new mean_estimator_statistic_type())
	{
		init();
	}


	/// Copy constructor.
	private: qn_application_simulation_model(qn_application_simulation_model const& that)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(that);

		//TODO
		DCS_EXCEPTION_THROW( ::std::runtime_error, "Copy-constructor not yet implemented." );
	}


	/// Copy assignment.
	private: qn_application_simulation_model& operator=(qn_application_simulation_model const& rhs)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(rhs);

		//TODO
		DCS_EXCEPTION_THROW( ::std::runtime_error, "Copy-assigment not yet implemented." );
	}


	/// The destructor.
	public: ~qn_application_simulation_model()
	{
		finit();
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


	public: qn_model_type const& qn_model() const
	{
		return *ptr_model_;
	}


	public: qn_model_type& qn_model()
	{
		return *ptr_model_;
	}


	private: void init()
	{
//		if (this->enabled())
//		{
			connect_to_event_sources();
//		}
	}


	private: void finit()
	{
//		if (this->enabled())
//		{
			disconnect_from_event_sources();
//		}
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
		ptr_model_->departure_event_source().connect(
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
		ptr_model_->departure_event_source().disconnect(
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

	protected: void do_enable(bool flag)
	{
		base_type::do_enable(flag);

		ptr_model_->enable(flag);

		// enable/disable statistics
		typedef typename output_statistic_container::iterator stat_iterator;

		ptr_num_arrs_stat_->enable(flag);
		ptr_num_deps_stat_->enable(flag);
		ptr_num_sla_viols_stat_->enable(flag);

		stat_iterator stat_end_it;
		stat_end_it = tier_num_arrs_stats_.end();
		for (stat_iterator stat_it = tier_num_arrs_stats_.begin(); stat_it != stat_end_it; ++stat_it)
		{
			(*stat_it)->enable(flag);
		}
		stat_end_it = tier_num_deps_stats_.end();
		for (stat_iterator stat_it = tier_num_deps_stats_.begin(); stat_it != stat_end_it; ++stat_it)
		{
			(*stat_it)->enable(flag);
		}

//		if (flag)
//		{
//			if (!this->enabled())
//			{
//				connect_to_event_sources();
//			}
//		}
//		else
//		{
//			if (this->enabled())
//			{
//				disconnect_from_event_sources();
//			}
//		}
	}


//	private: bool do_enabled() const
//	{
//		return ptr_model_->enabled();
//	}


	private: uint_type do_actual_num_arrivals() const
	{
		return ptr_model_->num_arrivals();
	}


	private: uint_type do_actual_num_departures() const
	{
		return ptr_model_->num_departures();
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
		return ptr_model_->get_node(node_from_tier(tier_id)).num_arrivals();
	}


	private: uint_type do_actual_tier_num_departures(uint_type tier_id) const
	{
		return ptr_model_->get_node(node_from_tier(tier_id)).num_departures();
	}


	private: real_type do_actual_tier_busy_time(uint_type tier_id) const
	{
		return ptr_model_->get_node(node_from_tier(tier_id)).busy_time();
	}


//	private: real_type do_actual_tier_queue_length(uint_type tier_id) const
//	{
//		return ptr_model_->get_node(node_from_tier(tier_id)).queue_length();
//	}


//	private: real_type do_tier_queue_length(uint_type tier_id) const
//	{
//		return ptr_model_->get_node(node_from_tier(tier_id)).queue_length();
//	}


	private: ::std::vector<user_request_type> do_tier_in_service_requests(uint_type tier_id) const
	{
		typedef typename qn_model_type::node_type node_type;
		typedef ::dcs::des::model::qn::service_station_node<qn_model_traits_type> service_node_type;
		typedef ::std::vector<customer_pointer> customer_container;
		typedef typename customer_container::const_iterator customer_iterator;

		node_type const& node = ptr_model_->get_node(node_from_tier(tier_id));

		// pre: tier must be a service-station node.
		DCS_ASSERT(
				node.category() == ::dcs::des::model::qn::service_station_node_category,
				throw ::std::runtime_error("[dcs::eesim::qn_application_simulation_model::do_tier_in_service_requests] Expected a service station node. Got another kind of node.")
			);

		service_node_type const* ptr_svc_node(dynamic_cast<service_node_type const*>(&node));

		// double-check: tier must be a service-station node.
		DCS_ASSERT(
				ptr_svc_node,
				throw ::std::runtime_error("[dcs::eesim::qn_application_simulation_model::do_tier_in_service_requests] Unable to get a service station node.")
			);

		::std::vector<user_request_type> requests;

		customer_container customers(ptr_svc_node->active_customers());
		customer_iterator customer_end_it(customers.end());
		for (customer_iterator it = customers.begin(); it != customer_end_it; ++it)
		{
			requests.push_back(make_request(*it));
		}

		return requests;
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


	private: void do_start_application()
	{
		// empty
	}


	private: void do_stop_application()
	{
		// empty
	}


	private: des_event_source_type& do_request_arrival_event_source()
	{
		return ptr_model_->arrival_event_source();
	}


	private: des_event_source_type const& do_request_arrival_event_source() const
	{
		return ptr_model_->arrival_event_source();
	}


	private: des_event_source_type& do_request_departure_event_source()
	{
		return ptr_model_->departure_event_source();
	}


	private: des_event_source_type const& do_request_departure_event_source() const
	{
		return ptr_model_->departure_event_source();
	}


	private: des_event_source_type& do_request_tier_arrival_event_source(uint_type tier_id)
	{
		return ptr_model_->get_node(node_from_tier(tier_id)).arrival_event_source();
	}


	private: des_event_source_type const& do_request_tier_arrival_event_source(uint_type tier_id) const
	{
		return ptr_model_->get_node(node_from_tier(tier_id)).arrival_event_source();
	}


	private: des_event_source_type& do_request_tier_service_event_source(uint_type tier_id)
	{
//FIXME
// * What should I do with delay nodes?
// * What should I do if dynamic_cast returns a null pointer

		typedef typename qn_model_type::node_type node_type;
		typedef ::dcs::des::model::qn::service_station_node<qn_model_traits_type> service_node_type;

		node_type& node = ptr_model_->get_node(node_from_tier(tier_id));

		// pre: tier must be a service-station node.
		DCS_ASSERT(
				node.category() == ::dcs::des::model::qn::service_station_node_category,
				throw ::std::runtime_error("[dcs::eesim::qn_application_simulation_model::do_request_tier_service_event_source] Expected a service station node. Got another kind of node.")
			);

		service_node_type* ptr_svc_node(dynamic_cast<service_node_type*>(&node));

		// double-check: tier must be a service-station node.
		DCS_ASSERT(
				ptr_svc_node,
				throw ::std::runtime_error("[dcs::eesim::qn_application_simulation_model::do_request_tier_service_event_source] Unable to get a service station node.")
			);

		return ptr_svc_node->service_event_source();
	}


	private: des_event_source_type const& do_request_tier_service_event_source(uint_type tier_id) const
	{
//FIXME
// * What should I do with delay nodes?
// * What should I do if dynamic_cast returns a null pointer

		typedef typename qn_model_type::node_type node_type;
		typedef ::dcs::des::model::qn::service_station_node<qn_model_traits_type> service_node_type;

		node_type const& node = ptr_model_->get_node(node_from_tier(tier_id));

		// pre: tier must be a service-station node.
		DCS_ASSERT(
				node.category() == ::dcs::des::model::qn::service_station_node_category,
				throw ::std::runtime_error("[dcs::eesim::qn_application_simulation_model::do_request_tier_service_event_source] Expected a service station node. Got another kind of node.")
			);

		service_node_type const* ptr_svc_node(dynamic_cast<service_node_type const*>(&node));

		// double-check: tier must be a service-station node.
		DCS_ASSERT(
				ptr_svc_node,
				throw ::std::runtime_error("[dcs::eesim::qn_application_simulation_model::do_request_tier_service_event_source] Unable to get a service station node.")
			);

		return ptr_svc_node->service_event_source();
	}


	private: des_event_source_type& do_request_tier_departure_event_source(uint_type tier_id)
	{
		return ptr_model_->get_node(node_from_tier(tier_id)).departure_event_source();
	}


	private: des_event_source_type const& do_request_tier_departure_event_source(uint_type tier_id) const
	{
		return ptr_model_->get_node(node_from_tier(tier_id)).departure_event_source();
	}


	private: void do_statistic(performance_measure_category category, output_statistic_pointer const& ptr_stat)
	{
		ptr_model_->statistic(detail::network_category(category), ptr_stat);
	}


	private: ::std::vector<output_statistic_pointer> do_statistic(performance_measure_category category) const
	{
		return ptr_model_->statistic(detail::network_category(category));
	}


	private: void do_tier_statistic(uint_type tier_id, performance_measure_category category, output_statistic_pointer const& ptr_stat)
	{
		ptr_model_->get_node(node_from_tier(tier_id)).statistic(detail::node_category(category), ptr_stat);
	}


	private: ::std::vector<output_statistic_pointer> do_tier_statistic(uint_type tier_id, performance_measure_category category) const
	{
		return ptr_model_->get_node(node_from_tier(tier_id)).statistic(detail::node_category(category));
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

		node_type& node = ptr_model_->get_node(node_from_tier(tier_id));

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
DCS_DEBUG_TRACE("New share: " << share);//XXX
DCS_DEBUG_TRACE("Actual machine capacity: " << this->tier_virtual_machine(tier_id)->vmm().hosting_machine().resource(category)->capacity());//XXX
		real_type multiplier;
//		multiplier = ::dcs::eesim::resource_scaling_factor(
//				this->application().reference_resource(category).capacity(),
//				this->tier_virtual_machine(tier_id)->vmm().hosting_machine().resource(category)->capacity()
//			);
//DCS_DEBUG_TRACE("Reference to Actual Machine Scaling Factor: " << multiplier);///XXX
//::std::cerr << "[qn_simulation_model] Reference to Actual Machine Scaling Factor: " << multiplier << ::std::endl;///XXX
//		multiplier *= share/this->application().tier(tier_id)->resource_share(category);
		multiplier = scale_resource_share(this->tier_virtual_machine(tier_id)->vmm().hosting_machine().resource(category)->capacity(),
										  this->application().reference_resource(category).capacity(),
										  share);
DCS_DEBUG_TRACE("New scaled share: " << multiplier);///XXX
DCS_DEBUG_TRACE("Old capacity multiplier: " << ptr_svc_node->capacity_multiplier());///XXX
		//ptr_svc_node->service_strategy().capacity_multiplier(multiplier);
		ptr_svc_node->capacity_multiplier(multiplier);//[sguazt] EXP
DCS_DEBUG_TRACE("New capacity multiplier: " << ptr_svc_node->capacity_multiplier());///XXX
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

//[XXX]
#ifdef DCS_EESIM_EXP_OUTPUT_VM_MEASURES
		for (uint_type tid = 0; tid < this->application().num_tiers(); ++tid)
		{
			this->request_tier_departure_event_source(tid).connect(
				::dcs::functional::bind(
					&self_type::process_request_tier_departure,
					this,
					::dcs::functional::placeholders::_1,
					::dcs::functional::placeholders::_2,
					tid
				)
			);
		}
#endif // DCS_EESIM_EXP_OUTPUT_VM_MEASURES
//[/XXX]

//		// schedule life-time events
//		this->schedule_application_start();
//		this->schedule_application_stop();

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

			(*tier_num_arrs_stats_[tier_id])(this->actual_tier_num_arrivals(tier_id));
			(*tier_num_deps_stats_[tier_id])(this->actual_tier_num_departures(tier_id));
		}

//[XXX]
#ifdef DCS_EESIM_EXP_OUTPUT_VM_MEASURES
		for (uint_type tid = 0; tid < this->application().num_tiers(); ++tid)
		{
			this->request_tier_departure_event_source(tid).disconnect(
				::dcs::functional::bind(
					&self_type::process_request_tier_departure,
					this,
					::dcs::functional::placeholders::_1,
					::dcs::functional::placeholders::_2,
					tid
				)
			);
		}
#endif // DCS_EESIM_EXP_OUTPUT_VM_MEASURES
//[/XXX]

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
				case queue_length_performance_measure:
					throw ::std::runtime_error("[dcs::eesim::qn_application_simulation_model::process_request_departure] Queue length as SLO category has not been implemented yet.");//FIXME
				case response_time_performance_measure:
					{
						real_type rt = req.departure_time()-req.arrival_time();
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
#ifdef DCS_EESIM_EXP_OUTPUT_VM_MEASURES
			detail::dump_app_measure<traits_type>(this->application().id(),
												  category,
												  measures.back(),
												  this->application().sla_cost_model().slo_value(category));
#endif // DCS_EESIM_EXP_OUTPUT_VM_MEASURES
		}

		if (!this->application().sla_cost_model().satisfied(categories.begin(), categories.end(), measures.begin()))
		{
			DCS_DEBUG_TRACE("Found SLA violation for measures: " << measures[0]);
::std::cerr << "APP " << this->application().id() << " -- SLA violation: " << measures[0] << " vs " << this->application().sla_cost_model().slo_value(response_time_performance_measure) << " (Clock: " << ctx.simulated_time() << ")" << ::std::endl;//XXX

			++num_sla_viols_;
		}

		DCS_DEBUG_TRACE("(" << this << ") END Processing REQUEST-DEPARTURE (Clock: " << ctx.simulated_time() << ")");
	}


#ifdef DCS_EESIM_EXP_OUTPUT_VM_MEASURES
	private: void process_request_tier_departure(des_event_type const& evt, des_engine_context_type& ctx, uint_type tid)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );

		DCS_DEBUG_TRACE("(" << this << ") BEGIN Processing REQUEST-TIER-DEPARTURE (Clock: " << ctx.simulated_time() << ")");

		typedef ::std::vector<performance_measure_category> category_container;
		typedef typename category_container::const_iterator category_iterator;
		typedef ::std::vector<real_type> measure_container;

		user_request_type req = make_request(evt);

		// check: make sure this request is at this tier
		if (tid != req.current_tier())
		{
			return;
		}

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
				case queue_length_performance_measure:
					throw ::std::runtime_error("[dcs::eesim::qn_application_simulation_model::process_request_departure] Queue length as SLO category has not been implemented yet.");//FIXME
				case response_time_performance_measure:
					{
						real_type rt = req.tier_departure_times(req.current_tier()).back()-req.tier_arrival_times(req.current_tier()).back();
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

			detail::dump_tier_measure<traits_type>(this->application().id(),
												   req.current_tier(),
												   category,
												   measures.back(),
												   this->application().performance_model().tier_measure(req.current_tier(), category));
		}

		DCS_DEBUG_TRACE("(" << this << ") END Processing REQUEST-TIER-DEPARTURE (Clock: " << ctx.simulated_time() << ")");
	}
#endif // DCS_EESIM_EXP_OUTPUT_VM_MEASURES

	//@} Event Handlers


	//@{ Class members

	private: user_request_type make_request(des_event_type const& evt) const
	{
//		typedef typename qn_model_type::customer_type customer_type;
//		typedef ::dcs::shared_ptr<customer_type> customer_pointer;

		customer_pointer ptr_customer = evt.template unfolded_state<customer_pointer>();

		return make_request(ptr_customer);
	}


	private: user_request_type make_request(customer_pointer const& ptr_customer) const
	{
//		typedef typename qn_model_type::customer_type customer_type;
//		typedef ::dcs::shared_ptr<customer_type> customer_pointer;
		typedef typename tier_mapping_container::const_iterator tier_node_iterator;
		typedef ::std::vector<real_type> time_container;
		typedef typename time_container::const_iterator time_iterator;
		typedef typename customer_type::utilization_profile_type customer_utilization_profile_type;
		typedef resource_utilization_profile<traits_type> request_utilization_profile_type;
		typedef typename customer_utilization_profile_type::const_iterator customer_utilization_profile_iterator;

		// check: safety check
		DCS_DEBUG_ASSERT( ptr_customer );

		user_request_type req;

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
			virtual_machine_pointer ptr_vm(this->tier_virtual_machine(tier_id));

			// check: paranoid-check
			DCS_DEBUG_ASSERT( ptr_vm );

			// It is possible that the VM for this tier has already been displaced or it is not powered-on
			if (!ptr_vm->deployed() || ptr_vm->power_state() != powered_on_power_status)
			{
				continue;
			}

			::std::vector<real_type> arr_times;
			::std::vector<real_type> dep_times;
			arr_times = ptr_customer->node_arrival_times(node_id);
			dep_times = ptr_customer->node_departure_times(node_id);
			::std::size_t na(arr_times.size());
			::std::size_t nd(dep_times.size());
			for (::std::size_t i = 0; i < na; ++i)
			{
				req.tier_arrival_time(tier_id, arr_times[i]);
				// check that for this arrival there is already a departure
				if (i < nd)
				{
					req.tier_departure_time(tier_id, dep_times[i]);
				}
			}

			::std::vector<customer_utilization_profile_type> profiles;
			profiles = ptr_customer->node_utilization_profiles(node_id);
			::std::size_t np(profiles.size());
			for (::std::size_t i = 0; i < np; ++i)
			{
				customer_utilization_profile_type const& customer_profile(profiles[i]);
				physical_resource_category category(cpu_resource_category); //FIXME: CPU resource category is hard-coded

				request_utilization_profile_type request_profile;
				customer_utilization_profile_iterator profile_end_it(customer_profile.end());
				for (customer_utilization_profile_iterator it = customer_profile.begin(); it != profile_end_it; ++it)
				{
					typename customer_utilization_profile_iterator::value_type const& item(*it);
					real_type util;
					util = ::dcs::eesim::scale_resource_utilization(
							this->application().reference_resource(category).capacity(),
							this->tier_virtual_machine(tier_id)->vmm().hosting_machine().resource(category)->capacity(),
							item.utilization()
						);
					request_profile(item.begin_time(), item.end_time(), util);
				}
				req.tier_utilization_profile(tier_id, category, request_profile);
			}
		}

		return req;
	}

	//@} Class members


	//@{ Data Members

	private: qn_model_pointer ptr_model_;
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
}; // qn_application_simulation_model

}} // Namespace dcs::eesim


#endif // DCS_EESIM_QN_APPLICATION_SIMULATION_MODEL_HPP
