#ifndef DCS_DES_CLOUD_CONFIG_READ_FILE_HPP
#define DCS_DES_CLOUD_CONFIG_READ_FILE_HPP


#include <dcs/des/cloud/config/configuration.hpp>


namespace dcs { namespace des { namespace cloud { namespace config {

template <typename ReaderT>
configuration<typename ReaderT::real_type,typename ReaderT::uint_type> read_file(::std::string const& fname, ReaderT reader)
{
	return reader.read(fname);
}

}}}} // Namespace dcs::des::cloud::config


#endif // DCS_DES_CLOUD_CONFIG_READ_FILE_HPP
