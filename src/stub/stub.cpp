/**
 * This software was developed at the National Institute of Standards and Technology (NIST) by
 * employees of the Federal Government in the course of their official duties. Pursuant to title
 * 17 Section 105 of the United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility  whatsoever for its use by other
 * parties, and makes no guarantees, expressed or implied, about its quality, reliability, or any
 * other characteristic.
 */

/// Dummy implementation.
#include "stub.h"

#include <fstream>
#include <iostream>
#include <algorithm>
#include <utility>

using std::cerr;
using std::endl;

using ReturnCode = Irex::ReturnStatus::ReturnCode;
using PixelFormat = Irex::IrisImage::PixelFormat;

ReturnStatus Stub::initialize(const string& configDir)
{
   return ReturnCode::Success;
}

ReturnStatus Stub::createTemplate(vector<Irex::IrisImage>& irides,
                                  const Irex::TemplateType type,
                                  vector<uint8_t>& templateData)
{
   // Only use the first iris image.
   const Irex::IrisImage& iris = irides.front();
  
   // Set template to be the pixel value in the center of the image.
   templateData.resize(1, iris.data[ iris.data.size() / 2 ]);

   return ReturnCode::Success;
}

ReturnStatus Stub::compareTemplates(const std::vector<uint8_t>& verifTemplate,
                                    const std::vector<uint8_t>& enrolTemplate,
                                    double& dissimilarity)
{
   dissimilarity = verifTemplate[0] + (enrolTemplate[0] << 8);

   return ReturnCode::Success;
}

std::shared_ptr<Irex::Interface> Irex::Interface::getImplementation()
{
    return std::make_shared<Stub>();
}
