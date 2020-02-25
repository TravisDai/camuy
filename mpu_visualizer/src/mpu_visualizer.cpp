#include <vector>
#include <random>
#include <chrono>
#include <iostream>
#include <cstddef>

#include "matrix_processing_unit.h"

int main(int argc, char** argv)
{

    using WeightDatatype = int32_t;
    using ActivationDatatype = int32_t;
    using AccumulatorDatatype = int32_t;


    constexpr size_t systolicArrayWidth{15UL};
    constexpr size_t systolicArrayHeight{9UL};
    constexpr size_t accumulatorArrayHeight{30UL};

    constexpr size_t activationFifoDepth{8UL};
    constexpr size_t unifiedBufferSizeMaxByte{1024UL*1024UL*1024UL};

    MatrixProcessingUnit<WeightDatatype, ActivationDatatype, AccumulatorDatatype> matrixProcessingUnit(
                                                                                            systolicArrayWidth,
                                                                                            systolicArrayHeight,
                                                                                            activationFifoDepth,
                                                                                            accumulatorArrayHeight,
                                                                                            unifiedBufferSizeMaxByte);

    matrixProcessingUnit.setDebugFlag(true);

    std::default_random_engine rng(static_cast<unsigned long>(
                                    std::chrono::duration_cast<std::chrono::milliseconds>(
                                    std::chrono::time_point_cast<std::chrono::milliseconds>(
                                    std::chrono::high_resolution_clock::now()).time_since_epoch()).count()));

    std::normal_distribution<float> matrixValueDistribution(0.0F, 32.0F);

    std::vector<ActivationDatatype> activationMatrix;
    std::vector<WeightDatatype> weightMatrix;
    std::vector<AccumulatorDatatype> resultMatrix;

    size_t multiplicationCount{0UL};

    const size_t sizeMConst{visualizationMatrixSizeM};
    const size_t sizeNConst{visualizationMatrixSizeN};
    const size_t sizeKConst{visualizationMatrixSizeK};

    activationMatrix.clear();

    for(size_t rowCount{0}; rowCount < sizeMConst; ++rowCount)
    {
        for(size_t columnCount{0}; columnCount < sizeKConst; ++columnCount)
        {
            activationMatrix.emplace_back(static_cast<ActivationDatatype>(
                                                        matrixValueDistribution(rng)));
        }
    }

    weightMatrix.clear();

    for(size_t rowCount{0}; rowCount < sizeKConst; ++rowCount)
    {
        for(size_t columnCount{0}; columnCount < sizeNConst; ++columnCount)
        {
            weightMatrix.emplace_back(static_cast<WeightDatatype>(
                                                matrixValueDistribution(rng)));
        }
    }

    matrixProcessingUnit.storeActivationMatrix(activationMatrix.data(),
                                                    sizeMConst, sizeKConst);

    const std::string weightMatrixNameString{"test" + std::to_string(multiplicationCount++)};

    matrixProcessingUnit.storeWeightMatrix(weightMatrixNameString,
                                                weightMatrix.data(),
                                                sizeKConst, sizeNConst);

    matrixProcessingUnit.runMultiplication(weightMatrixNameString);

    resultMatrix.clear();
    resultMatrix.resize(sizeMConst*sizeNConst);

    matrixProcessingUnit.loadResultMatrix(resultMatrix.data(),
                                            resultMatrix.size());

    Eigen::Map<const RMatrix<ActivationDatatype>> matrixAEigen(
                                                        activationMatrix.data(),
                                                                   sizeMConst, sizeKConst);

    Eigen::Map<const RMatrix<WeightDatatype>> matrixBEigen(
                                                    weightMatrix.data(),
                                                            sizeKConst, sizeNConst);

    RMatrix<AccumulatorDatatype> matrixCEigen = matrixAEigen*matrixBEigen;

    bool sanityCheckPassed{true};

    for(size_t rowCount{0}; rowCount < sizeMConst; ++rowCount)
    {
        for(size_t columnCount{0}; columnCount < sizeNConst; ++columnCount)
        {
            if(resultMatrix[rowCount*sizeNConst + columnCount] !=
                                matrixCEigen(rowCount, columnCount))
            {
                std::cout << "Systolic array output incorrect at ("
                            << columnCount << ", " << rowCount
                            << "): Expected value: "
                            << matrixCEigen(rowCount, columnCount)
                            << " actual value: "
                            << resultMatrix[rowCount*sizeNConst + columnCount] << std::endl;

                sanityCheckPassed = false;
            }
        }
    }

    assert(sanityCheckPassed);

    return 0;
}
