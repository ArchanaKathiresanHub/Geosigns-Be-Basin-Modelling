#ifndef HELPERFUNCTIONS_HH
#define HELPERFUNCTIONS_HH


template <class T>
inline std::string ToString (const T& t)
{
std::stringstream ss;
ss << t;
return ss.str();
}
template <typename T>
T ToNumber ( const std::string &Text )//Text not by const reference so that the function can be used with a 
{                               //character array as argument
	std::stringstream ss(Text);
	T result;
	return ss >> result ? result : 0;
}

#endif
