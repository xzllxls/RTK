/*=========================================================================
 *
 *  Copyright Insight Software Consortium & RTK Consortium
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0.txt
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *=========================================================================*/
#ifndef __srtkTemplateFunctions_h
#define __srtkTemplateFunctions_h

#include "srtkMacro.h"
#include "srtkCommon.h"
#include "srtkExceptionObject.h"

#include <vector>
#include <ostream>
#include <iterator>

namespace rtk {
namespace simple {

/** \brief A function which does nothing
 *
 * This function is to be used to mark parameters as unused to supress
 * compiler warning.
 */
template <typename T>
void SRTKCommon_HIDDEN Unused( const T &) {};

/**
 * \brief Output the element of an std::vector to the output stream
 *
 * The elements of the std::vector are required to have operator<<.
 *
 * The format of the output should be "[ T, T, T ]".
 */
template <typename T>
SRTKCommon_HIDDEN std::ostream & operator<<( std::ostream & os, const std::vector<T>& v)
{
  if ( v.empty() )
    {
    return os << "[ ]";
    }

  os << "[ ";
  std::copy( v.begin(), v.end()-1, std::ostream_iterator<T>(os, ", ") );
  return os << v.back() << " ]";
}

/** \brief Copy the elements of an std::vector into an ITK fixed width vector
 *
 * If there are more elements in paramter "in" than the templated ITK
 * vector type, they are truncated. If less, then an exception is
 * generated.
 */
template< typename TITKVector, typename TType>
TITKVector SRTKCommon_HIDDEN srtkSTLVectorToITK( const std::vector< TType > & in )
{
  typedef TITKVector itkVectorType;
  if ( in.size() < itkVectorType::Dimension )
    {
    srtkExceptionMacro(<<"Unable to convert vector to ITK type\n"
                      << "Expected vector of length " <<  itkVectorType::Dimension
                       << " but only got " << in.size() << " elements." );
    }
  itkVectorType out;
  for( unsigned int i = 0; i < itkVectorType::Dimension; ++i )
    {
    out[i] = in[i];
    }
  return out;
}

/** \brief Convert an ITK fixed width vector to a std::vector
 */
template<typename TType,  typename TITKVector>
std::vector<TType> SRTKCommon_HIDDEN srtkITKVectorToSTL( const TITKVector & in )
{
  std::vector<TType> out( TITKVector::Dimension );
  for( unsigned int i = 0; i < TITKVector::Dimension; ++i )
    {
    out[i] = static_cast<TType>(in[i]);
    }
  return out;
}

}
}

#endif
