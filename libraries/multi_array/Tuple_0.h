template <typename ENTRY>
class Tuple<0,ENTRY> 
{
public:

  enum { dim = 0, DIM_min_1 = 0 };
  typedef ENTRY Entry;

  inline Tuple() {
  }

  inline ~Tuple() {
  }

  inline Tuple(const ENTRY* data) {
  }

  inline explicit Tuple(const ENTRY& value) {
  }

/*    inline Tuple<1, ENTRY> cons(const ENTRY& value) const { */
/*      return Tuple<1, ENTRY>(value); */
/*    } */

/*    inline Tuple<1, ENTRY> snoc(const ENTRY& value) const { */
/*      return Tuple<1, ENTRY>(value); */
/*    } */

/*   inline Tuple<DIM2, ENTRY> construct(const Tuple<DIM2,ENTRY>& product) const { */
/*     Tuple<DIM2, ENTRY> result; */
/*     int d = 0; */
/*     for (; d < DIM2; ++d) */
/*       result[d] = product[d]; */
/*     assert(d == DIM2); */
/*     return result;      */
/*   } */
};
