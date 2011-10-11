#ifndef DCS_EESIM_DETAIL_SYSTEM_UTILITY_HPP
#define DCS_EESIM_DETAIL_SYSTEM_UTILITY_HPP


#include <cstdlib>
#include <fstream>
#include <string>
#include <unistd.h>
#include <vector>


namespace dcs { namespace eesim { namespace detail {

::std::string make_tmp_file(::std::string name)
{
    name += "XXXXXX";
    ::std::vector<char> tmpl(name.begin(), name.end());
    tmpl.push_back('\0');

    int fd = ::mkstemp(&tmpl[0]);
    if (fd != -1)
    {
        name.assign(tmpl.begin(), tmpl.end()-1);
        ::close(fd);
    }

    return name;
}


inline
::std::string make_tmp_file()
{
	return make_tmp_file(::std::string("tmpXXXXXX"));
}


::std::string make_tmp_file(::std::string name, ::std::ofstream& of)
{
    name += "XXXXXX";
    ::std::vector<char> tmpl(name.begin(), name.end());
    tmpl.push_back('\0');

    int fd = ::mkstemp(&tmpl[0]);
    if (fd != -1)
    {
        name.assign(tmpl.begin(), tmpl.end()-1);
        of.open(name.c_str(), ::std::ios_base::trunc | ::std::ios_base::out);
        ::close(fd);
    }

    return name;
}


inline
::std::string make_tmp_file(::std::string path, ::std::string name, ::std::ofstream& of)
{
    path += "/" + name;

    return make_tmp_file(path, of);
}

}}} // Namespace dcs::eesim::detail


#endif // DCS_EESIM_DETAIL_SYSTEM_UTILITY_HPP
