#include "Data.hpp"

Data::Data(float* const pData, unsigned int const sampleCount, unsigned int const featureCount,
        unsigned int const* const pSampleStrata, float const* const pSampleWeights,
        unsigned int const* const pFeatureTypes, unsigned int const sampleStratumCount) :
        mpDataMatrix(new Matrix(pData, sampleCount, featureCount)), mpRankedDataMatrix(
                new Matrix(sampleCount, featureCount)), mpHasFeatureRanksCached(
                new bool[mpDataMatrix->getColumnCount()]), mpSampleStrata(pSampleStrata), mpSampleWeights(
                pSampleWeights), mpFeatureTypes(pFeatureTypes), mSampleStratumCount(
                sampleStratumCount), mpSampleIndicesPerStratum(
                new unsigned int*[sampleStratumCount]), mpSampleCountPerStratum(
                new unsigned int[sampleStratumCount])
{
    for (unsigned int i = 0; i < mpDataMatrix->getColumnCount(); ++i)
        mpHasFeatureRanksCached[i] = false;

    unsigned int p_iterator_per_stratum[mSampleStratumCount];
    for (unsigned int i = 0; i < mSampleStratumCount; ++i)
    {
        mpSampleCountPerStratum[i] = 0;
        p_iterator_per_stratum[i] = 0;
    }

    for (unsigned int i = 0; i < mpDataMatrix->getRowCount(); ++i)
        ++mpSampleCountPerStratum[mpSampleStrata[i]];

    for (unsigned int i = 0; i < mSampleStratumCount; ++i)
        mpSampleIndicesPerStratum[i] = new unsigned int[mpSampleCountPerStratum[i]];

    for (unsigned int i = 0; i < mpDataMatrix->getRowCount(); ++i)
        mpSampleIndicesPerStratum[mpSampleStrata[i]][p_iterator_per_stratum[mpSampleStrata[i]]++] =
                i;
}

Data::~Data()
{
    delete mpDataMatrix;
    delete mpRankedDataMatrix;
    delete[] mpHasFeatureRanksCached;
    for (unsigned int i = 0; i < mSampleStratumCount; ++i)
        delete[] mpSampleIndicesPerStratum[i];
    delete[] mpSampleIndicesPerStratum;
    delete[] mpSampleCountPerStratum;
}

float const
Data::computeMiBetweenFeatures(unsigned int const i, unsigned int const j) const
{
    float r = std::numeric_limits<double>::quiet_NaN();

    bool const A_is_continuous = mpFeatureTypes[i] == FEATURE_CONTINUOUS;
    bool const A_is_discrete = mpFeatureTypes[i] == FEATURE_DISCRETE;
    bool const A_is_survival_event = mpFeatureTypes[i] == FEATURE_SURVIVAL_EVENT;

    bool const B_is_continuous = mpFeatureTypes[j] == FEATURE_CONTINUOUS;
    bool const B_is_discrete = mpFeatureTypes[j] == FEATURE_DISCRETE;

    if (A_is_continuous && B_is_continuous)
        r = computeCorrelationBetweenContinuousFeatures(i, j);
//    else if (A_is_survival_event && B_is_continuous)
//        r = computeConcordanceIndex(i, j, i + 1, mpDataMatrix, mpSampleWeights, mpSampleStrata,
//                true);
//    else if (A_is_discrete && B_is_continuous)
//        r = computeConcordanceIndex(i, j, -1, mpDataMatrix, mpSampleWeights, mpSampleStrata, true);
//    else if (A_is_continuous && B_is_discrete)
//        r = computeConcordanceIndex(j, i, -1, mpDataMatrix, mpSampleWeights, mpSampleStrata, true);
//    else if (A_is_discrete && B_is_discrete)
//        r = computeCramersV(i, j, mpDataMatrix, mpSampleWeights);

    return Math::computeMi(r);
}

float const
Data::computeCorrelationBetweenContinuousFeatures(unsigned int const i, unsigned int const j) const
{
    if (!mpHasFeatureRanksCached[i])
    {
        Math::placeRanksByFeatureIndex(&((*mpDataMatrix)(0, i)), &((*mpRankedDataMatrix)(0, i)),
                mpSampleIndicesPerStratum, mpSampleCountPerStratum, mSampleStratumCount);
        mpHasFeatureRanksCached[i] = true;
    }

    if (!mpHasFeatureRanksCached[j])
    {
        Math::placeRanksByFeatureIndex(&((*mpDataMatrix)(0, j)), &((*mpRankedDataMatrix)(0, j)),
                mpSampleIndicesPerStratum, mpSampleCountPerStratum, mSampleStratumCount);
        mpHasFeatureRanksCached[j] = true;
    }

    return Math::computePearsonCorrelation(&((*mpRankedDataMatrix)(0, i)),
            &((*mpRankedDataMatrix)(0, j)), mpSampleWeights, mpSampleIndicesPerStratum,
            mpSampleCountPerStratum, mSampleStratumCount);
}

unsigned int const
Data::getSampleCount() const
{
    return mpDataMatrix->getRowCount();
}

unsigned int const
Data::getFeatureCount() const
{
    return mpDataMatrix->getColumnCount();
}
