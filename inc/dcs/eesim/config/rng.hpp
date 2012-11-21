#ifndef DCS_EESIM_CONFIG_RNG_HPP
#define DCS_EESIM_CONFIG_RNG_HPP


#include <iosfwd>


namespace dcs { namespace eesim { namespace config {

enum rng_engine_category
{
	minstd_rand0_rng_engine,
	minstd_rand1_rng_engine,
	minstd_rand2_rng_engine,
	rand48_rng_engine,
	mt11213b_rng_engine,
	mt19937_rng_engine
};


enum rng_seeder_category
{
	none_rng_seeder,
	lcg_rng_seeder
};


template <typename UIntT>
struct rng_config
{
	typedef UIntT uint_type;

	uint_type seed;
	rng_engine_category engine;
	rng_seeder_category seeder;
};


template <typename CharT, typename CharTraitsT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, rng_engine_category category)
{
	switch (category)
	{
		case minstd_rand0_rng_engine:
			os << "minstd-rand0";
			break;
		case minstd_rand1_rng_engine:
			os << "minstd-rand1";
			break;
		case minstd_rand2_rng_engine:
			os << "minstd-rand2";
			break;
		case rand48_rng_engine:
			os << "rand48";
			break;
		case mt11213b_rng_engine:
			os << "mt11213b";
			break;
		case mt19937_rng_engine:
			os << "mt19937";
			break;
	}

	return os;
}


template <typename CharT, typename CharTraitsT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, rng_seeder_category category)
{
	switch (category)
	{
		case lcg_rng_seeder:
			os << "lcg";
			break;
		case none_rng_seeder:
			os << "none";
			break;
	}

	return os;
}


template <typename CharT, typename CharTraitsT, typename UIntT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, rng_config<UIntT> const& rng)
{
	os << "<(rng)"
	   << " engine: " << rng.engine
	   << ", seed: " << rng.seed
	   << ", seeder: " << rng.seeder
	   << ">";

	return os;
}

}}} // Namespace dcs::eesim::config


#endif // DCS_EESIM_CONFIG_RNG_HPP
