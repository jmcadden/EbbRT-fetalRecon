//          Copyright Boston University SESA Group 2013 - 2014.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "reconstruction.h"

#pragma GCC diagnostic ignored "-Wswitch"
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"

void parseInputParameters(int argc, char **argv) {
  try {
    po::options_description desc("Options");
    desc.add_options()("help,h", "Print usage messages")
      ("output,o", 
        po::value<string>(&ARGUMENTS.outputName)->required(),
        "Name for the reconstructed volume. Nifti or Analyze format.")
      ("mask,m", 
        po::value<string>(&ARGUMENTS.maskName), 
        "Binary mask to define the region od interest. Nifti or Analyze format.")
      ("input,i", 
        po::value<vector<string>>(&ARGUMENTS.inputStacks)->multitoken(),
        "[stack_1] .. [stack_N]  The input stacks. Nifti or Analyze format.")
      ("transformation,t",
        po::value<vector<string>>(&ARGUMENTS.inputTransformations)->multitoken(),
        "The transformations of the input stack to template in \'dof\' format "
        "used in IRTK. Only rough alignment with correct orienation and some "
        "overlap is needed. Use \'id\' for an identity transformation for at "
        "least one stack. The first stack with \'id\' transformation  will be "
        "resampled as template.")
      ("thickness", 
        po::value<vector<double>>(&ARGUMENTS.thickness)->multitoken(),
        "[th_1] .. [th_N] Give slice thickness.[Default: twice voxel size in z "
        "direction]")
      ("packages,p", 
        po::value<vector<int>>(&ARGUMENTS.packages)->multitoken(),
        "Give number of packages used during acquisition for each stack. The "
        "stacks will be split into packages during registration iteration 1 "
        "and then into odd and even slices within each package during "
        "registration iteration 2. The method will then continue with slice to "
        " volume approach. [Default: slice to volume registration only]")
      ("iterations", 
        po::value<int>(&ARGUMENTS.iterations)->default_value(1),
        "Number of registration-reconstruction iterations.")
      ("sigma", 
        po::value<double>(&ARGUMENTS.sigma)->default_value(12),
        "Stdev for bias field. [Default: 12mm]")
      ("resolution", 
        po::value<double>(&ARGUMENTS.resolution)->default_value(0.75),
        "Isotropic resolution of the volume. [Default: 0.75mm]")
      ("multires", 
        po::value<int>(&ARGUMENTS.levels)->default_value(3),
        "Multiresolution smooting with given number of levels. [Default: 3]")
      ("average", 
        po::value<double>(&ARGUMENTS.averageValue)->default_value(700),
        "Average intensity value for stacks [Default: 700]")
      ("delta", 
        po::value<double>(&ARGUMENTS.delta)->default_value(150),
        "Parameter to define what is an edge. [Default: 150]")
      ("lambda", 
        po::value<double>(&ARGUMENTS.lambda)->default_value(0.02),
        "Smoothing parameter. [Default: 0.02]")
      ("lastIterLambda",
        po::value<double>(&ARGUMENTS.lastIterLambda)->default_value(0.01),
        "Smoothing parameter for last iteration. [Default: 0.01]")
      ("smoothMask", 
        po::value<double>(&ARGUMENTS.smoothMask)->default_value(4),
        "Smooth the mask to reduce artefacts of manual segmentation. [Default: "
        "4mm]")
      ("globalBiasCorrection",
        po::value<bool>(&ARGUMENTS.globalBiasCorrection)->default_value(false),
        "Correct the bias in reconstructed image against previous estimation.")
      ("lowIntensityCutoff",
        po::value<double>(&ARGUMENTS.lowIntensityCutoff)->default_value(0.01),
        "Lower intensity threshold for inclusion of voxels in global bias "
        "correction.")
      ("forceExclude", 
        po::value<vector<int>>(&ARGUMENTS.forceExcluded)->multitoken(),
        "forceExclude [number of slices] [ind1] ... [indN]  "
        "Force exclusion of slices with these indices.")
      ("noIntensityMatching", 
        po::value<bool>(&ARGUMENTS.intensityMatching)->default_value(true),
        "Switch off intensity matching.")
      ("logPrefix", 
        po::value<string>(&ARGUMENTS.logId), 
        "Prefix for the log file.")
      ("debug", 
        po::value<bool>(&ARGUMENTS.debug)->default_value(false),
        "Debug mode - save intermediate results.")
      ("recIterationsFirst",
        po::value<int>(&ARGUMENTS.recIterationsFirst)->default_value(4),
        "Set number of superresolution iterations")
      ("recIterationsLast",
        po::value<int>(&ARGUMENTS.recIterationsLast)->default_value(13),
        "Set number of superresolution iterations for the last iteration")
      ("numStacksTuner",
        po::value<unsigned int>(&ARGUMENTS.numInputStacksTuner)->default_value(0),
        "Set number of input stacks that are really used (for tuner "
        "evaluation, use only first x)")
      ("noLog", 
        po::value<bool>(&ARGUMENTS.noLog)->default_value(false),
        "Do not redirect cout and cerr to log files.")
      ("devices,d", 
        po::value<vector<int>>(&ARGUMENTS.devicesToUse)->multitoken(),
        "Select the CP > 3.0 GPUs on which the reconstruction should be "
        "executed. Default: all devices > CP 3.0")
      ("tFolder", po::value<string>(&ARGUMENTS.tFolder),
        "[folder] Use existing slice-to-volume transformations to initialize "
        "the reconstruction.")
      ("sFolder", 
        po::value<string>(&ARGUMENTS.sFolder),
        "[folder] Use existing registered slices and replace loaded ones "
        "(have to be equally many as loaded from stacks).")
      ("referenceVolume", 
        po::value<string>(&ARGUMENTS.referenceVolumeName),
        "Name for an optional reference volume. Will be used as inital "
        "reconstruction.")
      ("T1PackageSize", 
        po::value<unsigned int>(&ARGUMENTS.T1PackageSize)->default_value(0),
        "is a test if you can register T1 to T2 using NMI and only one "
        "iteration")
      ("numDevicesToUse", 
        po::value<unsigned int>
        (&ARGUMENTS.numDevicesToUse)->default_value(UINT_MAX),
        "sets how many GPU devices to use in case of automatic device "
        "selection. Default is as many as available.")
      ("useCPU", po::bool_switch(&ARGUMENTS.useCPU)->default_value(false),
        "use CPU for reconstruction and registration; performs superresolution "
        "and robust statistics on CPU. Default is using the GPU")
      ("useCPUReg", 
        po::bool_switch(&ARGUMENTS.useCPUReg)->default_value(true),
        "use CPU for more flexible CPU registration; performs superresolution "
        "and robust statistics on GPU. [default, best result]")
      ("useAutoTemplate",
        po::bool_switch(&ARGUMENTS.useAutoTemplate)->default_value(false),
        "select 3D registration template stack automatically with matrix rank "
        "method.")
      ("useSINCPSF", 
        po::bool_switch(&ARGUMENTS.useSINCPSF)->default_value(false), 
        "use a more MRI like SINC point spread function (PSF) Will " 
        "be in plane sinc (Bartlett) and through plane Gaussian.")
      ("disableBiasCorrection",
        po::bool_switch(&ARGUMENTS.disableBiasCorr)->default_value(false),
        "disable bias field correction for cases with little or no bias field "
        "inhomogenities (makes it faster but less reliable for stron intensity "
        "bias)")
      ("numThreads", 
        po::value<int>(&ARGUMENTS.numThreads)->default_value(1),
        "Number of CPU threads to run for TBB")
      ("numFrontEndCpus", 
        po::value<int>(&ARGUMENTS.numFrontendCPUs)->default_value(1),
        "Number of front-end EbbRT nodes")
      ("numNodes", 
        po::value<int>(&ARGUMENTS.numBackendNodes)->default_value(1),
        "Number of back-end EbbRT nodes");

    po::variables_map vm;
    try {
      po::store(po::command_line_parser(argc, argv)
          .options(desc).allow_unregistered().run(), vm);
      if (vm.count("help")) {
        std::cout << "Application to perform reconstruction of volumetric MRI "
                     "from thick slices."
                  << std::endl
                  << desc 
                  << std::endl;
      }
      po::notify(vm);
    } catch (po::error &e) {
      std::cerr << "ERROR: " << e.what() << std::endl << std::endl;
      std::cerr << desc << std::endl;
      exit(EXIT_FAILURE);
    }
  } catch (std::exception &e) {
    std::cerr << "Unhandled exception while parsing arguments:  " << e.what()
              << ", application will now exit" << std::endl;
    exit(EXIT_FAILURE);
  }
}

vector<irtkRigidTransformation> getTransformations(int* templateNumber) {
  int nStacks = ARGUMENTS.inputStacks.size();
  vector<irtkRigidTransformation> stackTransformations;

  if (ARGUMENTS.debug) {
    std::cout << "Reading transformations" << std::endl;
  }

  for (int i = 0; i < nStacks; i++) {
    irtkTransformation *transformation;
    if (!ARGUMENTS.inputTransformations.empty()) {
      try {
        transformation =
          irtkTransformation::New((char *)
              (ARGUMENTS.inputTransformations[i].c_str()));
      } catch (...) {
        transformation = new irtkRigidTransformation;
        if (*templateNumber < 0)
          *templateNumber = 0;
      }
    } else {
      transformation = new irtkRigidTransformation;
      if (*templateNumber < 0)
        *templateNumber = 0;
    }

    irtkRigidTransformation *rigidTransf =
      dynamic_cast<irtkRigidTransformation *>(transformation);
    stackTransformations.push_back(*rigidTransf);
    delete rigidTransf;
  }

  if (*templateNumber < 0) {
    cerr << "Please identify the template by assigning id transformation."
         << endl;
    ebbrt::Cpu::Exit(EXIT_FAILURE);
  }

  return stackTransformations;
}

vector<irtkRealImage> getStacks(EbbRef<irtkReconstruction> reconstruction) {
  irtkRealImage stack;
  vector<irtkRealImage> stacks;
  int nStacks = ARGUMENTS.inputStacks.size();

  if (ARGUMENTS.debug) {
    std::cout << "Reading " << nStacks << " stacks" << std::endl;
  }

  for (int i = 0; i < nStacks; i++) {
    stack.Read(ARGUMENTS.inputStacks[i].c_str());
    stacks.push_back(stack);
  }

  return stacks;
}

void allocateBackends(EbbRef<irtkReconstruction> reconstruction) {

  if (ARGUMENTS.debug) {
    std::cout << "Allocating backend nodes" << std::endl;
  }

  auto bindir = boost::filesystem::system_complete(EXEC_NAME).parent_path() /
                "/bm/reconstruction.elf32";

  try {
    ebbrt::pool_allocator->AllocatePool(
        bindir.string(), ARGUMENTS.numBackendNodes);
  } catch (std::runtime_error& e) {
    std::cerr << e.what() << std::endl;
    ebbrt::Cpu::Exit(EXIT_FAILURE);
  }

  pool_allocator->waitPool().Then(
    [reconstruction](ebbrt::Future<void> f) {
    f.Get();
    // Store the nids into reconstruction object
    for (int i=0; i < ARGUMENTS.numBackendNodes; i++) {
      auto nid = pool_allocator->GetNidAt(i);
      reconstruction->AddNid(nid);
    }
  });
}

void initializeThikness(vector<irtkRealImage> stacks) {

  if (ARGUMENTS.debug) 
    std::cout << "Initializing thickness. Slice thickness is: " << std::endl;

  // Initialize 2*slice thickness if not given by user
  int nStacks = ARGUMENTS.inputStacks.size();

  if (ARGUMENTS.thickness.size() == 0) {
    for (int i = 0; i < nStacks; i++) {
      double dx, dy, dz;
      stacks[i].GetPixelSize(&dx, &dy, &dz);
      ARGUMENTS.thickness.push_back(dz * 2);
      if (ARGUMENTS.debug) 
        std::cout << ARGUMENTS.thickness[i] << " ";
    }
  }
  if (ARGUMENTS.debug) 
    cout << endl;
}

irtkRealImage* getMask(EbbRef<irtkReconstruction> reconstruction,
    vector<irtkRealImage> stacks, 
    vector<irtkRigidTransformation>& stackTransformations,
    int templateNumber) {

  if (ARGUMENTS.debug) {
    std::cout << "Initializing mask" << std::endl;
  }

  irtkRealImage *mask = NULL;

  if (!ARGUMENTS.maskName.empty()) {
    mask = new irtkRealImage((char *)(ARGUMENTS.maskName.c_str()));
  }
  // If no mask was given  try to create mask from the template image in case it
  // was padded
  if ((mask == NULL) && (ARGUMENTS.sFolder.empty())) {
    mask = new irtkRealImage(stacks[templateNumber]);
    *mask = reconstruction->CreateMask(*mask);
  }

  if (mask != NULL) {
    // first resample the mask to the space of the stack
    // for template stact the transformation is identity
    irtkRealImage m = *mask;
    // now do it really with best stack
    reconstruction->TransformMask(stacks[templateNumber], m,
                                  stackTransformations[templateNumber]);
    // Crop template stack
    reconstruction->CropImage(stacks[templateNumber], m);
  }

  return mask;
}

void applyMask(EbbRef<irtkReconstruction> reconstruction,
    vector<irtkRealImage>& stacks, 
    vector<irtkRigidTransformation>& stackTransformations,
    int templateNumber) {

  if (ARGUMENTS.debug) {
    std::cout << "Applying mask" << std::endl;
  }

  int nStacks = stacks.size();

  // Mask is transformed to the all other stacks and they are cropped
  for (int i = 0; i < nStacks; i++) {

    // template stack has been cropped already
    if ((i == templateNumber))
      continue;
    // transform the mask

    irtkRealImage m = reconstruction->GetMask();
    reconstruction->TransformMask(stacks[i], m, stackTransformations[i]);
    // Crop template stack
    reconstruction->CropImage(stacks[i], m);
  }
}

// TODO: This function should be done in parallel
void volumetricRegistration(EbbRef<irtkReconstruction> reconstruction,
    vector<irtkRealImage> stacks, 
    vector<irtkRigidTransformation>& stackTransformations,
    int templateNumber) {

  if (ARGUMENTS.debug) {
    std::cout << "Performing volumetric registrations" << std::endl;
  }

  if (ARGUMENTS.T1PackageSize == 0 && ARGUMENTS.sFolder.empty()) {
    // volumetric registration
    reconstruction->StackRegistrations(stacks, stackTransformations,
        templateNumber);
  }
}

void eraseInputStackTuner(vector<irtkRealImage> stacks, 
    vector<irtkRigidTransformation>& stackTransformations) {

  if (ARGUMENTS.debug) {
    std::cout << "Removing input stacks tuner" << std::endl;
  }

  if (ARGUMENTS.numInputStacksTuner > 0) {
    stacks.erase(
        stacks.begin() + ARGUMENTS.numInputStacksTuner, stacks.end());
    stackTransformations.erase(
        stackTransformations.begin() + ARGUMENTS.numInputStacksTuner,
        stackTransformations.end());
  }
}

void AppMain() {
  int templateNumber = -1;

  irtkRealImage* mask;

  vector<irtkRealImage> stacks;
  vector<irtkRigidTransformation> stackTransformations;

  auto reconstruction = irtkReconstruction::Create();
  allocateBackends(reconstruction);

  stacks = getStacks(reconstruction);
  stackTransformations = getTransformations(&templateNumber);

  reconstruction->InvertStackTransformations(stackTransformations);

  eraseInputStackTuner(stacks, stackTransformations);

  initializeThikness(stacks);

  reconstruction->SetParameters(ARGUMENTS);

  mask = getMask(reconstruction, stacks, stackTransformations, templateNumber);

  // Create template volume with isotropic resolution if resolution==0 
  // it will be determined from in-plane resolution of the image
  ARGUMENTS.resolution =
    reconstruction->CreateTemplate(stacks[templateNumber], 
        ARGUMENTS.resolution);

  // Set mask to reconstruction object.
  reconstruction->SetMask(mask, ARGUMENTS.smoothMask);

  volumetricRegistration(reconstruction, 
      stacks, stackTransformations, templateNumber);

  // Mask is transformed to the all other stacks and they are cropped
  applyMask(reconstruction, stacks, stackTransformations, templateNumber);

  volumetricRegistration(reconstruction, 
      stacks, stackTransformations, templateNumber);
  // Rescale intensities of the stacks to have the same average
  reconstruction->MatchStackIntensitiesWithMasking(
      stacks, stackTransformations, ARGUMENTS.averageValue, 
      !ARGUMENTS.intensityMatching);

  // Create slices and slice-dependent transformations
  reconstruction->CreateSlicesAndTransformations(stacks, stackTransformations,
      ARGUMENTS.thickness);

  reconstruction->MaskSlices();

  // if given read slice-to-volume registrations
  if (!ARGUMENTS.tFolder.empty())
    reconstruction->ReadTransformation((char *) ARGUMENTS.tFolder.c_str());
  // Initialize data structures for EM
  reconstruction->InitializeEM();

  reconstruction->WaitPool().Then(
    [reconstruction](ebbrt::Future<void> f) {
      f.Get();
      // Spawn work to backends
      ebbrt::event_manager->Spawn([reconstruction]() {
        reconstruction->Execute();
      });
  });

  /*
  reconstruction->waitReceive().Then([](ebbrt::Future<void> f) {
    f.Get();
    ebbrt::Cpu::Exit(EXIT_SUCCESS);
  });
  */
}

int main(int argc, char **argv) {
  void* status;

  EXEC_NAME = argv[0];
  parseInputParameters(argc, argv);

  pthread_t tid = ebbrt::Cpu::EarlyInit((size_t) ARGUMENTS.numFrontendCPUs);
  pthread_join(tid, &status);
  
  ebbrt::Cpu::Exit(EXIT_SUCCESS);
  return EXIT_SUCCESS;
}
