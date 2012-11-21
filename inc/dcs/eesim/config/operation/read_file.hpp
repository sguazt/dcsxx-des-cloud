#ifndef DCS_EESIM_CONFIG_READ_FILE_HPP
#define DCS_EESIM_CONFIG_READ_FILE_HPP


#include <dcs/eesim/config/configuration.hpp>


namespace dcs { namespace eesim { namespace config {

template <typename ReaderT>
configuration<typename ReaderT::real_type,typename ReaderT::uint_type> read_file(::std::string const& fname, ReaderT reader)
{
	return reader.read(fname);
}

}}} // Namespace dcs::eesim::config


#endif // DCS_EESIM_CONFIG_READ_FILE_HPP
