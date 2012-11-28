#ifndef DCS_DES_CLOUD_CONFIG_OPERATION_MAKE_LOGGER_HPP
#define DCS_DES_CLOUD_CONFIG_OPERATION_MAKE_LOGGER_HPP


#include <boost/variant.hpp>
#include <dcs/des/cloud/config/configuration.hpp>
#include <dcs/des/cloud/config/logging.hpp>
#include <dcs/des/cloud/logging/base_logger.hpp>
#include <dcs/des/cloud/logging/compact_logger.hpp>
#include <dcs/des/cloud/logging/dummy_logger.hpp>
#include <dcs/des/cloud/logging/minimal_logger.hpp>
#include <dcs/memory.hpp>
#include <iosfwd>
#include <stdexcept>


namespace dcs { namespace des { namespace cloud { namespace config {

template <typename TraitsT, typename RealT, typename UIntT>
::dcs::shared_ptr< ::dcs::des::cloud::logging::base_logger<TraitsT> > make_logger(configuration<RealT,UIntT> const& conf)
{
	typedef TraitsT traits_type;
	typedef ::dcs::des::cloud::logging::base_logger<TraitsT> logging_type;
	typedef configuration<RealT,UIntT> config_type;
	typedef typename config_type::logging_config_type logging_config_type;

	::dcs::shared_ptr<logging_type> ptr_logger;

	if (conf.logging().enabled)
	{
		switch (conf.logging().category)
		{
			case compact_logging:
				{
					typedef ::dcs::des::cloud::logging::compact_logger<traits_type> logger_impl_type;
					typedef typename logging_config_type::compact_logging_type logging_conf_impl_type;

					logging_conf_impl_type const& logging_conf_impl = ::boost::get<logging_conf_impl_type>(conf.logging().category_conf);

					ptr_logger = ::dcs::make_shared<logger_impl_type>();

					switch (logging_conf_impl.sink.category)
					{
						case console_logging_sink:
							{
								typedef console_logging_sink_config sink_impl_type; 

								sink_impl_type const& sink_impl = ::boost::get<sink_impl_type>(logging_conf_impl.sink.category_conf);

								switch (sink_impl.stream)
								{
									case stdout_stream_logging_sink:
										ptr_logger->sink(&::std::cout);
										break;
									case stderr_stream_logging_sink:
										ptr_logger->sink(&::std::cerr);
										break;
									case stdlog_stream_logging_sink:
										ptr_logger->sink(&::std::clog);
										break;
								}
							}
							break;
						case file_logging_sink:
							{
								typedef file_logging_sink_config sink_impl_type; 

								sink_impl_type const& sink_impl = ::boost::get<sink_impl_type>(logging_conf_impl.sink.category_conf);

								if (sink_impl.name.empty())
								{
									throw ::std::runtime_error("[dcs::des::cloud::config::make_logger] File name not specified for the file sink.");
								}

								ptr_logger->sink(sink_impl.name);
							}
							break;
					}
				}
				break;
			case minimal_logging:
				{
					typedef ::dcs::des::cloud::logging::minimal_logger<traits_type> logger_impl_type;
					typedef typename logging_config_type::minimal_logging_type logging_conf_impl_type;

					logging_conf_impl_type const& logging_conf_impl = ::boost::get<logging_conf_impl_type>(conf.logging().category_conf);

					ptr_logger = ::dcs::make_shared<logger_impl_type>();

					switch (logging_conf_impl.sink.category)
					{
						case console_logging_sink:
							{
								typedef console_logging_sink_config sink_impl_type; 

								sink_impl_type const& sink_impl = ::boost::get<sink_impl_type>(logging_conf_impl.sink.category_conf);

								switch (sink_impl.stream)
								{
									case stdout_stream_logging_sink:
										ptr_logger->sink(&::std::cout);
										break;
									case stderr_stream_logging_sink:
										ptr_logger->sink(&::std::cerr);
										break;
									case stdlog_stream_logging_sink:
										ptr_logger->sink(&::std::clog);
										break;
								}
							}
							break;
						case file_logging_sink:
							{
								typedef file_logging_sink_config sink_impl_type; 

								sink_impl_type const& sink_impl = ::boost::get<sink_impl_type>(logging_conf_impl.sink.category_conf);

								if (sink_impl.name.empty())
								{
									throw ::std::runtime_error("[dcs::des::cloud::config::make_logger] File name not specified for the file sink.");
								}

								ptr_logger->sink(sink_impl.name);
							}
							break;
					}
				}
				break;
		}
	}
	else
	{
		// Create a dummy logger in case no logging is enabled.

		typedef ::dcs::des::cloud::logging::dummy_logger<traits_type> logger_impl_type;
		ptr_logger = ::dcs::make_shared<logger_impl_type>();
	}

	return ptr_logger;
}

}}}} // Namespace dcs::des::cloud::config


#endif // DCS_DES_CLOUD_CONFIG_OPERATION_MAKE_LOGGER_HPP
