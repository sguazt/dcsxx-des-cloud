#ifndef DCS_EESIM_CONFIG_OPERATION_MAKE_RANOMD_NUMBER_GENERATOR_HPP
#define DCS_EESIM_CONFIG_OPERATION_MAKE_RANOMD_NUMBER_GENERATOR_HPP


#include <dcs/eesim/config/configuration.hpp>
#include <dcs/eesim/config/rng.hpp>
#include <dcs/math/random.hpp>
#include <dcs/memory.hpp>
#include <stdexcept>


namespace dcs { namespace eesim { namespace config {

//TODO: missing seeder stuff
/*
template <typename RealT, typename UIntT>
::dcs::shared_ptr< ::dcs::math::random::base_generator<RealT> > make_random_number_generator(configuration<RealT,UIntT> const& conf)
{
	typedef RealT real_type;
	typedef UIntT uint_type;
	typedef ::dcs::math::random::base_generator<real_type> rng_type;

	rng_engine_category engine_category = conf.rng().engine;
	rng_seeder_category seeder_category = conf.rng().seeder;
	uint_type seed = conf.rng().seed;

	::dcs::shared_ptr<rng_type> ptr_rng;

	switch (engine_category)
	{
		case minstd_rand0_rng_engine:
			{
				typedef ::dcs::math::random::uniform_01_adaptor<
								::dcs::math::random::minstd_rand0,
								real_type
							> rng_impl_type;


				ptr_rng = ::dcs::make_shared<rng_impl_type>(seed);
			}
			break;
		case minstd_rand1_rng_engine:
			{
				typedef ::dcs::math::random::uniform_01_adaptor<
								::dcs::math::random::minstd_rand1,
								real_type
							> rng_impl_type;


				ptr_rng = ::dcs::make_shared<rng_impl_type>(seed);
			}
			break;
		case minstd_rand2_rng_engine:
			{
				typedef ::dcs::math::random::uniform_01_adaptor<
								::dcs::math::random::minstd_rand2,
								real_type
							> rng_impl_type;


				ptr_rng = ::dcs::make_shared<rng_impl_type>(seed);
			}
			break;
		case rand48_rng_engine:
			{
				typedef ::dcs::math::random::uniform_01_adaptor<
								::dcs::math::random::rand48,
								real_type
							> rng_impl_type;


				ptr_rng = ::dcs::make_shared<rng_impl_type>(seed);
			}
			break;
		case mt11213b_rng_engine:
			{
				typedef ::dcs::math::random::uniform_01_adaptor<
								::dcs::math::random::mt11213b,
								real_type
							> rng_impl_type;


				ptr_rng = ::dcs::make_shared<rng_impl_type>(seed);
			}
			break;
		case mt19937_rng_engine:
			{
				typedef dcs::math::random::uniform_01_adaptor<
								dcs::math::random::mt19937,
								real_type
							> rng_impl_type;

				ptr_rng = ::dcs::make_shared<rng_impl_type>(seed);
			}
			break;
		default:
			throw ::std::runtime_error("[dcs::eesim::config::make_random_number_generator] Unhandled random number generator category.");
	}

	return ptr_rng;
}
*/

template <typename RealT, typename UIntT>
::dcs::shared_ptr< ::dcs::math::random::base_generator<UIntT> > make_random_number_generator(configuration<RealT,UIntT> const& conf)
{
	typedef RealT real_type;
	typedef UIntT uint_type;
	typedef ::dcs::math::random::base_generator<uint_type> rng_type;

	rng_engine_category engine_category = conf.rng().engine;
	rng_seeder_category seeder_category = conf.rng().seeder;
	uint_type seed = conf.rng().seed;

	::dcs::shared_ptr<rng_type> ptr_rng;

	switch (engine_category)
	{
		case minstd_rand0_rng_engine:
			{
				typedef ::dcs::math::random::uniform_int_adaptor<
							::dcs::math::random::minstd_rand0,
							uint_type
						> rng_impl_type;

				ptr_rng = ::dcs::make_shared<rng_impl_type>();
//				ptr_rng = ::dcs::shared_ptr<rng_type>(new rng_impl_type());
			}
			break;
		case minstd_rand1_rng_engine:
			{
				typedef ::dcs::math::random::uniform_int_adaptor<
							::dcs::math::random::minstd_rand1,
							uint_type
						> rng_impl_type;


				ptr_rng = ::dcs::make_shared<rng_impl_type>();
//				ptr_rng = ::dcs::shared_ptr<rng_type>(new rng_impl_type());
			}
			break;
		case minstd_rand2_rng_engine:
			{
				typedef ::dcs::math::random::uniform_int_adaptor<
							::dcs::math::random::minstd_rand2,
							uint_type
						> rng_impl_type;


				ptr_rng = ::dcs::make_shared<rng_impl_type>();
//				ptr_rng = ::dcs::shared_ptr<rng_type>(new rng_impl_type());
			}
			break;
		case rand48_rng_engine:
			{
				typedef ::dcs::math::random::uniform_int_adaptor<
							::dcs::math::random::rand48,
							uint_type
						> rng_impl_type;


				ptr_rng = ::dcs::make_shared<rng_impl_type>();
//				ptr_rng = ::dcs::shared_ptr<rng_type>(new rng_impl_type());
			}
			break;
		case mt11213b_rng_engine:
			{
				typedef ::dcs::math::random::uniform_int_adaptor<
							::dcs::math::random::mt11213b,
							uint_type
						> rng_impl_type;

				ptr_rng = ::dcs::make_shared<rng_impl_type>();
//				ptr_rng = ::dcs::shared_ptr<rng_type>(new rng_impl_type());
			}
			break;
		case mt19937_rng_engine:
			{
				typedef ::dcs::math::random::uniform_int_adaptor<
							::dcs::math::random::mt19937,
							uint_type
						> rng_impl_type;

				ptr_rng = ::dcs::make_shared<rng_impl_type>();
//				ptr_rng = ::dcs::shared_ptr<rng_type>(new rng_impl_type());
			}
			break;
		default:
			throw ::std::runtime_error("[dcs::eesim::config::make_random_number_generator] Unhandled random number generator category.");
	}

	ptr_rng->seed(seed);

	return ptr_rng;
}

}}} // Namespace dcs::eesim::config


#endif // DCS_EESIM_CONFIG_OPERATION_MAKE_RANOMD_NUMBER_GENERATOR_HPP
