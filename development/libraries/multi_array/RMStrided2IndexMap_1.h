template <>
class RMStrided2IndexMap<1>
{
public:

  enum { dim = 1, dim_min_1 = 0, dim_plus_1 = 2 };
  typedef int type;

public:

  inline RMStrided2IndexMap(const Tuple<1,int>& size)
  {}

  RMStrided2IndexMap()
  {}

  inline int index(const Tuple<1,int>& crd) const {
    int result = crd[0];
    return result;
  }

  Step2IndexMap<1> transpose() const {
    return Step2IndexMap<1>(Tuple<1,int>(1));
  }
};






