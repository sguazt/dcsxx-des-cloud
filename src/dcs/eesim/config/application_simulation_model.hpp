#ifndef DCS_EESIM_CONFIG_APPLICATION_SIMULATION_MODEL_HPP
#define DCS_EESIM_CONFIG_APPLICATION_SIMULATION_MODEL_HPP


#include <boost/variant.hpp>
#include <dcs/eesim/config/metric_category.hpp>
#include <dcs/eesim/config/numeric_multiarray.hpp>
#include <dcs/eesim/config/probability_distribution.hpp>
#include <dcs/eesim/config/statistic.hpp>
#include <dcs/macro.hpp>
#include <iosfwd>
#include <map>


namespace dcs { namespace eesim { namespace config {

enum application_simulation_model_category
{
	qn_model
};


enum qn_customer_class_category
{
	qn_closed_customer_class,
	qn_open_customer_class
};


enum qn_node_category
{
	qn_source_node,
	qn_sink_node,
	qn_queue_node,
	qn_delay_node
};


enum qn_scheduling_policy_category
{
	qn_fcfs_scheduling_policy,
	qn_lcfs_scheduling_policy,
	qn_processor_sharing_scheduling_policy,
	qn_round_robin_scheduling_policy
};


enum qn_routing_strategy_category
{
	qn_probabilistic_routing_strategy
};


enum qn_service_strategy_category
{
	qn_infinite_server_service_strategy,
	qn_load_independent_service_strategy,
	qn_processor_sharing_service_strategy,
	qn_round_robin_service_strategy
};


//struct qn_fcfs_scheduling_policy_config
//{
//};


template <typename RealT>
struct qn_probabilistic_routing_strategy_config
{
	typedef RealT real_type;
//	typedef ::std::vector<real_type> probability_container;
	typedef numeric_multiarray<real_type> probability_container;

	probability_container probabilities;
};


struct qn_fcfs_scheduling_policy_config
{
};


struct qn_lcfs_scheduling_policy_config
{
};


struct qn_processor_sharing_scheduling_policy_config
{
};


struct qn_round_robin_scheduling_policy_config
{
};


//template <typename RealT>
//struct qn_infinite_server_service_strategy_config
//{
//	typedef RealT real_type;
//	typedef probability_distribution_config<real_type> probability_distribution_config_type;
//	typedef ::std::vector<probability_distribution_config_type> probability_distribution_container;
//
//	probability_distribution_container distributions;
//};


template <typename RealT>
struct qn_load_independent_service_strategy_config
{
	typedef RealT real_type;
	typedef probability_distribution_config<real_type> probability_distribution_config_type;
	typedef ::std::vector<probability_distribution_config_type> probability_distribution_container;

	probability_distribution_container distributions;
};


template <typename RealT>
struct qn_processor_sharing_service_strategy_config
{
	typedef RealT real_type;
	typedef probability_distribution_config<real_type> probability_distribution_config_type;
	typedef ::std::vector<probability_distribution_config_type> probability_distribution_container;

	probability_distribution_container distributions;
};


template <typename RealT>
struct qn_round_robin_service_strategy_config
{
	typedef RealT real_type;
	typedef probability_distribution_config<real_type> probability_distribution_config_type;
	typedef ::std::vector<probability_distribution_config_type> probability_distribution_container;

	probability_distribution_container distributions;
	real_type quantum;
};


template <typename RealT, typename UIntT>
struct qn_delay_node_config
{
	typedef RealT real_type;
	typedef UIntT uint_type;
	typedef qn_probabilistic_routing_strategy_config<RealT> probabilistic_routing_strategy_config_type;
	typedef probability_distribution_config<real_type> probability_distribution_config_type;
	typedef ::std::vector<probability_distribution_config_type> probability_distribution_container;

	qn_routing_strategy_category routing_category;
	::boost::variant<probabilistic_routing_strategy_config_type> routing_conf;
	probability_distribution_container distributions;
};


template <typename RealT, typename UIntT>
struct qn_queue_node_config
{
	typedef RealT real_type;
	typedef UIntT uint_type;
	typedef qn_fcfs_scheduling_policy_config fcfs_scheduling_policy_config_type;
	typedef qn_lcfs_scheduling_policy_config lcfs_scheduling_policy_config_type;
	typedef qn_processor_sharing_scheduling_policy_config processor_sharing_scheduling_policy_config_type;
	typedef qn_round_robin_scheduling_policy_config round_robin_scheduling_policy_config_type;
	typedef qn_probabilistic_routing_strategy_config<RealT> probabilistic_routing_strategy_config_type;
	typedef qn_load_independent_service_strategy_config<real_type> load_independent_service_strategy_config_type;
	typedef qn_processor_sharing_service_strategy_config<real_type> processor_sharing_service_strategy_config_type;
	typedef qn_round_robin_service_strategy_config<real_type> round_robin_service_strategy_config_type;

	unsigned long num_servers;
	bool is_infinite;
	uint_type capacity;
	qn_scheduling_policy_category policy_category;
	::boost::variant<fcfs_scheduling_policy_config_type,
					 lcfs_scheduling_policy_config_type,
					 processor_sharing_scheduling_policy_config_type,
					 round_robin_scheduling_policy_config_type> policy_conf;
	qn_routing_strategy_category routing_category;
	::boost::variant<probabilistic_routing_strategy_config_type> routing_conf;
	qn_service_strategy_category service_category;
	::boost::variant<load_independent_service_strategy_config_type,
					 processor_sharing_service_strategy_config_type,
					 round_robin_service_strategy_config_type> service_conf;
};


template <typename RealT, typename UIntT>
struct qn_sink_node_config { };


template <typename RealT, typename UIntT>
struct qn_source_node_config
{
	typedef RealT real_type;
	typedef UIntT uint_type;
	typedef qn_probabilistic_routing_strategy_config<RealT> probabilistic_routing_strategy_config_type;

	qn_routing_strategy_category routing_category;
	::boost::variant<probabilistic_routing_strategy_config_type> routing_conf;
};


template <typename RealT, typename UIntT>
struct qn_node_config
{
	typedef RealT real_type;
	typedef UIntT uint_type;
	typedef qn_delay_node_config<real_type,uint_type> delay_node_config_type;
	typedef qn_queue_node_config<real_type,uint_type> queue_node_config_type;
	typedef qn_sink_node_config<real_type,uint_type> sink_node_config_type;
	typedef qn_source_node_config<real_type,uint_type> source_node_config_type;

	uint_type id;
	::std::string name;
	::std::string ref_tier;
	qn_node_category category;
	::boost::variant<delay_node_config_type,
					 queue_node_config_type,
					 sink_node_config_type,
					 source_node_config_type> category_conf;
};


template <typename UIntT>
struct qn_closed_class_config
{
	typedef UIntT uint_type;

	uint_type size;
};


template <typename RealT>
struct qn_open_class_config
{
	typedef RealT real_type;
	typedef probability_distribution_config<real_type> probability_distribution_config_type;

	probability_distribution_config_type distribution;
};


template <typename RealT, typename UIntT>
struct qn_customer_class_config
{
	typedef RealT real_type;
	typedef UIntT uint_type;
	typedef qn_closed_class_config<uint_type> closed_class_config_type;
	typedef qn_open_class_config<real_type> open_class_config_type;

	uint_type id;
	::std::string name;
	::std::string ref_node;
	qn_customer_class_category category;
	::boost::variant<closed_class_config_type,
					 open_class_config_type> category_conf;
};


template <typename RealT, typename UIntT>
struct qn_model_config
{
	typedef RealT real_type;
	typedef UIntT uint_type;
	typedef qn_node_config<real_type,uint_type> node_config_type;
	typedef qn_customer_class_config<real_type,uint_type> customer_class_config_type;
	typedef ::std::vector<node_config_type> node_container;
	typedef ::std::vector<customer_class_config_type> customer_class_container;

	node_container nodes;
	customer_class_container customer_classes;
};


template <typename RealT>
struct simulation_statistic_config
{
	typedef RealT real_type;
	typedef statistic_config<real_type> statistic_config_type;

	statistic_config_type statistic;
	real_type precision;
	real_type confidence_level;
};


template <typename RealT, typename UIntT>
struct application_simulation_model_config
{
	typedef RealT real_type;
	typedef UIntT uint_type;
	typedef simulation_statistic_config<real_type> statistic_config_type;
	typedef ::std::map<metric_category,statistic_config_type> statistic_container;
	typedef qn_model_config<real_type,uint_type> qn_model_config_type;

	application_simulation_model_category category;
	::boost::variant<qn_model_config_type> category_conf;
	statistic_container statistics;
};


template <typename CharT, typename CharTraitsT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, application_simulation_model_category category)
{
	switch (category)
	{
		case qn_model:
			os << "qn";
	}

	return os;
}


template <typename CharT, typename CharTraitsT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, qn_fcfs_scheduling_policy_config const& config)
{
	DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(config);

	os << "<(fcfs-scheduling-routing)>";

	return os;
}


template <typename CharT, typename CharTraitsT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, qn_lcfs_scheduling_policy_config const& config)
{
	DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(config);

	os << "<(lcfs-scheduling-routing)>";

	return os;
}


template <typename CharT, typename CharTraitsT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, qn_processor_sharing_scheduling_policy_config const& config)
{
	DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(config);

	os << "<(processor-sharing-scheduling-routing)>";

	return os;
}


template <typename CharT, typename CharTraitsT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, qn_round_robin_scheduling_policy_config const& config)
{
	DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(config);

	os << "<(round-robin-scheduling-routing)>";

	return os;
}


template <typename CharT, typename CharTraitsT, typename RealT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, qn_probabilistic_routing_strategy_config<RealT> const& routing_config)
{
	os << "<(probabilistic-routing)";
/*
	os << " probabilities: [";

	for (::std::size_t i = 0; i <  routing_config.probabilities.size(); ++i)
	{
		if (i != 0)
		{
			os << ",";
		}

		os << routing_config.probabilities[i];
	}

	os << "]";
*/
	os << " probabilities: " << routing_config.probabilities;

	os << ">";

	return os;
}


template <typename CharT, typename CharTraitsT, typename RealT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, qn_load_independent_service_strategy_config<RealT> const& service_config)
{
	os << "<(load-independent-service)";

	os << " distributions: [";
	for (::std::size_t i = 0; i < service_config.distributions.size(); ++i)
	{
		if (i != 0)
		{
			os << ", ";
		}
		os << service_config.distributions[i];
	}
	os << "]";

	os << ">";

	return os;
}


template <typename CharT, typename CharTraitsT, typename RealT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, qn_processor_sharing_service_strategy_config<RealT> const& service_config)
{
	os << "<(processor-sharing-service)";

	os << " distributions: [";
	for (::std::size_t i = 0; i < service_config.distributions.size(); ++i)
	{
		if (i != 0)
		{
			os << ", ";
		}
		os << service_config.distributions[i];
	}
	os << "]";

	os << ">";

	return os;
}


template <typename CharT, typename CharTraitsT, typename RealT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, qn_round_robin_service_strategy_config<RealT> const& service_config)
{
	os << "<(round-robin-service)";

	os << " quantum: " << service_config.quantum;
	os << ", distributions: [";
	for (::std::size_t i = 0; i < service_config.distributions.size(); ++i)
	{
		if (i != 0)
		{
			os << ", ";
		}
		os << service_config.distributions[i];
	}
	os << "]";

	os << ">";

	return os;
}


template <typename CharT, typename CharTraitsT, typename RealT, typename UIntT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, qn_delay_node_config<RealT,UIntT> const& node_config)
{
	typedef RealT real_type;
	typedef UIntT uint_type;
	typedef qn_delay_node_config<RealT,UIntT> node_config_type;

	os << "<(delay-node)";

	os << " routing-strategy: " << node_config.routing_conf;

	os << ", distributions: [";
	for (::std::size_t i = 0; i < node_config.distributions.size(); ++i)
	{
		if (i != 0)
		{
			os << ", ";
		}
		os << node_config.distributions[i];
	}
	os << "]";

	os << ">";

	return os;
}


template <typename CharT, typename CharTraitsT, typename RealT, typename UIntT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, qn_queue_node_config<RealT,UIntT> const& node_config)
{
	typedef RealT real_type;
	typedef UIntT uint_type;
	typedef qn_queue_node_config<RealT,UIntT> node_config_type;


	os << "<(queue-node)";

	os << " num-servers: " << node_config.num_servers;
	os << ", queue-policy: " << node_config.policy_conf;
//	os << ", ";
//	switch (node_config.policy_category)
//	{
//		case qn_fcfs_scheduling_policy:
//			os << "queue-policy: fcfs";
//			break;
//	}
	os << ", queue-size: ";
	if (node_config.is_infinite)
	{
		os << "+inf";
	}
	else
	{
		os << node_config.capacity;
	}

	os << " routing-strategy: " << node_config.routing_conf;
	os << " service-strategy: " << node_config.service_conf;

	os << ">";

	return os;
}


template <typename CharT, typename CharTraitsT, typename RealT, typename UIntT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, qn_sink_node_config<RealT,UIntT> const& node_config)
{
	DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( node_config );

	os << "<(sink-node)"
	   << ">";

	return os;
}


template <typename CharT, typename CharTraitsT, typename RealT, typename UIntT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, qn_source_node_config<RealT,UIntT> const& node_config)
{
	typedef RealT real_type;
	typedef UIntT uint_type;
	typedef qn_source_node_config<RealT,UIntT> node_config_type;

	os << "<(source-node)"
	   << " routing-strategy: " << node_config.routing_conf
	   << ">";

	return os;
}


template <typename CharT, typename CharTraitsT, typename RealT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, qn_open_class_config<RealT> const& class_config)
{
	os << "<(open-customer-class)"
	   << " " << class_config.distribution
	   << ">";

	return os;
}


template <typename CharT, typename CharTraitsT, typename UIntT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, qn_closed_class_config<UIntT> const& class_config)
{
	os << "<(closed-customer-class)"
	   << " size: " << class_config.size
	   << ">";

	return os;
}


template <typename CharT, typename CharTraitsT, typename RealT, typename UIntT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, qn_model_config<RealT,UIntT> const& model_config)
{
	typedef RealT real_type;
	typedef UIntT uint_type;
	typedef qn_model_config<RealT,UIntT> model_config_type;

	if (!model_config.nodes.empty())
	{
		os << ", [";
		for (::std::size_t i = 0; i < model_config.nodes.size(); ++i)
		{
			if (i > 0)
			{
				os << ", ";
			}

			typedef typename model_config_type::node_config_type node_config_type;
			node_config_type const& node = (model_config.nodes)[i];
			os << "<(node)"
			   << " id: " << node.id
			   << ", name: " << node.name
			   << ", reference-tier: " << node.ref_tier
			   << ", " << node.category_conf;
		}
		os << "]";
	}
	if (!model_config.customer_classes.empty())
	{
		os << ", [";
		for (::std::size_t i = 0; i < model_config.customer_classes.size(); ++i)
		{
			if (i > 0)
			{
				os << ", ";
			}

			typedef typename model_config_type::customer_class_config_type customer_class_config_type;
			customer_class_config_type const& cls = model_config.customer_classes[i];
			os << "<(customer-class)"
			   << " id: " << cls.id
			   << ", name: " << cls.name
			   << ", reference-node: " << cls.ref_node
			   << ", " << cls.category_conf;
			os << ">";
		}
		os << "]";
	}

	return os;
}

template <typename CharT, typename CharTraitsT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, statistic_category category)
{
	switch (category)
	{
		case mean_statistic:
			os << "mean";
			break;
	}

	return os;
}


template <typename CharT, typename CharTraitsT, typename RealT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, simulation_statistic_config<RealT> const& stat)
{
	os << "<(simulation-statistic)"
	   << " statistic: " << stat.statistic
	   << ", precision: " << stat.precision
	   << ", confidence-level: " << stat.confidence_level
	   << ">";

	return os;
}


template <typename CharT, typename CharTraitsT, typename RealT, typename UIntT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, application_simulation_model_config<RealT,UIntT> const& model)
{
	typedef application_simulation_model_config<RealT,UIntT> sim_model_type;
	typedef RealT real_type;
	typedef UIntT uint_type;

	os << "<(application_simulation_model)";

	os << " " << model.category_conf;

	// Statistics
	{
		typedef typename sim_model_type::statistic_container::const_iterator iterator;
		iterator begin_it = model.statistics.begin();
		iterator end_it = model.statistics.end();
		os << ", {";
		for (iterator it = begin_it; it != end_it; ++it)
		{
			if (it != begin_it)
			{
				os << ", ";
			}
			os << it->first << ": " << it->second;
		}
		os << "}";
	}

	os << ">";

	return os;
}

}}} // Namespace dcs::eesim::config


#endif // DCS_EESIM_CONFIG_APPLICATION_SIMULATION_MODEL_HPP
