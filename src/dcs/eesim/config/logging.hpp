#ifndef DCS_EESIM_CONFIG_LOGGIN_HPP
#define DCS_EESIM_CONFIG_LOGGIN_HPP


#include <boost/variant.hpp>
#include <iostream>
#include <string>


namespace dcs { namespace eesim { namespace config {

enum logging_category
{
	minimal_logging,
	compact_logging//,
//	standard_logging
};


enum logging_sink_category
{
	console_logging_sink,
	file_logging_sink
};


enum stream_logging_sink_category
{
	stdout_stream_logging_sink,
	stderr_stream_logging_sink,
	stdlog_stream_logging_sink
};


struct console_logging_sink_config
{
	stream_logging_sink_category stream;
};


struct file_logging_sink_config
{
	::std::string name;
};


struct logging_sink_config
{
	typedef console_logging_sink_config console_logging_sink_type;
	typedef file_logging_sink_config file_logging_sink_type;

	logging_sink_category category;
	::boost::variant<console_logging_sink_type,
					 file_logging_sink_type> category_conf;
};


struct compact_logging_config
{
	logging_sink_config sink;
};


struct minimal_logging_config
{
	logging_sink_config sink;
};


struct logging_config
{
	typedef minimal_logging_config minimal_logging_type;
	typedef compact_logging_config compact_logging_type;

	bool enabled;
    logging_category category;
    ::boost::variant<compact_logging_type,
					 minimal_logging_type> category_conf;
};


template <typename CharT, typename CharTraitsT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, logging_config const& log_conf)
{
	DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(log_conf);

//TODO
	os << "<(logging)"
	   << ">";

	return os;
}

}}} // Namespace dcs:eesim::config


#endif // DCS_EESIM_CONFIG_LOGGIN_HPP
