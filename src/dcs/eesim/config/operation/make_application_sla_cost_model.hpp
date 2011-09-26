#ifndef DCS_EESIM_CONFIG_OPERATION_MAKE_APPLICATION_SLA_COST_MODEL_HPP
#define DCS_EESIM_CONFIG_OPERATION_MAKE_APPLICATION_SLA_COST_MODEL_HPP


#include <boost/variant.hpp>
#include <dcs/eesim/config/application_sla.hpp>
#include <dcs/eesim/config/operation/make_performance_measure_category.hpp>
#include <dcs/eesim/performance_measure_category.hpp>
#include <dcs/perfeval/sla.hpp>
#include <stdexcept>


namespace dcs { namespace eesim { namespace config {

template <typename ValueT>
struct response_time_sla_checker
{
	typedef ValueT value_type;

	explicit response_time_sla_checker(value_type tolerance=value_type/*zero*/())
	: tol_(tolerance)
	{
	}

	bool operator()(value_type ref_value, value_type value) const
	{
//		return ref_value >= value;
		if (tol_ > 0)
		{
			return (value/ref_value - static_cast<value_type>(1)) <= tol_;
		}
		else
		{
			return ref_value >= value;
		}
	}

	private: value_type tol_;
};


template <typename ValueT>
struct throughput_sla_checker
{
	typedef ValueT value_type;

	explicit throughput_sla_checker(value_type tolerance=value_type/*zero*/())
	: tol_(tolerance)
	{
	}

	bool operator()(value_type ref_value, value_type value) const
	{
//		return ref_value <= value;
		if (tol_ > 0)
		{
			return (value/ref_value - static_cast<value_type>(1)) <= tol_;
		}
		else
		{
			return ref_value >= value;
		}
	}

	private: value_type tol_;
};


template <typename ValueT>
::dcs::perfeval::sla::any_metric_checker<ValueT> make_performance_measure_checker(metric_category category, ValueT tolerance)
{
	switch (category)
	{
		case response_time_metric:
			return ::dcs::perfeval::sla::any_metric_checker<ValueT>(response_time_sla_checker<ValueT>(tolerance));
		case throughput_metric:
			return ::dcs::perfeval::sla::any_metric_checker<ValueT>(throughput_sla_checker<ValueT>(tolerance));
		default:
			break;
	}

	throw ::std::runtime_error("[dcs::eesim::detail::make_performance_measure_checker] Unable to create a SLA checker for performance measure category.");
}


template <typename TraitsT, typename RealT>
::dcs::perfeval::sla::any_cost_model<
	::dcs::eesim::performance_measure_category,
	typename TraitsT::real_type,
	typename TraitsT::real_type
> make_application_sla_cost_model(application_sla_config<RealT> const& sla_conf)
{
	typedef TraitsT traits_type;
	typedef application_sla_config<RealT> sla_config_type;
	typedef typename traits_type::real_type target_real_type;
	typedef ::dcs::perfeval::sla::any_cost_model<
				::dcs::eesim::performance_measure_category,
				target_real_type,
				target_real_type> sla_type;
	typedef typename sla_config_type::metric_container metric_container;
	typedef typename metric_container::const_iterator metric_iterator;

	sla_type sla;

	switch (sla_conf.category)
	{
		case step_sla_model:
			{
				typedef ::dcs::perfeval::sla::step_cost_model<
								::dcs::eesim::performance_measure_category,
								target_real_type,
								target_real_type> sla_impl_type;
				typedef typename sla_config_type::step_sla_model_config_type sla_config_impl_type;

				sla_config_impl_type const& sla_conf_impl = ::boost::get<sla_config_impl_type>(sla_conf.category_conf);

				sla = ::dcs::perfeval::sla::make_any_cost_model(
						sla_impl_type(
								sla_conf_impl.penalty,
								sla_conf_impl.revenue
							)
					);

				metric_iterator metric_end_it = sla_conf.metrics.end();
				for (metric_iterator it = sla_conf.metrics.begin(); it != metric_end_it; ++it)
				{
					sla.add_slo(
							make_performance_measure_category(it->first),
							it->second.value,
							make_performance_measure_checker<target_real_type>(it->first, it->second.tolerance)
						);
				}

			}
			break;
		case none_sla_model:
			{
				typedef ::dcs::perfeval::sla::always_satisfied_cost_model<
								::dcs::eesim::performance_measure_category,
								target_real_type,
								target_real_type> sla_impl_type;
				typedef typename sla_config_type::none_sla_model_config_type sla_config_impl_type;

				//sla_config_impl_type const& sla_conf_impl = ::boost::get<sla_config_impl_type>(sla_conf.category_conf);

				sla = ::dcs::perfeval::sla::make_any_cost_model(sla_impl_type());
			}
			break;
	}

	return sla;
}

}}} // Namespace dcs::eesim::config


#endif // DCS_EESIM_CONFIG_OPERATION_MAKE_APPLICATION_SLA_COST_MODEL_HPP
