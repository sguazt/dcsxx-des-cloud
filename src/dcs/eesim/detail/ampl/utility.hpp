#ifndef DCS_EESIM_DETAIL_AMPL_UTILITY_HPP
#define DCS_EESIM_DETAIL_AMPL_UTILITY_HPP


#ifdef __GNUC__
# include <ext/stdio_filebuf.h>
#else // __GNUC__
# include <boost/iostreams/device/file_descriptor.hpp>
# include <boost/iostreams/stream_buffer.hpp>
#endif // __GNUC__
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <cctype>
#include <cerrno>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <dcs/debug.hpp>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>


namespace dcs { namespace eesim { namespace detail { namespace ampl {

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

::std::string make_tmp_file(::std::string path, ::std::string name, ::std::ofstream& of)
{
	path += "/" + name;

	return make_tmp_file(path, of);
}

template <typename ArgsT, typename ProducerT, typename ConsumerT>
bool run_ampl_command(::std::string const& cmd,
					  ArgsT const& args,
					  ProducerT& producer,
					  ConsumerT& consumer)
{

	DCS_DEBUG_ASSERT( !cmd.empty() );

	// Run the AMPL command

	// Create two pipes to let to communicate with AMPL.
	// Specifically, we want to write the input into AMPL (through the producer)
	// and to read the output from AMPL (through the consumer).
	// So, the child process read its input from the parent and write its output on
	// the pipe; while the parent write the child's input on the pipe and read its
	// input from the child.

	// pipefd:
	// - [0,1]: Where the parent write to and the child read from.
	// - [2,3]: Where the parent read from and the child write to.
	int pipefd[4];
	const ::std::size_t CHILD_RD(0);
	const ::std::size_t PARENT_WR(1);
	const ::std::size_t PARENT_RD(2);
	const ::std::size_t CHILD_WR(3);
	if (::pipe(&pipefd[0]) == -1)
	{
		char const* err_str(::strerror(errno));
		::std::ostringstream oss;
		oss << "[dcs::eesim::detail::ampl::run_ampl_command] pipe(2) failed: "
			<< err_str;
		throw ::std::runtime_error(oss.str());
	}
	if (::pipe(&pipefd[2]) == -1)
	{
		char const* err_str(::strerror(errno));
		::std::ostringstream oss;
		oss << "[dcs::eesim::detail::ampl::run_ampl_command] pipe(2) failed: "
			<< err_str;
		throw ::std::runtime_error(oss.str());
	}

	// Spawn a new process

	// Between fork() and execve() only async-signal-safe functions
	// must be called if multithreaded applications should be supported.
	// That's why the following code is executed before fork() is called.

	::pid_t pid = ::fork();

	// check: pid == -1 --> error
	if (pid == -1)
	{
		char const* err_str(::strerror(errno));
		::std::ostringstream oss;
		oss << "[dcs::eesim::detail::ampl::run_ampl_command] fork(2) failed: "
			<< err_str;
		throw ::std::runtime_error(oss.str());
	}

	if (pid == 0)
	{
		// The child

		// Get the maximum number of files this process is allowed to open
#if defined(F_MAXFD)
		int maxdescs = ::fcntl(-1, F_MAXFD, 0);
		if (maxdescs == -1)
		{
# if defined(_SC_OPEN_MAX)
			maxdescs = ::sysconf(_SC_OPEN_MAX);
# else // _SC_OPEN_MAX
			::rlimit limit;
			if (::getrlimit(RLIMIT_NOFILE, &limit) < 0)
			{
				char const* err_str = ::strerror(errno);
				::std::ostringstream oss;
				oss << "[dcs::eesim::detail::ampl::run_ampl_command] getrlimit(2) failed: "
					<< ::std::string(err_str);
				throw ::std::runtime_error(oss.str());
			}
			maxdescs = limit.rlim_cur;
# endif // _SC_OPEN_MAX
		}
#else // F_MAXFD
# if defined(_SC_OPEN_MAX)
		int maxdescs = ::sysconf(_SC_OPEN_MAX);
# else // _SC_OPEN_MAX
		::rlimit limit;
		if (::getrlimit(RLIMIT_NOFILE, &limit) < 0)
		{
			char const* err_str = ::strerror(errno);
			::std::ostringstream oss;
			oss << "[dcs::eesim::detail::ampl::run_ampl_command] getrlimit(2) failed: "
				<< ::std::string(err_str);
			throw ::std::runtime_error(oss.str());
		}
		maxdescs = limit.rlim_cur;
# endif // _SC_OPEN_MAX
#endif // F_MAXFD
		if (maxdescs == -1)
		{
			maxdescs = 1024;
		}

		::std::vector<bool> close_fd(maxdescs, true);

		// Associate the child's stdin/stdout to the pipe read/write fds.
		close_fd[STDIN_FILENO] = false;
		close_fd[STDOUT_FILENO] = false;
		if (pipefd[CHILD_WR] != STDOUT_FILENO)
		{
			if (::dup2(pipefd[CHILD_WR], STDOUT_FILENO) != STDOUT_FILENO)
			{
				char const* err_str(::strerror(errno));
				::std::ostringstream oss;
				oss << "[dcs::eesim::detail::ampl::run_ampl_command] dup2(2) failed: "
					<< err_str;
				throw ::std::runtime_error(oss.str());
			}
		}
		else
		{
			if (pipefd[CHILD_WR] < maxdescs)
			{
				close_fd[pipefd[CHILD_WR]] = false;
			}
			else
			{
				::close(pipefd[CHILD_WR]);
			}
		}
		if (pipefd[CHILD_RD] != STDIN_FILENO)
		{
			if (::dup2(pipefd[CHILD_RD], STDIN_FILENO) != STDIN_FILENO)
			{
				char const* err_str(::strerror(errno));
				::std::ostringstream oss;
				oss << "[dcs::eesim::detail::ampl::run_ampl_command] dup2(2) failed: "
					<< err_str;
				throw ::std::runtime_error(oss.str());
			}
		}
		else
		{
			if (pipefd[CHILD_RD] < maxdescs)
			{
				close_fd[pipefd[CHILD_RD]] = false;
			}
			else
			{
				::close(pipefd[CHILD_RD]);
			}
		}

		// Check if the command already has path information
		::std::string cmd_path;
		::std::string cmd_name;
		typename ::std::string::size_type pos;
		pos = cmd.find_last_of('/');
		if (pos != ::std::string::npos)
		{
			cmd_path = cmd.substr(0, pos);
			cmd_name = cmd.substr(pos+1);
		}

		//FIXME: use scoped_ptr in place of "new"

		::std::size_t nargs(args.size()+1);
		char** argv = new char*[nargs + 2];
		argv[0] = new char[cmd_name.size()+1];
		::std::strncpy(argv[0], cmd_name.c_str(), cmd_name.size()+1); // by convention, the first argument is always the command name
		typename ArgsT::size_type i(1);
		typename ArgsT::const_iterator end_it(args.end());
		for (typename ArgsT::const_iterator it = args.begin(); it != end_it; ++it)
		{
			argv[i] = new char[it->size()+1];
			::std::strncpy(argv[i], it->c_str(), it->size()+1);
			++i;
		}
		argv[nargs] = 0;

		// Close unused file descriptors
#ifdef DCS_DEBUG
		// Keep standard error open for debug
		close_fd[STDERR_FILENO] = false;
#endif // DCS_DEBUG
		for (int fd = 0; fd < maxdescs; ++fd)
		{
			if (close_fd[fd])
			{
				::close(fd);
			}
		}
#ifdef DCS_DEBUG
::std::cerr << "Executing AMPL: " << cmd;//XXX
for (::std::size_t i=0; i < args.size(); ++i)//XXX
{//XXX
::std::cerr << " " << args[i] << ::std::flush;//XXX
}//XXX
::std::cerr << ::std::endl;//XXX
#endif // DCS_DEBUG

		::execvp(cmd.c_str(), argv);
		// Actually we should delete argv and envp data. As we must not
		//         // call any non-async-signal-safe functions though we simply exit.
		::write(STDERR_FILENO, "execvp() failed\n", 17);
		_exit(127);
	}

	// The parent

    ::close(pipefd[CHILD_RD]);
    ::close(pipefd[CHILD_WR]);
/*
    if (pipefd[PARENT_WR] != STDOUT_FILENO)
    {
        if (::dup2(pipefd[PARENT_WR], STDOUT_FILENO) != STDOUT_FILENO)
        {
            char const* err_str(::strerror(errno));
            ::std::ostringstream oss;
            oss << "[dcs::eesim::detail::matlab::run_matlab] dup2(2) failed: "
                << err_str;
            throw ::std::runtime_error(oss.str());
        }
        ::close(pipefd[PARENT_WR]);
    }
    if (pipefd[PARENT_RD] != STDIN_FILENO)
    {
        if (::dup2(pipefd[PARENT_RD], STDIN_FILENO) != STDIN_FILENO)
        {
            char const* err_str(::strerror(errno));
            ::std::ostringstream oss;
            oss << "[dcs::eesim::detail::matlab::run_matlab] dup2(2) failed: "
                << err_str;
            throw ::std::runtime_error(oss.str());
        }
        ::close(pipefd[PARENT_RD]);
    }
*/

#ifdef __GNUC__
	::__gnu_cxx::stdio_filebuf<char> rdbuf(pipefd[PARENT_RD], ::std::ios::in);
	::__gnu_cxx::stdio_filebuf<char> wrbuf(pipefd[PARENT_WR], ::std::ios::out);
#else // __GNUC__
    typedef ::boost::iostreams::file_descriptor_source fd_device_type;
    typedef ::boost::iostreams::stream_buffer<fd_device_type> fd_streambuf_type;

	//fd_device_type rddev(STDIN_FILENO, ::boost::iostreams::close_handle);
	fd_device_type rddev(pipefd[PARENT_RD], ::boost::iostreams::close_handle);
	fd_streambuf_type rdbuf(rddev);
	::std::istream is(&rdbuf);

    //fd_device_type wrdev(STDOUT_FILENO, ::boost::iostreams::close_handle);
	fd_device_type wrfd_src(pipefd[PARENT_WR], ::boost::iostreams::close_handle);
	fd_streambuf_type wrbuf(wrdev);
	::std::ostream os(&wrbuf);
#endif // __GNUC__
	::std::istream is(&rdbuf);
	::std::ostream os(&wrbuf);

::std::cerr << "Producing..." << ::std::endl;//XXX
	// Write to the child process
	producer(os);

::std::cerr << "Consuming..." << ::std::endl;//XXX
	// Read the input from the child process
    consumer(is);

::std::cerr << "END parsing AMPL output" << ::std::endl;//XXX
::std::cerr << "IS state: " << is.good() << " - " << is.eof() << " - " << is.fail() << " - " << is.bad() << ::std::endl;//XXX

    // Wait the child termination (in order to prevent zombies)
    int status;
    if (::waitpid(pid, &status, 0) == -1)
    {
        char const* err_str = ::strerror(errno);
        ::std::ostringstream oss;
        oss << "[dcs::eesim::detail::ampl::run_ampl_command] waitpid(2) failed: "
            << ::std::string(err_str);
        throw ::std::runtime_error(oss.str());
    }

	bool ok(true);
	if (WIFEXITED(status))
	{
		if (WEXITSTATUS(status))
		{
			// status != 0 --> error in the execution
			::std::clog << "[Warning] AMPL command exited with status " << WEXITSTATUS(status) << ::std::endl;
			ok = false;
		}
	}
	else if (WIFSIGNALED(status))
	{
		::std::clog << "[Warning] AMPL command received signal " << WTERMSIG(status) << ::std::endl;
		ok = false;
	}
	else
	{
		ok = false;
	}

	return ok;
}

template <typename T>
void parse_str(::std::string const& text, T& x)
{
	::std::istringstream iss(text);
	while (iss.good())
	{
		char ch(iss.peek());

		if (::std::isspace(ch))
		{
			// Skip space
			iss.get();
		}
		else if (::std::isdigit(ch) || ch == '+' || ch == '-' || ch == '.')
		{
			// Found the beginning of a number

			iss >> x;
		}
		else
		{
			throw ::std::runtime_error("[dcs::eesim::detail::ampl::parse_str] Unable to parse a AMPL number");
		}
	}
}


void parse_str(::std::string const& text, ::std::string& x)
{
	::std::size_t sz(text.size());

	if (sz == 0)
	{
		x = ::std::string();
	}
	else
	{
		::std::size_t lpos(0);
		::std::size_t rpos(sz-1);

		// trim left
		while (lpos < sz && ::std::isspace(text[lpos]))
		{
			++lpos;
		}
		// trim right
		while (rpos >= 0 && ::std::isspace(text[rpos]))
		{
			--rpos;
		}

		x = text.substr(lpos, rpos-lpos+1);
	}
}


template <typename T>
void parse_str(::std::string const& text, ::boost::numeric::ublas::vector<T>& v)
{
	typename ::boost::numeric::ublas::vector<T>::size_type n(0);

	::std::istringstream iss(text);
	bool inside(false);
	bool done(false);
	while (iss.good() && !done)
	{
		char ch(iss.peek());
		bool ko(false);

		if (inside)
		{
			if (::std::isspace(ch) || ch == ';')
			{
				// Found an element separator
				iss.get();
//					while (iss.good() && (ch = iss.peek()) && ::std::isspace(ch))
//					{
//						iss.get();
//					}
			}
			else if (ch == ']')
			{
				// Found the end of the vector
//					iss.get();
//					inside = false;
				done = true;
			}
			else if (::std::isdigit(ch) || ch == '+' || ch == '-' || ch == '.')
			{
				// Found the beginning of a number
				T x;
				iss >> x;
				v.resize(n+1, true);
				v(n) = x;
				++n;
			}
			else
			{
				ko = true;
			}
		}
		else
		{
			if (ch == '[')
			{
				iss.get();
				v.resize(0, false);
				inside = true;
			}
			else if (::std::isspace(ch))
			{
				iss.get();
			}
			else
			{
				ko = true;
			}
		}

		if (ko)
		{
			throw ::std::runtime_error("[dcs::eesim::detail::ampl::parse_str] Unable to parse a AMPL vector.");
		}
	}
}


template <typename T>
void parse_str(::std::string const& text, ::boost::numeric::ublas::matrix<T>& A)
{
	typedef typename ::boost::numeric::ublas::matrix<T>::size_type size_type;

	size_type r(0);
	size_type c(0);
	size_type nc(0);

	::std::istringstream iss(text);
	bool inside(false);
	bool done(false);
	while (iss.good() && !done)
	{
		char ch(iss.peek());
		bool ko(false);

		if (inside)
		{
			if (::std::isspace(ch))
			{
				// Found a column separator
				iss.get();
//					while (iss.good() && (ch = iss.peek()) && ::std::isspace(ch))
//					{
//						iss.get();
//					}
			}
			else if (ch == ';')
			{
				// Found a row separator
				iss.get();
				++r;
				c = 0;
			}
			else if (ch == ']')
			{
				// Found the end of the matrix
//					iss.get();
//					inside = false;
				done = true;
			}
			else if (::std::isdigit(ch) || ch == '+' || ch == '-' || ch == '.')
			{
				// Found the beginning of a number
				T x;
				iss >> x;
				if (nc <= c)
				{
					nc = c+1;
				}
				A.resize(r+1, nc, true);
				A(r,c) = x;
				++c;
			}
			else
			{
				ko = true;
			}
		}
		else
		{
			// Note: outside of a matrix, only two types of character are
			// allowed: spaces and '['

			if (ch == '[')
			{
				iss.get();
				A.resize(0, 0, false);
				inside = true;
			}
			else if (::std::isspace(ch))
			{
				iss.get();
			}
			else
			{
				ko = true;
			}
		}

		if (ko)
		{
			throw ::std::runtime_error("[dcs::eesim::detail::ampl::parse_str] Unable to parse a AMPL matrix.");
		}
	}
}


inline
::std::string find_ampl_command()
{
	const ::std::string cmd_name("ampl");
	return cmd_name;
}

}}}} // Namespace dcs::eesim::detail::ampl

#endif // DCS_EESIM_DETAIL_AMPL_UTILITY_HPP
