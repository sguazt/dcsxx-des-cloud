#ifndef DCS_DES_CLOUD_CONFIG_OPERATION_MAKE_APPLICATION_PERFORMANCE_MODEL_HPP
#define DCS_DES_CLOUD_CONFIG_OPERATION_MAKE_APPLICATION_PERFORMANCE_MODEL_HPP


#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <boost/variant.hpp>
#include <dcs/des/cloud/application_performance_model_adaptor.hpp>
//#include <dcs/des/cloud/application_performance_model_traits.hpp>
#include <dcs/des/cloud/base_application_performance_model.hpp>
#include <dcs/des/cloud/config/application_performance_model.hpp>
#include <dcs/des/cloud/config/metric_category.hpp>
#include <dcs/des/cloud/config/operation/make_algebraic_type.hpp>
#include <dcs/des/cloud/fixed_application_performance_model.hpp>
#include <dcs/des/cloud/open_multi_bcmp_qn_application_performance_model.hpp>
#include <dcs/perfeval/qn/open_multi_bcmp_network.hpp>
#include <dcs/perfeval/qn/operation/visit_ratios.hpp>
#include <dcs/memory.hpp>


namespace dcs { namespace des { namespace cloud { namespace config {

template <typename TraitsT, typename RealT, typename UIntT>
::dcs::shared_ptr<
	::dcs::des::cloud::base_application_performance_model<TraitsT>
> make_application_performance_model(application_performance_model_config<RealT,UIntT> const& conf)
{
	typedef TraitsT traits_type;
	typedef typename traits_type::real_type real_type;
	typedef typename traits_type::uint_type uint_type;
	typedef base_application_performance_model<traits_type> performance_model_type;
	typedef application_performance_model_config<real_type,uint_type> config_type;

	::dcs::shared_ptr<performance_model_type> ptr_model;

	switch (conf.category)
	{
		case fixed_application_performance_model:
			{
				typedef typename config_type::fixed_config_type config_impl_type;
				typedef ::dcs::des::cloud::fixed_application_performance_model<traits_type> model_impl_type;
				typedef typename config_impl_type::measure_map::const_iterator measure_iterator;
				typedef typename config_impl_type::tier_measure_map::const_iterator tier_measure_iterator;

				config_impl_type const& conf_impl = ::boost::get<config_impl_type>(conf.category_conf);

				::dcs::shared_ptr<model_impl_type> ptr_model_impl = ::dcs::make_shared<model_impl_type>();

				measure_iterator app_meas_end_it(conf_impl.app_measures.end());
				for (measure_iterator it = conf_impl.app_measures.begin(); it != app_meas_end_it; ++it)
				{
					ptr_model_impl->application_measure(to_performance_measure_category(it->first), it->second);
				}

				tier_measure_iterator tier_meas_end_it(conf_impl.tier_measures.end());
				for (tier_measure_iterator tier_it = conf_impl.tier_measures.begin(); tier_it != tier_meas_end_it; ++tier_it)
				{
					measure_iterator meas_end_it(tier_it->second.end());
					for (measure_iterator meas_it = tier_it->second.begin(); meas_it != meas_end_it; ++meas_it)
					{
						ptr_model_impl->tier_measure(tier_it->first, to_performance_measure_category(meas_it->first), meas_it->second);
					}
				}

				//ptr_model = ::dcs::static_pointer_cast<performance_model_type>(ptr_model_impl);
				ptr_model = ptr_model_impl;
			}
			break;
		case open_multi_bcmp_qn_model:
			{
				typedef typename config_type::open_multi_bcmp_qn_config_type config_impl_type;
				typedef ::dcs::perfeval::qn::open_multi_bcmp_network<real_type, uint_type> model_impl_type;
				typedef typename ::dcs::des::cloud::application_performance_model_adaptor<
							TraitsT,
							model_impl_type
						> model_type;

				config_impl_type const& conf_impl = ::boost::get<config_impl_type>(conf.category_conf);

				::boost::numeric::ublas::vector<real_type> lambda;
				lambda = make_ublas_vector(conf_impl.arrival_rates);

				::boost::numeric::ublas::matrix<real_type> S;
				S = make_ublas_matrix(conf_impl.service_times);

				::boost::numeric::ublas::matrix<real_type> V;
				if (!conf_impl.visit_ratios.empty())
				{
					V = make_ublas_matrix(conf_impl.visit_ratios);
				}
				else
				{
					::boost::numeric::ublas::matrix<real_type> P;
					P = make_ublas_matrix(conf_impl.routing_probabilities);

					::boost::numeric::ublas::vector<real_type> v;
					v = ::dcs::perfeval::qn::visit_ratios(P, lambda);

					V.resize(v.size(), 1, false);
					::boost::numeric::ublas::column(V, 0) = v;
				}

				::boost::numeric::ublas::vector<real_type> m;
				m = make_ublas_vector(conf_impl.num_servers);

				ptr_model = ::dcs::make_shared<model_type>(
						model_impl_type(lambda, S, V, m)
					);
			}
			break;
	}

	return ptr_model;
}

}}}} // Namespace dcs::des::cloud::config


#endif // DCS_DES_CLOUD_CONFIG_OPERATION_MAKE_APPLICATION_PERFORMANCE_MODEL_HPP
