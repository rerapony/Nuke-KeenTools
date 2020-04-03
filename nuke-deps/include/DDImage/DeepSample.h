// Copyright (c) 2011 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_DeepSample_H
#define DDImage_DeepSample_H

#include "DDImage/ChannelMap.h"
#include "DDImage/DeepPixel.h"

#include <vector>

namespace DD {
  namespace Image {

    /// helper class representing a small float vector, one for each channel in a set
    class DDImage_API DeepSample
    {
      ChannelMap _map;

      typedef std::vector<float> SampleContainer;
      SampleContainer _samples;

    public:
      /// initialise based on a ChannelMap.  Set all samples to zero.
      DeepSample(const ChannelMap& map) : _map(map), _samples(_map.size(), 0)
      {
      }

      /// initialise based on a sample from a DeepPixel.  if fetchOrdered is true it will use
      /// getOrderedSample, otherwise getUnorderedSample
      DeepSample(const ChannelMap& map, const DeepPixel& pixel, int sample, bool fetchOrdered = false) : _map(map)
      {
        _samples.reserve(_map.size());
        const ChannelSet& chans = _map;

        foreach(z, chans) {
          if (pixel.channels().contains(z))
            if (fetchOrdered)
              _samples.push_back(pixel.getOrderedSample(sample, z));
            else
              _samples.push_back(pixel.getUnorderedSample(sample, z));
          else
            _samples.push_back(0);
        }
      }

      /// assignment operator.  should only ever assign between DeepSamples that share the same Map
      void operator=(const DeepSample& other)
      {
        //mFnAssert(&_map == &other._map);
        _map = other._map;
        _samples = other._samples;
      }
      void operator=(DeepSample& other)
      {
        //mFnAssert(&_map == &other._map);
        _map = other._map;
        _samples = other._samples;
      }

      bool operator==(DeepSample& other)
      {
       //mFnAssert(&_map == &other._map);
       return  (_map == other._map) && (_samples.size() == other._samples.size()) && (std::equal(_samples.begin(), _samples.end(), other._samples.begin()));

      }


      /// access the float for a particular channel
      float& operator[](Channel z)
      {
       // mFnAssert(_map.contains(z));
        return _samples[_map.chanNo(z)];
      }

      /// access the float for a particular channel
      const float& operator[](Channel z) const
      {
        //mFnAssert(_map.contains(z));
        return _samples[_map.chanNo(z)];
      }

      SampleContainer::iterator  begin()
      {
        return _samples.begin();
      }
      SampleContainer::iterator  end()
      {
        return _samples.end();
      }
      size_t size()
      {
        return _samples.size();
      }
    };

    /**
     * Helper class to wrap a number of DeepSample objects, and presenting an interface
     * that resembles a DeepPixel, for the convenience of templated algorithms.
     */
    class DeepSampleVector
    {
      typedef std::vector<DeepSample> DeepSampleContainer;
      DeepSampleContainer _deepSamples;
    public:

      // get number of samples
      size_t getSampleCount() const
      {
        return _deepSamples.size();
      }

      /// get sample, in order (farthest is first)
      const float& getOrderedSample(int depthIdx, DD::Image::Channel channel) const
      {
        return _deepSamples[_deepSamples.size() - 1 - depthIdx][channel];
      }

      /// get sample, in no particular order
      const float& getUnorderedSample(int depthIdx, DD::Image::Channel channel) const
      {
        return _deepSamples[depthIdx][channel];
      }

      // get number of samples
      size_t size() const
      {
        return _deepSamples.size();
      }

      /// get sample, in no particular order
      const DeepSample& operator[](int idx) const
      {
        return _deepSamples[idx];
      }

      /// get sample, in no particular order
      DeepSample& operator[](int idx)
      {
        return _deepSamples[idx];
      }

      /// resize the vector and allocate new elements
      void resize(size_t newSize, const ChannelMap& channelMap)
      {
        _deepSamples.resize(newSize, DeepSample(channelMap));
      }

      /// append a sample
      void push_back(const DeepSample& deepSample)
      {
        _deepSamples.push_back(deepSample);
      }

      DeepSampleContainer::iterator begin()
      {
        return _deepSamples.begin();
      }
      DeepSampleContainer::iterator end()
      {
        return _deepSamples.end();
      }
      void erase(DeepSampleContainer::iterator begin, DeepSampleContainer::iterator end)
      {
        _deepSamples.erase(begin, end);
      }


    };

  }
}

#endif
