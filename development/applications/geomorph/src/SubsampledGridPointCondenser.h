#ifndef __SubsampledGridPointCondenser_HH__
#define __SubsampledGridPointCondenser_HH__


namespace AllochMod {

  /** @addtogroup AllochMod
   *
   * @{
   */

  /// \file SubsampledGridPointCondenser.h
  /// \brief 

  /// \brief Provide a machanism for 
  class SubsampledGridPointCondenser {

  public :

    /// \var DefaultSubSampling
    /// \brief 
    static const int DefaultSubSampling = 1;

    SubsampledGridPointCondenser ();

    /// \brief 
    ///
    /// \param[in] initialXSubSampling 
    /// \param[in] initialYSubSampling 
    /// \warning Both parameters must be greater than zero.
    SubsampledGridPointCondenser ( const int initialXSubSampling,
                                   const int initialYSubSampling );

    /// \brief Define a new sub-sampling ratio.
    void setSubSampling ( const int newXSubSampling,
                          const int newYSubSampling );

    /// \brief Returns true if the subscript (i,j) is a member of the sub-sampled grid.
    bool operator ()( const int i,
                      const int j ) const;

  private :
    
    /// \brief The subsampling ratio for the x coordinate.
    int xSubSampling;

    /// \brief The subsampling ratio for the y coordinate.
    int ySubSampling;

  };

  /** @} */

}


#endif // __SubsampledGridPointCondenser_HH__
