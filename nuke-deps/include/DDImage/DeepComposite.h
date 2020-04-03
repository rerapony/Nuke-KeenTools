// Copyright (c) 2011 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_DeepComposite_H
#define DDImage_DeepComposite_H

#include "DDImage/ChannelMap.h"
#include "DDImage/DeepSample.h"

#include <vector>

namespace DD {
  namespace Image {

    class ChannelMap;
    class DeepPixel;
    class DeepOutPixel;
    class Row;

    // a function to combine perfectly overlapping samples sampleA[Chan_DeepFront] == sampleB[Chan_DeepFront] && sampleA[Chan_DeepBack] == sampleB[Chan_DeepBack]
    using CombineFunc = DeepSample (*)(const DD::Image::ChannelMap& channels, DeepSample sampleA, DeepSample sampleB);
    //  Commbine Functions for perfectly overlapping samples
    DDImage_API DeepSample PlusSamples(const ChannelMap& chanMap, DeepSample sampleA, DeepSample sampleB);
    DDImage_API DeepSample MergeSamples(const ChannelMap& chanMap, DeepSample sampleA, DeepSample sampleB);

    /// combines overlapping samples from within /deepPixel/ into a number of non-overlapping samples, output in /pixels/
    /// will omit any samples after a sample with alpha 1.
    DDImage_API void CombineOverlappingSamples(const ChannelMap& channels, const DeepPixel& deepPixel, DeepSampleVector& pixels);
    DDImage_API void CombineOverlappingSamples(const ChannelMap& channels, const DeepSampleVector& deepPixel, DeepSampleVector& pixels);
    DDImage_API void CombineOverlappingSamples(const ChannelMap& channels, const DeepPixel& deepPixel, DeepOutPixel& pixels);

    /// These functions together work similarly to CombineOVerlapping Samples to make a deep image tidy
    DDImage_API void DeoverlapSamples(const ChannelMap& channels, const DeepPixel& deepPixel, DeepOutPixel& pixels);
    DDImage_API void MergeOverlappingSamples(const DD::Image::ChannelSet& channels, const DeepPixel& in_pixel, DeepOutPixel& out_pixel, CombineFunc f);

    /// composite the samples together.  Samples can be a DeepPixel or a DeepSampleVector.
    DDImage_API void CompositeSamples(const DeepPixel& samples, const ChannelSet& chans, Row& row, int x, float* Zpix, float* Zfrontpix);
    DDImage_API void CompositeSamples(const DeepSampleVector& samples, const ChannelSet& chans, Row& row, int x, float* Zpix, float* Zfrontpix);

    /// composite the samples together.  Samples can be a DeepPixel or a DeepSampleVector.
    DDImage_API void CompositeSamples(const DeepPixel& samples, const ChannelSet& chans, Row& row, int x, float* Zpix, float* Zfrontpix, float* Zbackpix);
    DDImage_API void CompositeSamples(const DeepSampleVector& samples, const ChannelSet& chans, Row& row, int x, float* Zpix, float* Zfrontpix, float* Zbackpix);

    /// check whether any samples overlap.  Samples can be a DeepPixel or a DeepSampleVector.
    DDImage_API bool DetectOverlappingSamples(const DeepSampleVector& t);
    DDImage_API bool DetectOverlappingSamples(const DeepPixel& t);



  }
}

#endif
