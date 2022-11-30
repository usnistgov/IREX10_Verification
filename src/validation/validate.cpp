/**
 * This software was developed at the National Institute of Standards and Technology (NIST) by
 * employees of the Federal Government in the course of their official duties. Pursuant to title
 * 17 Section 105 of the United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility  whatsoever for its use by other
 * parties, and makes no guarantees, expressed or implied, about its quality, reliability, or any
 * other characteristic.
 */
#include "irex/irex.h"
#include "irex/structs.h"

#include <iostream>
#include <fstream>
#include <csignal>
#include <cstring>
#include <sys/types.h>
#include <sys/stat.h>
#include <algorithm>
#include <dirent.h>

const static int numCandidates = 1098;

using std::string;
using std::cerr;
using std::cout;
using std::endl;
using Irex::IrisImage;
using Irex::ReturnStatus;
using ReturnCode = ReturnStatus::ReturnCode;
using PixelFormat = IrisImage::PixelFormat;
using Label = Irex::IrisImage::Label;

const static string configDir = "./config";

/// A template with associated unique identifier
struct TemplateEntry
{
   std::vector<uint8_t> tmplate;
   string id;
};

/**
 * Creates an iris_image from a PPM or PGM file.
 * This function isn't intended to fully support the PPM format, only enough to read the
 * validation images.
 */
IrisImage readImage(const string &path)
{
   IrisImage iris;
   
   // Open file.
   std::ifstream is(path);
   if (!is)
   {
      cerr << "Error opening " << path << endl;
      raise(SIGTERM);
   }

   string magicNumber;

   // Read in magic number.
   is >> magicNumber;
   if (magicNumber != "P5" && magicNumber != "P6")
   {   
      cerr << "Image format unsupported." << endl;
      raise(SIGTERM);
   }   

   // Is the image RGB or grayscale?
   iris.pixelFormat = magicNumber == "P5" ? PixelFormat::Grayscale : PixelFormat::RGB;

   uint16_t maxValue;

   // Read in image dimensions and max pixel value.
   is >> iris.width >> iris.height >> maxValue;

   if (!is)
   { 
      cerr << "Premature end of file while reading header." << endl;
      raise(SIGTERM);
   }   

   // Skip line break.
   is.get();

   // Number of bytes to read.
   const uint32_t numBytes = iris.width * iris.height *
                             (iris.pixelFormat == PixelFormat::Grayscale ? 1 : 3);

   iris.data.resize(numBytes);

   // Read in raw pixel data.
   is.read((char*)iris.data.data(), numBytes);
   if (!is)
   {
      cerr << "Only read " << is.gcount() << " of " << numBytes << " bytes." << endl;
      raise(SIGTERM);
   }
   
   return iris;
}


void createTemplates(const std::shared_ptr<Irex::Interface> implementation,
                     const std::vector<string>& imagePaths,
                     std::vector<TemplateEntry>& templates,
                     const Irex::TemplateType type)
{
   for (const auto imagePath : imagePaths)
   {
      IrisImage iris = readImage(imagePath);

      std::vector<IrisImage> irides(1, iris);

      if (imagePath == "./images/search/Quinn.pgm")
      {
         // Test two-eye support by addding flipped version of iris as second image.
         std::reverse(iris.data.begin(), iris.data.end());
         irides.push_back(iris);

         // Eye labels must always be specified whenever more than one image is provided.
         irides.front().label = Label::LeftIris;
         irides.back().label = Label::RightIris;
      }

      TemplateEntry t;
      
      // Create enrollment template from image.
      Irex::ReturnStatus ret = implementation->createTemplate(irides, type, t.tmplate);

      switch (ret.code)
      {
         case ReturnCode::FormatError:
         case ReturnCode::ConfigDirError:
         case ReturnCode::ParticipantError:
            cerr << "Fatal Error during template creation." << endl;
            raise(SIGTERM);
            
         default:
            t.id = imagePath.substr( imagePath.find_last_of("/\\") + 1 );
            templates.push_back(t);
      }
   }
}

/**
 * Creates the enrollment database, searches against it, and outputs validation results to standard
 * output.
 */
int main(int argc, char** argv)
{
   // NOTE: The actual test driver may perform some of these steps in separate executables.

   std::vector<string> verifImages,
                       enrolImages;

   // Get list of search images.
   DIR* dir;

   if ((dir = opendir ("./images/search")) == NULL)
   {
      cerr << "Can't read images directory. Check if you unzipped the validation images." << endl;
      return EXIT_FAILURE;
   }

   struct dirent *entry;

   while ((entry = readdir (dir)) != NULL)
   {
      const std::string filename(entry->d_name);

      if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
         verifImages.push_back("./images/search/" + filename);
   }

   closedir(dir);

   // Get list of enrollment images.
   if ((dir = opendir ("./images/enroll")) == NULL)
   {
      cerr << "Can't read images directory" << endl;
      return EXIT_FAILURE;
   }

   while ((entry = readdir (dir)) != NULL)
   {
      const std::string filename(entry->d_name);

      if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
         enrolImages.push_back("./images/enroll/" + filename);
   }

   std::shared_ptr<Irex::Interface> implementation = Irex::Interface::getImplementation();

   std::vector<TemplateEntry> enrolTemplates;
   
   createTemplates(implementation, enrolImages, enrolTemplates,
                   Irex::TemplateType::Enrollment);


   std::vector<TemplateEntry> verifTemplates;

   // Create the search templates.
   createTemplates(implementation, verifImages, verifTemplates,
                   Irex::TemplateType::Verification);


   // Iterate over all combinations of verification and enrollment templates.
   for (const auto& verifTemplate : verifTemplates)
   {
      for (const auto& enrolTemplate : enrolTemplates)
      {
         double dissimilarity;

         // Compare templates.
         Irex::ReturnStatus ret =
            implementation->compareTemplates(verifTemplate.tmplate,
                                             enrolTemplate.tmplate,
                                             dissimilarity);

         if (ret.code == ReturnCode::FormatError ||
             ret.code == ReturnCode::ParticipantError)
         {
            cerr << "Fatal Error during comparison." << endl;
            return EXIT_FAILURE;
         }

         // Write comparison result to standard output.
         cout << verifTemplate.id << " " << enrolTemplate.id << " "
              << dissimilarity << " " << static_cast<int>(ret.code) << endl;
      }
   }

   return EXIT_SUCCESS;
}

