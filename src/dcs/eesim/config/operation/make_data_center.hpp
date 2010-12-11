#ifndef DCS_EESIM_CONFIG_OPERATION_MAKE_DATA_CENTER_HPP
#define DCS_EESIM_CONFIG_OPERATION_MAKE_DATA_CENTER_HPP


#include <algorithm>
//#include <boost/numeric/ublas/io.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/matrix_proxy.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <boost/variant.hpp>
#include <cstddef>
#include <dcs/des/model/qn/closed_customer_class.hpp>
#include <dcs/des/model/qn/customer_class.hpp>
#include <dcs/des/model/qn/delay_station_node.hpp>
#include <dcs/des/model/qn/fcfs_queueing_strategy.hpp>
#include <dcs/des/model/qn/load_independent_service_strategy.hpp>
#include <dcs/des/model/qn/network_node.hpp>
#include <dcs/des/model/qn/open_customer_class.hpp>
#include <dcs/des/model/qn/probabilistic_routing_strategy.hpp>
#include <dcs/des/model/qn/queueing_network.hpp>
#include <dcs/des/model/qn/queueing_station_node.hpp>
#include <dcs/des/model/qn/queueing_strategy.hpp>
#include <dcs/des/model/qn/routing_strategy.hpp>
#include <dcs/des/model/qn/service_strategy.hpp>
#include <dcs/des/model/qn/sink_node.hpp>
#include <dcs/des/model/qn/source_node.hpp>
#include <dcs/eesim/application_performance_model_adaptor.hpp>
#include <dcs/eesim/application_performance_model_traits.hpp>
#include <dcs/eesim/application_simulation_model_adaptor.hpp>
#include <dcs/eesim/application_simulation_model_traits.hpp>
#include <dcs/eesim/application_tier.hpp>
#include <dcs/eesim/base_application_controller.hpp>
#include <dcs/eesim/base_application_performance_model.hpp>
#include <dcs/eesim/base_application_simulation_model.hpp>
#include <dcs/eesim/base_physical_machine_controller.hpp>
#include <dcs/eesim/config/application.hpp>
#include <dcs/eesim/config/application_controller.hpp>
#include <dcs/eesim/config/application_performance_model.hpp>
#include <dcs/eesim/config/application_simulation_model.hpp>
#include <dcs/eesim/config/application_sla.hpp>
#include <dcs/eesim/config/application_tier.hpp>
#include <dcs/eesim/config/configuration.hpp>
#include <dcs/eesim/config/physical_machine.hpp>
#include <dcs/eesim/config/physical_resource.hpp>
#include <dcs/eesim/config/probability_distribution.hpp>
#include <dcs/eesim/conservative_physical_machine_controller.hpp>
#include <dcs/eesim/data_center.hpp>
#include <dcs/eesim/dummy_application_controller.hpp>
#include <dcs/eesim/lqr_application_controller.hpp>
#include <dcs/eesim/multi_tier_application.hpp>
#include <dcs/eesim/open_multi_bcmp_qn_application_performance_model.hpp>
#include <dcs/eesim/physical_machine.hpp>
#include <dcs/eesim/physical_resource.hpp>
#include <dcs/eesim/physical_resource_category.hpp>
#include <dcs/eesim/proportional_physical_machine_controller.hpp>
#include <dcs/eesim/qn_application_simulation_model.hpp>
#include <dcs/math/stats/distributions.hpp>
#include <dcs/memory.hpp>
#include <dcs/perfeval/energy.hpp>
#include <dcs/perfeval/qn/open_multi_bcmp_network.hpp>
#include <dcs/perfeval/qn/operation/visit_ratios.hpp>
#include <map>
#include <stdexcept>
#include <vector>


namespace dcs { namespace eesim { namespace config {

namespace detail { namespace /*<unnamed>*/ {

template <typename TraitsT>
struct simulation_info
{
	typedef TraitsT traits_type;
	typedef typename traits_type::uniform_random_generator_type uniform_random_generator_type;
	typedef typename traits_type::des_engine_type des_engine_type;

	::dcs::shared_ptr<uniform_random_generator_type> ptr_rng;
	::dcs::shared_ptr<des_engine_type> ptr_engine;
};


template <typename T>
::boost::numeric::ublas::matrix<T> make_ublas_matrix(numeric_matrix<T> const& m)
{
	typedef typename numeric_matrix<T>::size_type size_type;

	size_type nr = m.num_rows();
	size_type nc = m.num_columns();

	::boost::numeric::ublas::matrix<T> res(nr, nc);

	for (size_type r = 0; r < nr; ++r)
	{
		for (size_type c = 0; c < nc; ++c)
		{
			res(r,c) = m(r,c);
		}
	}

	return res;
}


::dcs::eesim::physical_resource_category make_physical_resource_category(::dcs::eesim::config::physical_resource_category category)
{
	switch (category)
	{
		case cpu_resource:
			return ::dcs::eesim::cpu_resource_category;
			break;
		case mem_resource:
			return ::dcs::eesim::memory_resource_category;
			break;
		case disk_resource:
			return ::dcs::eesim::storage_resource_category;
			break;
		case nic_resource:
//			return ::dcs::eesim::network_resource_category;
//			return ::dcs::eesim::network_up_resource_category;
//			return ::dcs::eesim::network_down_resource_category;
			throw ::std::runtime_error("NIC resource not yet handled.");
			break;
	}
}


template <typename RealT, typename VariantT>
::dcs::shared_ptr< ::dcs::perfeval::energy::base_model<RealT> > make_energy_model(energy_model_category category, VariantT const& variant_config)
{
	typedef ::dcs::perfeval::energy::base_model<RealT> base_model_type;

	::dcs::shared_ptr<base_model_type> ptr_model;

	switch (category)
	{
		case constant_energy_model:
			{
				typedef constant_energy_model_config<RealT> model_config_type;
				typedef ::dcs::perfeval::energy::constant_model<RealT> model_type;
				model_config_type const& model = ::boost::get<model_config_type>(variant_config);
				ptr_model = ::dcs::make_shared<model_type>(model.c0);
			}
			break;
		case fan2007_energy_model:
			{
				typedef fan2007_energy_model_config<RealT> model_config_type;
				typedef ::dcs::perfeval::energy::fan2007_model<RealT> model_type;
				model_config_type const& model = ::boost::get<model_config_type>(variant_config);
				ptr_model = ::dcs::make_shared<model_type>(
							model.c0,
							model.c1,
							model.c2,
							model.r
					);
			}
			break;
	}

	return ptr_model;
}


template <typename TraitsT, typename RealT>
::dcs::shared_ptr< ::dcs::eesim::physical_resource<TraitsT> > make_physical_resource(physical_resource_config<RealT> const& resource_conf)
{
	typedef ::dcs::eesim::physical_resource<TraitsT> physical_resource_type;
	typedef physical_resource_config<RealT> physical_resource_config_type;

	::dcs::shared_ptr<physical_resource_type> ptr_res;

	ptr_res = ::dcs::make_shared<physical_resource_type>();
	if (!resource_conf.name.empty())
	{
		ptr_res->name(resource_conf.name);
	}
	ptr_res->category(
		make_physical_resource_category(resource_conf.type)
	);
	ptr_res->capacity(resource_conf.capacity);
	ptr_res->utilization_threshold(resource_conf.threshold);
	ptr_res->energy_model(
		make_energy_model<RealT>(
			resource_conf.energy_model_type,
			resource_conf.energy_model_conf
		)
	);

	return ptr_res;
}


template <typename TraitsT, typename RealT>
::dcs::shared_ptr< ::dcs::eesim::physical_machine<TraitsT> > make_physical_machine(physical_machine_config<RealT> const& machine_conf)
{
	typedef ::dcs::eesim::physical_machine<TraitsT> physical_machine_type;
	typedef physical_machine_config<RealT> physical_machine_config_type;

	::dcs::shared_ptr<physical_machine_type> ptr_mach;

	ptr_mach = ::dcs::make_shared<physical_machine_type>();
	if (!machine_conf.name.empty())
	{
		ptr_mach->name(machine_conf.name);
	}

	typedef typename physical_machine_config_type::resource_container::const_iterator iterator;
	iterator end_it = machine_conf.resources.end();
	for (iterator it = machine_conf.resources.begin(); it != end_it; ++it)
	{
		ptr_mach->add_resource(
			make_physical_resource<TraitsT>(it->second)
		);
	}

	return ptr_mach;
}


template <typename TraitsT, typename RealT>
::dcs::shared_ptr< ::dcs::eesim::base_physical_machine_controller<TraitsT> > make_physical_machine_controller(physical_machine_controller_config<RealT> const& controller_conf)
{
	typedef TraitsT traits_type;
	typedef ::dcs::eesim::base_physical_machine_controller<traits_type> controller_type;
	typedef physical_machine_controller_config<RealT> controller_config_type;

	::dcs::shared_ptr<controller_type> ptr_controller;

	switch (controller_conf.category)
	{
		case conservative_physical_machine_controller:
			{
				//typedef typename controller_config_type::conservative_controller_config_type controller_config_impl_type;
				typedef ::dcs::eesim::conservative_physical_machine_controller<traits_type> controller_impl_type;

				//controller_config_impl_type const& controller_conf_impl = ::boost::get<controller_config_impl_type>(controller_conf.category_conf);

				// Note: there is nothing to configure

				ptr_controller = ::dcs::make_shared<controller_impl_type>();
			}
			break;
		case proportional_physical_machine_controller:
			{
				//typedef typename controller_config_type::proportional_controller_config_type controller_config_impl_type;
				typedef ::dcs::eesim::proportional_physical_machine_controller<traits_type> controller_impl_type;

				//controller_config_impl_type const& controller_conf_impl = ::boost::get<controller_config_impl_type>(controller_conf.category_conf);

				// Note: there is nothing to configure

				ptr_controller = ::dcs::make_shared<controller_impl_type>();
			}
			break;
	}

	ptr_controller->sampling_time(controller_conf.sampling_time);

	return ptr_controller;
}


template <typename TraitsT, typename RealT, typename UIntT>
::dcs::shared_ptr< ::dcs::eesim::base_application_performance_model<TraitsT> > make_application_performance_model(application_performance_model_config<RealT,UIntT> const& perf_model_conf)
{
	typedef typename TraitsT::real_type real_type;
	typedef typename TraitsT::uint_type uint_type;
	typedef base_application_performance_model<TraitsT> performance_model_type;
	typedef application_performance_model_config<RealT,UIntT> model_config_type;

	::dcs::shared_ptr<performance_model_type> ptr_model;

	switch (perf_model_conf.type)
	{
		case open_multi_bcmp_qn_model:
			{
				typedef typename model_config_type::open_multi_bcmp_qn_config model_impl_config_type;
				typedef ::dcs::perfeval::qn::open_multi_bcmp_network<real_type, uint_type> model_impl_type;
				typedef typename ::dcs::eesim::application_performance_model_adaptor<
							TraitsT,
							model_impl_type
						> model_type;

				model_impl_config_type const& model_impl_conf = ::boost::get<model_impl_config_type>(perf_model_conf.type_conf);

				::boost::numeric::ublas::vector<real_type> lambda(model_impl_conf.arrival_rates.size());
				::std::copy(model_impl_conf.arrival_rates.begin(),
							model_impl_conf.arrival_rates.end(),
							lambda.begin());

				::boost::numeric::ublas::matrix<real_type> S(
						model_impl_conf.service_times.num_rows(),
						model_impl_conf.service_times.num_columns()
					);
				for (::std::size_t r = 0; r < S.size1(); ++r)
				{
					for (::std::size_t c = 0; c < S.size2(); ++c)
					{
						S(r,c) = model_impl_conf.service_times(r,c);
					}
				}

				::boost::numeric::ublas::matrix<real_type> V;
				if (!model_impl_conf.visit_ratios.empty())
				{
					V.resize(model_impl_conf.visit_ratios.num_rows(),
							 model_impl_conf.visit_ratios.num_columns(),
							 false);
					for (::std::size_t r = 0; r < V.size1(); ++r)
					{
						for (::std::size_t c = 0; c < V.size2(); ++c)
						{
							V(r,c) = model_impl_conf.visit_ratios(r,c);
						}
					}
				}
				else
				{
					::boost::numeric::ublas::matrix<real_type> P(
						model_impl_conf.routing_probabilities.num_rows(),
						model_impl_conf.routing_probabilities.num_columns())
					;
					for (::std::size_t r = 0; r < P.size1(); ++r)
					{
						for (::std::size_t c = 0; c < P.size2(); ++c)
						{
							P(r,c) = model_impl_conf.routing_probabilities(r,c);
						}
					}
					::boost::numeric::ublas::vector<real_type> v;
					v = ::dcs::perfeval::qn::visit_ratios(P, lambda);
					::boost::numeric::ublas::column(V, 0) = v;
				}

				::boost::numeric::ublas::vector<real_type> m(
						model_impl_conf.num_servers.size()
					);
				::std::copy(model_impl_conf.num_servers.begin(),
							model_impl_conf.num_servers.end(),
							m.begin());

				ptr_model = ::dcs::make_shared<model_type>(
					model_impl_type(lambda, S, V, m)
				);
			}
			break;
	}

	return ptr_model;
}


template <typename RealT>
::dcs::math::stats::any_distribution<RealT> make_probability_distribution(probability_distribution_config<RealT> const& distr_conf)
{
	typedef RealT real_type;
	typedef probability_distribution_config<RealT> distribution_config_type;
	typedef ::dcs::math::stats::any_distribution<real_type> distribution_type;

	distribution_type distr;

	switch (distr_conf.category)
	{
		case exponential_probability_distribution:
			{
				typedef typename distribution_config_type::exponential_distribution_config_type distribution_config_impl_type;
				typedef ::dcs::math::stats::exponential_distribution<real_type> distribution_impl_type;

				distribution_config_impl_type const& distr_conf_impl = ::boost::get<distribution_config_impl_type>(distr_conf.category_conf);
				//ptr_distr = ::dcs::make_shared<distribution_impl_type>(distr_conf.rate);
				distr = ::dcs::math::stats::make_any_distribution(distribution_impl_type(distr_conf_impl.rate));
			}
			break;
		case gamma_probability_distribution:
			{
				typedef typename distribution_config_type::gamma_distribution_config_type distribution_config_impl_type;
				typedef ::dcs::math::stats::gamma_distribution<real_type> distribution_impl_type;

				distribution_config_impl_type const& distr_conf_impl = ::boost::get<distribution_config_impl_type>(distr_conf.category_conf);
				//ptr_distr = ::dcs::make_shared<distribution_impl_type>(distr_conf.shape, distr_conf.scale);
				distr = ::dcs::math::stats::make_any_distribution(distribution_impl_type(distr_conf_impl.shape, distr_conf_impl.scale));
			}
			break;
		case normal_probability_distribution:
			{
				typedef typename distribution_config_type::normal_distribution_config_type distribution_config_impl_type;
				typedef ::dcs::math::stats::normal_distribution<real_type> distribution_impl_type;

				distribution_config_impl_type const& distr_conf_impl = ::boost::get<distribution_config_impl_type>(distr_conf.category_conf);
				//ptr_distr = ::dcs::make_shared<distribution_impl_type>(distr_conf_impl.mean, distr_conf_impl.sd);
				distr = ::dcs::math::stats::make_any_distribution(distribution_impl_type(distr_conf_impl.mean, distr_conf_impl.sd));
			}
			break;
		case map_probability_distribution:
			{
				typedef typename distribution_config_type::map_distribution_config_type distribution_config_impl_type;
				typedef ::dcs::math::stats::map_distribution<real_type> distribution_impl_type;

				distribution_config_impl_type const& distr_conf_impl = ::boost::get<distribution_config_impl_type>(distr_conf.category_conf);
				//ptr_distr = ::dcs::make_shared<distribution_impl_type>(distr_conf_impl.mean, distr_conf_impl.sd);

				switch (distr_conf_impl.characterization_category)
				{
					case standard_map_characterization:
						{
							typedef typename distribution_config_impl_type::standard_characterization_config_type characterization_config_impl_type;
							typedef ::boost::numeric::ublas::matrix<real_type> matrix_type;

							characterization_config_impl_type const& characterization_conf_impl = ::boost::get<characterization_config_impl_type>(distr_conf_impl.characterization_conf);

							matrix_type D0;
							matrix_type D1;

							D0 = make_ublas_matrix(characterization_conf_impl.D0);
							D1 = make_ublas_matrix(characterization_conf_impl.D1);

							distr = ::dcs::math::stats::make_any_distribution(distribution_impl_type(D0, D1));
						}
						break;
					case casale2009_map_characterization:
						{
							throw ::std::runtime_error("CASALE2009 not yet handled");
						}
						break;
				}
			}
			break;
	}

	return distr;
}


template <typename TraitsT, typename RealT, typename UIntT>
::dcs::shared_ptr< ::dcs::eesim::base_application_simulation_model<TraitsT> > make_application_simulation_model(application_simulation_model_config<RealT,UIntT> const& sim_model_conf, simulation_info<TraitsT> const& sim_info)
{
	typedef application_simulation_model_config<RealT,UIntT> model_config_type;
	typedef TraitsT traits_type;
	typedef typename traits_type::real_type real_type;
	typedef typename traits_type::uint_type uint_type;
	typedef typename traits_type::uniform_random_generator_type urng_type;
	typedef typename traits_type::des_engine_type des_engine_type;
	typedef base_application_simulation_model<traits_type> simulation_model_type;

	::dcs::shared_ptr<simulation_model_type> ptr_model;

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
				typedef typename ::dcs::eesim::application_simulation_model_adaptor<traits_type,model_impl_type> model_type;
				typedef ::dcs::des::model::qn::network_node<model_impl_traits_type> node_type;
				typedef ::std::map< ::std::string, typename node_type::identifier_type > name_to_id_container;

				model_impl_type model_impl(sim_info.ptr_rng, sim_info.ptr_engine, false);

				name_to_id_container node_names_ids;

				model_impl_config_type const& model_impl_conf = ::boost::get<model_impl_config_type>(sim_model_conf.category_conf);

				// Network nodes
				node_iterator node_end_it = model_impl_conf.nodes.end();
				for (node_iterator node_it = model_impl_conf.nodes.begin(); node_it != node_end_it; ++node_it)
				{
					::dcs::shared_ptr<node_type> ptr_node;

					if (node_names_ids.count(node_it->name))
					{
						throw ::std::runtime_error("[dcs::eesim::config::detail::make_application_simulation_model] The name of a node must be unique for a given model.");
					}

					switch (node_it->category)
					{
						case qn_delay_node:
							{
								typedef ::dcs::des::model::qn::delay_station_node<model_impl_traits_type> node_impl_type;
								typedef typename node_config_type::delay_node_config_type node_config_impl_type;
								typedef ::dcs::math::stats::any_distribution<real_type> distribution_type;
								typedef ::std::vector<distribution_type> distribution_container;
								typedef ::dcs::des::model::qn::routing_strategy<model_impl_traits_type> routing_strategy_type;

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

									distr = make_probability_distribution(node_conf_impl.distributions[i]);

									distrs.push_back(distr);
								}

								// Routing strategy
								switch (node_conf_impl.routing_category)
								{
									case qn_probabilistic_routing_strategy:
										{
											typedef typename node_config_impl_type::probabilistic_routing_strategy_config_type routing_config_impl_type;
											typedef ::dcs::des::model::qn::probabilistic_routing_strategy<model_impl_traits_type> routing_impl_type;
											typedef typename routing_config_impl_type::probability_container probability_container;
											typedef typename probability_container::const_iterator iterator;

											::dcs::shared_ptr<routing_impl_type> ptr_routing_impl;
											ptr_routing_impl = ::dcs::make_shared<routing_impl_type>(sim_info.ptr_rng);

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
								typedef ::dcs::des::model::qn::service_strategy<model_impl_traits_type> service_strategy_type;
								typedef ::dcs::des::model::qn::routing_strategy<model_impl_traits_type> routing_strategy_type;
								typedef ::dcs::des::model::qn::queueing_strategy<model_impl_traits_type> queueing_strategy_type;

								//::dcs::shared_ptr<node_impl_type> ptr_node_impl;
								::dcs::shared_ptr<service_strategy_type> ptr_service;
								::dcs::shared_ptr<routing_strategy_type> ptr_routing;
								::dcs::shared_ptr<queueing_strategy_type> ptr_queueing;

								node_config_impl_type const& node_conf_impl = ::boost::get<node_config_impl_type>(node_it->category_conf);

								// Service strategy
								switch (node_conf_impl.service_category)
								{
									case qn_infinite_server_service_strategy:
										throw ::std::runtime_error("[dcs::eesim::config::detail::make_application_simulation_model] Service strategy for a queueing station node cannot be of type 'infinite-server'.");
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

												distr = make_probability_distribution(service_conf.distributions[i]);

												distrs.push_back(distr);
											}
											ptr_service = ::dcs::make_shared<service_strategy_impl_type>(distrs.begin(), distrs.end());
										}
										break;
								}

								// Routing strategy
								switch (node_conf_impl.routing_category)
								{
									case qn_probabilistic_routing_strategy:
										{
											typedef typename node_config_impl_type::probabilistic_routing_strategy_config_type routing_config_impl_type;
											typedef ::dcs::des::model::qn::probabilistic_routing_strategy<model_impl_traits_type> routing_impl_type;
											typedef typename routing_config_impl_type::probability_container probability_container;
											typedef typename probability_container::const_iterator iterator;

											::dcs::shared_ptr<routing_impl_type> ptr_routing_impl;
											ptr_routing_impl = ::dcs::make_shared<routing_impl_type>(sim_info.ptr_rng);

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
								typedef ::dcs::des::model::qn::routing_strategy<model_impl_traits_type> routing_strategy_type;

								node_config_impl_type const& node_conf_impl = ::boost::get<node_config_impl_type>(node_it->category_conf);

								::dcs::shared_ptr<routing_strategy_type> ptr_routing;

								// Routing strategy
								switch (node_conf_impl.routing_category)
								{
									case qn_probabilistic_routing_strategy:
										{
											typedef typename node_config_impl_type::probabilistic_routing_strategy_config_type routing_config_impl_type;
											typedef ::dcs::des::model::qn::probabilistic_routing_strategy<model_impl_traits_type> routing_impl_type;
											typedef typename routing_config_impl_type::probability_container probability_container;
											typedef typename probability_container::const_iterator iterator;

											::dcs::shared_ptr<routing_impl_type> ptr_routing_impl;
											ptr_routing_impl = ::dcs::make_shared<routing_impl_type>(sim_info.ptr_rng);

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

					model_impl.add_node(ptr_node);
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
								distr = make_probability_distribution(customer_class_impl_conf.distribution);
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

					model_impl.add_class(ptr_customer_class);
				}

				ptr_model = ::dcs::make_shared<model_type>(model_impl);
			}
			break;
	}

	return ptr_model;
}


template <typename TraitsT, typename RealT>
::dcs::shared_ptr< ::dcs::eesim::base_application_controller<TraitsT> > make_application_controller(application_controller_config<RealT> const& controller_conf)
{
	typedef TraitsT traits_type;
	typedef ::dcs::eesim::base_application_controller<traits_type> controller_type;
	typedef application_controller_config<RealT> controller_config_type;

	::dcs::shared_ptr<controller_type> ptr_controller;

	switch (controller_conf.category)
	{
		case lqr_application_controller:
			{
				//typedef typename controller_config_type::lqr_controller_config_type controller_config_impl_type;
				typedef ::dcs::eesim::lqr_application_controller<traits_type> controller_impl_type;

				//controller_config_impl_type const& controller_conf_impl = ::boost::get<controller_config_impl_type>(controller_conf.category_conf);

				// Note: there is nothing to configure

				ptr_controller = ::dcs::make_shared<controller_impl_type>();
			}
			break;
		case dummy_application_controller:
			{
				//typedef typename controller_config_type::dummy_controller_config_type controller_config_impl_type;
				typedef ::dcs::eesim::dummy_application_controller<traits_type> controller_impl_type;

				//controller_config_impl_type const& controller_conf_impl = ::boost::get<controller_config_impl_type>(controller_conf.category_conf);

				// Note: there is nothing to configure

				ptr_controller = ::dcs::make_shared<controller_impl_type>();
			}
			break;
	}

	ptr_controller->sampling_time(controller_conf.sampling_time);

	return ptr_controller;
}


//FIXME: utilization threshold not yet handled.
template <typename TraitsT, typename RealT, typename UIntT>
::dcs::shared_ptr< ::dcs::eesim::multi_tier_application<TraitsT> > make_application(application_config<RealT,UIntT> const& app_conf, simulation_info<TraitsT> const& sim_info)
{
	typedef TraitsT traits_type;
	typedef RealT real_type;
	typedef UIntT uint_type;
	typedef ::dcs::eesim::multi_tier_application<traits_type> application_type;
	typedef application_config<real_type,uint_type> application_config_type;

	::dcs::shared_ptr<application_type> ptr_app;

	ptr_app = ::dcs::make_shared<application_type>();

	// Name
	if (!app_conf.name.empty())
	{
		ptr_app->name(app_conf.name);
	}

	// Tiers
	{
		typedef ::dcs::eesim::application_tier<traits_type> tier_type;
		typedef typename application_config_type::tier_config_type tier_config_type;
		typedef typename application_config_type::tier_container::const_iterator iterator;
		typedef typename tier_config_type::share_container::const_iterator share_iterator;

		iterator end_it = app_conf.tiers.end();
		for (iterator it = app_conf.tiers.begin(); it != end_it; ++it)
		{
			::dcs::shared_ptr<tier_type> ptr_tier;

			ptr_tier = ::dcs::make_shared<tier_type>();

			ptr_tier->name(it->name);

			share_iterator share_end_it = it->shares.end();
			for (share_iterator share_it = it->shares.begin(); share_it != share_end_it; ++share_it)
			{
				ptr_tier->resource_share(
					make_physical_resource_category(share_it->first),
					share_it->second
				);
			}

			ptr_app->tier(ptr_tier);
		}
	}

	// Reference resources
	{
		typedef typename application_config_type::reference_resource_container::const_iterator iterator;
		iterator end_it = app_conf.reference_resources.end();
		for (iterator it = app_conf.reference_resources.begin(); it != end_it; ++it)
		{
			ptr_app->reference_resource(
				make_physical_resource_category(it->first),
				it->second,
				RealT(1)
			);
		}
	}

	// Performance model
	ptr_app->performance_model(
		make_application_performance_model<TraitsT>(app_conf.perf_model)
	);

	// Simulation model
	ptr_app->simulation_model(
		make_application_simulation_model<TraitsT>(app_conf.sim_model, sim_info)
	);

	return ptr_app;
}

}} // Namespace detail::<unnamed>


template <typename TraitsT, typename RealT, typename UIntT>
::dcs::shared_ptr< ::dcs::eesim::data_center<TraitsT> > make_data_center(configuration<RealT,UIntT> const& conf,
																		 ::dcs::shared_ptr<typename TraitsT::uniform_random_generator_type> const& ptr_rng,
																		 ::dcs::shared_ptr<typename TraitsT::des_engine_type> const& ptr_des_eng)
{
	typedef TraitsT traits_type;
	typedef RealT real_type;
	typedef UIntT uint_type;
	typedef ::dcs::eesim::data_center<traits_type> data_center_type;
	typedef configuration<real_type,uint_type> configuration_type;
	typedef typename configuration_type::data_center_config_type data_center_config_type;

	// pre: random number generator pointer must be a valid pointer
	DCS_ASSERT(
		ptr_rng,
		throw ::std::invalid_argument("[dcs::eesim::config::make_data_center] Invalid random number generator.")
	);
	// pre: DES engine pointer must be a valid pointer
	DCS_ASSERT(
		ptr_des_eng,
		throw ::std::invalid_argument("[dcs::eesim::config::make_data_center] Invalid DES engine.")
	);

	::dcs::shared_ptr<data_center_type> ptr_dc = ::dcs::make_shared<data_center_type>();

	detail::simulation_info<traits_type> sim_info;

	sim_info.ptr_rng = ptr_rng;
	sim_info.ptr_engine = ptr_des_eng;

	// Make physical machines
	{
		typedef typename data_center_config_type::physical_machine_config_container::const_iterator iterator;
		iterator end_it = conf.data_center().physical_machines().end();
		for (iterator it = conf.data_center().physical_machines().begin(); it != end_it; ++it)
		{
			::dcs::shared_ptr< ::dcs::eesim::physical_machine<traits_type> > ptr_mach;

			ptr_mach = detail::make_physical_machine<traits_type>(*it);

			::dcs::shared_ptr< ::dcs::eesim::base_physical_machine_controller<traits_type> > ptr_mach_controller;

			ptr_mach_controller = detail::make_physical_machine_controller<traits_type>(it->controller);
			ptr_mach_controller->controlled_machine(ptr_mach);

			ptr_dc->add_physical_machine(ptr_mach, ptr_mach_controller);
		}
	}

	// Make applications
	{
		typedef typename data_center_config_type::application_config_container::const_iterator iterator;
		iterator end_it = conf.data_center().applications().end();
		for (iterator it = conf.data_center().applications().begin(); it != end_it; ++it)
		{
			::dcs::shared_ptr< ::dcs::eesim::multi_tier_application<traits_type> > ptr_app;

			ptr_app = detail::make_application<traits_type>(*it, sim_info);

			::dcs::shared_ptr< ::dcs::eesim::base_application_controller<traits_type> > ptr_app_controller;

			ptr_app_controller = detail::make_application_controller<traits_type>(it->controller);
			ptr_app_controller->controlled_application(ptr_app);

			ptr_dc->add_application(ptr_app, ptr_app_controller);
		}
	}

	return ptr_dc;
}


}}} // Namespace dcs::eesim::config


#endif // DCS_EESIM_CONFIG_OPERATION_MAKE_DATA_CENTER_HPP
