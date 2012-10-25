#ifndef _MULTI_ARRAY_CMADVANCER_H_
#define _MULTI_ARRAY_CMADVANCER_H_

#include "multi_array/CloseOpenInterval.h"

namespace multi_array {

template <int DIM>
class CMAdvancer
{
private:

  CloseOpenInterval<DIM,int> m_domain;

public:

  CMAdvancer(const CloseOpenInterval<DIM,int>& dmn):
    m_domain(dmn)
  {}

  const CloseOpenInterval<DIM,int>& domain() const {
    return m_domain;
  }

  /// When possible, go to the next domain coordinate.
  ///
  /// Go to the next coordinate of the domain, and when there is no
  /// next coordinate, stay.
  /// @param[in,out] crd: the present and next domain coordinate.
  /// @return: false when we were at the end of the domain, true else.
  inline bool advance(Tuple<DIM,int>& crd) {
    // If crd is the last element of m_domain, we want to stay at
    // crd. However, circularAdvance goes in that case back to the
    // start of m_domain. We don't want that so we must make a copy of
    // crd:
    Tuple<DIM,int> next = crd;

    // We assume here that issuing an advance when at the end of
    // m_domain is a relatively rare case. In that case it is
    // efficient to use circularAdvance:
    if (!circularAdvance(next) )
      return false;
    else {
      crd = next;
      return true;
    } 
  }

  /// Go to the next domain coordinate.
  ///
  /// Go to the next coordinate of the domain, and when there is no
  /// next coordinate, return to the beginning of the domain.
  /// @param[in,out] crd: the present and next domain coordinate.
  /// @return: false when we were at the end of the domain, true else.
  inline bool circularAdvance(Tuple<DIM,int>& crd) {
    int d = 0;
    ++crd[d];
    while (crd[d] >= m_domain.high()[d] ) {
      assert(crd[d] == m_domain.high()[d] );
      crd[d] = m_domain.low()[d];
      ++d;
      if (d >= DIM)
        return false;
      ++crd[d];
    }
    assert(d >= 0);
    return true;
  }

  /// When possible, advance with step.
  ///
  /// Increase coordinate crd with step, and when that is not possible
  /// stay.
  /// @param[in,out] crd: the present and next domain coordinate.
  /// @param[in] step: the desired step. The step must be within the
  /// size of the domain. (A more general step slows the method down
  /// quite a bit.)
  /// @return: false when the step couldn't be made, true else.
  inline bool advance(Tuple<DIM,int>& crd, const Tuple<DIM,int>& step) {
    // If crd is the last element of m_domain, we want to stay at
    // crd. However, circularAdvance goes in that case back to the
    // start of m_domain. We don't want that so we must make a copy of
    // crd:
    Tuple<DIM,int> next = crd;

    // We assume here that issuing an advance when at the end of
    // m_domain is a relatively rare case. In that case it is
    // efficient to use circularAdvance:
    if (!circularAdvance(next,step) )
      return false;
    else {
      crd = next;
      return true;
    } 
  }

  /// Alternative to circularAdvance in case of a row major non-unit step.
  ///
  /// Increase coordinate crd with step in a circular fashion,
  /// i.e. when the step couldn't be made, continue at the beginning
  /// of the domain. 
  /// @param[in,out] crd: the present and next domain coordinate.
  /// @param[in] step: the desired step. The step must be within the
  /// size of the domain. (A more general step slows the method down
  /// quite a bit.)
  /// @return: false when we had to continue at the beginning of the
  /// domain, true else.
  inline bool circularAdvance(Tuple<DIM,int>& crd, const Tuple<DIM,int>& step) {

    // Test the precondition:
    assert(interval(Tuple<DIM,int>::filled(0),m_domain.size()).contains(step));

    int d = 0;
    crd[d] += step[d];
    int leftOver = crd[d] - m_domain.high()[d];
    while (leftOver >= 0) {
      assert(crd[d] - step[d] < m_domain.high()[d] );
      crd[d] = m_domain.low()[d] + leftOver;
      ++d;
      if (d >= DIM)
	return false;
      crd[d] += step[d];
      leftOver = crd[d] - m_domain.high()[d];
    }
    assert(d >= 0);
    return true;
  }

  /// When possible, go to the previous domain coordinate.
  ///
  /// Go to the previous coordinate of the domain, and when there is no
  /// previous coordinate, stay.
  /// @param[in,out] crd: the present and previous domain coordinate.
  /// @return: false when we were at the beginning of the domain, true
  /// else.
  inline bool recede(Tuple<DIM,int>& crd) {
    // If crd is the first element of m_domain, we want to stay at
    // crd. However, circularAdvance goes in that case back to the
    // end of m_domain. We don't want that so we must make a copy of
    // crd:
    Tuple<DIM,int> prior = crd;

    // We assume here that issuing an recede when at the beginning of
    // m_domain is a relatively rare case. In that case it is
    // efficient to use circularAdvance:
    if (!circularRecede(prior) )
      return false;
    else {
      crd = prior;
      return true;
    } 
  }

  /// Go to the previous domain coordinate.
  ///
  /// Go to the previous coordinate of the domain, and when there is
  /// no previous coordinate, return to the end of the domain.
  /// @param[in,out] crd: the present and previous domain coordinate.
  /// @return: false when we were at the start of the domain, true else.
  inline bool circularRecede(Tuple<DIM,int>& crd) {
    int d = 0;
    --crd[d];
    while (crd[d] < m_domain.low()[d] ) {
      assert(crd[d] == m_domain.low()[d]-1 );
      crd[d] = m_domain.high()[d];
      ++d;
      if (d >= DIM)
        return false;
      --crd[d];
    }
    assert(d >= 0);
    return true;
  }

  /// When possible, recede with step.
  ///
  /// Increase coordinate crd with step, and when that is not possible
  /// stay.
  /// @param[in,out] crd: the present and next domain coordinate.
  /// @param[in] step: the desired step. The step must be within the
  /// size of the domain. (A more general step slows the method down
  /// quite a bit.)
  /// @return: false when the step couldn't be made, true else.
  inline bool recede(Tuple<DIM,int>& crd, const Tuple<DIM,int>& step) {
    // If crd is the last element of m_domain, we want to stay at
    // crd. However, circularRecede goes in that case back to the
    // start of m_domain. We don't want that so we must make a copy of
    // crd:
    Tuple<DIM,int> prior = crd;

    // We assume here that issuing an recede when at the end of
    // m_domain is a relatively rare case. In that case it is
    // efficient to use circularRecede:
    if (!circularRecede(prior,step) )
      return false;
    else {
      crd = prior;
      return true;
    } 
  }

  /// Alternative to circularRecede in case of a row major non-unit step.
  ///
  /// Increase coordinate crd with step in a circular fashion,
  /// i.e. when the step couldn't be made, continue at the beginning
  /// of the domain. 
  /// @param[in,out] crd: the present and prior domain coordinate.
  /// @param[in] step: the desired step. The step must be within the
  /// size of the domain. (A more general step slows the method down
  /// quite a bit.)
  /// @return: false when we had to continue at the beginning of the
  /// domain, true else.
  inline bool circularRecede(Tuple<DIM,int>& crd, const Tuple<DIM,int>& step) {

    // Test the precondition:
    assert(interval(Tuple<DIM,int>::filled(0),m_domain.size()).contains(step));

    int d = 0;
    crd[d] -= step[d];
    int leftOver = crd[d] - m_domain.low()[d];
    while (leftOver < 0) {
      assert(crd[d] + step[d] >= m_domain.low()[d] );
      crd[d] = m_domain.high()[d] - leftOver;
      ++d;
      if (d >= DIM)
	return false;
      crd[d] -= step[d];
      leftOver = crd[d] - m_domain.low()[d];
    }
    assert(d >= 0);
    return true;
  }
};

} // namespace multi_array

#endif






