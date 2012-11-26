/**
 * \file dcs/eesim/detail/matlab/utility.hpp
 *
 * \brief Utilities to interface with the MATLAB environment.
 *
 * Copyright (C) 2009-2012  Distributed Computing System (DCS) Group, Computer
 * Science Department - University of Piemonte Orientale, Alessandria (Italy).
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
 *
 * \author Marco Guazzone (marco.guazzone@gmail.com)
 */

#ifndef DCS_EESIM_DETAIL_MATLAB_UTILITY_HPP
#define DCS_EESIM_DETAIL_MATLAB_UTILITY_HPP


//FIXME: It seems this is not the best way to check for a POSIX-compliant system
//#if _POSIX_C_SOURCE < 1 && !_XOPEN_SOURCE && !_POSIX_SOURCE
//#	error "Unable to find a POSIX compliant system."
//#endif // _POSIX_C_SOURCE


#include <boost/numeric/ublas/expression_types.hpp>
#include <boost/numeric/ublas/traits.hpp>
#include <boost/numeric/ublasx/operation/num_columns.hpp>
#include <boost/numeric/ublasx/operation/num_rows.hpp>
#include <boost/numeric/ublasx/operation/size.hpp>
#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/iostreams/stream_buffer.hpp>
#include <cctype>
#include <cerrno>
//#include <csignal>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <dcs/debug.hpp>
#include <fcntl.h>
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


namespace dcs { namespace eesim { namespace detail { namespace matlab {

template <typename ValueT>
static ::std::string to_str(::std::vector<ValueT> const& v, bool column = true)
{
	typedef ::std::vector<ValueT> vector_type;
	typedef typename vector_type::size_type size_type;

	size_type n(v.size());
	::std::ostringstream oss;
	oss << "[";
	for (size_type i = 0; i < n; ++i)
	{
		if (i > 0)
		{
			if (column)
			{
				oss << ";";
			}
			else
			{
				oss << " ";
			}
		}
		oss << v[i];
	}
	oss << "]";

	return oss.str();
}


template <typename VectorExprT>
::std::string to_str(::boost::numeric::ublas::vector_expression<VectorExprT> const& v, bool column = true)
{
	namespace ublas = ::boost::numeric::ublas;
	namespace ublasx = ::boost::numeric::ublasx;

	typedef VectorExprT vector_type;
	typedef typename ublas::vector_traits<vector_type>::size_type size_type;

	size_type n(ublasx::size(v));
	::std::ostringstream oss;
	oss << "[";
	for (size_type i = 0; i < n; ++i)
	{
		if (i > 0)
		{
			if (column)
			{
				oss << ";";
			}
			else
			{
				oss << " ";
			}
		}
		oss << v()(i);
	}
	oss << "]";

	return oss.str();
}


template <typename MatrixExprT>
::std::string to_str(::boost::numeric::ublas::matrix_expression<MatrixExprT> const& A)
{
	namespace ublas = ::boost::numeric::ublas;
	namespace ublasx = ::boost::numeric::ublasx;

	typedef MatrixExprT matrix_type;
	typedef typename ublas::matrix_traits<matrix_type>::size_type size_type;

	size_type nr(ublasx::num_rows(A));
	size_type nc(ublasx::num_columns(A));
	::std::ostringstream oss;
	oss << "[";
	for (size_type r = 0; r < nr; ++r)
	{
		if (r > 0)
		{
			oss << ";";
		}
		for (size_type c = 0; c < nc; ++c)
		{
			if (c > 0)
			{
				oss << " ";
			}
			oss << A()(r,c);
		}
	}
	oss << "]";

	return oss.str();
}


template <typename ArgsT, typename ConsumerT>
bool run_matlab_command(::std::string const& cmd,
						ArgsT const& args,
						ConsumerT& consumer)
{
	int pipefd[2];

	// Create a pipe to let to communicate with MATLAB.
	// Specifically, we want to read the output from MATLAB.
	// So, the parent process read from the pipe, while the child process
	// write on it.
	if (::pipe(pipefd) == -1)
	{
		char const* err_str = ::strerror(errno);
		::std::ostringstream oss;
		oss << "[dcs::eesim::detail::matlab::run_matlab_command] pipe(2) failed: "
			<< ::std::string(err_str);
		throw ::std::runtime_error(oss.str());
	}

//	// Install signal handlers
//	struct ::sigaction sig_act;
//	struct ::sigaction old_sigterm_act;
//	struct ::sigaction old_sigint_act;
//	//::memset(&sig_act, 0, sizeof(sig_act));
//	::sigemptyset(&sig_act.sa_mask);
//	sig_act.sa_flags = 0;
//	sig_act.sa_handler = self_type::process_signals;
//	::sigaction(SIGTERM, &sig_act, &old_sigterm_act);
//	::sigaction(SIGINT, &sig_act, &old_sigint_act);

	// Spawn a new process

	// Between fork() and execve() only async-signal-safe functions
	// must be called if multithreaded applications should be supported.
	// That's why the following code is executed before fork() is called.

	::pid_t pid = ::fork();

	// check: pid == -1 --> error
	if (pid == -1)
	{
		char const* err_str = ::strerror(errno);
		::std::ostringstream oss;
		oss << "[dcs::eesim::detail::matlab::run_matlab_command] fork(2) failed: "
			<< ::std::string(err_str);
		throw ::std::runtime_error(oss.str());
	}

	if (pid == 0)
	{
		// The child

//		// Cancel signal handler set for parent
//		sig_act.sa_handler = SIG_DFL;
//		::sigaction(SIGTERM, &sig_act, 0);
//		::sigaction(SIGINT, &sig_act, 0);

		// Get the maximum number of files this process is allowed to open
#if defined(F_MAXFD)
		int maxdescs = ::fcntl(-1, F_MAXFD, 0);
		if (maxdescs == -1)
		{
#if defined(_SC_OPEN_MAX)
			maxdescs = ::sysconf(_SC_OPEN_MAX);
#else
			::rlimit limit;
			if (::getrlimit(RLIMIT_NOFILE, &limit) < 0)
			{
				char const* err_str = ::strerror(errno);
				::std::ostringstream oss;
				oss << "[dcs::eesim::detail::matlab::run_matlab_command] getrlimit(2) failed: "
					<< ::std::string(err_str);
				throw ::std::runtime_error(oss.str());
			}
			maxdescs = limit.rlim_cur;
#endif // _SC_OPEN_MAX
		}
#else // F_MAXFD
#if defined(_SC_OPEN_MAX)
		int maxdescs = ::sysconf(_SC_OPEN_MAX);
#else // _SC_OPEN_MAX
		::rlimit limit;
		if (::getrlimit(RLIMIT_NOFILE, &limit) < 0)
		{
			char const* err_str = ::strerror(errno);
			::std::ostringstream oss;
			oss << "[dcs::eesim::detail::matlab::run_matlab_command] getrlimit(2) failed: "
				<< ::std::string(err_str);
			throw ::std::runtime_error(oss.str());
		}
		maxdescs = limit.rlim_cur;
#endif // _SC_OPEN_MAX
#endif // F_MAXFD
		if (maxdescs == -1)
		{
			maxdescs = 1024;
		}

		::std::vector<bool> close_fd(maxdescs, true);

		// Associate the child's stdout to the pipe write fd.
		close_fd[STDOUT_FILENO] = false;
		if (pipefd[1] != STDOUT_FILENO)
		{
			if (::dup2(pipefd[1], STDOUT_FILENO) != STDOUT_FILENO)
			{
				char const* err_str = ::strerror(errno);
				::std::ostringstream oss;
				oss << "[dcs::eesim::detail::matlab::run_matlab_command] dup2(2) failed: "
					<< ::std::string(err_str);
				throw ::std::runtime_error(oss.str());
			}
		}
		else
		{
			close_fd[pipefd[1]] = false;
		}
//			::close(STDOUT_FILENO);
//			::dup(pipefd[1]);

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

		::std::size_t nargs = args.size()+1;
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

		//char** envp(0);

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

//[XXX]
#ifdef DCS_DEBUG
::std::cerr << "Executing MATLAB: " << cmd;//XXX
for (::std::size_t i=0; i < args.size(); ++i)//XXX
{//XXX
::std::cerr << " " << args[i] << ::std::flush;//XXX
}//XXX
::std::cerr << ::std::endl;//XXX
#endif // DCS_DEBUG
//[/XXX]
//DCS_DEBUG_TRACE("Executing: " << cmd << " " << args[0] << " " << args[1] << " " << args[2] << " - " << args[3]);

		//::execve(cmd.c_str(), argv, envp);
		::execvp(cmd.c_str(), argv);

		// Actually we should delete argv and envp data. As we must not
		// call any non-async-signal-safe functions though we simply exit.
		::write(STDERR_FILENO, "execvp() failed\n", 17);
		//_exit(EXIT_FAILURE);
		_exit(127);
	}

	// The parent

//		// Associate the parent's stdin to the pipe read fd.
	::close(pipefd[1]);
//		::close(STDIN_FILENO);
//		::dup(pipefd[0]);
	if (pipefd[0] != STDIN_FILENO)
	{
		if (::dup2(pipefd[0], STDIN_FILENO) != STDIN_FILENO)
		{
			char const* err_str = ::strerror(errno);
			::std::ostringstream oss;
			oss << "[dcs::eesim::detail::matlab::run_matlab_command] dup2(2) failed: "
				<< ::std::string(err_str);
			throw ::std::runtime_error(oss.str());
		}
		::close(pipefd[0]);
	}

	typedef ::boost::iostreams::file_descriptor_source fd_device_type;
	typedef ::boost::iostreams::stream_buffer<fd_device_type> fd_streambuf_type;
	//fd_device_type fd_src(pipefd[0], ::boost::iostreams::close_handle);
	fd_device_type fd_src(STDIN_FILENO, ::boost::iostreams::close_handle);
	fd_streambuf_type fd_buf(fd_src);
	::std::istream is(&fd_buf);

	consumer(is);

DCS_DEBUG_TRACE("END parsing MATLAB output");//XXX
DCS_DEBUG_TRACE("IS state: " << is.good() << " - " << is.eof() << " - " << is.fail() << " - " << is.bad());//XXX

	// Wait the child termination (in order to prevent zombies)
	int status;
//		::pid_t wait_pid;
//		wait_pid = ::wait(&status);
//		if (wait_pid != pid)
//		{
//			throw ::std::runtime_error("[dcs::eesim::detail::rls_miso_matlab_app_proxy::run_matlab] Unexpected child process.");
//		}
	if (::waitpid(pid, &status, 0) == -1)
	{
		char const* err_str = ::strerror(errno);
		::std::ostringstream oss;
		oss << "[dcs::eesim::detail::matlab::run_matlab_command] waitpid(2) failed: "
			<< ::std::string(err_str);
		throw ::std::runtime_error(oss.str());
	}
DCS_DEBUG_TRACE("MATLAB exited");//XXX
	bool ok(true);
	if (WIFEXITED(status))
	{
DCS_DEBUG_TRACE("MATLAB exited with a call to 'exit(" << WEXITSTATUS(status) << ")'");//XXX
		if (WEXITSTATUS(status))
		{
			// status != 0 --> error in the execution
			::std::clog << "[Warning] MATLAB command exited with status " << WEXITSTATUS(status) << ::std::endl;
			ok = false;
		}
	}
	else if (WIFSIGNALED(status))
	{
DCS_DEBUG_TRACE("MATLAB exited with a call to 'kill(" << WTERMSIG(status) << ")'");//XXX
	   ::std::clog << "[Warning] MATLAB command received signal " << WTERMSIG(status) << ::std::endl;
		ok = false;
	}
	else
	{
DCS_DEBUG_TRACE("MATLAB exited with an unexpected way");//XXX
		ok = false;
	}

//	// Restore signal handler
//	::sigaction(SIGTERM, &old_sigterm_act, 0);
//	::sigaction(SIGINT, &old_sigint_act, 0);
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
			throw ::std::runtime_error("[dcs::eesim::detail::matlab::parse_str] Unable to parse a MATLAB number");
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
			throw ::std::runtime_error("[dcs::eesim::detail::matlab::parse_str] Unable to parse a MATLAB vector.");
		}
	}
}


template <typename T>
void parse_str(::std::string const& text, ::boost::numeric::ublas::matrix<T>& A)
{
	typename ::boost::numeric::ublas::matrix<T>::size_type r(0);
	typename ::boost::numeric::ublas::matrix<T>::size_type c(0);
	typename ::boost::numeric::ublas::matrix<T>::size_type nc(0);

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
			throw ::std::runtime_error("[dcs::eesim::detail::matlab::parse_str] Unable to parse a MATLAB matrix.");
		}
	}
}


inline
::std::string find_matlab_command()
{
	const ::std::string cmd_name("matlab");
	return cmd_name;
}

}}}} // Namespace dcs::eesim::detail::matlab

#endif // DCS_EESIM_DETAIL_MATLAB_UTILITY_HPP
