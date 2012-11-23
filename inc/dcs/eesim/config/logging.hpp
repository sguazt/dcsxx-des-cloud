/**
 * \file dcs/eesim/config/logging.hpp
 *
 * \brief Configuration for the logging subsystem.
 *
 * \author Marco Guazzone (marco.guazzone@gmail.com)
 *
 * <hr/>
 *
 * Copyright (C) 2009-2012  Marco Guazzone (marco.guazzone@gmail.com)
 *                          [Distributed Computing System (DCS) Group,
 *                           Computer Science Institute,
 *                           Department of Science and Technological Innovation,
 *                           University of Piemonte Orientale,
 *                           Alessandria (Italy)]
 *
 * This file is part of dcsxx-des-cloud.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef DCS_EESIM_CONFIG_LOGGING_HPP
#define DCS_EESIM_CONFIG_LOGGING_HPP


#include <boost/variant.hpp>
#include <iosfwd>
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


#endif // DCS_EESIM_CONFIG_LOGGING_HPP
