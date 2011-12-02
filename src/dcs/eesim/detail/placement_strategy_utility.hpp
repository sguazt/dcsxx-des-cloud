#ifndef DCS_EESIM_DETAIL_PLACEMENT_STRATEGY_UTILITY_HPP
#define DCS_EESIM_DETAIL_PLACEMENT_STRATEGY_UTILITY_HPP


#include <dcs/debug.hpp>
#include <dcs/eesim/physical_resource_category.hpp>
#include <dcs/memory.hpp>
#include <functional>
#include <map>


namespace dcs { namespace eesim { namespace detail {

//FIXME: only the single-resource (CPU) case is handled
template <typename PM>
struct pm_comparator
{
	bool operator()(::dcs::shared_ptr<PM> const& ptr_pm1, ::dcs::shared_ptr<PM> const& ptr_pm2)
	{
		return (((ptr_pm1->resource(cpu_resource_category)->capacity()*ptr_pm1->resource(cpu_resource_category)->utilization_threshold())
				 <
				 (ptr_pm2->resource(cpu_resource_category)->capacity()*ptr_pm2->resource(cpu_resource_category)->utilization_threshold()))
				||
				(((ptr_pm1->resource(cpu_resource_category)->capacity()*ptr_pm1->resource(cpu_resource_category)->utilization_threshold())
				 ==
				 (ptr_pm2->resource(cpu_resource_category)->capacity()*ptr_pm2->resource(cpu_resource_category)->utilization_threshold()))
				 &&
				 ptr_pm1->id() < ptr_pm2->id())
			   );
	}
};


/// Compare two physical machines according to the capacity they can provide by means of their resources.
template <typename PhyMachT, typename CompareT = ::std::less<typename PhyMachT::real_type> >
class physical_machine_comparator
{
	public: physical_machine_comparator(CompareT cmp = CompareT())
	: cmp_(cmp)
	{
	}


	//FIXME: We assume that a machine is less than another one if it has physical resouces with smaller capacity
	//FIXME: We do not consider machine with different type and number of physical resources.

	public: bool operator()(PhyMachT const& lhs, PhyMachT const& rhs) const
	{
		typedef typename PhyMachT::resource_pointer resource_pointer;

		::std::vector<resource_pointer> lhs_res_cont(lhs.resources());

		::std::size_t n(lhs_res_cont.size());
		for (::std::size_t i = 0; i < n; ++i)
		{
			resource_pointer ptr_lhs_res(lhs_res_cont[i]);

			/// check: paranoid check
			DCS_DEBUG_ASSERT( ptr_lhs_res );

			resource_pointer ptr_rhs_res;
			try
			{
				ptr_rhs_res = rhs.resource(ptr_lhs_res->category());
			}
			catch (...)
			{
				// Category not found on the other machine -> skip
				continue;
			}

			/// check: paranoid check
			DCS_DEBUG_ASSERT( ptr_rhs_res );

			bool res;
			res = cmp_(ptr_lhs_res->capacity()*ptr_lhs_res->utilization_threshold(),
					   ptr_rhs_res->capacity()*ptr_rhs_res->utilization_threshold());
			if (!res)
			{
				return false;
			}
		}

		return true;
	}

	private: CompareT cmp_;
}; // physical_machine_comparator


template <typename PhyMachT>
struct physical_machine_less_comparator
{
	bool operator()(PhyMachT const& lhs, PhyMachT const& rhs) const
	{
		return physical_machine_comparator< PhyMachT,
											::std::less<typename PhyMachT::real_type> >()(lhs, rhs);
	}
};


template <typename PhyMachT>
struct physical_machine_greater_comparator
{
	bool operator()(PhyMachT const& lhs, PhyMachT const& rhs) const
	{
		return physical_machine_comparator< PhyMachT,
											::std::greater<typename PhyMachT::real_type> >()(lhs, rhs);
	}
};


/// Compare two physical machines according to the capacity they can provide by means of their resources.
template <typename PhyMachT, typename CompareT = ::std::less<typename PhyMachT::real_type> >
class ptr_physical_machine_comparator
{
	public: ptr_physical_machine_comparator(CompareT cmp = CompareT())
	: cmp_(cmp)
	{
	}


	//FIXME: We assume that a machine is less than another one if it has physical resouces with smaller capacity
	//FIXME: We do not consider machine with different type and number of physical resources.

	public: bool operator()(::dcs::shared_ptr<PhyMachT> const& lhs, ::dcs::shared_ptr<PhyMachT> const& rhs) const
	{
		typedef typename PhyMachT::resource_pointer resource_pointer;

		::std::vector<resource_pointer> lhs_res_cont(lhs->resources());

		::std::size_t n(lhs_res_cont.size());
		for (::std::size_t i = 0; i < n; ++i)
		{
			resource_pointer ptr_lhs_res(lhs_res_cont[i]);

			/// check: paranoid check
			DCS_DEBUG_ASSERT( ptr_lhs_res );

			resource_pointer ptr_rhs_res;
			try
			{
				ptr_rhs_res = rhs->resource(ptr_lhs_res->category());
			}
			catch (...)
			{
				// Category not found on the other machine -> skip
				continue;
			}

			/// check: paranoid check
			DCS_DEBUG_ASSERT( ptr_rhs_res );

			bool res;
			res = cmp_(ptr_lhs_res->capacity()*ptr_lhs_res->utilization_threshold(),
					   ptr_rhs_res->capacity()*ptr_rhs_res->utilization_threshold());
			if (!res)
			{
				return false;
			}
		}

		return true;
	}

	private: CompareT cmp_;
}; // physical_machine_comparator


template <typename PhyMachT>
struct ptr_physical_machine_less_comparator
{
	bool operator()(::dcs::shared_ptr<PhyMachT> const& lhs, ::dcs::shared_ptr<PhyMachT> const& rhs) const
	{
		return ptr_physical_machine_comparator< PhyMachT,
												::std::less<typename PhyMachT::real_type> >()(lhs, rhs);
	}
};


template <typename PhyMachT>
struct ptr_physical_machine_greater_comparator
{
	bool operator()(::dcs::shared_ptr<PhyMachT> const& lhs, ::dcs::shared_ptr<PhyMachT> const& rhs) const
	{
		return ptr_physical_machine_comparator< PhyMachT,
												::std::greater<typename PhyMachT::real_type> >()(lhs, rhs);
	}
};


/// Compare two virtual machines according to the share they want.
template <typename VirtMachT, typename CompareT = ::std::less<typename VirtMachT::real_type> >
class virtual_machine_comparator
{
	public: virtual_machine_comparator(CompareT cmp = CompareT())
	: cmp_(cmp)
	{
	}

	public: bool operator()(VirtMachT const& lhs, VirtMachT const& rhs) const
	{
		//FIXME: We assume that a VM is less than another one if it has physical resouces with smaller capacity
		//FIXME: We do not consider VM with different type and number of physical resources.

		typedef typename VirtMachT::real_type real_type;
		typedef typename VirtMachT::resource_share_container share_container;
		typedef typename share_container::value_type share_pair_type;

		share_container lhs_shares(lhs.wanted_resource_shares());

		::std::size_t n(lhs_shares.size());
		for (::std::size_t i = 0; i < n; ++i)
		{
			share_pair_type lhs_share_pair(lhs_shares[i]);
			real_type lhs_share(lhs_share_pair.second);
			real_type rhs_share(0);

			try
			{
				rhs_share = rhs.wanted_resource_share(lhs_share_pair.first);
			}
			catch (...)
			{
				// Category not found on the other VM -> skip
				continue;
			}

			bool res(cmp_(lhs_share, rhs_share));
			if (!res)
			{
				return false;
			}
		}

		return true;
	}


	private: CompareT cmp_;
}; // virtual_machine_comparator


template <typename VirtMachT>
struct virtual_machine_less_comparator
{
	bool operator()(VirtMachT const& lhs, VirtMachT const& rhs) const
	{
		return virtual_machine_comparator< VirtMachT,
										   ::std::less<typename VirtMachT::real_type> >()(lhs, rhs);
	}
};


template <typename VirtMachT>
struct virtual_machine_greater_comparator
{
	bool operator()(VirtMachT const& lhs, VirtMachT const& rhs) const
	{
		return virtual_machine_comparator< VirtMachT,
										   ::std::greater<typename VirtMachT::real_type> >()(lhs, rhs);
	}
};


/// Compare two virtual machines according to the share they want.
template <typename VirtMachT, typename CompareT = ::std::less<typename VirtMachT::real_type> >
class ptr_virtual_machine_comparator
{
	public: ptr_virtual_machine_comparator(CompareT cmp = CompareT())
	: cmp_(cmp)
	{
	}

	public: bool operator()(::dcs::shared_ptr<VirtMachT> const& lhs, ::dcs::shared_ptr<VirtMachT> const& rhs) const
	{
		//FIXME: We assume that a VM is less than another one if it has physical resouces with smaller capacity
		//FIXME: We do not consider VM with different type and number of physical resources.

		typedef typename VirtMachT::real_type real_type;
		typedef typename VirtMachT::resource_share_container share_container;
		typedef typename share_container::value_type share_pair_type;

		share_container lhs_shares(lhs->wanted_resource_shares());

		::std::size_t n(lhs_shares.size());
		for (::std::size_t i = 0; i < n; ++i)
		{
			share_pair_type lhs_share_pair(lhs_shares[i]);
			real_type lhs_share(lhs_share_pair.second);
			real_type rhs_share(0);

			try
			{
				rhs_share = rhs->wanted_resource_share(lhs_share_pair.first);
			}
			catch (...)
			{
				// Category not found on the other VM -> skip
				continue;
			}

			bool res(cmp_(lhs_share, rhs_share));
			if (!res)
			{
				return false;
			}
		}

		return true;
	}


	private: CompareT cmp_;
}; // virtual_machine_comparator


template <typename VirtMachT>
struct ptr_virtual_machine_less_comparator
{
	bool operator()(::dcs::shared_ptr<VirtMachT> const& lhs, ::dcs::shared_ptr<VirtMachT> const& rhs) const
	{
		return ptr_virtual_machine_comparator< VirtMachT,
											   ::std::less<typename VirtMachT::real_type> >()(lhs, rhs);
	}
};


template <typename VirtMachT>
struct ptr_virtual_machine_greater_comparator
{
	bool operator()(::dcs::shared_ptr<VirtMachT> const& lhs, ::dcs::shared_ptr<VirtMachT> const& rhs) const
	{
		return ptr_virtual_machine_comparator< VirtMachT,
											   ::std::greater<typename VirtMachT::real_type> >()(lhs, rhs);
	}
};


/// Compare two virtual machines according to the share they want.
template <typename VirtMachT, typename CompareT = ::std::less<typename VirtMachT::real_type> >
class ptr_virtual_machine_by_share_comparator
{
	public: typedef VirtMachT virtual_machine_type;
	public: typedef CompareT comparator_type;
	public: typedef ::dcs::shared_ptr<virtual_machine_type> virtual_machine_pointer;
	public: typedef typename virtual_machine_type::real_type real_type;
	public: typedef typename virtual_machine_type::identifier_type virtual_machine_identifier_type;
	public: typedef ::std::map<physical_resource_category,real_type> share_container;
	public: typedef ::std::map<virtual_machine_identifier_type,share_container> virtual_machine_share_container;

	public: template <typename ForwardIterT>
	ptr_virtual_machine_by_share_comparator(ForwardIterT share_first, ForwardIterT share_last, comparator_type cmp = comparator_type())
	: shares_(share_first, share_last),
	  cmp_(cmp)
	{
	}


	public: bool operator()(virtual_machine_pointer const& lhs, virtual_machine_pointer const& rhs) const
	{
		// Return true if the number of physical resources of the first VM such
		// that the comparator function return true is greater than the one for
		// the second VM.
 
		typedef typename share_container::const_iterator share_iterator;

		int cmps(0);

		share_iterator share_end_it(shares_.at(lhs->id()).end());
		for (share_iterator share_it = shares_.at(lhs->id()).begin(); share_it != share_end_it; ++share_it)
		{
			physical_resource_category category(share_it->first);

			if (shares_.at(rhs->id()).count(category) == 0)
			{
				continue;
			}

			real_type lhs_share(share_it->second);
			real_type rhs_share(shares_.at(rhs->id()).at(category));

			cmps += cmp_(lhs_share, rhs_share) ? 1 : -1;
		}

		return (cmps > 0) ? true : false;
	}


	private: virtual_machine_share_container shares_;
	private: CompareT cmp_;
}; // virtual_machine_by_share_comparator


template <typename VirtMachT>
class ptr_virtual_machine_greater_by_share_comparator: public ptr_virtual_machine_by_share_comparator< VirtMachT,::std::greater<typename VirtMachT::real_type> >
{
	private: typedef ptr_virtual_machine_by_share_comparator<VirtMachT,::std::greater<typename VirtMachT::real_type> > base_type;
	public: typedef VirtMachT virtual_machine_type;
	public: typedef ::dcs::shared_ptr<virtual_machine_type> virtual_machine_pointer;
	public: typedef typename virtual_machine_type::real_type real_type;
	public: typedef typename virtual_machine_type::identifier_type virtual_machine_identifier_type;
	public: typedef typename virtual_machine_type::resource_share_container share_container;
	public: typedef ::std::map<virtual_machine_identifier_type,share_container> virtual_machine_share_container;


	public: template <typename ForwardIterT>
	ptr_virtual_machine_greater_by_share_comparator(ForwardIterT share_first, ForwardIterT share_last)
	: base_type(share_first, share_last)
	{
	}


	public: bool operator()(virtual_machine_pointer const& lhs, virtual_machine_pointer const& rhs) const
	{
		return base_type::operator()(lhs, rhs);
	}
}; // ptr_virtual_machine_greater_by_share_comparator

}}} // Namespace dcs::eesim::detail


#endif // DCS_EESIM_DETAIL_PLACEMENT_STRATEGY_UTILITY_HPP
