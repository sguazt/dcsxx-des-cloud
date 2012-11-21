#ifndef DCS_EESIM_IDENTIFIER_GENERATOR_HPP
#define DCS_EESIM_IDENTIFIER_GENERATOR_HPP


namespace dcs { namespace eesim {

template <typename T>
class identifier_generator
{
	public: typedef T value_type;


	public: explicit identifier_generator(value_type x0 = value_type/*zero*/())
	: x_(x0)
	{
	}


	public: value_type operator()()
	{
		return x_++;
	}


	public: void reset(value_type x0 = value_type/*zero*/())
	{
		x_ = x0;
	}


	private: value_type x_;
};

}} // Namespace dcs::eesim


#endif // DCS_EESIM_IDENTIFIER_GENERATOR_HPP
