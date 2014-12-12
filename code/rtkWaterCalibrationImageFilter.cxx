/*=========================================================================
 *
 *  Copyright RTK Consortium
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

#include <itkImageRegionConstIterator.h>
#include <itkImageRegionIterator.h>

#include "rtkWaterCalibrationImageFilter.h"
#include "rtkWaterPrecorrectionImageFilter.h"

namespace rtk
{
WaterCalibrationImageFilter::WaterCalibrationImageFilter()
{
  m_Order = 1.0f;
}

void WaterCalibrationImageFilter
::ThreadedGenerateData( const OutputImageRegionType & outputRegionForThread, ThreadIdType itkNotUsed(threadId) )
{
  itk::ImageRegionConstIterator< ImageType > itIn(this->GetInput(), outputRegionForThread);
  itk::ImageRegionIterator< ImageType >      itOut(this->GetOutput(), outputRegionForThread);

  itIn.GoToBegin();
  itOut.GoToBegin();
  while ( !itIn.IsAtEnd() )
    {
    float v = itIn.Get();
    itOut.Set( std::pow(v, m_Order) );
    ++itIn;
    ++itOut;
    }
}
} // end namespace rtk