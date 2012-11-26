#ifndef DCS_EESIM_CONFIG_OPERATION_MAKE_DES_ENGINE_HPP
#define DCS_EESIM_CONFIG_OPERATION_MAKE_DES_ENGINE_HPP


#include <boost/variant.hpp>
#include <dcs/des/engine.hpp>
#include <dcs/des/replications/engine.hpp>
#include <dcs/eesim/config/configuration.hpp>
#include <dcs/eesim/config/simulation.hpp>
#include <dcs/memory.hpp>
#include <stdexcept>


namespace dcs { namespace eesim { namespace config {

template <typename RealT, typename UIntT>
::dcs::shared_ptr< ::dcs::des::engine<RealT> > make_des_engine(configuration<RealT,UIntT> const& conf)
{
	typedef RealT real_type;
	typedef UIntT uint_type;
	typedef ::dcs::des::engine<real_type> engine_type;

	::dcs::shared_ptr<engine_type> ptr_eng;

	switch (conf.simulation().output_analysis.category)
	{
		case independent_replications_output_analysis:
			{
				typedef independent_replications_output_analysis_config<real_type,uint_type> output_analysis_config_type;
				typedef ::dcs::des::replications::engine<real_type,uint_type> engine_impl_type;

				output_analysis_config_type const& analysis = ::boost::get<output_analysis_config_type>(conf.simulation().output_analysis.category_conf);

				real_type min_replication_size(0);
				uint_type min_num_replications(0);

				switch (analysis.num_replications_category)
				{
					case constant_num_replications_detector:
						{
							typedef typename output_analysis_config_type::constant_num_replications_detector_type num_replications_detector_config_type;

							num_replications_detector_config_type const& num_replications_detector_conf = ::boost::get<num_replications_detector_config_type>(analysis.num_replications_category_conf);

							min_num_replications = num_replications_detector_conf.num_replications;
						}
						break;
					case banks2005_num_replications_detector:
						{
							typedef typename output_analysis_config_type::banks2005_num_replications_detector_type num_replications_detector_config_type;

							num_replications_detector_config_type const& num_replications_detector_conf = ::boost::get<num_replications_detector_config_type>(analysis.num_replications_category_conf);

							min_num_replications = num_replications_detector_conf.min_num_replications;
						}
						break;
				}
				switch (analysis.replication_size_category)
				{
					case fixed_duration_replication_size_detector:
						{
							typedef typename output_analysis_config_type::fixed_duration_replication_size_detector_type replication_size_detector_config_type;

							replication_size_detector_config_type const& replication_size_detector_conf = ::boost::get<replication_size_detector_config_type>(analysis.replication_size_category_conf);

							min_replication_size = replication_size_detector_conf.replication_duration;
						}
						break;
					case fixed_num_obs_replication_size_detector:
						{
							typedef typename output_analysis_config_type::fixed_num_obs_replication_size_detector_type replication_size_detector_config_type;

							//replication_size_detector_config_type const& replication_size_detector_conf = ::boost::get<replication_size_detector_config_type>(analysis.replication_size_category_conf);

							//min_replication_size = ::dcs::math::constants::infinity<uint_type>::value;
							min_replication_size = uint_type(1);
						}
						break;
				}

				ptr_eng = ::dcs::make_shared<engine_impl_type>(
					min_replication_size,
					min_num_replications
				);
			}
			break;
		default:
			throw ::std::runtime_error("[dcs::eesim::config::make_des_engine] Unhandled output analysis category.");
	}

	return ptr_eng;
}

}}} // Namespace dcs::eesim::config


#endif // DCS_EESIM_CONFIG_OPERATION_MAKE_DES_ENGINE_HPP
