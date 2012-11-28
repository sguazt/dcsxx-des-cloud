#ifndef DCS_DES_CLOUD_CONFIG_OPERATION_MAKE_APPLICATION_SIMULATION_MODEL_HPP
#define DCS_DES_CLOUD_CONFIG_OPERATION_MAKE_APPLICATION_SIMULATION_MODEL_HPP


#include <boost/variant.hpp>
#include <cstddef>
#include <dcs/des/model/qn/base_routing_strategy.hpp>
#include <dcs/des/model/qn/base_service_strategy.hpp>
#include <dcs/des/model/qn/closed_customer_class.hpp>
#include <dcs/des/model/qn/customer_class.hpp>
#include <dcs/des/model/qn/delay_station_node.hpp>
#include <dcs/des/model/qn/deterministic_routing_strategy.hpp>
#include <dcs/des/model/qn/fcfs_queueing_strategy.hpp>
#include <dcs/des/model/qn/lcfs_queueing_strategy.hpp>
#include <dcs/des/model/qn/load_independent_service_strategy.hpp>
#include <dcs/des/model/qn/network_node.hpp>
#include <dcs/des/model/qn/open_customer_class.hpp>
#include <dcs/des/model/qn/probabilistic_routing_strategy.hpp>
#include <dcs/des/model/qn/ps_queueing_strategy.hpp>
#include <dcs/des/model/qn/ps_service_strategy.hpp>
#include <dcs/des/model/qn/queueing_network.hpp>
#include <dcs/des/model/qn/queueing_station_node.hpp>
#include <dcs/des/model/qn/queueing_strategy.hpp>
#include <dcs/des/model/qn/rr_queueing_strategy.hpp>
#include <dcs/des/model/qn/rr_service_strategy.hpp>
#include <dcs/des/model/qn/sink_node.hpp>
#include <dcs/des/model/qn/source_node.hpp>
#include <dcs/des/cloud/base_application_simulation_model.hpp>
#include <dcs/des/cloud/config/application.hpp>
#include <dcs/des/cloud/config/configuration.hpp>
#include <dcs/des/cloud/config/operation/make_output_statistic.hpp>
#include <dcs/des/cloud/config/operation/make_probability_distribution.hpp>
#include <dcs/des/cloud/config/statistic.hpp>
#include <dcs/des/cloud/multi_tier_application.hpp>
#include <dcs/des/cloud/performance_measure_category.hpp>
#include <dcs/des/cloud/qn_application_simulation_model.hpp>
#include <dcs/des/cloud/utility.hpp>
#include <dcs/math/stats/distribution/any_distribution.hpp>
#include <dcs/memory.hpp>
#include <map>
#include <set>
#include <string>
#include <stdexcept>
#include <vector>


namespace dcs { namespace des { namespace cloud { namespace config {

template <
	typename TraitsT,
	typename RealT,
	typename UIntT
>
::dcs::shared_ptr<
	::dcs::des::cloud::base_application_simulation_model<TraitsT>
> make_application_simulation_model(application_config<RealT,UIntT> const& app_conf,
									::dcs::des::cloud::multi_tier_application<TraitsT> const& app,
									configuration<RealT,UIntT> const& conf,
									::dcs::shared_ptr<typename TraitsT::uniform_random_generator_type> const& ptr_rng,
									::dcs::shared_ptr<typename TraitsT::des_engine_type> const& ptr_engine)
{
	typedef application_simulation_model_config<RealT,UIntT> model_config_type;
	typedef TraitsT traits_type;
	typedef typename traits_type::real_type real_type;
	typedef typename traits_type::uint_type uint_type;
	typedef typename traits_type::uniform_random_generator_type urng_type;
	typedef typename traits_type::des_engine_type des_engine_type;
	typedef ::dcs::des::cloud::base_application_simulation_model<traits_type> simulation_model_type;

	::dcs::shared_ptr<simulation_model_type> ptr_model;

	model_config_type const& sim_model_conf(app_conf.sim_model);

	switch (sim_model_conf.category)
	{
		case qn_model:
			{
				typedef typename model_config_type::qn_model_config_type model_impl_config_type;
				typedef typename model_impl_config_type::node_config_type node_config_type;
				typedef typename model_impl_config_type::node_container::const_iterator node_iterator;
				typedef typename model_impl_config_type::customer_class_config_type customer_class_config_type;
				typedef typename model_impl_config_type::customer_class_container::const_iterator customer_class_iterator;
				typedef ::dcs::des::model::qn::queueing_network<uint_type,real_type,urng_type,des_engine_type> model_impl_type;
				typedef ::dcs::des::model::qn::queueing_network_traits<model_impl_type> model_impl_traits_type;
				typedef ::dcs::shared_ptr<model_impl_type> model_impl_pointer;
				//typedef ::dcs::des::cloud::application_simulation_model_adaptor<traits_type,model_impl_type> simulation_model_impl_type;
				typedef ::dcs::des::cloud::qn_application_simulation_model<traits_type,uint_type,real_type,urng_type,des_engine_type> simulation_model_impl_type;
				typedef ::dcs::des::model::qn::network_node<model_impl_traits_type> node_type;
				typedef ::std::map< ::std::string, typename node_type::identifier_type > name_to_id_container;
				typedef ::std::map<uint_type, typename node_type::identifier_type> id_to_id_container;

				model_impl_pointer ptr_model_impl(new model_impl_type(ptr_rng, ptr_engine, false));

				name_to_id_container node_names_ids;
				id_to_id_container tier_node_ids;

				model_impl_config_type const& model_impl_conf = ::boost::get<model_impl_config_type>(sim_model_conf.category_conf);

				// Network nodes
				node_iterator node_end_it = model_impl_conf.nodes.end();
				for (node_iterator node_it = model_impl_conf.nodes.begin(); node_it != node_end_it; ++node_it)
				{
					::dcs::shared_ptr<node_type> ptr_node;

					if (node_names_ids.count(node_it->name))
					{
						throw ::std::runtime_error("[dcs::des::cloud::config::detail::make_application_simulation_model] The name of a node must be unique for a given model.");
					}

					switch (node_it->category)
					{
						case qn_delay_node:
							{
								typedef ::dcs::des::model::qn::delay_station_node<model_impl_traits_type> node_impl_type;
								typedef typename node_config_type::delay_node_config_type node_config_impl_type;
								typedef ::dcs::math::stats::any_distribution<real_type> distribution_type;
								typedef ::std::vector<distribution_type> distribution_container;
								typedef ::dcs::des::model::qn::base_routing_strategy<model_impl_traits_type> routing_strategy_type;

								//::dcs::shared_ptr<node_impl_type> ptr_node_impl;
								::dcs::shared_ptr<routing_strategy_type> ptr_routing;

								node_config_impl_type const& node_conf_impl = ::boost::get<node_config_impl_type>(node_it->category_conf);

								// Think times
								::std::size_t ndistrs = node_conf_impl.distributions.size();
								distribution_container distrs;
								for (::std::size_t i = 0; i < ndistrs; ++i)
								{
									//::dcs::shared_ptr<distribution_type> ptr_distr;
									distribution_type distr;

									distr = make_probability_distribution<traits_type>(node_conf_impl.distributions[i]);

									distrs.push_back(distr);
								}

								// Routing strategy
								switch (node_conf_impl.routing_category)
								{
									case qn_deterministic_routing_strategy:
										{
											typedef typename node_config_impl_type::deterministic_routing_strategy_config_type routing_config_impl_type;
											typedef ::dcs::des::model::qn::deterministic_routing_strategy<model_impl_traits_type> routing_impl_type;
											typedef typename routing_config_impl_type::destination_container destination_container;
											typedef typename destination_container::const_iterator iterator;

											::dcs::shared_ptr<routing_impl_type> ptr_routing_impl;
											ptr_routing_impl = ::dcs::make_shared<routing_impl_type>();

											routing_config_impl_type const& routing_conf_impl = ::boost::get<routing_config_impl_type>(node_conf_impl.routing_conf);
											iterator route_end_it = routing_conf_impl.destinations.end();
											for (iterator route_it = routing_conf_impl.destinations.begin(); route_it != route_end_it; ++route_it)
											{
												ptr_routing_impl->add_route(node_it->id,
																			(route_it.index())[0],
																			*route_it,
																			(route_it.index())[1]);
											}
											ptr_routing = ptr_routing_impl;
										}
										break;
									case qn_probabilistic_routing_strategy:
										{
											typedef typename node_config_impl_type::probabilistic_routing_strategy_config_type routing_config_impl_type;
											typedef ::dcs::des::model::qn::probabilistic_routing_strategy<model_impl_traits_type> routing_impl_type;
											typedef typename routing_config_impl_type::probability_container probability_container;
											typedef typename probability_container::const_iterator iterator;

											::dcs::shared_ptr<routing_impl_type> ptr_routing_impl;
											ptr_routing_impl = ::dcs::make_shared<routing_impl_type>(ptr_rng);

											routing_config_impl_type const& routing_conf_impl = ::boost::get<routing_config_impl_type>(node_conf_impl.routing_conf);
											iterator route_end_it = routing_conf_impl.probabilities.end();
											for (iterator route_it = routing_conf_impl.probabilities.begin(); route_it != route_end_it; ++route_it)
											{
												ptr_routing_impl->add_route(node_it->id,
																			(route_it.index())[0],
																			(route_it.index())[2],
																			(route_it.index())[1],
																			*route_it);
											}
											ptr_routing = ptr_routing_impl;
										}
										break;
								}

								ptr_node = ::dcs::make_shared<node_impl_type>(node_it->id, node_it->name, distrs.begin(), distrs.end(), ptr_routing);
							}
							break;
						case qn_queue_node:
							{
								typedef ::dcs::des::model::qn::queueing_station_node<model_impl_traits_type> node_impl_type;
								typedef typename node_config_type::queue_node_config_type node_config_impl_type;
								typedef ::dcs::des::model::qn::base_service_strategy<model_impl_traits_type> service_strategy_type;
								typedef ::dcs::des::model::qn::base_routing_strategy<model_impl_traits_type> routing_strategy_type;
								typedef ::dcs::des::model::qn::queueing_strategy<model_impl_traits_type> queueing_strategy_type;

								//::dcs::shared_ptr<node_impl_type> ptr_node_impl;
								::dcs::shared_ptr<service_strategy_type> ptr_service;
								::dcs::shared_ptr<routing_strategy_type> ptr_routing;
								::dcs::shared_ptr<queueing_strategy_type> ptr_queueing;

								node_config_impl_type const& node_conf_impl = ::boost::get<node_config_impl_type>(node_it->category_conf);

								// Check for conf consistency
								if (node_conf_impl.service_category == qn_infinite_server_service_strategy)
								{
									throw ::std::runtime_error("[dcs::des::cloud::config::detail::make_application_simulation_model] Service strategy for a queueing station node cannot be of type 'infinite-server'.");
								}
								if (node_conf_impl.service_category != qn_processor_sharing_service_strategy
									&&
									node_conf_impl.policy_category == qn_processor_sharing_scheduling_policy)
								{
									throw ::std::runtime_error("[dcs::des::cloud::config::detail::make_application_simulation_model] Processor-Sharing scheduling policy can only be used with Processor-Sharing service strategy.");
								}
								if (node_conf_impl.service_category != qn_round_robin_service_strategy
									&&
									node_conf_impl.policy_category == qn_round_robin_scheduling_policy)
								{
										throw ::std::runtime_error("[dcs::des::cloud::config::detail::make_application_simulation_model] Round-Robin scheduling policy can only be used with Round-Robin service strategy.");
								}

								// Service strategy
								switch (node_conf_impl.service_category)
								{
									case qn_infinite_server_service_strategy:
										throw ::std::runtime_error("[dcs::des::cloud::config::detail::make_application_simulation_model] Service strategy for a queueing station node cannot be of type 'infinite-server'.");
									case qn_load_independent_service_strategy:
										{
											typedef typename node_config_impl_type::load_independent_service_strategy_config_type service_config_type;
											typedef typename service_config_type::probability_distribution_config_type distribution_config_type;
											//typedef ::dcs::math::stats::base_distribution<real_type> distribution_type;
											typedef ::dcs::math::stats::any_distribution<real_type> distribution_type;
											typedef ::std::vector<distribution_type> distribution_container;
											typedef ::dcs::des::model::qn::load_independent_service_strategy<model_impl_traits_type> service_strategy_impl_type;

											service_config_type const& service_conf = ::boost::get<service_config_type>(node_conf_impl.service_conf);

											::std::size_t ndistrs = service_conf.distributions.size();
											distribution_container distrs;
											for (::std::size_t i = 0; i < ndistrs; ++i)
											{
												//::dcs::shared_ptr<distribution_type> ptr_distr;
												distribution_type distr;

												distr = make_probability_distribution<traits_type>(service_conf.distributions[i]);

												distrs.push_back(distr);
											}
											ptr_service = ::dcs::make_shared<service_strategy_impl_type>(distrs.begin(), distrs.end());
										}
										break;
									case qn_processor_sharing_service_strategy:
										{
											typedef typename node_config_impl_type::processor_sharing_service_strategy_config_type service_config_type;
											typedef typename service_config_type::probability_distribution_config_type distribution_config_type;
											//typedef ::dcs::math::stats::base_distribution<real_type> distribution_type;
											typedef ::dcs::math::stats::any_distribution<real_type> distribution_type;
											typedef ::std::vector<distribution_type> distribution_container;
											typedef ::dcs::des::model::qn::ps_service_strategy<model_impl_traits_type> service_strategy_impl_type;

											if (node_conf_impl.policy_category != qn_processor_sharing_scheduling_policy)
											{
												throw ::std::runtime_error("[dcs::des::cloud::config::detail::make_application_simulation_model] Processor-Sharing service strategy needs Processor-Sharing scheduling policy.");
											}

											service_config_type const& service_conf = ::boost::get<service_config_type>(node_conf_impl.service_conf);

											::std::size_t ndistrs = service_conf.distributions.size();
											distribution_container distrs;
											for (::std::size_t i = 0; i < ndistrs; ++i)
											{
												distribution_type distr;

												distr = make_probability_distribution<traits_type>(service_conf.distributions[i]);

												distrs.push_back(distr);
											}
											ptr_service = ::dcs::make_shared<service_strategy_impl_type>(distrs.begin(), distrs.end());
										}
										break;
									case qn_round_robin_service_strategy:
										{
											typedef typename node_config_impl_type::round_robin_service_strategy_config_type service_config_type;
											typedef typename service_config_type::probability_distribution_config_type distribution_config_type;
											//typedef ::dcs::math::stats::base_distribution<real_type> distribution_type;
											typedef ::dcs::math::stats::any_distribution<real_type> distribution_type;
											typedef ::std::vector<distribution_type> distribution_container;
											typedef ::dcs::des::model::qn::rr_service_strategy<model_impl_traits_type> service_strategy_impl_type;

											if (node_conf_impl.policy_category != qn_round_robin_scheduling_policy)
											{
												throw ::std::runtime_error("[dcs::des::cloud::config::detail::make_application_simulation_model] Round-Robin service strategy needs Round-Robin scheduling policy.");
											}

											service_config_type const& service_conf = ::boost::get<service_config_type>(node_conf_impl.service_conf);

											::std::size_t ndistrs = service_conf.distributions.size();
											distribution_container distrs;
											for (::std::size_t i = 0; i < ndistrs; ++i)
											{
												distribution_type distr;

												distr = make_probability_distribution<traits_type>(service_conf.distributions[i]);

												distrs.push_back(distr);
											}
											ptr_service = ::dcs::make_shared<service_strategy_impl_type>(service_conf.quantum,
																										 distrs.begin(),
																										 distrs.end());
										}
										break;
								}

								// Routing strategy
								switch (node_conf_impl.routing_category)
								{
									case qn_deterministic_routing_strategy:
										{
											typedef typename node_config_impl_type::deterministic_routing_strategy_config_type routing_config_impl_type;
											typedef ::dcs::des::model::qn::deterministic_routing_strategy<model_impl_traits_type> routing_impl_type;
											typedef typename routing_config_impl_type::destination_container destination_container;
											typedef typename destination_container::const_iterator iterator;

											::dcs::shared_ptr<routing_impl_type> ptr_routing_impl;
											ptr_routing_impl = ::dcs::make_shared<routing_impl_type>();

											routing_config_impl_type const& routing_conf_impl = ::boost::get<routing_config_impl_type>(node_conf_impl.routing_conf);
											iterator route_end_it = routing_conf_impl.destinations.end();
											for (iterator route_it = routing_conf_impl.destinations.begin(); route_it != route_end_it; ++route_it)
											{
												ptr_routing_impl->add_route(node_it->id,
																			(route_it.index())[0],
																			*route_it,
																			(route_it.index())[1]);
											}
											ptr_routing = ptr_routing_impl;
										}
										break;
									case qn_probabilistic_routing_strategy:
										{
											typedef typename node_config_impl_type::probabilistic_routing_strategy_config_type routing_config_impl_type;
											typedef ::dcs::des::model::qn::probabilistic_routing_strategy<model_impl_traits_type> routing_impl_type;
											typedef typename routing_config_impl_type::probability_container probability_container;
											typedef typename probability_container::const_iterator iterator;

											::dcs::shared_ptr<routing_impl_type> ptr_routing_impl;
											ptr_routing_impl = ::dcs::make_shared<routing_impl_type>(ptr_rng);

											routing_config_impl_type const& routing_conf_impl = ::boost::get<routing_config_impl_type>(node_conf_impl.routing_conf);
											iterator route_end_it = routing_conf_impl.probabilities.end();
											for (iterator route_it = routing_conf_impl.probabilities.begin(); route_it != route_end_it; ++route_it)
											{
												ptr_routing_impl->add_route(node_it->id,
																			(route_it.index())[0],
																			(route_it.index())[2],
																			(route_it.index())[1],
																			*route_it);
											}
											ptr_routing = ptr_routing_impl;
										}
										break;
								}

								// Queueing strategy
								switch (node_conf_impl.policy_category)
								{
									case qn_fcfs_scheduling_policy:
										{
											typedef ::dcs::des::model::qn::fcfs_queueing_strategy<model_impl_traits_type> queueing_impl_type;

											if (node_conf_impl.is_infinite)
											{
												ptr_queueing = ::dcs::make_shared<queueing_impl_type>();
											}
											else
											{
												ptr_queueing = ::dcs::make_shared<queueing_impl_type>(node_conf_impl.capacity);
											}
										}
										break;
									case qn_lcfs_scheduling_policy:
										{
											typedef ::dcs::des::model::qn::lcfs_queueing_strategy<model_impl_traits_type> queueing_impl_type;

											if (node_conf_impl.is_infinite)
											{
												ptr_queueing = ::dcs::make_shared<queueing_impl_type>();
											}
											else
											{
												ptr_queueing = ::dcs::make_shared<queueing_impl_type>(node_conf_impl.capacity);
											}
										}
										break;
									case qn_processor_sharing_scheduling_policy:
										{
											typedef ::dcs::des::model::qn::ps_queueing_strategy<model_impl_traits_type> queueing_impl_type;

											if (node_conf_impl.is_infinite)
											{
												ptr_queueing = ::dcs::make_shared<queueing_impl_type>();
											}
											else
											{
												throw ::std::runtime_error("[dcs::des::cloud::config::detail::make_application_simulation_model] Finite capacity queues with Processor-Sharing policy are not handled yet.");
											}
										}
										break;
									case qn_round_robin_scheduling_policy:
										{
											typedef ::dcs::des::model::qn::rr_queueing_strategy<model_impl_traits_type> queueing_impl_type;

											if (node_conf_impl.is_infinite)
											{
												ptr_queueing = ::dcs::make_shared<queueing_impl_type>();
											}
											else
											{
												throw ::std::runtime_error("[dcs::des::cloud::config::detail::make_application_simulation_model] Finite capacity queues with Round-Robin policy are not handled yet.");
											}
										}
										break;
								}

								ptr_node = ::dcs::make_shared<node_impl_type>(node_it->id, node_it->name, ptr_queueing, ptr_service, ptr_routing);
							}
							break;
						case qn_sink_node:
							{
								typedef ::dcs::des::model::qn::sink_node<model_impl_traits_type> node_impl_type;
								typedef typename node_config_type::sink_node_config_type node_config_impl_type;

								//node_config_impl_type const& node_conf_impl = ::boost::get<node_config_impl_type>(node_it->category_conf);

								ptr_node = ::dcs::make_shared<node_impl_type>(node_it->id, node_it->name);
							}
							break;
						case qn_source_node:
							{
								typedef ::dcs::des::model::qn::source_node<model_impl_traits_type> node_impl_type;
								typedef typename node_config_type::source_node_config_type node_config_impl_type;
								typedef ::dcs::des::model::qn::base_routing_strategy<model_impl_traits_type> routing_strategy_type;

								node_config_impl_type const& node_conf_impl = ::boost::get<node_config_impl_type>(node_it->category_conf);

								::dcs::shared_ptr<routing_strategy_type> ptr_routing;

								// Routing strategy
								switch (node_conf_impl.routing_category)
								{
									case qn_deterministic_routing_strategy:
										{
											typedef typename node_config_impl_type::deterministic_routing_strategy_config_type routing_config_impl_type;
											typedef ::dcs::des::model::qn::deterministic_routing_strategy<model_impl_traits_type> routing_impl_type;
											typedef typename routing_config_impl_type::destination_container destination_container;
											typedef typename destination_container::const_iterator iterator;

											::dcs::shared_ptr<routing_impl_type> ptr_routing_impl;
											ptr_routing_impl = ::dcs::make_shared<routing_impl_type>();

											routing_config_impl_type const& routing_conf_impl = ::boost::get<routing_config_impl_type>(node_conf_impl.routing_conf);
											iterator route_end_it = routing_conf_impl.destinations.end();
											for (iterator route_it = routing_conf_impl.destinations.begin(); route_it != route_end_it; ++route_it)
											{
												ptr_routing_impl->add_route(node_it->id,
																			(route_it.index())[0],
																			*route_it,
																			(route_it.index())[1]);
											}
											ptr_routing = ptr_routing_impl;
										}
										break;
									case qn_probabilistic_routing_strategy:
										{
											typedef typename node_config_impl_type::probabilistic_routing_strategy_config_type routing_config_impl_type;
											typedef ::dcs::des::model::qn::probabilistic_routing_strategy<model_impl_traits_type> routing_impl_type;
											typedef typename routing_config_impl_type::probability_container probability_container;
											typedef typename probability_container::const_iterator iterator;

											::dcs::shared_ptr<routing_impl_type> ptr_routing_impl;
											ptr_routing_impl = ::dcs::make_shared<routing_impl_type>(ptr_rng);

											routing_config_impl_type const& routing_conf_impl = ::boost::get<routing_config_impl_type>(node_conf_impl.routing_conf);
											iterator route_end_it = routing_conf_impl.probabilities.end();
											for (iterator route_it = routing_conf_impl.probabilities.begin(); route_it != route_end_it; ++route_it)
											{
												ptr_routing_impl->add_route(node_it->id,
																			(route_it.index())[0],
																			(route_it.index())[2],
																			(route_it.index())[1],
																			*route_it);
											}
											ptr_routing = ptr_routing_impl;
										}
										break;
								}

								ptr_node = ::dcs::make_shared<node_impl_type>(node_it->id, node_it->name, ptr_routing);
							}
							break;
					}

					node_names_ids[ptr_node->name()] = ptr_node->id();
					if (!(node_it->ref_tier.empty()))
					{
						for (::std::size_t tier_id = 0; tier_id < app.num_tiers(); ++tier_id)
						{
							if (!app.tier(tier_id)->name().compare(node_it->ref_tier))
							{
								tier_node_ids[tier_id] = ptr_node->id();
								break;
							}
						}
					}

					ptr_model_impl->add_node(ptr_node);
				}

				// Customer classes
				customer_class_iterator customer_class_end_it = model_impl_conf.customer_classes.end();
				for (customer_class_iterator class_it = model_impl_conf.customer_classes.begin(); class_it != customer_class_end_it; ++class_it)
				{
					typedef ::dcs::des::model::qn::customer_class<model_impl_traits_type> customer_class_type;

					::dcs::shared_ptr<customer_class_type> ptr_customer_class;

					switch (class_it->category)
					{
						case qn_open_customer_class:
							{
								typedef typename customer_class_config_type::open_class_config_type customer_class_config_impl_type;
								typedef ::dcs::des::model::qn::open_customer_class<model_impl_traits_type> customer_class_impl_type;
								typedef ::dcs::math::stats::any_distribution<real_type> distribution_type;

								customer_class_config_impl_type const& customer_class_impl_conf = ::boost::get<customer_class_config_impl_type>(class_it->category_conf);

								distribution_type distr;
								distr = make_probability_distribution<traits_type>(customer_class_impl_conf.distribution);
								ptr_customer_class = ::dcs::make_shared<customer_class_impl_type>(class_it->id, class_it->name, distr);
							}
							break;
						case qn_closed_customer_class:
							{
								typedef typename customer_class_config_type::closed_class_config_type customer_class_config_impl_type;
								typedef ::dcs::des::model::qn::closed_customer_class<model_impl_traits_type> customer_class_impl_type;

								customer_class_config_impl_type const& customer_class_impl_conf = ::boost::get<customer_class_config_impl_type>(class_it->category_conf);

								ptr_customer_class = ::dcs::make_shared<customer_class_impl_type>(class_it->id, class_it->name, customer_class_impl_conf.size);
							}
							break;
					}

					ptr_customer_class->reference_node(node_names_ids[class_it->ref_node]);

					ptr_model_impl->add_class(ptr_customer_class);
				}

				::dcs::shared_ptr<simulation_model_impl_type> ptr_sim_model_impl;

				ptr_sim_model_impl = ::dcs::make_shared<simulation_model_impl_type>(ptr_model_impl);

				// Register tier mappings
				typename id_to_id_container::const_iterator tier_node_end_it = tier_node_ids.end();
				for (typename id_to_id_container::const_iterator it = tier_node_ids.begin();
					 it != tier_node_end_it;
					 ++it)
				{
					ptr_sim_model_impl->tier_node_mapping(it->first, it->second);
				}

				ptr_model = ptr_sim_model_impl;
			}
			break;
	}

	// Statistics
/*
	{
		typedef typename model_config_type::statistic_container statistic_container;
		typedef typename statistic_container::const_iterator iterator;

		iterator end_it = sim_model_conf.statistics.end();
		for (iterator it = sim_model_conf.statistics.begin(); it != end_it; ++it)
		{
			performance_measure_category category;
			category = make_performance_measure_category(it->first);
			ptr_model->statistic(
					category,
					make_output_statistic<traits_type>(it->second, conf.simulation(), conf, sim_info, true, true)
				);

			for (::std::size_t tier_id = 0; tier_id < app.num_tiers(); ++tier_id)
			{
				ptr_model->tier_statistic(
						tier_id,
						category,
						make_output_statistic<traits_type>(it->second, conf.simulation(), conf, sim_info, true, false)
					);
			}
		}
	}
*/
	{
        ::std::vector<performance_measure_category> perf_metrics(performance_measure_categories());
        ::std::vector<performance_measure_category> sla_perf_metrics_vec(app.sla_cost_model().slo_categories());
        ::std::set<performance_measure_category> sla_perf_metrics(sla_perf_metrics_vec.begin(), sla_perf_metrics_vec.end());
		sla_perf_metrics_vec.clear();
        typedef typename ::std::vector<performance_measure_category>::const_iterator iterator;
        iterator end_it = perf_metrics.end();
        for (iterator it = perf_metrics.begin(); it != end_it; ++it)
		{
			performance_measure_category category(*it);
			bool analyzable(false);

			if (sla_perf_metrics.count(category) > 0)
			{
				analyzable = true;
			}
//			bool analyzable(true);

//FIXME: actually we create statistics for all possible performance metrics.
//Since the kind of statistic (mean, quantile,...) is currently specifiable only for
//SLA statistic we need to do the trick below: for SLA metric use the type of statistic
//specified in the configuration file, while for the remaning ones use the mean estimator.
//This needs to be fixed later, e.g. by letting the user to specify in the conf file what
//kind of metrics is interested in, its associated statistic type and what of them are to
//be treated as SLA metrics.
			statistic_config<RealT> stat_conf;
			if (app_conf.sla.metrics.count(to_metric_category(category)) > 0)
			{
				stat_conf = app_conf.sla.metrics.at(to_metric_category(category)).statistic;
			}
			else
			{
				stat_conf.category = mean_statistic;
				stat_conf.category_conf = mean_statistic_config();
			}

			if (::dcs::des::cloud::for_application(category))
			{
				ptr_model->statistic(
						category,
						make_output_statistic<traits_type>(
								//::dcs::des::mean_statistic,//FIXME: statistic category is hard-coded
								//app_conf.sla.metrics.at(to_metric_category(category)).statistic,
								stat_conf,
								conf.simulation(),
								//ptr_rng,
								ptr_engine,
								analyzable,
								true
							)
					);
			}

			if (::dcs::des::cloud::for_application_tier(category))
			{
				::std::size_t num_tiers = app.num_tiers();
				for (::std::size_t tier_id = 0; tier_id < num_tiers; ++tier_id)
				{
					ptr_model->tier_statistic(
							tier_id,
							category,
							make_output_statistic<traits_type>(
									//::dcs::des::mean_statistic,//FIXME: statistic category is hard-coded
									//app_conf.sla.metrics.at(to_metric_category(category)).statistic,
									stat_conf,
									conf.simulation(),
									//ptr_rng,
									ptr_engine,
									analyzable,
									false
								)
						);
				}
			}
		}
	}

	return ptr_model;
}

}}}} // Namespace dcs::des::cloud::config


#endif // DCS_DES_CLOUD_CONFIG_OPERATION_MAKE_APPLICATION_SIMULATION_MODEL_HPP
