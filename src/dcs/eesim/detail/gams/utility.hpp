#ifndef DCS_EESIM_DETAIL_GAMS_UTILITY_HPP
#define DCS_EESIM_DETAIL_GAMS_UTILITY_HPP


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
#include <dcs/eesim/optimal_solver_ids.hpp>
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


namespace dcs { namespace eesim { namespace detail { namespace gams {

template <typename ArgsT, typename ProducerT>
bool run_gams_command_producer(::std::string const& cmd,
							   ArgsT const& args,
							   ProducerT& producer)
{

	DCS_DEBUG_ASSERT( !cmd.empty() );

	// Run the GAMS command

	int pipefd[2];
	const ::std::size_t CHILD_RD(0);
	const ::std::size_t PARENT_WR(1);
	if (::pipe(pipefd) == -1)
	{
		char const* err_str(::strerror(errno));
		::std::ostringstream oss;
		oss << "[dcs::eesim::detail::gams::run_gams_command_producer] pipe(2) failed: "
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
		oss << "[dcs::eesim::detail::gams::run_gams_command_producer] fork(2) failed: "
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
				oss << "[dcs::eesim::detail::gams::run_gams_command] getrlimit(2) failed: "
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
			oss << "[dcs::eesim::detail::gams::run_gams_command] getrlimit(2) failed: "
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

		// Associate the child's stdin to the pipe read fds.
		close_fd[STDIN_FILENO] = false;
		close_fd[STDOUT_FILENO] = false;
		if (pipefd[0] != STDIN_FILENO)
		{
			if (::dup2(pipefd[0], STDIN_FILENO) != STDIN_FILENO)
			{
				char const* err_str(::strerror(errno));
				::std::ostringstream oss;
				oss << "[dcs::eesim::detail::gams::run_gams_command_producer] dup2(2) failed: "
					<< err_str;
				throw ::std::runtime_error(oss.str());
			}
		}
		else
		{
			if (pipefd[0] < maxdescs)
			{
				close_fd[pipefd[0]] = false;
			}
			else
			{
				::close(pipefd[0]);
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
::std::cerr << "Executing GAMS: " << cmd;//XXX
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

    ::close(pipefd[0]);
/*
    if (pipefd[1] != STDOUT_FILENO)
    {
        if (::dup2(pipefd[1], STDOUT_FILENO) != STDOUT_FILENO)
        {
            char const* err_str(::strerror(errno));
            ::std::ostringstream oss;
            oss << "[dcs::eesim::detail::matlab::run_matlab] dup2(2) failed: "
                << err_str;
            throw ::std::runtime_error(oss.str());
        }
        ::close(pipefd[1]);
    }
*/

#ifdef __GNUC__
	::__gnu_cxx::stdio_filebuf<char> wrbuf(pipefd[1], ::std::ios::out);
#else // __GNUC__
    typedef ::boost::iostreams::file_descriptor_source fd_device_type;
    typedef ::boost::iostreams::stream_buffer<fd_device_type> fd_streambuf_type;

    //fd_device_type wrdev(STDOUT_FILENO, ::boost::iostreams::close_handle);
	fd_device_type wrfd_src(pipefd[1], ::boost::iostreams::close_handle);
	fd_streambuf_type wrbuf(wrdev);
	::std::ostream os(&wrbuf);
#endif // __GNUC__
	::std::ostream os(&wrbuf);

::std::cerr << "Producing..." << ::std::endl;//XXX
	// Write to the child process
	producer(os);

::std::cerr << "END parsing GAMS output" << ::std::endl;//XXX

    // Wait the child termination (in order to prevent zombies)
    int status;
    if (::waitpid(pid, &status, 0) == -1)
    {
        char const* err_str = ::strerror(errno);
        ::std::ostringstream oss;
        oss << "[dcs::eesim::detail::gams::run_gams_command_producer] waitpid(2) failed: "
            << ::std::string(err_str);
        throw ::std::runtime_error(oss.str());
    }

	bool ok(true);
	if (WIFEXITED(status))
	{
		if (WEXITSTATUS(status))
		{
			// status != 0 --> error in the execution
			::std::clog << "[Warning] GAMS command exited with status " << WEXITSTATUS(status) << ::std::endl;
			ok = false;
		}
	}
	else if (WIFSIGNALED(status))
	{
		::std::clog << "[Warning] GAMS command received signal " << WTERMSIG(status) << ::std::endl;
		ok = false;
	}
	else
	{
		ok = false;
	}

	return ok;
}


template <typename ArgsT, typename ProducerT, typename ConsumerT>
bool run_gams_command(::std::string const& cmd,
					  ArgsT const& args,
					  ProducerT& producer,
					  ConsumerT& consumer)
{

	DCS_DEBUG_ASSERT( !cmd.empty() );

	// Run the GAMS command

	// Create two pipes to let to communicate with GAMS.
	// Specifically, we want to write the input into GAMS (through the producer)
	// and to read the output from GAMS (through the consumer).
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
		oss << "[dcs::eesim::detail::gams::run_gams_command] pipe(2) failed: "
			<< err_str;
		throw ::std::runtime_error(oss.str());
	}
	if (::pipe(&pipefd[2]) == -1)
	{
		char const* err_str(::strerror(errno));
		::std::ostringstream oss;
		oss << "[dcs::eesim::detail::gams::run_gams_command] pipe(2) failed: "
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
		oss << "[dcs::eesim::detail::gams::run_gams_command] fork(2) failed: "
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
				oss << "[dcs::eesim::detail::gams::run_gams_command] getrlimit(2) failed: "
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
			oss << "[dcs::eesim::detail::gams::run_gams_command] getrlimit(2) failed: "
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
				oss << "[dcs::eesim::detail::gams::run_gams_command] dup2(2) failed: "
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
				oss << "[dcs::eesim::detail::gams::run_gams_command] dup2(2) failed: "
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
::std::cerr << "Executing GAMS: " << cmd;//XXX
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

::std::cerr << "END parsing GAMS output" << ::std::endl;//XXX
::std::cerr << "IS state: " << is.good() << " - " << is.eof() << " - " << is.fail() << " - " << is.bad() << ::std::endl;//XXX

    // Wait the child termination (in order to prevent zombies)
    int status;
    if (::waitpid(pid, &status, 0) == -1)
    {
        char const* err_str = ::strerror(errno);
        ::std::ostringstream oss;
        oss << "[dcs::eesim::detail::gams::run_gams_command] waitpid(2) failed: "
            << ::std::string(err_str);
        throw ::std::runtime_error(oss.str());
    }

	bool ok(true);
	if (WIFEXITED(status))
	{
		if (WEXITSTATUS(status))
		{
			// status != 0 --> error in the execution
			::std::clog << "[Warning] GAMS command exited with status " << WEXITSTATUS(status) << ::std::endl;
			ok = false;
		}
	}
	else if (WIFSIGNALED(status))
	{
		::std::clog << "[Warning] GAMS command received signal " << WTERMSIG(status) << ::std::endl;
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
		// look ahead next character

		char ch(iss.peek());
		if (::std::isdigit(ch) || ch == '+' || ch == '-' || ch == '.')
		{
			// Found the beginning of a number
			break;
		}

		// Skip non-numerical chars (e.g., space, letters, ...)
		iss.get();
	}

	if (iss.good())
	{
		iss >> x;
	}
	else
	{
		throw ::std::runtime_error("[dcs::eesim::detail::gams::parse_str] Unable to parse a GAMS number");
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
		// Look for terminating ';'
		if ((rpos = text.rfind(';', rpos)) == ::std::string::npos)
		{
			rpos = sz-1;
		}
		else if (rpos > 0)
		{
			--rpos;
		}
		// trim right
		while (rpos > 0 && ::std::isspace(text[rpos]))
		{
			--rpos;
		}

		if (rpos >= lpos)
		{
			x = text.substr(lpos, rpos-lpos+1);
		}
		else
		{
			throw ::std::runtime_error("[dcs::eesim::detail::gams::parse_str] Unable to parse a GAMS string");
		}
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
			}
			else if (ch == ']')
			{
				// Found the end of the vector
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
			throw ::std::runtime_error("[dcs::eesim::detail::gams::parse_str] Unable to parse a GAMS vector.");
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
			throw ::std::runtime_error("[dcs::eesim::detail::gams::parse_str] Unable to parse a GAMS matrix.");
		}
	}
}


inline
::std::string find_gams_command()
{
	const ::std::string cmd_name("gams");
	return cmd_name;
}


inline
::std::string to_gams_solver(optimal_solver_ids solver_id)
{
	switch (solver_id)
	{
		case alphaecp_optimal_solver_id:
			return "alphaecp";
		case baron_optimal_solver_id:
			return "baron";
		case bdmlp_optimal_solver_id:
			return "bdmlp";
		case bonmin_optimal_solver_id:
			return "bonmin";
		case cbc_optimal_solver_id:
			return "cbc";
		case conopt_optimal_solver_id:
			return "conopt";
		case couenne_optimal_solver_id:
			return "couenne";
		case cplex_optimal_solver_id:
			return "cplex";
//		case decis_optimal_solver_id:
//			return "decism";
		case dicopt_optimal_solver_id:
			return "dicopt";
		case gurobi_optimal_solver_id:
			return "gurobi";
		case ipopt_optimal_solver_id:
			return "ipopt";
		case knitro_optimal_solver_id:
			return "knitro";
		case lgo_optimal_solver_id:
			return "lgo";
		case lindoglobal_optimal_solver_id:
			return "lindoglobal";
		case minos_optimal_solver_id:
			return "minos";
		case mosek_optimal_solver_id:
			return "mosek";
//		case msnlp_optimal_solver_id:
//			return "msnlp";
		case nlpec_optimal_solver_id:
			return "nlpec";
//		case oqnlp_optimal_solver_id:
//			return "oqnlp";
//		case osl_optimal_solver_id:
//			return "osl";
//		case os_optimal_solver_id:
//			return "os";
//		case osicplex_optimal_solver_id:
//			return "osicplex";
//		case osiglpk_optimal_solver_id:
//			return "osiglpk";
//		case osigurobi_optimal_solver_id:
//			return "osigurobi";
//		case osimosek_optimal_solver_id:
//			return "osimosek";
//		case osisoplex_optimal_solver_id:
//			return "osisoplex";
//		case osixpress_optimal_solver_id:
//			return "osixpress";
		case path_optimal_solver_id:
			return "path";
		case pathnlp_optimal_solver_id:
			return "pathnlp";
		case sbb_optimal_solver_id:
			return "sbb";
		case scip_optimal_solver_id:
			return "scip";
		case snopt_optimal_solver_id:
			return "snopt";
//		case xa_optimal_solver_id:
//			return "xa";
		case xpressmp_optimal_solver_id:
			return "xpress";
		default:
			break;
	}

	throw ::std::runtime_error("[dcs::eesim::detail::gams::to_gams_solver] Solver not usable from GAMS.");
}

}}}} // Namespace dcs::eesim::detail::gams

#endif // DCS_EESIM_DETAIL_GAMS_UTILITY_HPP
