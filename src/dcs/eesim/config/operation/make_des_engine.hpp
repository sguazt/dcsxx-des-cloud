#ifndef DCS_EESIM_CONFIG_OPERATION_MAKE_DES_ENGINE_HPP
#define DCS_EESIM_CONFIG_OPERATION_MAKE_DES_ENGINE_HPP


#include <boost/variant.hpp>
#include <dcs/des/engine.hpp>
#include <dcs/des/replications/engine.hpp>
#include <dcs/eesim/config/configuration.hpp>
#include <dcs/memory.hpp>


namespace dcs { namespace eesim { namespace config {

template <typename RealT, typename UIntT>
::dcs::shared_ptr< ::dcs::des::engine<RealT> > make_des_engine(configuration<RealT,UIntT> const& conf)
{
	typedef ::dcs::des::engine<RealT> engine_type;

	::dcs::shared_ptr<engine_type> ptr_eng;

	switch (conf.simulation().output_analysis_type)
	{
		case independent_replications_output_analysis:
			{
				typedef independent_replications_output_analysis_config<RealT,UIntT> output_analysis_config_type;
				output_analysis_config_type analysis;
				analysis = ::boost::get<output_analysis_config_type>(conf.simulation().output_analysis_conf);
				ptr_eng = ::dcs::make_shared< ::dcs::des::replications::engine<RealT,UIntT> >(
					analysis.replication_duration,
					analysis.num_replications
				);
			}
	}

	return ptr_eng;
}

}}} // Namespace dcs::eesim::config


#endif // DCS_EESIM_CONFIG_OPERATION_MAKE_DES_ENGINE_HPP
