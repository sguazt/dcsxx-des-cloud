#ifndef DCS_EESIM_DETAIL_NEOS_CLIENT_HPP
#define DCS_EESIM_DETAIL_NEOS_CLIENT_HPP


#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <cstddef>
#include <dcs/assert.hpp>
#include <dcs/debug.hpp>
#include <dcs/eesim/detail/neos/base64.hpp>
#include <dcs/eesim/optimal_solver_categories.hpp>
#include <dcs/eesim/optimal_solver_ids.hpp>
#include <dcs/eesim/optimal_solver_input_methods.hpp>
#include <dcs/string/algorithm/to_lower.hpp>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unistd.h>
#include <xmlrpc-c/base.hpp>
#include <xmlrpc-c/client_simple.hpp>


namespace dcs { namespace eesim { namespace detail { namespace neos {

enum job_statuses
{
	done_job_status,
	running_job_status,
	waiting_job_status,
	unknown_job_job_status,
	bad_password_job_status
};


/*
enum solver_categories
{
	bco_solver_category, ///< Bound Constrained Optimization
	co_solver_category, ///< Combinatorial Optimization and Integer Programming
	cp_solver_category, ///< Complementarity Problems
	go_solver_category, ///< Global Optimization
	kestrel_solver_category, ///< Kestrel
	lno_solver_category, ///< Linear Network Programming
	lp_solver_category, ///< Linear Programming
	milp_solver_category, ///< Mixed Integer Linear Programming
	minco_solver_category, ///< Mixed Integer Nonlinearly Constrained Optimization
	multi_solver_category, ///< Multi-Solvers
	nco_solver_category, ///< Nonlinearly Constrained Optimization
	ndo_solver_category, ///< Nondifferentiable Optimization
	sdp_solver_category, ///< Semidefinite Programming
	sio_solver_category, ///< Semi-infinite Optimization
	slp_solver_category, ///< Stochastic Linear Programming
	socp_solver_category, ///< Second Order Conic Programming
	uco_solver_category ///< Unconstrained Optimization
};
*/


/*
enum solver_ids
{
	alphaecp_solver_id, ///< AlphaECP (http://archimedes.scs.uiuc.edu/alphaecp/alphaecp.html)
	asa_solver_id, ///< ASA (http://www.ingber.com/)
	baron_solver_id, ///< BARON (http://archimedes.scs.uiuc.edu/baron/baron.html)
	bdmlp_solver_id, ///< BDMLP (http://www.mcs.anl.gov/otc/Guide/OptWeb/continuous/constrained/linearprog/)
	biqmac_solver_id, ///< BiqMac (http://biqmac.uni-klu.ac.at/)
	blmvm_solver_id, ///< BLMVM (http://www.mcs.anl.gov/BLMVM)
	bnbs_solver_id, ///< bnbs (http://plato.la.asu.edu/)
	bonmin_solver_id, ///< COIN-OR Bonmin (https://projects.coin-or.org/Bonmin)
	bpmpd_solver_id, ///< bpmpd (http://www.sztaki.hu/~meszaros/bpmpd/)
	cbc_solver_id, ///< COIN-OR Cbc (https://projects.coin-or.org/Cbc)
	clp_solver_id, ///< COIN-OR Clp (http://www.coin-or.org/)
	concorde_solver_id, ///< concorde (http://www.tsp.gatech.edu/concorde.html)
	condor_solver_id, ///< condor (http://www.applied-mathematics.net/CONDORManual/CONDORManual.html)
	conopt_solver_id, ///< CONOPT (http://www.conopt.com/)
	couenne_solver_id, ///< COIN-OR Couenne (https://projects.coin-or.org/Couenne)
	csdp_solver_id, ///< csdp (https://projects.coin-or.org/Csdp/)
	ddsip_solver_id, ///< ddsip (http://plato.asu.edu/ddsip-man.pdf)
	dicopt_solver_id, ///< DICOPT (http://archimedes.scs.uiuc.edu/dicopt/dicopt.html)
	dsdp_solver_id, ///< DSDP (http://www.mcs.anl.gov/DSDP)
	feaspump_solver_id, ///< feaspump (http://www.dei.unipd.it/~fisch/papers/feasibility_pump_201.pdf)
	filmint_solver_id, ///< FilMINT (http://www.mcs.anl.gov/home/otc/Guide/OptWeb/continuous/constrained/nonlinearcon)
//	filter_solver_id, ///< filter (http://www.mcs.anl.gov/home/otc/Guide/OptWeb/continuous/constrained/nonlinearcon)
	filtermpec_solver_id, ///< filterMPEC (http://www.mcs.anl.gov/home/otc/Guide/OptWeb/continuous/constrained/nonlinearcon)
	fortmp_solver_id, ///< FortMP (http://www.optirisk-systems.com/)
	gamsampl_solver_id, ///< GAMS-AMPL
	glpk_solver_id, ///< Glpk (http://www.gnu.org/software/glpk/glpk.html)
	gurobi_solver_id, ///< Gurobi (http://www.gurobi.com/)
	icos_solver_id, ///< icos (http://sites.google.com/site/ylebbah/icos)
	ipopt_solver_id, ///< COIN-OR Ipopt (https://projects.coin-or.org/Ipopt)
	knitro_solver_id, ///< KNITRO (http://www.ziena.com/knitro)
	lancelot_solver_id, ///< LANCELOT (http://www.mcs.anl.gov/otc/Guide/OptWeb/continuous/constrained/nonlinearcon)
	lbfgsb_solver_id, ///< L-BFGS-B (http://www.mcs.anl.gov/otc/GUIDE/OptWeb/continuous/constrained/boundcon/)
	lindoglobal_solver_id, ///< LINDOGlobal (http://archimedes.scs.uiuc.edu/lindoglobal/lindoglobal.html)
	loqo_solver_id, ///< LOQO (http://www.princeton.edu/~rvdb/loqo/LOQO.html)
	lrambo_solver_id, ///< LRAMBO (http://www.mcs.anl.gov/LRAMBO)
	miles_solver_id, ///< MILES (http://www.mcs.anl.gov/otc/Guide/OptWeb/complementarity)
	minlp_solver_id, ///< MINLP (http://www.mcs.anl.gov/otc/Guide/OptWeb/discrete/integerprog/index.html)
	minos_solver_id, ///< MINOS (http://www.sbsi-sol-optimize.com/asp/sol_product_minos.htm)
	minto_solver_id, ///< MINTO (http://www.mcs.anl.gov/otc/Guide/OptWeb/discrete/integerprog/index.html)
	mosek_solver_id, ///< MOSEK (http://www.mosek.com/)
	mslip_solver_id, ///< MSLiP (http://sba.management.dal.ca/profs/hgassmann/mslip.html)
	netflo_solver_id, ///< NETFLO (ftp://dimacs.rutgers.edu/pub/netflow/mincost/solver-1)
	nlpec_solver_id, ///< NLPEC (http://www.mcs.anl.gov/otc/Guide/OptWeb/complementarity)
	nmtr_solver_id, ///< NMTR (http://www.mcs.anl.gov/home/otc/Guide/OptWeb/continuous/unconstrained/)
	nsips_solver_id, ///< nsips (http://www.norg.uminho.pt/aivaz/nsips.html)
	ooqp_solver_id, ///< OOQP (http://www.cs.wisc.edu/~swright/ooqp)
	path_solver_id, ///< PATH (http://www.cs.wisc.edu/cpnet/aboutcp.html)
	pathnlp_solver_id, ///< PATHNLP (http://www.mcs.anl.gov/otc/Guide/OptWeb/complementarity)
	penbmi_solver_id, ///< penbmi (http://www.penopt.com/)
	pennon_solver_id, ///< PENNON (http://www2.am.uni-erlangen.de/~kocvara/pennon/)
	pensdp_solver_id, ///< pensdp (http://www.penopt.com/pensdp.html)
	pcx_solver_id, ///< PCx (http://www.cs.wisc.edu/~swright/PCx)
	pgapack_solver_id, ///< PGAPack (http://www-fp.mcs.anl.gov/CCST/research/reports_pre1998/comp_bio/stalk/pgapack.html)
	pswarm_solver_id, ///< PSwarm (http://www.norg.uminho.pt/aivaz/pswarm)
	qsoptex_solver_id, ///< qsopt_ex (http://www.dii.uchile.cl/~daespino/QSoptExact_doc/main.html)
	relax4_solver_id, ///< RELAX4 (http://www.mcs.anl.gov/otc/Guide/OptWeb/continuous/constrained/network/)
	sbb_solver_id, ///< SBB (http://www.mcs.anl.gov/otc/Guide/OptWeb/discrete/integerprog/index.html)
	scip_solver_id, ///< scip (http://scip.zib.de/)
	sdpa_solver_id, ///< SDPA (http://sdpa.sourceforge.net/)
	sdplr_solver_id, ///< sdplr (http://dollar.biz.uiowa.edu/~burer/software/SDPLR/)
	sdpt3_solver_id, ///< sdpt3 (http://www.math.nus.edu.sg/~mattohkc/sdpt3.html)
	sedumi_solver_id, ///< sedumi (http://plato.asu.edu/ftp/SeDuMi_Guide_11.pdf)
	snopt_solver_id, ///< SNOPT (http://www.sbsi-sol-optimize.com/asp/sol_product_snopt.htm)
	symphony_solver_id, ///< SYMPHONY (https://projects.coin-or.org/SYMPHONY)
	tron_solver_id, ///< TRON (http://www.mcs.anl.gov/~more/tron/)
	xpressmp_solver_id, ///< XpressMP (http://www.dashoptimization.com/)
	worhp_solver_id ///< WORHP (http://www.worhp.de/)
};
*/


/*
enum input_methods
{
	ampl_input_method, ///< AMPL (http://www.ampl.com)
	c_input_method, ///< C
	cplex_input_method, ///< CPLEX (http://plato.asu.edu/cplex_lp.pdf)
	dimacs_input_method, ///< DIMACS (http://www.mcs.anl.gov/otc/Guide/OptWeb/continuous/constrained/network/dimacs_mcf.html)
	fortran_input_method, ///< Fortran
	gams_input_method, ///< GAMS (http://www.gams.com)
	lp_input_method, ///< LP (http://plato.asu.edu/ftp/lp_format.txt)
	matlab_input_method, ///< MATLAB
	matlabbinary_input_method, ///< MATLAB_BINARY (http://plato.asu.edu/ftp/usrguide.pdf)
	mps_input_method, ///< MPS (http://en.wikipedia.org/wiki/MPS_(format))
	netflo_input_method, ///< NETFLO (http://www.mcs.anl.gov/otc/Guide/SoftwareGuide/Blurbs/netflo.html)
	qps_input_method, ///< QPS (http://plato.asu.edu/QPS.pdf)
	relax4_input_method, ///< RELAX4 (http://www.mcs.anl.gov/otc/Guide/OptWeb/continuous/constrained/network/relax4-format.html)
	sdpa_input_method, ///< SDPA (http://www.nmt.edu/~sdplib/FORMAT)
	sdplr_input_method, ///< SDPLR (http://dollar.biz.uiowa.edu/~sburer/files/SDPLR/files/SDPLR-1.03-beta-usrguide.pdf)
	smps_input_method, ///< SMPS (http://myweb.dal.ca/gassmann/smps2.htm)
	sparse_input_method, ///< SPARSE
	sparsesdpa_input_method, ///< SPARSE_SDPA (http://plato.asu.edu/ftp/sdpa_format.txt)
	tsp_input_method, ///< TSP (http://plato.asu.edu/tsplib.pdf)
	zimpl_input_method ///< ZIMPL (http://www.zib.de/koch/zimpl/download/zimpl.pdf)
};
*/


/*
bco:BLMVM:C
bco:L-BFGS-B:AMPL
bco:TRON:FORTRAN
co:BiqMac:SPARSE
co:concorde:TSP
cp:filterMPEC:AMPL
cp:KNITRO:AMPL
cp:MILES:GAMS
cp:NLPEC:GAMS
cp:PATH:AMPL
cp:PATH:GAMS
go:ASA:AMPL
go:BARON:GAMS
go:icos:AMPL
go:LINDOGlobal:GAMS
go:PGAPack:AMPL
go:PSwarm:AMPL
kestrel:ALPHAECP:GAMS
kestrel:BARON:GAMS
kestrel:BDMLP:GAMS
kestrel:Bonmin:AMPL
kestrel:BONMIN:GAMS
kestrel:Cbc:AMPL
kestrel:CBC:GAMS
kestrel:COUENNE:GAMS
kestrel:CONOPT:GAMS
kestrel:DICOPT:GAMS
kestrel:filter:AMPL
kestrel:FilMINT:AMPL
kestrel:GAMS-AMPL:GAMS
kestrel:KNITRO:GAMS
kestrel:Ipopt:AMPL
kestrel:IPOPT:GAMS
kestrel:L-BFGS-B:AMPL
kestrel:LANCELOT:AMPL
kestrel:LindoGlobal:GAMS
kestrel:LOQO:AMPL
kestrel:MILES:GAMS
kestrel:MINLP:AMPL
kestrel:MINOS:AMPL
kestrel:MINOS:GAMS
kestrel:MOSEK:GAMS
kestrel:NLPEC:GAMS
kestrel:PATH:GAMS
kestrel:PATHNLP:GAMS
kestrel:SBB:GAMS
kestrel:SCIP:GAMS
kestrel:SNOPT:AMPL
kestrel:snopt:GAMS
kestrel:XPRESS:GAMS
lno:MOSEK:AMPL
lno:MOSEK:GAMS
lno:NETFLO:DIMACS
lno:NETFLO:NETFLO
lno:RELAX4:DIMACS
lno:RELAX4:RELAX4
lp:BDMLP:GAMS
lp:bpmpd:AMPL
lp:bpmpd:LP
lp:bpmpd:MPS
lp:bpmpd:QPS
lp:Clp:MPS
lp:FortMP:MPS
lp:Gurobi:AMPL
lp:Gurobi:GAMS
lp:Gurobi:MPS
lp:MOSEK:AMPL
lp:MOSEK:GAMS
lp:MOSEK:MPS
lp:OOQP:AMPL
lp:PCx:AMPL
lp:XpressMP:AMPL
lp:XpressMP:GAMS
milp:Cbc:AMPL
milp:Cbc:GAMS
milp:Cbc:MPS
milp:feaspump:AMPL
milp:feaspump:CPLEX
milp:feaspump:MPS
milp:Glpk:GAMS
milp:Gurobi:AMPL
milp:Gurobi:GAMS
milp:Gurobi:MPS
milp:MINTO:AMPL
milp:MOSEK:GAMS
milp:qsopt_ex:AMPL
milp:qsopt_ex:LP
milp:qsopt_ex:MPS
milp:scip:AMPL
milp:scip:CPLEX
milp:scip:GAMS
milp:scip:MPS
milp:scip:ZIMPL
milp:SYMPHONY:MPS
milp:XpressMP:AMPL
milp:XpressMP:GAMS
minco:AlphaECP:GAMS
minco:BARON:GAMS
minco:Bonmin:AMPL
minco:Bonmin:GAMS
minco:Couenne:AMPL
minco:Couenne:GAMS
minco:DICOPT:GAMS
minco:FilMINT:AMPL
minco:LINDOGlobal:GAMS
minco:MINLP:AMPL
minco:SBB:GAMS
MULTI:GAMS-AMPL:GAMS
nco:CONOPT:AMPL
nco:CONOPT:GAMS
nco:filter:AMPL
nco:Ipopt:AMPL
nco:Ipopt:GAMS
nco:KNITRO:AMPL
nco:KNITRO:GAMS
nco:LANCELOT:AMPL
nco:LOQO:AMPL
nco:LRAMBO:C
nco:MINOS:AMPL
nco:MINOS:GAMS
nco:MOSEK:AMPL
nco:MOSEK:GAMS
nco:PATHNLP:GAMS
nco:PENNON:AMPL
nco:SNOPT:AMPL
nco:SNOPT:GAMS
nco:WORHP:AMPL
ndo:condor:AMPL
sdp:csdp:MATLAB_BINARY
sdp:csdp:SPARSE_SDPA
sdp:DSDP:SDPA
sdp:penbmi:MATLAB
sdp:penbmi:MATLAB_BINARY
sdp:pensdp:MATLAB_BINARY
sdp:pensdp:SPARSE_SDPA
sdp:sdpa:MATLAB_BINARY
sdp:sdpa:SPARSE_SDPA
sdp:sdplr:MATLAB_BINARY
sdp:sdplr:SDPLR
sdp:sdplr:SPARSE_SDPA
sdp:sdpt3:MATLAB_BINARY
sdp:sdpt3:SPARSE_SDPA
sdp:sedumi:MATLAB_BINARY
sdp:sedumi:SPARSE_SDPA
sio:nsips:AMPL
slp:bnbs:SMPS
slp:ddsip:LP
slp:ddsip:MPS
slp:MSLiP:SMPS
socp:MOSEK:GAMS
socp:MOSEK:MPS
uco:NMTR:C
uco:NMTR:Fortran
*/


struct solver_category_info
{
	optimal_solver_categories category;
	::std::string short_name;
	::std::string full_name;
};


struct solver_info
{
	optimal_solver_categories category;
	optimal_solver_ids solver;
	optimal_solver_input_methods input_method;
};


struct job_credentials
{
	int id;
	::std::string password;
}; // job_credentials


struct submitted_job_info
{
	solver_info solver;
	job_statuses status;
};


namespace detail { namespace /*<unnamed>*/ {

inline
static ::std::string make_url(::std::string const& host, int port)
{
	::std::ostringstream oss;
	oss << "http://" << host << ":" << port;
	return oss.str();
}


inline
static ::std::string to_string(xmlrpc_c::value::type_t type)
{
	switch (type)
	{
		case xmlrpc_c::value::TYPE_INT:
			return "int";
		case xmlrpc_c::value::TYPE_BOOLEAN:
			return "bool";
		case xmlrpc_c::value::TYPE_DOUBLE:
			return "double";
		case xmlrpc_c::value::TYPE_DATETIME:
			return "datetime";
		case xmlrpc_c::value::TYPE_STRING:
			return "string";
		case xmlrpc_c::value::TYPE_BYTESTRING:
			return "bytestring";
		case xmlrpc_c::value::TYPE_ARRAY:
			return "array";
		case xmlrpc_c::value::TYPE_STRUCT:
			return "struct";
		case xmlrpc_c::value::TYPE_C_PTR:
			return "c_ptr";
		case xmlrpc_c::value::TYPE_NIL:
			return "nil";
		case xmlrpc_c::value::TYPE_I8:
			return "i8";
		case xmlrpc_c::value::TYPE_DEAD:
			return "dead";
	}

	throw ::std::runtime_error("[dcs::eesim::detail::neos::to_string] Unknown XML-RPC type.");
}


inline
static job_statuses make_job_status(::std::string const& s)
{
	::std::string ss(::dcs::string::to_lower_copy(s));

	if (!ss.compare("done"))
	{
		return done_job_status;
	}
	if (!ss.compare("running"))
	{
		return running_job_status;
	}
	if (!ss.compare("waiting"))
	{
		return waiting_job_status;
	}
	if (!ss.compare("unknown job"))
	{
		return unknown_job_job_status;
	}
	if (!ss.compare("bad password"))
	{
		return bad_password_job_status;
	}

	throw ::std::runtime_error("[dcs::eesim::detail::neos::make_job_status] Unknown NEOS job status.");
}


inline
static ::std::string to_string(job_statuses status)
{
	switch (status)
	{
		case done_job_status:
			return "Done";
		case running_job_status:
			return "Running";
		case waiting_job_status:
			return "Waiting";
		case unknown_job_job_status:
			return "Unknown Job";
		case bad_password_job_status:
			return "Bad Password";
	}

	throw ::std::runtime_error("[dcs::eesim::detail::neos::to_string] Unknown NEOS job status.");
}


inline
static optimal_solver_ids make_solver_id(::std::string const& s)
{
	::std::string ss(::dcs::string::to_lower_copy(s));

	if (!ss.compare("alphaecp"))
	{
		return alphaecp_optimal_solver_id;
	}
	if (!ss.compare("asa"))
	{
		return asa_optimal_solver_id;
	}
	if (!ss.compare("baron"))
	{
		return baron_optimal_solver_id;
	}
	if (!ss.compare("bdmlp"))
	{
		return bdmlp_optimal_solver_id;
	}
	if (!ss.compare("biqmac"))
	{
		return biqmac_optimal_solver_id;
	}
	if (!ss.compare("blmvm"))
	{
		return blmvm_optimal_solver_id;
	}
	if (!ss.compare("bnbs"))
	{
		return bnbs_optimal_solver_id;
	}
	if (!ss.compare("bonmin"))
	{
		return bonmin_optimal_solver_id;
	}
	if (!ss.compare("bpmpd"))
	{
		return bpmpd_optimal_solver_id;
	}
	if (!ss.compare("cbc"))
	{
		return cbc_optimal_solver_id;
	}
	if (!ss.compare("clp"))
	{
		return clp_optimal_solver_id;
	}
	if (!ss.compare("concorde"))
	{
		return concorde_optimal_solver_id;
	}
	if (!ss.compare("condor"))
	{
		return condor_optimal_solver_id;
	}
	if (!ss.compare("conopt"))
	{
		return conopt_optimal_solver_id;
	}
	if (!ss.compare("couenne"))
	{
		return couenne_optimal_solver_id;
	}
	if (!ss.compare("csdp"))
	{
		return csdp_optimal_solver_id;
	}
	if (!ss.compare("ddsip"))
	{
		return ddsip_optimal_solver_id;
	}
	if (!ss.compare("dicopt"))
	{
		return dicopt_optimal_solver_id;
	}
	if (!ss.compare("dsdp"))
	{
		return dsdp_optimal_solver_id;
	}
	if (!ss.compare("feaspump"))
	{
		return feaspump_optimal_solver_id;
	}
	if (!ss.compare("filmint"))
	{
		return filmint_optimal_solver_id;
	}
	if (!ss.compare("filter"))
	{
		return filter_optimal_solver_id;
	}
	if (!ss.compare("filtermpec"))
	{
		return filtermpec_optimal_solver_id;
	}
//	if (!ss.compare("filter") || !ss.compare("filtermpec"))
//	{
//		return filtermpec_optimal_solver_id;
//	}
	if (!ss.compare("fortmp"))
	{
		return fortmp_optimal_solver_id;
	}
	if (!ss.compare("gams-ampl"))
	{
		return gamsampl_optimal_solver_id;
	}
	if (!ss.compare("glpk"))
	{
		return glpk_optimal_solver_id;
	}
	if (!ss.compare("gurobi"))
	{
		return gurobi_optimal_solver_id;
	}
	if (!ss.compare("icos"))
	{
		return icos_optimal_solver_id;
	}
	if (!ss.compare("ipopt"))
	{
		return ipopt_optimal_solver_id;
	}
	if (!ss.compare("knitro"))
	{
		return knitro_optimal_solver_id;
	}
	if (!ss.compare("lancelot"))
	{
		return lancelot_optimal_solver_id;
	}
	if (!ss.compare("l-bfgs-b"))
	{
		return lbfgsb_optimal_solver_id;
	}
	if (!ss.compare("lindoglobal"))
	{
		return lindoglobal_optimal_solver_id;
	}
	if (!ss.compare("loqo"))
	{
		return loqo_optimal_solver_id;
	}
	if (!ss.compare("lrambo"))
	{
		return lrambo_optimal_solver_id;
	}
	if (!ss.compare("miles"))
	{
		return miles_optimal_solver_id;
	}
	if (!ss.compare("minlp"))
	{
		return minlp_optimal_solver_id;
	}
	if (!ss.compare("minos"))
	{
		return minos_optimal_solver_id;
	}
	if (!ss.compare("minto"))
	{
		return minto_optimal_solver_id;
	}
	if (!ss.compare("mosek"))
	{
		return mosek_optimal_solver_id;
	}
	if (!ss.compare("mslip"))
	{
		return mslip_optimal_solver_id;
	}
	if (!ss.compare("netflo"))
	{
		return netflo_optimal_solver_id;
	}
	if (!ss.compare("nlpec"))
	{
		return nlpec_optimal_solver_id;
	}
	if (!ss.compare("nmtr"))
	{
		return nmtr_optimal_solver_id;
	}
	if (!ss.compare("nsips"))
	{
		return nsips_optimal_solver_id;
	}
	if (!ss.compare("ooqp"))
	{
		return ooqp_optimal_solver_id;
	}
	if (!ss.compare("path"))
	{
		return path_optimal_solver_id;
	}
	if (!ss.compare("pathnlp"))
	{
		return pathnlp_optimal_solver_id;
	}
	if (!ss.compare("penbmi"))
	{
		return penbmi_optimal_solver_id;
	}
	if (!ss.compare("pennon"))
	{
		return pennon_optimal_solver_id;
	}
	if (!ss.compare("pensdp"))
	{
		return pensdp_optimal_solver_id;
	}
	if (!ss.compare("pcx"))
	{
		return pcx_optimal_solver_id;
	}
	if (!ss.compare("pgapack"))
	{
		return pgapack_optimal_solver_id;
	}
	if (!ss.compare("pswarm"))
	{
		return pswarm_optimal_solver_id;
	}
	if (!ss.compare("qsopt_ex"))
	{
		return qsoptex_optimal_solver_id;
	}
	if (!ss.compare("relax4"))
	{
		return relax4_optimal_solver_id;
	}
	if (!ss.compare("sbb"))
	{
		return sbb_optimal_solver_id;
	}
	if (!ss.compare("scip"))
	{
		return scip_optimal_solver_id;
	}
	if (!ss.compare("sdpa"))
	{
		return sdpa_optimal_solver_id;
	}
	if (!ss.compare("sdplr"))
	{
		return sdplr_optimal_solver_id;
	}
	if (!ss.compare("sdpt3"))
	{
		return sdpt3_optimal_solver_id;
	}
	if (!ss.compare("sedumi"))
	{
		return sedumi_optimal_solver_id;
	}
	if (!ss.compare("snopt"))
	{
		return snopt_optimal_solver_id;
	}
	if (!ss.compare("symphony"))
	{
		return symphony_optimal_solver_id;
	}
	if (!ss.compare("tron"))
	{
		return tron_optimal_solver_id;
	}
	if (!ss.compare("xpressmp"))
	{
		return xpressmp_optimal_solver_id;
	}
	if (!ss.compare("worhp"))
	{
		return worhp_optimal_solver_id;
	}

	throw ::std::runtime_error("[dcs::eesim::detail::neos::make_optimal_solver_category] Unknown NEOS solver id.");
}


static ::std::string to_string(optimal_solver_ids id)
{
	switch (id)
	{
		case alphaecp_optimal_solver_id:
			return "AlphaECP";
		case asa_optimal_solver_id:
			return "ASA";
		case baron_optimal_solver_id:
			return "BARON";
		case bdmlp_optimal_solver_id:
			return "BDMLP";
		case biqmac_optimal_solver_id:
			return "BiqMac";
		case blmvm_optimal_solver_id:
			return "BLMVM";
		case bnbs_optimal_solver_id:
			return "bnbs";
		case bonmin_optimal_solver_id:
			return "Bonmin";
		case bpmpd_optimal_solver_id:
			return "bpmpd";
		case cbc_optimal_solver_id:
			return "Cbc";
		case clp_optimal_solver_id:
			return "Clp";
		case concorde_optimal_solver_id:
			return "concorde";
		case condor_optimal_solver_id:
			return "condor";
		case conopt_optimal_solver_id:
			return "CONOPT";
		case couenne_optimal_solver_id:
			return "Couenne";
		case csdp_optimal_solver_id:
			return "csdp";
		case ddsip_optimal_solver_id:
			return "ddsip";
		case dicopt_optimal_solver_id:
			return "DICOPT";
		case dsdp_optimal_solver_id:
			return "DSDP";
		case feaspump_optimal_solver_id:
			return "feaspump";
		case filmint_optimal_solver_id:
			return "FilMINT";
		case filter_optimal_solver_id:
			return "filter";
		case filtermpec_optimal_solver_id:
			return "filterMPEC";
		case fortmp_optimal_solver_id:
			return "FortMP";
		case gamsampl_optimal_solver_id:
			return "GAMS-AMPL";
		case glpk_optimal_solver_id:
			return "Glpk";
		case gurobi_optimal_solver_id:
			return "Gurobi";
		case icos_optimal_solver_id:
			return "icos";
		case ipopt_optimal_solver_id:
			return "Ipopt";
		case knitro_optimal_solver_id:
			return "KNITRO";
		case lancelot_optimal_solver_id:
			return "LANCELOT";
		case lbfgsb_optimal_solver_id:
			return "L-BFGS-B";
		case lindoglobal_optimal_solver_id:
			return "LINDOGlobal";
		case loqo_optimal_solver_id:
			return "LOQO";
		case lrambo_optimal_solver_id:
			return "LRAMBO";
		case miles_optimal_solver_id:
			return "MILES";
		case minlp_optimal_solver_id:
			return "MINLP";
		case minos_optimal_solver_id:
			return "MINOS";
		case minto_optimal_solver_id:
			return "MINTO";
		case mosek_optimal_solver_id:
			return "MOSEK";
		case mslip_optimal_solver_id:
			return "MSLiP";
		case netflo_optimal_solver_id:
			return "NETFLO";
		case nlpec_optimal_solver_id:
			return "NLPEC";
		case nmtr_optimal_solver_id:
			return "NMTR";
		case nsips_optimal_solver_id:
			return "nsips";
		case ooqp_optimal_solver_id:
			return "OOQP";
		case path_optimal_solver_id:
			return "PATH";
		case pathnlp_optimal_solver_id:
			return "PATHNLP";
		case penbmi_optimal_solver_id:
			return "penbmi";
		case pennon_optimal_solver_id:
			return "PENNON";
		case pensdp_optimal_solver_id:
			return "pensdp";
		case pcx_optimal_solver_id:
			return "PCx";
		case pgapack_optimal_solver_id:
			return "PGAPack";
		case pswarm_optimal_solver_id:
			return "PSwarm";
		case qsoptex_optimal_solver_id:
			return "qsopt_ex";
		case relax4_optimal_solver_id:
			return "RELAX4";
		case sbb_optimal_solver_id:
			return "SBB";
		case scip_optimal_solver_id:
			return "scip";
		case sdpa_optimal_solver_id:
			return "SDPA";
		case sdplr_optimal_solver_id:
			return "SDPLR";
		case sdpt3_optimal_solver_id:
			return "sdpt3";
		case sedumi_optimal_solver_id:
			return "sedumi";
		case snopt_optimal_solver_id:
			return "SNOPT";
		case symphony_optimal_solver_id:
			return "SYMPHONY";
		case tron_optimal_solver_id:
			return "TRON";
		case xpressmp_optimal_solver_id:
			return "XpressMP";
		case worhp_optimal_solver_id:
			return "WORHP";
		default:
			break;
	}

	throw ::std::runtime_error("[dcs::eesim::detail::neos::to_string] Unknown NEOS solver id.");
}


inline
static optimal_solver_categories make_solver_category(::std::string const& s)
{
	::std::string ss(::dcs::string::to_lower_copy(s));

	if (!ss.compare("bco"))
	{
		return bco_optimal_solver_category;
	}
	if (!ss.compare("co"))
	{
		return co_optimal_solver_category;
	}
	if (!ss.compare("cp"))
	{
		return cp_optimal_solver_category;
	}
	if (!ss.compare("go"))
	{
		return go_optimal_solver_category;
	}
	if (!ss.compare("kestrel"))
	{
		return kestrel_optimal_solver_category;
	}
	if (!ss.compare("lno"))
	{
		return lno_optimal_solver_category;
	}
	if (!ss.compare("lp"))
	{
		return lp_optimal_solver_category;
	}
	if (!ss.compare("milp"))
	{
		return milp_optimal_solver_category;
	}
	if (!ss.compare("minco"))
	{
		return minco_optimal_solver_category;
	}
	if (!ss.compare("multi"))
	{
		return multi_optimal_solver_category;
	}
	if (!ss.compare("nco"))
	{
		return nco_optimal_solver_category;
	}
	if (!ss.compare("ndo"))
	{
		return ndo_optimal_solver_category;
	}
	if (!ss.compare("sdp"))
	{
		return sdp_optimal_solver_category;
	}
	if (!ss.compare("sio"))
	{
		return sio_optimal_solver_category;
	}
	if (!ss.compare("slp"))
	{
		return slp_optimal_solver_category;
	}
	if (!ss.compare("socp"))
	{
		return socp_optimal_solver_category;
	}
	if (!ss.compare("uco"))
	{
		return uco_optimal_solver_category;
	}

	throw ::std::runtime_error("[dcs::eesim::detail::neos::make_optimal_solver_category] Unknown NEOS solver category.");
}


inline
static ::std::string to_string(optimal_solver_categories category)
{
	switch (category)
	{
		case bco_optimal_solver_category:
			return "bco";
		case co_optimal_solver_category:
			return "co";
		case cp_optimal_solver_category:
			return "cp";
		case go_optimal_solver_category:
			return "go";
		case kestrel_optimal_solver_category:
			return "kestrel";
		case lno_optimal_solver_category:
			return "lnp";
		case lp_optimal_solver_category:
			return "lp";
		case milp_optimal_solver_category:
			return "milp";
		case minco_optimal_solver_category:
			return "minco";
		case multi_optimal_solver_category:
			return "MULTI";
		case nco_optimal_solver_category:
			return "nco";
		case ndo_optimal_solver_category:
			return "ndo";
		case sdp_optimal_solver_category:
			return "sdp";
		case sio_optimal_solver_category:
			return "sio";
		case slp_optimal_solver_category:
			return "slp";
		case socp_optimal_solver_category:
			return "socp";
		case uco_optimal_solver_category:
			return "uco";
	}

	throw ::std::runtime_error("[dcs::eesim::detail::neos::to_string] Unknown NEOS solver category.");
}


inline
static optimal_solver_input_methods make_input_method(::std::string const& s)
{
	::std::string ss(::dcs::string::to_lower_copy(s));

	if (!ss.compare("ampl"))
	{
		return ampl_optimal_solver_input_method;
	}
	if (!ss.compare("c"))
	{
		return c_optimal_solver_input_method;
	}
	if (!ss.compare("cplex"))
	{
		return cplex_optimal_solver_input_method;
	}
	if (!ss.compare("dimacs"))
	{
		return dimacs_optimal_solver_input_method;
	}
	if (!ss.compare("fortran"))
	{
		return fortran_optimal_solver_input_method;
	}
	if (!ss.compare("gams"))
	{
		return gams_optimal_solver_input_method;
	}
	if (!ss.compare("lp"))
	{
		return lp_optimal_solver_input_method;
	}
	if (!ss.compare("matlab"))
	{
		return matlab_optimal_solver_input_method;
	}
	if (!ss.compare("matlab_binary"))
	{
		return matlabbinary_optimal_solver_input_method;
	}
	if (!ss.compare("mps"))
	{
		return mps_optimal_solver_input_method;
	}
	if (!ss.compare("netflo"))
	{
		return netflo_optimal_solver_input_method;
	}
	if (!ss.compare("qps"))
	{
		return qps_optimal_solver_input_method;
	}
	if (!ss.compare("relax4"))
	{
		return relax4_optimal_solver_input_method;
	}
	if (!ss.compare("sdpa"))
	{
		return sdpa_optimal_solver_input_method;
	}
	if (!ss.compare("sdplr"))
	{
		return sdplr_optimal_solver_input_method;
	}
	if (!ss.compare("smps"))
	{
		return smps_optimal_solver_input_method;
	}
	if (!ss.compare("sparse"))
	{
		return sparse_optimal_solver_input_method;
	}
	if (!ss.compare("sparse_sdpa"))
	{
		return sparsesdpa_optimal_solver_input_method;
	}
	if (!ss.compare("tsp"))
	{
		return tsp_optimal_solver_input_method;
	}
	if (!ss.compare("zimpl"))
	{
		return zimpl_optimal_solver_input_method;
	}

	throw ::std::runtime_error("[dcs::eesim::detail::neos::make_optimal_solver_input_method] Unknown NEOS input method.");
}


inline
static ::std::string to_string(optimal_solver_input_methods method)
{
	switch (method)
	{
		case ampl_optimal_solver_input_method:
			return "AMPL";
		case c_optimal_solver_input_method:
			return "C";
		case cplex_optimal_solver_input_method:
			return "CPLEX";
		case dimacs_optimal_solver_input_method:
			return "DIMACS";
		case fortran_optimal_solver_input_method:
			return "Fortran";
		case gams_optimal_solver_input_method:
			return "GAMS";
		case lp_optimal_solver_input_method:
			return "LP";
		case matlab_optimal_solver_input_method:
			return "MATLAB";
		case matlabbinary_optimal_solver_input_method:
			return "MATLAB_BINARY";
		case mps_optimal_solver_input_method:
			return "MPS";
		case netflo_optimal_solver_input_method:
			return "NETFLO";
		case qps_optimal_solver_input_method:
			return "QPS";
		case relax4_optimal_solver_input_method:
			return "RELAX4";
		case sdpa_optimal_solver_input_method:
			return "SDPA";
		case sdplr_optimal_solver_input_method:
			return "SDPLR";
		case smps_optimal_solver_input_method:
			return "SMPS";
		case sparse_optimal_solver_input_method:
			return "SPARSE";
		case sparsesdpa_optimal_solver_input_method:
			return "SPARSE_SDPA";
		case tsp_optimal_solver_input_method:
			return "TSP";
		case zimpl_optimal_solver_input_method:
			return "ZIMPL";
	}

	throw ::std::runtime_error("[dcs::eesim::detail::neos::to_string] Unknown NEOS input method.");
}


inline
static solver_info make_solver_info(::std::string const& s)
{
	// Extract the job info from:
	//  <solver-category>:<solver-name>:<input-method>

	solver_info info;
	::std::size_t bpos(0);
	::std::size_t epos(0);
	::std::size_t nk(3);
	for (::std::size_t k = 1; k <= nk; ++k)
	{
		epos = s.find(':', bpos);
		if (epos == ::std::string::npos && k < nk)
		{
			::std::ostringstream oss;
			oss << "[dcs::eesim::detail::neos::detail::mkae_solver_info] Cannot find ':' in '" << s << "' from position '" << bpos << "'.";
			throw ::std::runtime_error(oss.str());
		}
		::std::string item(s.substr(bpos, (epos != ::std::string::npos) ? (epos-bpos) : ::std::string::npos));
		switch (k)
		{
			case 1:
				info.category = detail::make_solver_category(item);
				break;
			case 2:
				info.solver = detail::make_solver_id(item);
				break;
			case 3:
				info.input_method = detail::make_input_method(item);
				break;
		}
	}

	return info;
}


inline
static solver_category_info make_solver_category_info(::std::string const& s)
{
	// Extract the abbreviated and full category name from:
	//  <abbreviated-name>:<full-name>

	solver_category_info info;

	::std::size_t bpos(0);
	::std::size_t epos(0);
	::std::size_t nk(2);
	for (::std::size_t k = 1; k <= nk; ++k)
	{
		epos = s.find(':', bpos);
		if (epos == ::std::string::npos && k < nk)
		{
			::std::ostringstream oss;
			oss << "[dcs::eesim::detail::neos::detail::make_solver_category_info] Cannot find ':' in '" << s << "' from position '" << bpos << "'.";
			throw ::std::runtime_error(oss.str());
		}
		::std::string item(s.substr(bpos, (epos != ::std::string::npos) ? (epos-bpos) : ::std::string::npos));
		switch (k)
		{
			case 1:
				info.short_name = item;
				info.category = detail::make_solver_category(item);
				break;
			case 2:
				info.full_name = detail::make_solver_id(item);
				break;
		}
	}

	return info;
}


inline
static solver_info make_solver_info(::std::string const& s, optimal_solver_categories category)
{
	// Extract the solver info from:
	//  <solver-name>:<input-method>

	solver_info info;
	info.category = category;
	::std::size_t bpos(0);
	::std::size_t epos(0);
	::std::size_t nk(2);
	for (::std::size_t k = 1; k <= nk; ++k)
	{
		epos = s.find(':', bpos);
		if (epos == ::std::string::npos && k < nk)
		{
			::std::ostringstream oss;
			oss << "[dcs::eesim::detail::neos::detail::make_solver_info] Cannot find ':' in '" << s << "' from position '" << bpos << "'.";
			throw ::std::runtime_error(oss.str());
		}
		::std::string item(s.substr(bpos, (epos != ::std::string::npos) ? (epos-bpos) : ::std::string::npos));
		switch (k)
		{
			case 1:
				info.solver = detail::make_solver_id(item);
				break;
			case 2:
				info.input_method = detail::make_input_method(item);
				break;
		}
	}

	return info;
}


inline
static submitted_job_info make_submitted_job_info(::std::string const& s)
{
	submitted_job_info info;

	::std::size_t pos(s.rfind(':'));
	info.solver = make_solver_info(s.substr(0, pos));
	info.status = make_job_status(s.substr(pos));

	return info;
}

}} // Namespace detail::<unnamed>


/**
 * \brief Implements the XML-RPC NEOS API.
 *
 * See http://www.neos-server.org/neos/NEOS-API.html
 *
 * \author Marco Guazzone, &lt;marco.guazzone@mfn.unipmn.it&gt;
 */
class client
{
	private: typedef xmlrpc_c::cbytestring cbytestring;
	private: typedef xmlrpc_c::carray carray;
	//private: typedef typename carray::const_iterator carray_iterator;

	public: static const ::std::string default_neos_host;
	public: static const int default_neos_port;
	//private: static const float default_zzz_time;


	public: explicit client(::std::string const& host = default_neos_host, int port = default_neos_port)
	: url_(detail::make_url(host,port))
	{
	}


	public: ::std::string help() const
	{
		::std::string res;

		xmlrpc_c::clientSimple neos;

		xmlrpc_c::value rpc_res;
		neos.call(url_, "help", &rpc_res);
		if (rpc_res.type() != xmlrpc_c::value::TYPE_STRING)
		{
			::std::ostringstream oss;
			oss << "[dcs::eesim::detail::neos::client::help] Expected type '" << detail::to_string(xmlrpc_c::value::TYPE_STRING) << "' (" << xmlrpc_c::value::TYPE_STRING << "), got type '" << detail::to_string(rpc_res.type()) << "' (" << rpc_res.type() << ").";
			throw ::std::runtime_error(oss.str());
		}

		res = xmlrpc_c::value_string(rpc_res);

		DCS_DEBUG_TRACE("Message: " << res);

		return res;
	}


	public: ::std::string welcome() const
	{
		::std::string res;

		xmlrpc_c::clientSimple neos;

		xmlrpc_c::value rpc_res;
		neos.call(url_, "welcome", &rpc_res);
		if (rpc_res.type() != xmlrpc_c::value::TYPE_STRING)
		{
			::std::ostringstream oss;
			oss << "[dcs::eesim::detail::neos::client::welcome] Expected type '" << detail::to_string(xmlrpc_c::value::TYPE_STRING) << "' (" << xmlrpc_c::value::TYPE_STRING << "), got type '" << detail::to_string(rpc_res.type()) << "' (" << rpc_res.type() << ").";
			throw ::std::runtime_error(oss.str());
		}

		res = xmlrpc_c::value_string(rpc_res);

		DCS_DEBUG_TRACE("Message: " << res);

		return res;
	}


	public: bool ping() const
	{
		bool res(false);

		xmlrpc_c::clientSimple neos;

		xmlrpc_c::value rpc_res;
		neos.call(url_, "ping", &rpc_res);
		if (rpc_res.type() != xmlrpc_c::value::TYPE_STRING)
		{
			::std::ostringstream oss;
			oss << "[dcs::eesim::detail::neos::client::ping] Expected type '" << detail::to_string(xmlrpc_c::value::TYPE_STRING) << "' (" << xmlrpc_c::value::TYPE_STRING << "), got type '" << detail::to_string(rpc_res.type()) << "' (" << rpc_res.type() << ").";
			throw ::std::runtime_error(oss.str());
		}

		::std::string const msg = xmlrpc_c::value_string(rpc_res);

		DCS_DEBUG_TRACE("Message: " << msg);

		// If the NEOS server is up return 'NeosServer is alive'

		::std::string imsg(::dcs::string::to_lower_copy(msg));
		if (!imsg.compare("neosserver is alive"))
		{
			res = true;
		}

		return res;
	}


	public: ::std::string queue() const
	{
		::std::string res;

		xmlrpc_c::clientSimple neos;

		xmlrpc_c::value rpc_res;
		neos.call(url_, "printQueue", &rpc_res);
		if (rpc_res.type() != xmlrpc_c::value::TYPE_STRING)
		{
			::std::ostringstream oss;
			oss << "[dcs::eesim::detail::neos::client::queue] Expected type '" << detail::to_string(xmlrpc_c::value::TYPE_STRING) << "' (" << xmlrpc_c::value::TYPE_STRING << "), got type '" << detail::to_string(rpc_res.type()) << "' (" << rpc_res.type() << ").";
			throw ::std::runtime_error(oss.str());
		}

		res = xmlrpc_c::value_string(rpc_res);

		DCS_DEBUG_TRACE("Message: " << res);

		return res;
	}


	public: ::std::string solver_template(optimal_solver_categories category, optimal_solver_ids solver, optimal_solver_input_methods method) const
	{
		::std::string res;

		xmlrpc_c::clientSimple neos;

		xmlrpc_c::value rpc_res;
		neos.call(url_,
				  "getSolverTemplate",
				  "sss",
				  &rpc_res,
				  detail::to_string(category).c_str(),
				  detail::to_string(solver).c_str(),
				  detail::to_string(method).c_str());
		if (rpc_res.type() != xmlrpc_c::value::TYPE_STRING)
		{
			::std::ostringstream oss;
			oss << "[dcs::eesim::detail::neos::client::solver_template] Expected type '" << detail::to_string(xmlrpc_c::value::TYPE_STRING) << "' (" << xmlrpc_c::value::TYPE_STRING << "), got type '" << detail::to_string(rpc_res.type()) << "' (" << rpc_res.type() << ").";
			throw ::std::runtime_error(oss.str());
		}

		res = xmlrpc_c::value_string(rpc_res);

		DCS_DEBUG_TRACE("Message: " << res);

		return res;
	}


	public: ::std::string xml(optimal_solver_categories category, optimal_solver_ids solver, optimal_solver_input_methods method) const
	{
		return solver_template(category, solver, method);
	}


	public: ::std::vector<solver_info> all_solvers() const
	{
		typedef carray::const_iterator carray_iterator;

		::std::vector<solver_info> res;

		xmlrpc_c::clientSimple neos;

		xmlrpc_c::value rpc_res;
		neos.call(url_, "listAllSolvers", &rpc_res);
		if (rpc_res.type() != xmlrpc_c::value::TYPE_ARRAY)
		{
			::std::ostringstream oss;
			oss << "[dcs::eesim::detail::neos::client::all_solvers] Expected type '" << detail::to_string(xmlrpc_c::value::TYPE_ARRAY) << "' (" << xmlrpc_c::value::TYPE_ARRAY << "), got type '" << detail::to_string(rpc_res.type()) << "' (" << rpc_res.type() << ").";
			throw ::std::runtime_error(oss.str());
		}

		// The returned message is a newline-separated string, where each
		// newline-separated item has the form:
		//  <solver-category>:<solver-name>:<input-method>

		carray const solvers = xmlrpc_c::value_array(rpc_res).vectorValueValue();
		carray_iterator end_it(solvers.end());
		for (carray_iterator it = solvers.begin(); it != end_it; ++it)
		{
			if (it->type() != xmlrpc_c::value::TYPE_STRING)
			{
				::std::ostringstream oss;
				oss << "[dcs::eesim::detail::neos::client::all_solvers] Expected type '" << detail::to_string(xmlrpc_c::value::TYPE_STRING) << "' (" << xmlrpc_c::value::TYPE_STRING << "), got type '" << detail::to_string(it->type()) << "' (" << it->type() << ").";
				throw ::std::runtime_error(oss.str());
			}

			::std::string const msg = xmlrpc_c::value_string(*it);

			DCS_DEBUG_TRACE("Message: " << msg);

			res.push_back(detail::make_solver_info(msg));
		}

		return res;
	}


	public: ::std::vector<solver_category_info> categories() const
	{
		typedef carray::const_iterator carray_iterator;

		::std::vector<solver_category_info> res;

		xmlrpc_c::clientSimple neos;

		xmlrpc_c::value rpc_res;
		neos.call(url_, "listCategories", &rpc_res);
		if (rpc_res.type() != xmlrpc_c::value::TYPE_ARRAY)
		{
			::std::ostringstream oss;
			oss << "[dcs::eesim::detail::neos::client::categories] Expected type '" << detail::to_string(xmlrpc_c::value::TYPE_ARRAY) << "' (" << xmlrpc_c::value::TYPE_ARRAY << "), got type '" << detail::to_string(rpc_res.type()) << "' (" << rpc_res.type() << ").";
			throw ::std::runtime_error(oss.str());
		}

		// The returned message is a newline-separated string, where each
		// newline-separated item has the form:
		//  <abbreviated-name>:<full-name>

		carray const categories = xmlrpc_c::value_array(rpc_res).vectorValueValue();
		carray_iterator end_it(categories.end());
		for (carray_iterator it = categories.begin(); it != end_it; ++it)
		{
			if (it->type() != xmlrpc_c::value::TYPE_STRING)
			{
				::std::ostringstream oss;
				oss << "[dcs::eesim::detail::neos::client::categories] Expected type '" << detail::to_string(xmlrpc_c::value::TYPE_STRING) << "' (" << xmlrpc_c::value::TYPE_STRING << "), got type '" << detail::to_string(it->type()) << "' (" << it->type() << ").";
				throw ::std::runtime_error(oss.str());
			}

			::std::string const msg = xmlrpc_c::value_string(*it);

			DCS_DEBUG_TRACE("Message: " << msg);

			res.push_back(detail::make_solver_category_info(msg));
		}

		return res;
	}


	public: ::std::vector<solver_info> solvers_in_category(optimal_solver_categories category) const
	{
		typedef carray::const_iterator carray_iterator;

		::std::vector<solver_info> res;

		xmlrpc_c::clientSimple neos;

		xmlrpc_c::value rpc_res;
		neos.call(url_, "listSolversInCategory", "s", &rpc_res, detail::to_string(category).c_str());
		if (rpc_res.type() != xmlrpc_c::value::TYPE_ARRAY)
		{
			::std::ostringstream oss;
			oss << "[dcs::eesim::detail::neos::client::solvers_in_categories] Expected type '" << detail::to_string(xmlrpc_c::value::TYPE_ARRAY) << "' (" << xmlrpc_c::value::TYPE_ARRAY << "), got type '" << detail::to_string(rpc_res.type()) << "' (" << rpc_res.type() << ").";
			throw ::std::runtime_error(oss.str());
		}

		// The returned message is a newline-separated string, where each
		// newline-separated item has the form:
		//  <solver-name>:<input-method>

		carray const solvers = xmlrpc_c::value_array(rpc_res).vectorValueValue();
		carray_iterator end_it(solvers.end());
		for (carray_iterator it = solvers.begin(); it != end_it; ++it)
		{
			if (it->type() != xmlrpc_c::value::TYPE_STRING)
			{
				::std::ostringstream oss;
				oss << "[dcs::eesim::detail::neos::client::solvers_in_categories] Expected type '" << detail::to_string(xmlrpc_c::value::TYPE_STRING) << "' (" << xmlrpc_c::value::TYPE_STRING << "), got type '" << detail::to_string(it->type()) << "' (" << it->type() << ").";
				throw ::std::runtime_error(oss.str());
			}

			::std::string const msg = xmlrpc_c::value_string(*it);

			DCS_DEBUG_TRACE("Message: " << msg);

			res.push_back(detail::make_solver_info(msg, category));
		}

		return res;
	}


	public: job_credentials submit_job(::std::string const& xml) const
	{
		job_credentials res;

		xmlrpc_c::clientSimple neos;

		xmlrpc_c::value rpc_res;
		neos.call(url_, "submitJob", "s", &rpc_res, xml.c_str());
		if (rpc_res.type() != xmlrpc_c::value::TYPE_ARRAY)
		{
			::std::ostringstream oss;
			oss << "[dcs::eesim::detail::neos::client::submit_job] Expected type '" << detail::to_string(xmlrpc_c::value::TYPE_ARRAY) << "' (" << xmlrpc_c::value::TYPE_ARRAY << "), got type '" << detail::to_string(rpc_res.type()) << "' (" << rpc_res.type() << ").";
			throw ::std::runtime_error(oss.str());
		}

		carray const job_creds = xmlrpc_c::value_array(rpc_res).vectorValueValue();
		if (job_creds.size() != 2)
		{
			throw ::std::runtime_error("Expected (jobnumber,password) credentials.");
		}
		if (job_creds[0].type() != xmlrpc_c::value::TYPE_INT)
		{
			::std::ostringstream oss;
			oss << "[dcs::eesim::detail::neos::client::submit_job] Expected type '" << detail::to_string(xmlrpc_c::value::TYPE_INT) << "' (" << xmlrpc_c::value::TYPE_INT << "), got type '" << detail::to_string(job_creds[0].type()) << "' (" << job_creds[0].type() << ").";
			throw ::std::runtime_error(oss.str());
		}
		if (job_creds[1].type() != xmlrpc_c::value::TYPE_STRING)
		{
			::std::ostringstream oss;
			oss << "[dcs::eesim::detail::neos::client::submit_job] Expected type '" << detail::to_string(xmlrpc_c::value::TYPE_STRING) << "' (" << xmlrpc_c::value::TYPE_STRING << "), got type '" << detail::to_string(job_creds[1].type()) << "' (" << job_creds[1].type() << ").";
			throw ::std::runtime_error(oss.str());
		}
		res.id = xmlrpc_c::value_int(job_creds[0]);
		res.password = xmlrpc_c::value_string(job_creds[1]);

		DCS_DEBUG_TRACE("jobNumber = " << res.id << "\tpassword = " << res.password);

		return res;
	}


	public: job_statuses job_status(job_credentials const& creds) const
	{
		job_statuses res;

		xmlrpc_c::clientSimple neos;

		xmlrpc_c::value rpc_res;
		neos.call(url_, "getJobStatus", "is", &rpc_res, creds.id, creds.password.c_str());
		if (rpc_res.type() != xmlrpc_c::value::TYPE_STRING)
		{
			::std::ostringstream oss;
			oss << "[dcs::eesim::detail::neos::client::job_status] Expected type '" << detail::to_string(xmlrpc_c::value::TYPE_STRING) << "' (" << xmlrpc_c::value::TYPE_STRING << "), got type '" << detail::to_string(rpc_res.type()) << "' (" << rpc_res.type() << ").";
			throw ::std::runtime_error(oss.str());
		}

		::std::string const status_str = xmlrpc_c::value_string(rpc_res);
		res = detail::make_job_status(status_str);

		DCS_DEBUG_TRACE("Status: " << status_str << " (" << res << ").");

		return res;
	}


	public: submitted_job_info job_info(job_credentials const& creds) const
	{
		submitted_job_info res;

		xmlrpc_c::clientSimple neos;

		xmlrpc_c::value rpc_res;
		neos.call(url_, "getJobInfo", "is", &rpc_res, creds.id, creds.password.c_str());
		if (rpc_res.type() != xmlrpc_c::value::TYPE_STRING)
		{
			::std::ostringstream oss;
			oss << "[dcs::eesim::detail::neos::client::job_info] Expected type '" << detail::to_string(xmlrpc_c::value::TYPE_STRING) << "' (" << xmlrpc_c::value::TYPE_STRING << "), got type '" << detail::to_string(rpc_res.type()) << "' (" << rpc_res.type() << ").";
			throw ::std::runtime_error(oss.str());
		}

		::std::string const msg = xmlrpc_c::value_string(rpc_res);
		res = detail::make_submitted_job_info(msg);

		DCS_DEBUG_TRACE("Message: " << msg);

		return res;
	}


	///FIXME: what is the return type of 'killJob'?
	public: void kill_job(job_credentials const& creds, ::std::string const& kill_msg = "") const
	{
		xmlrpc_c::clientSimple neos;

		xmlrpc_c::value rpc_res;
		neos.call(url_, "killJob", "iss", &rpc_res, creds.id, creds.password.c_str(), kill_msg.c_str());
		if (rpc_res.type() != xmlrpc_c::value::TYPE_STRING)
		{
			::std::ostringstream oss;
			oss << "[dcs::eesim::detail::neos::client::kill_job] Expected type '" << detail::to_string(xmlrpc_c::value::TYPE_STRING) << "' (" << xmlrpc_c::value::TYPE_STRING << "), got type '" << detail::to_string(rpc_res.type()) << "' (" << rpc_res.type() << ").";
			throw ::std::runtime_error(oss.str());
		}
	}


	public: ::std::string intermediate_results(job_credentials const& creds, int& offset, bool blocking = true) const
	{
		// pre: offset > 0
		DCS_ASSERT(
				offset > 0,
				throw ::std::invalid_argument("[dcs::eesim::detail::neos::client::intermedia_reusults] Invalid offset.")
			);

		::std::string res;

		::std::string method;
		if (blocking)
		{
			method = "getIntermediateResults";
		}
		else
		{
			method = "getIntermediateResultsNonBlocking";
		}

		xmlrpc_c::clientSimple neos;

		xmlrpc_c::value rpc_res;
		neos.call(url_, method, "isi", &rpc_res, creds.id, creds.password.c_str(), offset);
		if (rpc_res.type() != xmlrpc_c::value::TYPE_ARRAY)
		{
			::std::ostringstream oss;
			oss << "[dcs::eesim::detail::neos::client::intermediate_results] Expected type '" << detail::to_string(xmlrpc_c::value::TYPE_ARRAY) << "' (" << xmlrpc_c::value::TYPE_ARRAY << "), got type '" << detail::to_string(rpc_res.type()) << "' (" << rpc_res.type() << ").";
			throw ::std::runtime_error(oss.str());
		}

		carray const msg_offs = xmlrpc_c::value_array(rpc_res).vectorValueValue();
		if (msg_offs.size() != 2)
		{
			throw std::runtime_error("[dcs::eesim::detail::neos::client::intermediate_results] Expected (msg,offset) result pair.");
		}
		if (msg_offs[0].type() != xmlrpc_c::value::TYPE_BYTESTRING)
		{
			::std::ostringstream oss;
			oss << "[dcs::eesim::detail::neos::client::intermediate_results] Expected type '" << detail::to_string(xmlrpc_c::value::TYPE_BYTESTRING) << "' (" << xmlrpc_c::value::TYPE_BYTESTRING << "), got type '" << detail::to_string(rpc_res.type()) << "' (" << rpc_res.type() << ").";
			throw ::std::runtime_error(oss.str());
		}
		if (msg_offs[1].type() != xmlrpc_c::value::TYPE_INT)
		{
			::std::ostringstream oss;
			oss << "[dcs::eesim::detail::neos::client::intermediate_results] Expected type '" << detail::to_string(xmlrpc_c::value::TYPE_INT) << "' (" << xmlrpc_c::value::TYPE_INT << "), got type '" << detail::to_string(rpc_res.type()) << "' (" << rpc_res.type() << ").";
			throw ::std::runtime_error(oss.str());
		}

		cbytestring const msg_enc = xmlrpc_c::value_bytestring(msg_offs[0]).vectorUcharValue();

		res = base64_decode(base64_encode(msg_enc.data(), msg_enc.size()));

		offset = xmlrpc_c::value_int(msg_offs[1]);

		DCS_DEBUG_TRACE("Message: " << res << " - Offset: " << offset);

		return res;
	}


	public: ::std::string final_results(job_credentials const& creds, bool blocking = true) const
	{
		::std::string res;

		::std::string method;
		if (blocking)
		{
			method = "getFinalResults";
		}
		else
		{
			method = "getFinalResultsNonBlocking";
		}

		xmlrpc_c::clientSimple neos;

		xmlrpc_c::value rpc_res;
		neos.call(url_, method, "is", &rpc_res, creds.id, creds.password.c_str());
		if (rpc_res.type() != xmlrpc_c::value::TYPE_BYTESTRING)
		{
			::std::ostringstream oss;
			oss << "[dcs::eesim::detail::neos::client::final_results] Expected type '" << detail::to_string(xmlrpc_c::value::TYPE_BYTESTRING) << "' (" << xmlrpc_c::value::TYPE_BYTESTRING << "), got type '" << detail::to_string(rpc_res.type()) << "' (" << rpc_res.type() << ").";
			throw ::std::runtime_error(oss.str());
		}

		cbytestring const msg_enc = xmlrpc_c::value_bytestring(rpc_res).vectorUcharValue();

		res = base64_decode(base64_encode(msg_enc.data(), msg_enc.size()));

		DCS_DEBUG_TRACE("Message: " << res);

		return res;
	}


	private: ::std::string url_;
}; // client

//const ::std::string client::default_neos_host("neos-dev1.discovery.wisc.edu");
const ::std::string client::default_neos_host("www.neos-server.org");
const int client::default_neos_port(3332);
//const float client::default_zzz_time(1);


/// Execute the given job on the NEOS server and return the result.
inline
::std::string execute_job(client const& neos,
						  ::std::string const& job_xml)
{
	// pre: !empty(job_xml)
	DCS_ASSERT(
			!job_xml.empty(),
			throw ::std::invalid_argument("[dcs::eesim::detail::neos::client::execute_job] Invalid job.")
		);

	::std::string res;

	job_credentials creds;
	creds = neos.submit_job(job_xml);

::std::cerr << "Job Crediantials: (" << creds.id << "," << creds.password << ")" << ::std::endl;//XXX
	res = neos.final_results(creds, false);

	if (res.empty())
	{
		float zzz_time(5);
		unsigned int num_trials(0);
		unsigned int max_num_trials(20);

		job_statuses status;
		do
		{
			++num_trials;

::std::cerr << "Waiting... (Trial: " << num_trials << ", Zzz: " << zzz_time << ")" << ::std::endl;//XXX
			::sleep(zzz_time);
			zzz_time *= 1.5; // exponential backoff (1.5 -> 50% increase per back-off)

			status = neos.job_status(creds);
		}
		while ((status == running_job_status || status == waiting_job_status) && num_trials <= max_num_trials);

		if (status == done_job_status)
		{
			res = neos.final_results(creds);
		}
		else
		{
::std::cerr << "Killing..." << ::std::endl;//XXX
			neos.kill_job(creds);
		}
	}

::std::cerr << "Result: " << res << ::std::endl;//XXX
	return res;
}


/// Create an AMPL job XML from the given XML template.
::std::string make_ampl_job(::std::string const& xml_tmpl,
							::std::string const& model,
							::std::string const& data,
							::std::string const& commands = "",
							::std::string const& options = "",
							::std::string const& comments = "")
{
	// pre: !empty(model)
	DCS_ASSERT(
			!model.empty(),
			throw ::std::invalid_argument("[dcs::eesim::detail::neos::client::make_ampl_job] Invalid AMPL model.")
		);
	// pre: !empty(data)
	DCS_ASSERT(
			!data.empty(),
			throw ::std::invalid_argument("[dcs::eesim::detail::neos::client::make_ampl_job] Invalid AMPL data.")
		);

	::boost::property_tree::ptree xml_tree;
	::std::istringstream iss(xml_tmpl);
	::std::ostringstream oss;

	::boost::property_tree::read_xml(iss, xml_tree);
	xml_tree.put("document.model", "reset;"+model);
	xml_tree.put("document.data", data);
	xml_tree.put("document.commands", commands);
	xml_tree.put("document.options", options);
	xml_tree.put("document.comments", comments);
	::boost::property_tree::write_xml(oss, xml_tree);

	return oss.str();
}


/// Execute the given AMPL job on the NEOS server and return the result.
inline
::std::string execute_ampl_job(client const& neos,
							   optimal_solver_categories category,
							   optimal_solver_ids solver,
							   ::std::string const& model,
							   ::std::string const& data,
							   ::std::string const& commands = "",
							   ::std::string const& options = "",
							   ::std::string const& comments = "")
{
	::std::string xml(neos.solver_template(category, solver, ampl_optimal_solver_input_method));

	return execute_job(neos, make_ampl_job(xml, model, data, commands, options, comments));
}


/// Create an GAMS job XML from the given XML template.
::std::string make_gams_job(::std::string const& xml_tmpl,
							::std::string const& model,
							::std::string const& options = "",
							::std::string const& gdx = "",
							bool want_gdx = false,
							bool want_log = false,
							::std::string const& comments = "")
{
	// pre: !empty(model)
	DCS_ASSERT(
			!model.empty(),
			throw ::std::invalid_argument("[dcs::eesim::detail::neos::client::make_gams_job] Invalid GAMS model.")
		);

	::boost::property_tree::ptree xml_tree;
	::std::istringstream iss(xml_tmpl);
	::std::ostringstream oss;

	::boost::property_tree::read_xml(iss, xml_tree);
	xml_tree.put("document.model", model);
	xml_tree.put("document.options", options);
	xml_tree.put("document.gdx", gdx);
	xml_tree.put("document.wantgdx", want_gdx);
	xml_tree.put("document.wantlog", want_log);
	xml_tree.put("document.comments", comments);
	::boost::property_tree::write_xml(oss, xml_tree);

	return oss.str();
}


/// Execute the given GAMS job on the NEOS server and return the result.
inline
::std::string execute_gams_job(client const& neos,
							   optimal_solver_categories category,
							   optimal_solver_ids solver,
							   ::std::string const& model,
							   ::std::string const& options = "",
							   ::std::string const& gdx = "",
							   bool want_gdx = false,
							   bool want_log = false,
							   ::std::string const& comments = "")
{
	::std::string xml(neos.solver_template(category, solver, gams_optimal_solver_input_method));

	return execute_job(neos, make_gams_job(xml, model, options, gdx, want_gdx, want_log, comments));
}

}}}} // Namespace dcs::eesim::detail::neos


#endif // DCS_EESIM_DETAIL_NEOS_CLIENT_HPP
