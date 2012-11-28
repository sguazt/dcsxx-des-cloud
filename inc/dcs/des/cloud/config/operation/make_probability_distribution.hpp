#ifndef DCS_DES_CLOUD_CONFIG_OPERATION_MAKE_PROBABILITY_DISTRIBUTION_HPP
#define DCS_DES_CLOUD_CONFIG_OPERATION_MAKE_PROBABILITY_DISTRIBUTION_HPP


#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <boost/variant.hpp>
#include <dcs/des/cloud/config/operation/make_algebraic_type.hpp>
#include <dcs/des/cloud/config/probability_distribution.hpp>
#include <dcs/des/cloud/workload.hpp>
#include <dcs/math/stats/distributions.hpp>
#include <stdexcept>


namespace dcs { namespace des { namespace cloud { namespace config {

template <typename TraitsT, typename RealT>
::dcs::math::stats::any_distribution<RealT> make_probability_distribution(probability_distribution_config<RealT> const& distr_conf)
{
	typedef TraitsT traits_type;
	typedef RealT real_type;
	typedef probability_distribution_config<RealT> distribution_config_type;
	typedef ::dcs::math::stats::any_distribution<real_type> distribution_type;

	distribution_type distr;

	switch (distr_conf.category)
	{
		case degenerate_probability_distribution:
			{
				typedef typename distribution_config_type::degenerate_distribution_config_type distribution_config_impl_type;
				typedef ::dcs::math::stats::degenerate_distribution<real_type> distribution_impl_type;

				distribution_config_impl_type const& distr_conf_impl = ::boost::get<distribution_config_impl_type>(distr_conf.category_conf);
				//ptr_distr = ::dcs::make_shared<distribution_impl_type>(distr_conf.rate);
				distr = ::dcs::math::stats::make_any_distribution(distribution_impl_type(distr_conf_impl.k));
			}
			break;
		case erlang_probability_distribution:
			{
				typedef typename distribution_config_type::erlang_distribution_config_type distribution_config_impl_type;
				typedef ::dcs::math::stats::erlang_distribution<real_type> distribution_impl_type;

				distribution_config_impl_type const& distr_conf_impl = ::boost::get<distribution_config_impl_type>(distr_conf.category_conf);
				//ptr_distr = ::dcs::make_shared<distribution_impl_type>(distr_conf.rate);
				distr = ::dcs::math::stats::make_any_distribution(distribution_impl_type(distr_conf_impl.num_stages, distr_conf_impl.rate));
			}
			break;
		case exponential_probability_distribution:
			{
				typedef typename distribution_config_type::exponential_distribution_config_type distribution_config_impl_type;
				typedef ::dcs::math::stats::exponential_distribution<real_type> distribution_impl_type;

				distribution_config_impl_type const& distr_conf_impl = ::boost::get<distribution_config_impl_type>(distr_conf.category_conf);
				//ptr_distr = ::dcs::make_shared<distribution_impl_type>(distr_conf.rate);
				//distr = ::dcs::math::stats::make_any_distribution(distribution_impl_type(distr_conf_impl.rate));
				distribution_impl_type distr_impl(distr_conf_impl.rate);
				distr = ::dcs::math::stats::make_any_distribution(distr_impl);
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
							throw ::std::runtime_error("[dcs::des::cloud::config::detail::<unnamed>::make_probability_distribution] casale-2009 MAP characterization has not been handled yet.");
						}
						break;
				}
			}
			break;
		case mmpp_probability_distribution:
			{
//				typedef typename distribution_config_type::mmpp_distribution_config_type distribution_config_impl_type;
//				typedef ::dcs::math::stats::mmpp_distribution<real_type> distribution_impl_type;
//
//				distribution_config_impl_type const& distr_conf_impl = ::boost::get<distribution_config_impl_type>(distr_conf.category_conf);
//				//ptr_distr = ::dcs::make_shared<distribution_impl_type>(distr_conf_impl.mean, distr_conf_impl.sd);
//
//				typedef ::boost::numeric::ublas::matrix<real_type> matrix_type;
//				typedef ::boost::numeric::ublas::vector<real_type> vector_type;
//
//				matrix_type Q;
//				vector_type lambda;
//
//				Q = make_ublas_matrix(distr_conf_impl.Q);
//				lambda = make_ublas_vector(distr_conf_impl.rates);
//
//				distr = ::dcs::math::stats::make_any_distribution(distribution_impl_type(lambda, Q));
				typedef typename distribution_config_type::mmpp_distribution_config_type distribution_config_impl_type;
				typedef ::dcs::des::cloud::mmpp_interarrivals_workload_model<traits_type,real_type> distribution_impl_type;

				distribution_config_impl_type const& distr_conf_impl = ::boost::get<distribution_config_impl_type>(distr_conf.category_conf);

				typedef ::boost::numeric::ublas::matrix<real_type> matrix_type;
				typedef ::boost::numeric::ublas::vector<real_type> vector_type;

				matrix_type Q;
				vector_type lambda;
				vector_type p0;

				Q = make_ublas_matrix(distr_conf_impl.Q);
				lambda = make_ublas_vector(distr_conf_impl.rates);
				p0 = make_ublas_vector(distr_conf_impl.p0);

				distr = ::dcs::math::stats::make_any_distribution(distribution_impl_type(lambda, Q, p0));
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
		case pmpp_probability_distribution:
			{
				typedef typename distribution_config_type::pmpp_distribution_config_type distribution_config_impl_type;
				typedef ::dcs::math::stats::pmpp_distribution<real_type> distribution_impl_type;

				distribution_config_impl_type const& distr_conf_impl = ::boost::get<distribution_config_impl_type>(distr_conf.category_conf);

				if (distr_conf_impl.rates.size() > 2)
				{
					throw ::std::runtime_error("[dcs::des::cloud::config::detail::<unnamed>::make_probability_distribution] PMPP processes with a number of states > 2 have not been handled yet.");
				}

				distr = ::dcs::math::stats::make_any_distribution(
							distribution_impl_type(
									distr_conf_impl.rates[0],
									distr_conf_impl.rates[1],
									distr_conf_impl.shape,
									distr_conf_impl.min
								)
					);
			}
			break;
		case timed_step_probability_distribution://EXP
			{
				typedef typename distribution_config_type::timed_step_distribution_config_type distribution_config_impl_type;
				typedef typename distribution_config_impl_type::phase_container::const_iterator phase_iterator;
				typedef ::dcs::des::cloud::timed_step_workload_model<traits_type,real_type> distribution_impl_type;

				distribution_config_impl_type const& distr_conf_impl = ::boost::get<distribution_config_impl_type>(distr_conf.category_conf);

				distribution_impl_type distr_impl;
				phase_iterator phase_end_it(distr_conf_impl.phases.end());
				for (phase_iterator it = distr_conf_impl.phases.begin(); it != phase_end_it; ++it)
				{
					distribution_type phase_distr = make_probability_distribution<traits_type>(*(it->second));
					//distr_impl.add_phase(it->first, make_probability_distribution<traits_type>(*(it->second)));
					distr_impl.add_phase(it->first, phase_distr);
				}
				distr = ::dcs::math::stats::make_any_distribution(distr_impl);
			}
			break;
	}

	return distr;
}

}}}} // Namespace dcs::des::cloud::config


#endif // DCS_DES_CLOUD_CONFIG_OPERATION_MAKE_PROBABILITY_DISTRIBUTION_HPP
