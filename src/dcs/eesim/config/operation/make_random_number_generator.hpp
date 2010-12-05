#ifndef DCS_EESIM_CONFIG_OPERATION_MAKE_RANOMD_NUMBER_GENERATOR_HPP
#define DCS_EESIM_CONFIG_OPERATION_MAKE_RANOMD_NUMBER_GENERATOR_HPP


#include <dcs/eesim/config/configuration.hpp>
#include <dcs/eesim/config/rng.hpp>
#include <dcs/math/random.hpp>
#include <dcs/memory.hpp>


namespace dcs { namespace eesim { namespace config {

//TODO: missing seeder stuff
template <typename RealT, typename UIntT>
::dcs::shared_ptr< ::dcs::math::random::base_generator<RealT> > make_random_number_generator(configuration<RealT,UIntT> const& conf)
{
	typedef ::dcs::math::random::base_generator<RealT> generator_type;

	rng_engine_category engine_category = conf.rng().engine;
	rng_seeder_category seeder_category = conf.rng().seeder;
	UIntT seed = conf.rng().seed;

	::dcs::shared_ptr<generator_type> ptr_rng;

	switch (engine_category)
	{
		case minstd_rand0_rng_engine:
			{
				typedef ::dcs::math::random::uniform_01_adaptor<
								::dcs::math::random::minstd_rand0,
								RealT
							> rng_type;


				ptr_rng = ::dcs::make_shared<rng_type>(seed);
			}
			break;
		case minstd_rand1_rng_engine:
			{
				typedef ::dcs::math::random::uniform_01_adaptor<
								::dcs::math::random::minstd_rand1,
								RealT
							> rng_type;


				ptr_rng = ::dcs::make_shared<rng_type>(seed);
			}
			break;
		case minstd_rand2_rng_engine:
			{
				typedef ::dcs::math::random::uniform_01_adaptor<
								::dcs::math::random::minstd_rand2,
								RealT
							> rng_type;


				ptr_rng = ::dcs::make_shared<rng_type>(seed);
			}
			break;
		case rand48_rng_engine:
			{
				typedef ::dcs::math::random::uniform_01_adaptor<
								::dcs::math::random::rand48,
								RealT
							> rng_type;


				ptr_rng = ::dcs::make_shared<rng_type>(seed);
			}
			break;
		case mt11213b_rng_engine:
			{
				typedef ::dcs::math::random::uniform_01_adaptor<
								::dcs::math::random::mt11213b,
								RealT
							> rng_type;


				ptr_rng = ::dcs::make_shared<rng_type>(seed);
			}
			break;
		case mt19937_rng_engine:
			{
				typedef dcs::math::random::uniform_01_adaptor<
								dcs::math::random::mt19937,
								RealT
							> rng_type;

				ptr_rng = ::dcs::make_shared<rng_type>(seed);
			}
			break;
	}

	return ptr_rng;
}

}}} // Namespace dcs::eesim::config


#endif // DCS_EESIM_CONFIG_OPERATION_MAKE_RANOMD_NUMBER_GENERATOR_HPP
