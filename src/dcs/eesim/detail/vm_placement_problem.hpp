#ifndef DCS_EESIM_DETAIL_VM_PLACEMENT_PROBLEM_HPP
#define DCS_EESIM_DETAIL_VM_PLACEMENT_PROBLEM_HPP


#include <boost/numeric/ublasx/operation/num_columns.hpp>
#include <boost/numeric/ublasx/operation/num_rows.hpp>
#include <cstddef>
#include <dcs/eesim/physical_resource_category.hpp>
#include <map>
#include <utility>
#include <vector>


namespace dcs { namespace eesim { namespace detail {

template <typename TraitsT>
class vm_placement_problem_result
{
	public: typedef TraitsT traits_type;
	public: typedef typename traits_type::real_type real_type;
	public: typedef typename traits_type::physical_machine_identifier_type physical_machine_identifier_type;
	public: typedef typename traits_type::virtual_machine_identifier_type virtual_machine_identifier_type;
    public: typedef ::std::pair<physical_machine_identifier_type,virtual_machine_identifier_type> physical_virtual_machine_pair_type;
    public: typedef ::std::vector< ::std::pair<physical_resource_category,real_type> > resource_share_container;
    public: typedef ::std::map<physical_virtual_machine_pair_type,resource_share_container> physical_virtual_machine_map;


	public: void reset()
	{
		solved_ = false;
		cost_ = ::std::numeric_limits<real_type>::infinity();
		placement_.clear();
	}


	public: bool solved() const
	{
		return solved_;
	}


	public: void solved(bool value)
	{
		solved_ = value;
	}


	public: real_type cost() const
	{
		return cost_;
	}


	public: void cost(real_type value)
	{
		cost_ = value;
	}


	public: physical_virtual_machine_map const& placement() const
	{
		return placement_;
	}


	public: physical_virtual_machine_map& placement()
	{
		return placement_;
	}


	public: void placement(physical_virtual_machine_map const& value)
	{
		placement_ = value;
	}


	private: bool solved_;
	private: real_type cost_;
	private: physical_virtual_machine_map placement_;
};


template <typename TraitsT, typename ProblemDescrT, typename ProblemResultT>
vm_placement_problem_result<TraitsT> make_vm_placement_problem_result(ProblemDescrT problem_descr, ProblemResultT problem_res)
{
	typedef TraitsT traits_type;
	typedef vm_placement_problem_result<traits_type> result_type;
	typedef typename result_type::real_type real_type;
	typedef typename result_type::resource_share_container resource_share_container;
	typedef typename result_type::physical_machine_identifier_type physical_machine_identifier_type;
	typedef typename result_type::virtual_machine_identifier_type virtual_machine_identifier_type;

	result_type res;

	res.cost(problem_res.cost());

	typename ProblemResultT::smallint_matrix_type placement_flags(problem_res.virtual_machine_placement());
	typename ProblemResultT::real_matrix_type placement_shares(problem_res.virtual_machine_shares());

	::std::size_t npm(::boost::numeric::ublasx::num_rows(placement_flags));
	::std::size_t nvm(::boost::numeric::ublasx::num_columns(placement_flags));

	for (::std::size_t i = 0; i < npm; ++i)
	{
		physical_machine_identifier_type pm_id(problem_descr.pm_ids[i]);

		real_type share_sum(0); // Normalization factor
		for (::std::size_t k = 0; k < 2; ++k)
		{
			for (::std::size_t j = 0; j < nvm; ++j)
			{
				if (placement_flags(i,j))
				{
					if (k == 0)
					{
						share_sum += placement_shares(i,j);
					}
					else
					{
						virtual_machine_identifier_type vm_id(problem_descr.vm_ids[j]);
						real_type share(placement_shares(i,j));
						if (share_sum > 1)
						{
							share /= share_sum;
						}
						//FIXME: CPU resource category is hard-coded.
						resource_share_container shares(1, ::std::make_pair(cpu_resource_category, share));
						res.placement()[::std::make_pair(pm_id, vm_id)] = shares;
					}
				}
			}
		}
	}

	return res;
}

}}} // Namespace dcs::eesim::detail

#endif // DCS_EESIM_DETAIL_VM_PLACEMENT_PROBLEM_HPP
