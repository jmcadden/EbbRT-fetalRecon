#include <irtkImage.h>
#include <irtkTransformation.h>
#include <irtkGaussianBlurring.h>

#include <ebbrt/EbbRef.h>
#include <ebbrt/IOBuf.h>
#include <ebbrt/LocalIdMap.h>
#include <ebbrt/Message.h>
#include <ebbrt/SharedEbb.h>
#include <ebbrt/SpinBarrier.h>
#include <ebbrt/StaticIOBuf.h>
#include <ebbrt/UniqueIOBuf.h>
#include <ebbrt/Future.h>
#include <ebbrt/Cpu.h>

#include <ebbrt/SpinLock.h>
#include <ebbrt/native/Clock.h>

#include "../parameters.h"
#include "../utils.h"

using namespace ebbrt;
using namespace std;

class irtkReconstruction : public ebbrt::Messagable<irtkReconstruction>, public irtkObject {

  private:
    // Ebb creation parameters 
    std::unordered_map<uint32_t, ebbrt::Promise<void>> _promise_map;
    std::mutex _m;
    uint32_t _id{0};

    // Input parameters

    int _numThreads;

    double _delta; 
    double _lambda; 
    double _lowIntensityCutoff; 

    bool _globalBiasCorrection; 
    bool _debug;

    // Internal parameters

    ebbrt::Promise<int> _future;

    int _sigmaBias;

    int _directions[13][3];

    double _qualityFactor;
    double _step; 
    double _sigmaSCPU;
    double _sigmaS2CPU;
    double _mixSCPU;
    double _mixCPU;
    double _alpha;
    double _maxIntensity;
    double _minIntensity;
    double _averageVolumeWeight;

    bool _adaptive;

    vector<float> _stackFactor;

    vector<double> _scaleCPU;
    vector<double> _sliceWeightCPU;
    vector<double> _slicePotential;

    vector<int> _stackIndex;
    vector<int> _sliceInsideCPU;

    irtkRealImage _reconstructed;
    irtkRealImage _mask;
    irtkRealImage _volumeWeights;

    vector<irtkRigidTransformation> _transformations;

    vector<irtkRealImage> _slices;
    vector<irtkRealImage> _weights;
    vector<irtkRealImage> _bias;

    vector<SLICECOEFFS> _volcoeffs;

  public:
    // Constructor
    irtkReconstruction(ebbrt::EbbId ebbid);

    // Ebb creation functions
    static ebbrt::EbbRef<irtkReconstruction>
      Create(ebbrt::EbbId id = ebbrt::ebb_allocator->Allocate());

    static irtkReconstruction& HandleFault(ebbrt::EbbId id);
    ebbrt::Future<void> Ping(ebbrt::Messenger::NetworkId nid);

    void ReceiveMessage(ebbrt::Messenger::NetworkId nid,
        std::unique_ptr<ebbrt::IOBuf>&& buffer);

    // Reconstruction functions
    void CoeffInit(ebbrt::IOBuf::DataPointer& dp);

    void ParallelCoeffInit(int start, int end);

    void ReturnFromCoeffInit(ebbrt::Messenger::NetworkId frontEndNid);

    void StoreParameters(struct reconstructionParameters parameters);

    struct coeffInitParameters StoreCoeffInitParameters(
        ebbrt::IOBuf::DataPointer& dp);

    void InitializeEMValues();

    void InitializeEM();

    // Deserializer functions
    void DeserializeSlice(ebbrt::IOBuf::DataPointer& dp, irtkRealImage& tmp);

    void DeserializeSliceVector(ebbrt::IOBuf::DataPointer& dp, int nSlices);

    void DeserializeTransformations(ebbrt::IOBuf::DataPointer& dp, 
        irtkRigidTransformation& tmp);

    // Debugging functions
    inline double SumImage(irtkRealImage img);

    inline void PrintImageSums();

    inline void PrintVectorSums(vector<irtkRealImage> images, string name);

    inline void PrintAttributeVectorSums();
};

inline double irtkReconstruction::SumImage(irtkRealImage img) {
  float sum = 0.0;
  irtkRealPixel *ap = img.GetPointerToVoxels();

  for (int j = 0; j < img.GetNumberOfVoxels(); j++) {
    sum += (float)*ap;
    ap++;
  }
  return (double)sum;
}

inline void irtkReconstruction::PrintImageSums() {
  /*
     cout << "_externalRegistrationTargetImage: " 
     << SumImage(_externalRegistrationTargetImage) << endl; 
     */

  cout << "_reconstructed: " 
    << SumImage(_reconstructed) << endl;

  cout << "_mask: "
    << SumImage(_mask) << endl;
}

inline void irtkReconstruction::PrintVectorSums(vector<irtkRealImage> images, 
    string name) {
  for (int i = 0; i < (int) images.size(); i++) {
    cout << fixed << name << "[" << i << "]: " << SumImage(images[i]) << endl;
  }
}

inline void irtkReconstruction::PrintAttributeVectorSums() {
  PrintVectorSums(_slices, "slices");
  //PrintVectorSums(_simulatedSlices, "simulatedSlices");
  //PrintVectorSums(_simulatedInside, "simulatedInside");
  //PrintVectorSums(_simulatedWeights, "simulatedWeights");
  PrintVectorSums(_weights, "weights");
  PrintVectorSums(_bias, "bias");
}