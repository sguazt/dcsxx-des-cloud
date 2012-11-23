/**
 * \file dcs/eesim/config/optimal_solver_ids.hpp
 *
 * \brief Configuration for optimal solver identifiers.
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

#ifndef DCS_EESIM_CONFIG_OPTIMAL_SOLVER_IDS_HPP
#define DCS_EESIM_CONFIG_OPTIMAL_SOLVER_IDS_HPP


#include <dcs/eesim/optimal_solver_ids.hpp>
#include <iosfwd>


namespace dcs { namespace eesim { namespace config {

typedef ::dcs::eesim::optimal_solver_ids optimal_solver_ids;

/*
enum optimal_solver_ids
{
	alphaecp_optimal_solver_id, ///< AlphaECP (http://archimedes.scs.uiuc.edu/alphaecp/alphaecp.html)
	asa_optimal_solver_id, ///< ASA (http://www.ingber.com/)
	baron_optimal_solver_id, ///< BARON (http://archimedes.scs.uiuc.edu/baron/baron.html)
	bdmlp_optimal_solver_id, ///< BDMLP (http://www.mcs.anl.gov/otc/Guide/OptWeb/continuous/constrained/linearprog/)
	biqmac_optimal_solver_id, ///< BiqMac (http://biqmac.uni-klu.ac.at/)
	blmvm_optimal_solver_id, ///< BLMVM (http://www.mcs.anl.gov/BLMVM)
	bnbs_optimal_solver_id, ///< bnbs (http://plato.la.asu.edu/)
	bonmin_optimal_solver_id, ///< COIN-OR Bonmin (https://projects.coin-or.org/Bonmin)
	bpmpd_optimal_solver_id, ///< bpmpd (http://www.sztaki.hu/~meszaros/bpmpd/)
	cbc_optimal_solver_id, ///< COIN-OR Cbc (https://projects.coin-or.org/Cbc)
	clp_optimal_solver_id, ///< COIN-OR Clp (http://www.coin-or.org/)
	concorde_optimal_solver_id, ///< concorde (http://www.tsp.gatech.edu/concorde.html)
	condor_optimal_solver_id, ///< condor (http://www.applied-mathematics.net/CONDORManual/CONDORManual.html)
	conopt_optimal_solver_id, ///< CONOPT (http://www.conopt.com/)
	couenne_optimal_solver_id, ///< COIN-OR Couenne (https://projects.coin-or.org/Couenne)
	csdp_optimal_solver_id, ///< csdp (https://projects.coin-or.org/Csdp/)
	ddsip_optimal_solver_id, ///< ddsip (http://plato.asu.edu/ddsip-man.pdf)
	dicopt_optimal_solver_id, ///< DICOPT (http://archimedes.scs.uiuc.edu/dicopt/dicopt.html)
	dsdp_optimal_solver_id, ///< DSDP (http://www.mcs.anl.gov/DSDP)
	feaspump_optimal_solver_id, ///< feaspump (http://www.dei.unipd.it/~fisch/papers/feasibility_pump_201.pdf)
	filmint_optimal_solver_id, ///< FilMINT (http://www.mcs.anl.gov/home/otc/Guide/OptWeb/continuous/constrained/nonlinearcon)
	filtermpec_optimal_solver_id, ///< filterMPEC (http://www.mcs.anl.gov/home/otc/Guide/OptWeb/continuous/constrained/nonlinearcon)
	fortmp_optimal_solver_id, ///< FortMP (http://www.optirisk-systems.com/)
	gamsampl_optimal_solver_id, ///< GAMS-AMPL
	glpk_optimal_solver_id, ///< Glpk (http://www.gnu.org/software/glpk/glpk.html)
	gurobi_optimal_solver_id, ///< Gurobi (http://www.gurobi.com/)
	icos_optimal_solver_id, ///< icos (http://sites.google.com/site/ylebbah/icos)
	ipopt_optimal_solver_id, ///< COIN-OR Ipopt (https://projects.coin-or.org/Ipopt)
	knitro_optimal_solver_id, ///< KNITRO (http://www.ziena.com/knitro)
	lancelot_optimal_solver_id, ///< LANCELOT (http://www.mcs.anl.gov/otc/Guide/OptWeb/continuous/constrained/nonlinearcon)
	lbfgsb_optimal_solver_id, ///< L-BFGS-B (http://www.mcs.anl.gov/otc/GUIDE/OptWeb/continuous/constrained/boundcon/)
	lindoglobal_optimal_solver_id, ///< LINDOGlobal (http://archimedes.scs.uiuc.edu/lindoglobal/lindoglobal.html)
	loqo_optimal_solver_id, ///< LOQO (http://www.princeton.edu/~rvdb/loqo/LOQO.html)
	lrambo_optimal_solver_id, ///< LRAMBO (http://www.mcs.anl.gov/LRAMBO)
	miles_optimal_solver_id, ///< MILES (http://www.mcs.anl.gov/otc/Guide/OptWeb/complementarity)
	minlp_optimal_solver_id, ///< MINLP (http://www.mcs.anl.gov/otc/Guide/OptWeb/discrete/integerprog/index.html)
	minos_optimal_solver_id, ///< MINOS (http://www.sbsi-sol-optimize.com/asp/sol_product_minos.htm)
	minto_optimal_solver_id, ///< MINTO (http://www.mcs.anl.gov/otc/Guide/OptWeb/discrete/integerprog/index.html)
	mosek_optimal_solver_id, ///< MOSEK (http://www.mosek.com/)
	mslip_optimal_solver_id, ///< MSLiP (http://sba.management.dal.ca/profs/hgassmann/mslip.html)
	netflo_optimal_solver_id, ///< NETFLO (ftp://dimacs.rutgers.edu/pub/netflow/mincost/solver-1)
	nlpec_optimal_solver_id, ///< NLPEC (http://www.mcs.anl.gov/otc/Guide/OptWeb/complementarity)
	nmtr_optimal_solver_id, ///< NMTR (http://www.mcs.anl.gov/home/otc/Guide/OptWeb/continuous/unconstrained/)
	nsips_optimal_solver_id, ///< nsips (http://www.norg.uminho.pt/aivaz/nsips.html)
	ooqp_optimal_solver_id, ///< OOQP (http://www.cs.wisc.edu/~swright/ooqp)
	path_optimal_solver_id, ///< PATH (http://www.cs.wisc.edu/cpnet/aboutcp.html)
	pathnlp_optimal_solver_id, ///< PATHNLP (http://www.mcs.anl.gov/otc/Guide/OptWeb/complementarity)
	penbmi_optimal_solver_id, ///< penbmi (http://www.penopt.com/)
	pennon_optimal_solver_id, ///< PENNON (http://www2.am.uni-erlangen.de/~kocvara/pennon/)
	pensdp_optimal_solver_id, ///< pensdp (http://www.penopt.com/pensdp.html)
	pcx_optimal_solver_id, ///< PCx (http://www.cs.wisc.edu/~swright/PCx)
	pgapack_optimal_solver_id, ///< PGAPack (http://www-fp.mcs.anl.gov/CCST/research/reports_pre1998/comp_bio/stalk/pgapack.html)
	pswarm_optimal_solver_id, ///< PSwarm (http://www.norg.uminho.pt/aivaz/pswarm)
	qsoptex_optimal_solver_id, ///< qsopt_ex (http://www.dii.uchile.cl/~daespino/QSoptExact_doc/main.html)
	relax4_optimal_solver_id, ///< RELAX4 (http://www.mcs.anl.gov/otc/Guide/OptWeb/continuous/constrained/network/)
	sbb_optimal_solver_id, ///< SBB (http://www.mcs.anl.gov/otc/Guide/OptWeb/discrete/integerprog/index.html)
	scip_optimal_solver_id, ///< scip (http://scip.zib.de/)
	sdpa_optimal_solver_id, ///< SDPA (http://sdpa.sourceforge.net/)
	sdplr_optimal_solver_id, ///< sdplr (http://dollar.biz.uiowa.edu/~burer/software/SDPLR/)
	sdpt3_optimal_solver_id, ///< sdpt3 (http://www.math.nus.edu.sg/~mattohkc/sdpt3.html)
	sedumi_optimal_solver_id, ///< sedumi (http://plato.asu.edu/ftp/SeDuMi_Guide_11.pdf)
	snopt_optimal_solver_id, ///< SNOPT (http://www.sbsi-sol-optimize.com/asp/sol_product_snopt.htm)
	symphony_optimal_solver_id, ///< SYMPHONY (https://projects.coin-or.org/SYMPHONY)
	tron_optimal_solver_id, ///< TRON (http://www.mcs.anl.gov/~more/tron/)
	xpressmp_optimal_solver_id, ///< XpressMP (http://www.dashoptimization.com/)
	worhp_optimal_solver_id ///< WORHP (http://www.worhp.de/)
};
*/


template <typename CharT, typename CharTraitsT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, optimal_solver_ids id)
{
	switch (id)
	{
		case acrs_optimal_solver_id:
			os << "ACRS";
			break;
		case algencan_optimal_solver_id:
			os << "ALGENCAN";
			break;
		case alphaecp_optimal_solver_id:
			os << "AlphaECP";
			break;
		case asa_optimal_solver_id:
			os << "ASA";
			break;
		case baron_optimal_solver_id:
			os << "BARON";
			break;
		case bdmlp_optimal_solver_id:
			os << "BDMLP";
			break;
		case biqmac_optimal_solver_id:
			os << "BiqMac";
			break;
		case blmvm_optimal_solver_id:
			os << "BLMVM";
			break;
		case bnbs_optimal_solver_id:
			os << "bnbs";
			break;
		case bonmin_optimal_solver_id:
			os << "Bonmin";
			break;
		case bpmpd_optimal_solver_id:
			os << "bpmpd";
			break;
		case cbc_optimal_solver_id:
			os << "Cbc";
			break;
		case clp_optimal_solver_id:
			os << "Clp";
			break;
		case concorde_optimal_solver_id:
			os << "concorde";
			break;
		case condor_optimal_solver_id:
			os << "condor";
			break;
		case conopt_optimal_solver_id:
			os << "conopt";
			break;
		case couenne_optimal_solver_id:
			os << "Couenne";
			break;
		case cplex_optimal_solver_id:
			os << "CPLEX";
			break;
		case csdp_optimal_solver_id:
			os << "csdp";
			break;
		case ddsip_optimal_solver_id:
			os << "ddsip";
			break;
		case dicopt_optimal_solver_id:
			os << "DICOPT";
			break;
		case donlp2_optimal_solver_id:
			os << "DONLP2";
			break;
		case dsdp_optimal_solver_id:
			os << "DSDP";
			break;
		case feaspump_optimal_solver_id:
			os << "feaspump";
			break;
		case filmint_optimal_solver_id:
			os << "FilMINT";
			break;
		case filter_optimal_solver_id:
			os << "filter";
		case filtermpec_optimal_solver_id:
			os << "filterMPEC";
			break;
		case fortmp_optimal_solver_id:
			os << "FortMP";
			break;
		case fsqp_optimal_solver_id:
			os << "FSQP";
			break;
		case gamsampl_optimal_solver_id:
			os << "GAMS-AMPL";
			break;
		case glpk_optimal_solver_id:
			os << "Glpk";
			break;
		case gurobi_optimal_solver_id:
			os << "Gurobi";
			break;
		case icos_optimal_solver_id:
			os << "icos";
			break;
		case ipopt_optimal_solver_id:
			os << "Ipopt";
			break;
		case knitro_optimal_solver_id:
			os << "KNITRO";
			break;
		case lancelot_optimal_solver_id:
			os << "LANCELOT";
			break;
		case lbfgsb_optimal_solver_id:
			os << "L-BFGS-B";
			break;
		case lgo_optimal_solver_id:
			os << "LGO";
			break;
		case lindoglobal_optimal_solver_id:
			os << "LINDOGlobal";
			break;
		case loqo_optimal_solver_id:
			os << "LOQO";
			break;
		case lpsolve_optimal_solver_id:
			os << "LP_SOLVE";
			break;
		case lrambo_optimal_solver_id:
			os << "LRAMBO";
			break;
		case miles_optimal_solver_id:
			os << "MILES";
			break;
		case minlp_optimal_solver_id:
			os << "MINLP";
			break;
		case minos_optimal_solver_id:
			os << "MINOS";
			break;
		case minto_optimal_solver_id:
			os << "MINTO";
			break;
		case mosek_optimal_solver_id:
			os << "MOSEK";
			break;
		case mslip_optimal_solver_id:
			os << "MSLiP";
			break;
		case mlocpsoa_optimal_solver_id:
			os << "MLOCPSOA";
			break;
		case netflo_optimal_solver_id:
			os << "NETFLO";
			break;
		case nlpec_optimal_solver_id:
			os << "NLPEC";
			break;
		case nmtr_optimal_solver_id:
			os << "NMTR";
			break;
		case nomad_optimal_solver_id:
			os << "NOMAD";
			break;
		case npsol_optimal_solver_id:
			os << "NPSOL";
			break;
		case nsips_optimal_solver_id:
			os << "nsips";
			break;
		case ooqp_optimal_solver_id:
			os << "OOQP";
			break;
		case path_optimal_solver_id:
			os << "PATH";
			break;
		case pathnlp_optimal_solver_id:
			os << "PATHNLP";
			break;
		case penbmi_optimal_solver_id:
			os << "penbmi";
			break;
		case pennon_optimal_solver_id:
			os << "PENNON";
			break;
		case pensdp_optimal_solver_id:
			os << "pensdp";
			break;
		case pcx_optimal_solver_id:
			os << "PCx";
			break;
		case pgapack_optimal_solver_id:
			os << "PGAPack";
			break;
		case pswarm_optimal_solver_id:
			os << "PSwarm";
			break;
		case qsoptex_optimal_solver_id:
			os << "qsopt_ex";
			break;
		case relax4_optimal_solver_id:
			os << "RELAX4";
			break;
		case sbb_optimal_solver_id:
			os << "SBB";
			break;
		case scip_optimal_solver_id:
			os << "scip";
			break;
		case sdpa_optimal_solver_id:
			os << "SDPA";
			break;
		case sdplr_optimal_solver_id:
			os << "sdplr";
			break;
		case sdpt3_optimal_solver_id:
			os << "sdpt3";
			break;
		case sedumi_optimal_solver_id:
			os << "sedumi";
			break;
		case snopt_optimal_solver_id:
			os << "SNOPT";
			break;
		case symphony_optimal_solver_id:
			os << "SYMPHONY";
			break;
		case tron_optimal_solver_id:
			os << "TRON";
			break;
		case worhp_optimal_solver_id:
			os << "WORHP";
			break;
		case wsatoip_optimal_solver_id:
			os << "WSAT(OIP)";
			break;
		case xpressmp_optimal_solver_id:
			os << "XpressMP";
			break;
	}

	return os;
}


}}} // Namespace dcs::eesim::config


#endif // DCS_EESIM_CONFIG_OPTIMAL_SOLVER_IDS_HPP
