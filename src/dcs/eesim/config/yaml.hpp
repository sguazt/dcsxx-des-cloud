#ifndef DCS_EESIM_CONFIG_YAML_HPP
#define DCS_EESIM_CONFIG_YAML_HPP


#include <cstddef>
#include <dcs/assert.hpp>
#include <dcs/debug.hpp>
#include <dcs/eesim/config/application.hpp>
#include <dcs/eesim/config/application_performance_model.hpp>
#include <dcs/eesim/config/application_simulation_model.hpp>
#include <dcs/eesim/config/application_sla.hpp>
#include <dcs/eesim/config/application_tier.hpp>
#include <dcs/eesim/config/configuration.hpp>
#include <dcs/eesim/config/data_center.hpp>
#include <dcs/eesim/config/initial_placement_strategy.hpp>
#include <dcs/eesim/config/metric_category.hpp>
#include <dcs/eesim/config/numeric_matrix.hpp>
#include <dcs/eesim/config/numeric_multiarray.hpp>
#include <dcs/eesim/config/physical_machine.hpp>
#include <dcs/eesim/config/physical_resource.hpp>
#include <dcs/eesim/config/probability_distribution.hpp>
#include <dcs/eesim/config/rng.hpp>
#include <dcs/eesim/config/simulation.hpp>
#include <fstream>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>
#include <yaml-cpp/yaml.h>


namespace dcs { namespace eesim { namespace config {

namespace detail { namespace /*<unnamed>*/ {

physical_resource_category text_to_physical_resource_category(::std::string const& str)
{
	if (!str.compare("cpu"))
	{
		return cpu_resource;
	}
	if (!str.compare("mem"))
	{
		return mem_resource;
	}
	if (!str.compare("disk"))
	{
		return disk_resource;
	}
	if (!str.compare("nic"))
	{
		return nic_resource;
	}

	throw ::std::runtime_error("[dcs::eesim::config::detail::text_to_physical_resource_category] Unknown physical resource category.");
}


metric_category text_to_metric_category(::std::string str)
{
	if (!str.compare("response-time"))
	{
		return response_time_metric;
	}
	else if (!str.compare("throughput"))
	{
		return throughput_metric;
	}

	throw ::std::runtime_error("[dcs::eesim::config::metric_category] Unknown metric category.");
}


energy_model_category text_to_energy_model_category(::std::string const& str)
{
	if (!str.compare("constant"))
	{
		return constant_energy_model;
	}
	if (!str.compare("fan-2007"))
	{
		return fan2007_energy_model;
	}

	throw ::std::runtime_error("[dcs::eesim::config::detail::text_to_energy_model_category] Unknown energy model category.");
}


rng_engine_category text_to_rng_engine_category(::std::string const& str)
{
	if (!str.compare("minstd-rand0"))
	{
		return minstd_rand0_rng_engine;
	}
	if (!str.compare("minstd-rand1"))
	{
		return minstd_rand1_rng_engine;
	}
	if (!str.compare("minstd-rand2"))
	{
		return minstd_rand2_rng_engine;
	}
	if (!str.compare("rand48"))
	{
		return rand48_rng_engine;
	}
	if (!str.compare("mt11213b"))
	{
		return mt11213b_rng_engine;
	}
	if (!str.compare("mt19937"))
	{
		return mt19937_rng_engine;
	}

	throw ::std::runtime_error("[dcs::eesim::config::detail::text_to_rng_engine_category] Unknown random number generation engine category.");
}


rng_seeder_category text_to_rng_seeder_category(::std::string const& str)
{
	if (!str.compare("lcg"))
	{
		return lcg_rng_seeder;
	}
	if (!str.compare("none"))
	{
		return none_rng_seeder;
	}

	throw ::std::runtime_error("[dcs::eesim::config::detail::text_to_rng_seeder_category] Unknown random number generation seeder category.");
}


output_analysis_category text_to_output_analysis_category(::std::string const& str)
{
	if (!str.compare("independent-replications"))
	{
		return independent_replications_output_analysis;
	}

	throw ::std::runtime_error("[dcs::eesim::config::detail::text_to_output_analysis_category] Unknown simulation output analysis category.");
}


statistic_category text_to_statistic_category(::std::string const& str)
{
	if (!str.compare("mean"))
	{
		return mean_statistic;
	}

	throw ::std::runtime_error("[dcs::eesim::config::detail::text_to_statistic_category] Unknown simulation statistic category.");
}


probability_distribution_category text_to_probability_distribution_category(::std::string const& str)
{
	if (!str.compare("exponential"))
	{
		return exponential_probability_distribution;
	}
	if (!str.compare("gamma"))
	{
		return gamma_probability_distribution;
	}
	if (!str.compare("normal"))
	{
		return normal_probability_distribution;
	}

	throw ::std::runtime_error("[dcs::eesim::config::detail::text_to_probability_distribution_category] Unknown probability distribution category.");
}


qn_node_category text_to_qn_node_category(::std::string const& str)
{
	if (!str.compare("delay"))
	{
		return qn_delay_node;
	}
	if (!str.compare("queue"))
	{
		return qn_queue_node;
	}
	if (!str.compare("source"))
	{
		return qn_source_node;
	}
	if (!str.compare("sink"))
	{
		return qn_sink_node;
	}

	throw ::std::runtime_error("[dcs::eesim::config::detail::text_to_qn_node_category] Unknown queueing network node category.");
}


qn_customer_class_category text_to_qn_customer_class_category(::std::string const& str)
{
	if (!str.compare("closed"))
	{
		return qn_closed_customer_class;
	}
	if (!str.compare("open"))
	{
		return qn_open_customer_class;
	}

	throw ::std::runtime_error("[dcs::eesim::config::detail::text_to_qn_customer_class_category Unknown queueing network customer class category.");
}


qn_routing_strategy_category text_to_qn_routing_strategy_category(::std::string const& str)
{
	if (!str.compare("probabilistic"))
	{
		return qn_probabilistic_routing_strategy;
	}

	throw ::std::runtime_error("[dcs::eesim::config::detail::text_to_qn_routing_strategy_category Unknown queueing network routing strategy category.");
}


qn_service_strategy_category text_to_qn_service_strategy_category(::std::string const& str)
{
	if (!str.compare("infinite-server"))
	{
		return qn_infinite_server_service_strategy;
	}
	if (!str.compare("load-independent"))
	{
		return qn_load_independent_service_strategy;
	}

	throw ::std::runtime_error("[dcs::eesim::config::detail::text_to_qn_service_strategy_category Unknown queueing network service strategy category.");
}


initial_placement_strategy_category text_to_initial_placement_strategy_category(::std::string const& str)
{
	if (!str.compare("first-fit"))
	{
		return first_fit_initial_placement_strategy;
	}

	throw ::std::runtime_error("[dcs::eesim::config::detail::text_to_initial_placement_strategy_category Unknown init VM placement strategy category.");
}

}} // Namespace detail::<unnamed>


template <typename UIntT>
void operator>>(::YAML::Node const& node, rng_config<UIntT>& rng)
{
	::std::string label;

	node["engine"] >> label;
	rng.engine = detail::text_to_rng_engine_category(label);
	node["seed"] >> rng.seed;
	if (node.FindValue("seeder"))
	{
		node["seeder"] >> label;
		rng.seeder = detail::text_to_rng_seeder_category(label);
	}
	else
	{
		rng.seeder = none_rng_seeder;
	}
}


template <typename RealT>
void operator>>(::YAML::Node const& node, statistic_config<RealT>& stat)
{
	if (node.FindValue("type"))
	{
		statistic_category category;
		::std::string label;
		node["type"] >> label;
		category = detail::text_to_statistic_category(label);
	}
	else
	{
		stat.type = mean_statistic;
	}
	if (node.FindValue("precision"))
	{
		node["precision"] >> stat.precision;
	}
	else
	{
		stat.precision = ::std::numeric_limits<RealT>::infinity();
	}
	if (node.FindValue("confidence-level"))
	{
		node["confidence-level"] >> stat.confidence_level;
	}
	else
	{
		stat.confidence_level = 0.95;
	}
}


template <typename RealT, typename UIntT>
void operator>>(::YAML::Node const& node, independent_replications_output_analysis_config<RealT,UIntT>& analysis)
{
	// Read number of replications
	if (node.FindValue("num-replications"))
	{
		node["num-replications"] >> analysis.num_replications;
	}
	else
	{
		analysis.num_replications = 1;
	}
	// Read length of each replication
	if (node.FindValue("replication-duration"))
	{
		node["replication-duration"] >> analysis.replication_duration;
	}
	else
	{
		analysis.replication_duration = 1000;
	}
	// Read statistics
	::YAML::Node const& subnode = node["statistics"];
	for (::YAML::Iterator it = subnode.begin(); it != subnode.end(); ++it)
	{
		::YAML::Node const& key_node = it.first();
		::YAML::Node const& value_node = it.second();

		::std::string label;
		metric_category category;
		statistic_config<RealT> stat;

		key_node >> label;
		category = detail::text_to_metric_category(label);

		value_node >> stat;

		analysis.statistics[category] = stat;
	}
}


template <typename RealT, typename UIntT>
void operator>>(::YAML::Node const& node, simulation_config<RealT,UIntT>& sim)
{
	::std::string label;

	::YAML::Node const& subnode = node["output-analysis"];
	subnode["type"] >> label;

	output_analysis_category category;
	category = detail::text_to_output_analysis_category(label);

	sim.output_analysis_type = category;

	switch (category)
	{
		case independent_replications_output_analysis:
			{
				independent_replications_output_analysis_config<RealT,UIntT> conf;
				subnode >> conf;

				sim.output_analysis_conf = conf;
			}
			break;
	}
}


template <typename T>
void operator>>(::YAML::Node const& node, numeric_matrix<T>& m)
{
	::std::size_t nr;
	::std::size_t nc;
	::std::vector<T> data;
	bool byrow;
	node["rows"] >> nr;
	node["cols"] >> nc;
	node["byrow"] >> byrow;
	node["data"] >> data;
	m = numeric_matrix<T>(nr, nc, data.begin(), data.end(), byrow);
}


template <typename T>
void operator>>(::YAML::Node const& node, numeric_multiarray<T>& a)
{
	typedef ::std::size_t size_type;

	::std::vector<size_type> dims;
	::std::vector<size_type> bydims;
	::std::vector<T> data;

	node["dims"] >> dims;
	if (node.FindValue("bydims"))
	{
		node["bydims"] >> bydims;
	}
	else
	{
		bydims = ::std::vector<size_type>(dims.size());
		for (size_type i = 0; i < bydims.size(); ++i)
		{
			bydims[i] = i;
		}
	}
	node["data"] >> data;
	a = numeric_multiarray<T>(dims.begin(), dims.end(), data.begin(), data.end(), bydims.begin(), bydims.end());
}


template <typename RealT, typename UIntT>
void operator>>(::YAML::Node const& node, application_performance_model_config<RealT,UIntT>& model)
{
	application_performance_model_category type;
	::std::string type_lbl;
	node["type"] >> type_lbl;
	if (!type_lbl.compare("open-multi-bcmp-qn"))
	{
		type = open_multi_bcmp_qn_model;
	}
	else
	{
		throw ::std::runtime_error("[dcs::eesim::config::>>] Unknown application performance model.");
	}

	model.type = type;

	switch (type)
	{
		case open_multi_bcmp_qn_model:
			{
				typename application_performance_model_config<RealT,UIntT>::open_multi_bcmp_qn_config conf;
				node["arrival-rates"] >> conf.arrival_rates;
				if (node.FindValue("visit-ratios"))
				{
					node["visit-ratios"] >> conf.visit_ratios;
				}
				else if (node.FindValue("routing-probabilities"))
				{
					node["routing-probabilities"] >> conf.routing_probabilities;
				}
				else
				{
					throw ::std::runtime_error("[dcs::eesim::config::>>] Missing both visit ratios and routing probabilities.");
				}
				node["service-times"] >> conf.service_times;
				node["num-servers"] >> conf.num_servers;

				model.type_conf = conf;
			}
			break;
	}
}


template <typename RealT>
void operator>>(::YAML::Node const& node, probability_distribution_config<RealT>& distr_conf)
{
	typedef probability_distribution_config<RealT> distribution_config_type;

	::std::string label;

	node["type"] >> label;
	distr_conf.category = detail::text_to_probability_distribution_category(label);

	switch (distr_conf.category)
	{
		case exponential_probability_distribution:
			{
				typedef typename distribution_config_type::exponential_distribution_config_type distribution_config_impl_type;

				distribution_config_impl_type& distr_conf_impl = ::boost::get<distribution_config_impl_type>(distr_conf.category_conf);

				node["rate"] >> distr_conf_impl.rate;

				distr_conf.category_conf = distr_conf_impl;
			}
			break;
		case gamma_probability_distribution:
			{
				typedef typename distribution_config_type::gamma_distribution_config_type distribution_config_impl_type;

				distribution_config_impl_type& distr_conf_impl = ::boost::get<distribution_config_impl_type>(distr_conf.category_conf);

				node["scale"] >> distr_conf_impl.scale;
				node["shape"] >> distr_conf_impl.shape;

				distr_conf.category_conf = distr_conf_impl;
			}
			break;
		case normal_probability_distribution:
			{
				typedef typename distribution_config_type::normal_distribution_config_type distribution_config_impl_type;

				distribution_config_impl_type& distr_conf_impl = ::boost::get<distribution_config_impl_type>(distr_conf.category_conf);

				node["mean"] >> distr_conf_impl.mean;
				node["sd"] >> distr_conf_impl.sd;

				distr_conf.category_conf = distr_conf_impl;
			}
			break;
	}
}


template <typename RealT, typename UIntT>
void operator>>(::YAML::Node const& node, qn_node_config<RealT,UIntT>& node_conf)
{
	typedef RealT real_type;
	typedef UIntT uint_type;
	typedef qn_node_config<RealT,UIntT> node_config_type;

	::std::string label;

	if (node.FindValue("name"))
	{
		node["name"] >> node_conf.name;
	}
//	if (node.FindValue("id"))
//	{
//		node["id"] >> node_conf.id;
//	}
//	else
//	{
//		node_conf.id = i;
//	}
	node["type"] >> label;
	node_conf.category = detail::text_to_qn_node_category(label);

	switch (node_conf.category)
	{
		case qn_delay_node:
			{
				typedef typename node_config_type::delay_node_config_type node_impl_config_type;

				node_impl_config_type node_impl_conf;

				// Read routing strategy
				{
					::YAML::Node const& routing_node = node["routing-strategy"];
					routing_node["type"] >> label;
					node_impl_conf.routing_category = detail::text_to_qn_routing_strategy_category(label);
					switch (node_impl_conf.routing_category)
					{
						case qn_probabilistic_routing_strategy:
							{
								typedef typename node_impl_config_type::probabilistic_routing_strategy_config_type routing_config_impl_type;

								routing_config_impl_type routing_impl_conf;

								routing_node["probabilities"] >> routing_impl_conf.probabilities;

								node_impl_conf.routing_conf = routing_impl_conf;
							}
							break;
					}
				}
				// Read service strategy
				{
					::YAML::Node const& service_node = node["service-strategy"];
					qn_service_strategy_category service_category;
					if (service_node.FindValue("type"))
					{
						service_node["type"] >> label;
						service_category = detail::text_to_qn_service_strategy_category(label);
					}
					else
					{
						service_category = qn_infinite_server_service_strategy;
					}
					switch (service_category)
					{
						case qn_infinite_server_service_strategy:
							{
								typedef typename node_impl_config_type::probability_distribution_config_type distribution_config_type;

								::YAML::Node const& distributions_node = service_node["distributions"];
								for (::std::size_t i = 0; i < distributions_node.size(); ++i)
								{
									::YAML::Node const& distribution_node = distributions_node[i];
									distribution_config_type distribution_conf;

									distribution_node["distribution"] >> distribution_conf;

									node_impl_conf.distributions.push_back(distribution_conf);
								}
							}
							break;
						default:
							throw ::std::runtime_error("[dcs::eesim::config::>>] The service strategy of a delay node can only be 'infinite-server'.");
					}
				}

				node_conf.category = qn_delay_node;
				node_conf.category_conf = node_impl_conf;
			}
			break;
		case qn_queue_node:
			{
				typedef typename node_config_type::queue_node_config_type node_impl_config_type;

				node_impl_config_type node_impl_conf;

				if (node.FindValue("num-servers"))
				{
					node["num-servers"] >> node_impl_conf.num_servers;
				}
				else
				{
					node_impl_conf.num_servers = 1;
				}
				if (node.FindValue("policy"))
				{
					node["policy"] >> label;
					if (!label.compare("fcfs"))
					{
						node_impl_conf.policy_category = qn_fcfs_scheduling_policy;
					}
					else
					{
						throw ::std::runtime_error("[dcs::eesim::config::>>] Unknown queueing network scheduling policy.");
					}
				}
				else
				{
					node_impl_conf.policy_category = qn_fcfs_scheduling_policy;
				}
				if (node.FindValue("capacity"))
				{
					node["capacity"] >> node_impl_conf.capacity;
					node_impl_conf.is_infinite = false;
				}
				else
				{
					node_impl_conf.is_infinite = true;
					node_impl_conf.capacity = 0;
				}
//				::YAML::Node const& routing_node = node["routing-strategy"];
//				routing_node["type"] >> label;
//				if (!label.compare("probabilistic"))
//				{
//					typedef typename node_impl_config_type::probabilistic_routing_strategy_config_type routing_config_impl_type;
//
//					routing_config_impl_type routing_impl_conf;
//
//					routing_node["probabilities"] >> routing_impl_conf.probabilities;
//
//					node_impl_conf.routing_category = qn_probabilistic_routing_strategy;
//					node_impl_conf.routing_conf = routing_impl_conf;
//				}
//				else
//				{
//					throw ::std::runtime_error("[dcs::eesim::config::>>] Unknown queueing network routing strategy.");
//				}
				// Read routing strategy
				{
					::YAML::Node const& routing_node = node["routing-strategy"];
					routing_node["type"] >> label;
					node_impl_conf.routing_category = detail::text_to_qn_routing_strategy_category(label);
					switch (node_impl_conf.routing_category)
					{
						case qn_probabilistic_routing_strategy:
							{
								typedef typename node_impl_config_type::probabilistic_routing_strategy_config_type routing_config_impl_type;

								routing_config_impl_type routing_impl_conf;

								routing_node["probabilities"] >> routing_impl_conf.probabilities;

								node_impl_conf.routing_conf = routing_impl_conf;
							}
							break;
					}
				}
//				::YAML::Node const& service_node = node["service-strategy"];
//				service_node["type"] >> label;
//				if (!label.compare("load-independent"))
//				{
//					typedef typename node_impl_config_type::load_independent_service_strategy_config_type service_config_impl_type;
//					typedef typename service_config_impl_type::probability_distribution_config_type distribution_config_type;
//
//					service_config_impl_type service_impl_conf;
//
//					::YAML::Node const& distributions_node = service_node["distributions"];
//					for (::std::size_t i = 0; i < distributions_node.size(); ++i)
//					{
//						::YAML::Node const& distribution_node = distributions_node[i];
//						distribution_config_type distribution_conf;
//
//						distribution_node["distribution"] >> distribution_conf;
//
//						service_impl_conf.distributions.push_back(distribution_conf);
//					}
//
//					node_impl_conf.service_category = qn_load_independent_service_strategy;
//					node_impl_conf.service_conf = service_impl_conf;
//				}
//				else
//				{
//					throw ::std::runtime_error("[dcs::eesim::config::>>] Unknown queueing network routing strategy.");
//				}
				// Read service strategy
				{
					::YAML::Node const& service_node = node["service-strategy"];
					if (service_node.FindValue("type"))
					{
						service_node["type"] >> label;
						node_impl_conf.service_category = detail::text_to_qn_service_strategy_category(label);
					}
					else
					{
						node_impl_conf.service_category = qn_load_independent_service_strategy;
					}

					switch (node_impl_conf.service_category)
					{
						case qn_infinite_server_service_strategy:
							throw ::std::runtime_error("[dcs::eesim::config::>>] The service strategy of a queue node cannot be 'infinite-server'.");
						case qn_load_independent_service_strategy:
							{
								typedef typename node_impl_config_type::load_independent_service_strategy_config_type service_config_impl_type;
								typedef typename service_config_impl_type::probability_distribution_config_type distribution_config_type;

								service_config_impl_type service_impl_conf;

								::YAML::Node const& distributions_node = service_node["distributions"];
								for (::std::size_t i = 0; i < distributions_node.size(); ++i)
								{
									::YAML::Node const& distribution_node = distributions_node[i];
									distribution_config_type distribution_conf;

									distribution_node["distribution"] >> distribution_conf;

									service_impl_conf.distributions.push_back(distribution_conf);
								}

								node_impl_conf.service_conf = service_impl_conf;
							}
							break;
					}
				}

				node_conf.category = qn_queue_node;
				node_conf.category_conf = node_impl_conf;
			}
			break;
		case qn_sink_node:
			{
				typedef typename node_config_type::sink_node_config_type node_impl_config_type;

				node_impl_config_type node_impl_conf;

				node_conf.category = qn_sink_node;

				node_conf.category_conf = node_impl_conf;
			}
			break;
		case qn_source_node:
			{
				typedef typename node_config_type::source_node_config_type node_impl_config_type;

				node_impl_config_type node_impl_conf;

//				::YAML::Node const& routing_node = node["routing-strategy"];
//				routing_node["type"] >> label;
//				if (!label.compare("probabilistic"))
//				{
//					typedef typename node_impl_config_type::probabilistic_routing_strategy_config_type routing_config_impl_type;
//
//					routing_config_impl_type routing_impl_conf;
//
//					routing_node["probabilities"] >> routing_impl_conf.probabilities;
//
//					node_impl_conf.routing_category = qn_probabilistic_routing_strategy;
//					node_impl_conf.routing_conf = routing_impl_conf;
//				}
//				else
//				{
//					throw ::std::runtime_error("[dcs::eesim::config::>>] Unknown queueing network routing strategy.");
//				}
				// Read routing strategy
				{
					::YAML::Node const& routing_node = node["routing-strategy"];
					routing_node["type"] >> label;
					node_impl_conf.routing_category = detail::text_to_qn_routing_strategy_category(label);
					switch (node_impl_conf.routing_category)
					{
						case qn_probabilistic_routing_strategy:
							{
								typedef typename node_impl_config_type::probabilistic_routing_strategy_config_type routing_config_impl_type;

								routing_config_impl_type routing_impl_conf;

								routing_node["probabilities"] >> routing_impl_conf.probabilities;

								node_impl_conf.routing_conf = routing_impl_conf;
							}
							break;
					}
				}

				node_conf.category = qn_source_node;
				node_conf.category_conf = node_impl_conf;
			}
			break;
	}
}


template <typename RealT, typename UIntT>
void operator>>(::YAML::Node const& node, qn_customer_class_config<RealT,UIntT>& customer_class_conf)
{
	typedef RealT real_type;
	typedef UIntT uint_type;
	typedef qn_customer_class_config<RealT,UIntT> customer_class_config_type;

	::std::string label;

	if (node.FindValue("name"))
	{
		node["name"] >> customer_class_conf.name;
	}

	node["type"] >> label;
	customer_class_conf.category = detail::text_to_qn_customer_class_category(label);

//	node["reference-node"] >> node_id;
	::YAML::Node const& ref_node = node["reference-node"];
	if (customer_class_conf.category == qn_open_customer_class)
	{
		ref_node["type"] >> label;

		qn_node_category node_cat = detail::text_to_qn_node_category(label);

		if (node_cat != qn_source_node)
		{
			throw ::std::runtime_error("[dcs::eesim::config::>>] Reference nodes of an open customer class can only be source nodes.");
		}
	}
	else
	{
		ref_node["type"] >> label;

		qn_node_category node_cat = detail::text_to_qn_node_category(label);

		if (node_cat != qn_queue_node && node_cat != qn_delay_node)
		{
			throw ::std::runtime_error("[dcs::eesim::config::>>] Reference nodes of an closed customer class can only be queueing or delay nodes.");
		}
	}
	ref_node["name"] >> customer_class_conf.ref_node;

	switch (customer_class_conf.category)
	{
		case qn_open_customer_class:
			{
				typedef typename customer_class_config_type::open_class_config_type customer_class_config_impl_type;

				customer_class_config_impl_type customer_class_conf_impl;

				node["distribution"] >> customer_class_conf_impl.distribution;

				customer_class_conf.category_conf = customer_class_conf_impl;
			}
			break;
		case qn_closed_customer_class:
			{
				typedef typename customer_class_config_type::closed_class_config_type customer_class_config_impl_type;

				customer_class_config_impl_type customer_class_conf_impl;

				node["size"] >> customer_class_conf_impl.size;

				customer_class_conf.category_conf = customer_class_conf_impl;
			}
			break;
	}
}


template <typename RealT, typename UIntT>
void operator>>(::YAML::Node const& node, application_simulation_model_config<RealT,UIntT>& model)
{
	typedef application_simulation_model_config<RealT,UIntT> sim_model_type;
	typedef RealT real_type;
	typedef UIntT uint_type;

	application_simulation_model_category model_category;
	::std::string label;
	node["type"] >> label;
	if (!label.compare("qn"))
	{
		model_category = qn_model;
	}
	else
	{
		throw ::std::runtime_error("[dcs::eesim::config::>>] Unknown application simulation model.");
	}

	model.category = model_category;

	switch (model_category)
	{
		case qn_model:
			{
				typedef typename sim_model_type::qn_model_config_type qn_config_type;
				qn_config_type conf;

//TODO: allow the user to specify a unique (global) routing probability matrix
//				// Read (optional) global routing probability table
//				if (node.FindValue("routing-strategy"))
//				{
//					::YAML::Node const& subnode = node["routing-strategy"];
//
//					subnode["type"] >> label;
//
//					conf.routing_category = detail::text_to_routing_strategy_category(label);
//
//					switch (conf.routing_category)
//					{
//						case qn_probabilistic_routing_strategy:
//							{
//								typedef typename qn_config_type::probabilistic_routing_strategy_confg_type routing_config_impl_type;
//
//								routing_config_impl_type routing_conf_impl;
//
//								numeric_multiarray<real_type> a;
//								subnode["probabilities"] >> routing_conf_impl.probabilities;
//							}
//							break;
//					}
//				}

				// Read nodes
				{
					typedef typename qn_config_type::node_config_type node_config_type;

					::YAML::Node const& subnode = node["nodes"];
					for (::std::size_t i = 0; i < subnode.size(); ++i)
					{
						node_config_type node_conf;

						::YAML::Node const& node_node = subnode[i]["node"];

						node_node >> node_conf;

						node_conf.id = i;

						conf.nodes.push_back(node_conf);
					}
				}

				// Read customer classes
				{
					typedef typename qn_config_type::customer_class_config_type customer_class_config_type;

					::YAML::Node const& subnode = node["customer-classes"];
					for (::std::size_t i = 0; i < subnode.size(); ++i)
					{
						customer_class_config_type customer_class_conf;

						::YAML::Node const& customer_class_node = subnode[i]["customer-class"];

						customer_class_node >> customer_class_conf;

						customer_class_conf.id = i;

						conf.customer_classes.push_back(customer_class_conf);
					}
				}

				model.category_conf = conf;
			}
			break;
		default:
			break;
	}
}


template <typename RealT>
void operator>>(::YAML::Node const& node, sla_metric_config<RealT>& metric)
{
	node["value"] >> metric.value;

	::YAML::Node const& subnode = node["cost-model"];
	::std::string label;
	subnode["type"] >> label;
	if (!label.compare("step"))
	{
		step_sla_model_config<RealT> model;
		subnode["penalty"] >> model.penalty;
		subnode["revenue"] >> model.revenue;

		metric.model_type = step_sla_model;
		metric.model_conf = model;
	}
	else
	{
		throw ::std::runtime_error("[dcs::eesim::config::>>] Unknown SLA cost model.");
	}
}


template <typename RealT>
void operator>>(::YAML::Node const& node, application_sla_config<RealT>& sla)
{
	for(::YAML::Iterator it = node.begin(); it != node.end(); ++it)
	{
		::YAML::Node const& key_node = it.first();
		::YAML::Node const& value_node = it.second();

		::std::string label;
		metric_category category;
		key_node >> label;
		category = detail::text_to_metric_category(label);

		sla_metric_config<RealT> metric_conf;
		value_node >> metric_conf;
		sla.metrics[category] = metric_conf;
	}
}


template <typename RealT>
void operator>>(::YAML::Node const& node, application_tier_config<RealT>& tier)
{
	if (node.FindValue("name"))
	{
		node["name"] >> tier.name;
	}
	if (node.FindValue("shares"))
	{
		::YAML::Node const& subnode = node["shares"];
		for (::YAML::Iterator it = subnode.begin(); it != subnode.end(); ++it)
		{
			::YAML::Node const& key_node = it.first();
			::YAML::Node const& value_node = it.second();

			physical_resource_category category;
			RealT share;
			::std::string resource;

			key_node >> resource;
			value_node >> share;

			category = detail::text_to_physical_resource_category(resource);

			tier.shares[category] = share;
		}
	}
}


template <typename RealT, typename UIntT>
void operator>>(::YAML::Node const& node, application_config<RealT,UIntT>& app)
{
	// Read (optional) name
	if (node.FindValue("name"))
	{
		node["name"] >> app.name;
	}
	// Read performance model
	node["perf-model"] >> app.perf_model;
	// Read simulation model
	node["sim-model"] >> app.sim_model;
	// Read reference resources
	{
		::YAML::Node const& subnode = node["reference-resources"];

		for (::YAML::Iterator it = subnode.begin(); it != subnode.end(); ++it)
		{
			::YAML::Node const& key_node = it.first();
			::YAML::Node const& value_node = it.second();

			physical_resource_category category;
			RealT capacity;
			::std::string resource;

			key_node >> resource;
			value_node >> capacity;

			category = detail::text_to_physical_resource_category(resource);

			app.reference_resources[category] = capacity;
		}
	}
	// Read SLA model
	node["sla"] >> app.sla;
	// Read tiers
	{
		::YAML::Node const& subnode = node["tiers"];
		for (::std::size_t i = 0; i < subnode.size(); ++i)
		{
			application_tier_config<RealT> tier;

			subnode[i]["tier"] >> tier;

			app.tiers.push_back(tier);
		}
	}
}


template <typename RealT>
void operator>>(::YAML::Node const& node, physical_resource_config<RealT>& res)
{
	// Read the (optional) name
	if (node.FindValue("name"))
	{
		node["name"] >> res.name;
	}
	// Read resource capacity
	node["capacity"] >> res.capacity;
	// Read (optonal) resource utilization threshold
	if (node.FindValue("threshold"))
	{
		node["threshold"] >> res.threshold;
	}
	else
	{
		res.threshold = 1;
	}
	// Read energy model
	{
		::std::string category_lbl;
		::YAML::Node const& subnode = node["energy-model"];
		subnode["type"] >> category_lbl;

		energy_model_category category;
		category = detail::text_to_energy_model_category(category_lbl);
		res.energy_model_type = category;

		switch (category)
		{
			case constant_energy_model:
				{
					constant_energy_model_config<RealT> model_conf;

					subnode["c0"] >> model_conf.c0;

					res.energy_model_conf = model_conf;
				}
				break;
			case fan2007_energy_model:
				{
					fan2007_energy_model_config<RealT> model_conf;

					subnode["c0"] >> model_conf.c0;
					subnode["c1"] >> model_conf.c1;
					subnode["c2"] >> model_conf.c2;
					subnode["r"] >> model_conf.r;

					res.energy_model_conf = model_conf;
				}
				break;
		}
	}
}


template <typename RealT>
void operator>>(::YAML::Node const& node, physical_machine_config<RealT>& mach)
{
	// Read (optional) name
	if (node.FindValue("name"))
	{
		node["name"] >> mach.name;
	}
	// Read resources
	{
		::YAML::Node const& subnode = node["resources"];
		for (::YAML::Iterator it = subnode.begin(); it != subnode.end(); ++it)
		{
			::YAML::Node const& key_node = it.first();
			::YAML::Node const& value_node = it.second();

			physical_resource_category category;
			physical_resource_config<RealT> resource_conf;
			::std::string resource;

			key_node >> resource;
			value_node >> resource_conf;

			category = detail::text_to_physical_resource_category(resource);

			resource_conf.type = category;
			mach.resources[category] = resource_conf;
		}
	}
}

template <typename RealT, typename UIntT>
class yaml_reader
{
	public: typedef RealT real_type;
	public: typedef UIntT uint_type;
	public: typedef configuration<RealT,UIntT> configuration_type;
	private: static const ::std::string tag_dcs_eesim;
	private: static const ::std::string tag_dcs_eesim_app;
	private: static const ::std::string tag_dcs_eesim_matrix;
//	private: static const ::std::string tag_dcs_eesim_perf_qn;
//	private: static const ::std::string tag_dcs_eesim_sim_qn;


	public: yaml_reader()
	{
	}


	public: yaml_reader(::std::string const& fname)
	{
		read(fname);
	}


	public: configuration_type read(::std::string const& fname)
	{
		::std::ifstream ifs(fname.c_str());

		configuration_type conf;

		conf = read(ifs);

		ifs.close();

		return conf;
	}


	public: template <typename CharT, typename CharTraitsT>
		configuration_type read(::std::basic_istream<CharT,CharTraitsT>& is)
	{
		configuration_type conf;

		try
		{
			::YAML::Parser parser(is);
			::YAML::Node doc;
			while(parser.GetNextDocument(doc))
			{
				conf = read(doc);
			}
		}
		catch(::YAML::Exception const& e)
		{
			::std::cerr << e.what() << "\n";
		}

		return conf;
	}


	public: configuration_type read(YAML::Node const& doc)
	{
		configuration_type conf;
		::std::string label;

		// Read random number generation settings
		{
			rng_config<uint_type> rng;

			doc["random-number-generation"] >> rng;

			conf.rng(rng);
		}
		// Read simulation settings
		{
			simulation_config<real_type,uint_type> sim;

			doc["simulation"] >> sim;

			conf.simulation(sim);
		}
		// Read data center
		{
			::YAML::Node const& node = doc["data-center"];

			typedef typename configuration_type::data_center_config_type data_center_config_type;

			data_center_config_type dc;

			// Read applications
			{
				::YAML::Node const& subnode = node["applications"];
				::std::size_t n = subnode.size();
				for (::std::size_t i = 0; i < n; ++i)
				{
					application_config<real_type,uint_type> app;

					::YAML::Node const& app_node = subnode[i]["application"];
					app_node >> app;

					dc.add_application(app);
				}
			}
			// Read physical machines
			{
				::YAML::Node const& subnode = node["physical-machines"];
				::std::size_t n = subnode.size();
				for (::std::size_t i = 0; i < n; ++i)
				{
					physical_machine_config<real_type> mach;

					::YAML::Node const& mach_node = subnode[i]["physical-machine"];
					mach_node >> mach;

					dc.add_physical_machine(mach);
				}
			}
			// Initial placement
			{
				::YAML::Node const& subnode = node["initial-placement-strategy"];
				subnode["type"] >> label;

				dc.initial_placement_category(
					detail::text_to_initial_placement_strategy_category(label)
				);

				switch (dc.initial_placement_category())
				{
					case first_fit_initial_placement_strategy:
						{
							typedef first_fit_initial_placement_strategy_config initial_placement_config_type;

							initial_placement_config_type initial_placement_conf;

							dc.initial_placement_strategy_conf(initial_placement_conf);
						}
						break;
				}
			}

			conf.data_center(dc);
		}

		return conf;
	}


	private: ::std::string type_to_string(::YAML::CONTENT_TYPE type)
	{
			switch (type)
			{
				case ::YAML::CT_SCALAR:
					return "SCALAR";
				case ::YAML::CT_SEQUENCE:
					return "SEQUENCE";
				case ::YAML::CT_MAP:
					return "MAP";
				case ::YAML::CT_NONE:
					return "(empty)";
				default:
					return "(unknown)";
			}
	}


/*
	private: void traverse(YAML::Node const& node, unsigned int depth = 0)
	{
		::std::string out;
		::YAML::CONTENT_TYPE type = node.GetType();
		::std::string indent((size_t)depth, '\t');

		::std::string tag = node.GetTag();

		if (tag.empty())
		{
			switch (type)
			{
				case ::YAML::CT_SCALAR:
					node >> out;
					::std::cout << indent << "SCALAR: " << out << ::std::endl;
					break;
				case ::YAML::CT_SEQUENCE:
					::std::cout << indent << "SEQUENCE:" << ::std::endl;
					for (unsigned int i = 0; i < node.size(); i++) {
						const ::YAML::Node & subnode = node[i];
						::std::cout << indent << "[" << i << "]:" << ::std::endl;
						traverse(subnode, depth + 1);
					}
					break;
				case ::YAML::CT_MAP:
					::std::cout << indent << "MAP:" << ::std::endl;
					for (::YAML::Iterator i = node.begin(); i != node.end(); ++i) {
						const ::YAML::Node & key   = i.first();
						const ::YAML::Node & value = i.second();
						key >> out;
						::std::cout << indent << "KEY: " << out << ::std::endl;
						::std::cout << indent << "VALUE:" << ::std::endl;
						traverse(value, depth + 1);
					}
					break;
				case ::YAML::CT_NONE:
					::std::cout << indent << "(empty)" << ::std::endl;
					break;
				default:
					::std::cerr << "Warning: traverse: unknown/unsupported node type" << ::std::endl;
			}
		}
//		else if (!tag.compare(0, tag_dcs_eesim.length(), tag_dcs_eesim))
//		{
//			::std::cout << indent << "(tag: " << node.GetTag() << ")" << ::std::endl;
//		}
		else if (!tag.compare(tag_dcs_eesim_sim_qn_node))
		{
			::std::cout << indent << "(tag: " << node.GetTag() << ")" << ::std::endl;
			detail::matrix<double> m;
			node >> m;
			::std::cout << "Matrix: " << m << std::endl;
		}
		else if (!tag.compare(tag_dcs_eesim_matrix))
		{
			::std::cout << indent << "(tag: " << node.GetTag() << ")" << ::std::endl;
			detail::matrix<double> m;
			node >> m;
			::std::cout << "Matrix: " << m << std::endl;
		}
		else
		{
			::std::cerr << "Warning: traverse: unknown/unsupported node tag '" << tag << "'" << ::std::endl;
		}
	}
*/
};

template <typename RealT, typename UIntT>
const ::std::string yaml_reader<RealT,UIntT>::tag_dcs_eesim = ::std::string("tag:dcs.di.unipmn.it,2010:eesim/");

template <typename RealT, typename UIntT>
const ::std::string yaml_reader<RealT,UIntT>::tag_dcs_eesim_app = yaml_reader::tag_dcs_eesim + ::std::string("app");

template <typename RealT, typename UIntT>
const ::std::string yaml_reader<RealT,UIntT>::tag_dcs_eesim_matrix = ::std::string("tag:dcs.di.unipmn.it,2010:eesim/matrix");


}}} // Namespace dcs::eesim::config

#endif // DCS_EESIM_CONFIG_YAML_HPP
