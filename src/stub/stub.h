/**
 * This software was developed at the National Institute of Standards and Technology (NIST) by
 * employees of the Federal Government in the course of their official duties. Pursuant to title
 * 17 Section 105 of the United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility  whatsoever for its use by other
 * parties, and makes no guarantees, expressed or implied, about its quality, reliability, or any
 * other characteristic.
 */
#ifndef STUB_H_
#define STUB_H_

#include "irex/irex.h"

using std::string;
using std::vector;

using ReturnStatus = Irex::ReturnStatus;

class Stub : public Irex::Interface
{
   public:
      ~Stub() override = default;
   
      ReturnStatus initialize(const string& configDir) override;

      ReturnStatus createTemplate(vector<Irex::IrisImage>& irides,
                                  const Irex::TemplateType type,
                                  vector<uint8_t>& templateData) override;
    
      ReturnStatus compareTemplates(const vector<uint8_t>& verifTemplate,
                                    const vector<uint8_t>& enrolTemplate,
                                    double& dissimilarity) override;

      static std::shared_ptr<Interface> getImplementation();
};

#endif // end namespace STUB_H_
