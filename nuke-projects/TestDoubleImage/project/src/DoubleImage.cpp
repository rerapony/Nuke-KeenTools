// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

static const char* const CLASS = "DoubleImage";

static const char* const HELP =
  "Make image twice as wide";

// Standard plug-in include files.

#include "DDImage/Iop.h"
#include "DDImage/NukeWrapper.h"
using namespace DD::Image;
#include "DDImage/Row.h"
#include "DDImage/Tile.h"
#include "DDImage/Knobs.h"

#include <math.h>

using namespace std;

class DoubleImage : public Iop
{

	int _param_wider;
	int dx;
	
public:
  
	int minimum_inputs() const { return 1; }
	int maximum_inputs() const { return 1; }
  
  //! Constructor. Initialize user controls to their default values.

  DoubleImage (Node* node) : Iop (node)
  {
	  _param_wider = 3;
  }

  ~DoubleImage ()
	{
	}
  
  void _validate(bool);
  void _request(int x, int y, int r, int t, ChannelMask channels, int count);
  
  //! This function does all the work.

  void engine ( int y, int x, int r, ChannelMask channels, Row& out );

  //! Return the name of the class.

  const char* Class() const { return CLASS; }
  const char* node_help() const { return HELP; }

  //! Information to the plug-in manager of DDNewImage/Nuke.

  void knobs(Knob_Callback) override;
  static const Iop::Description description;

}; 


/*! This is a function that creates an instance of the operator, and is
   needed for the Iop::Description to work.
 */
static Iop* AddInputsCreate(Node* node)
{
  return new DoubleImage(node);
}

const Iop::Description DoubleImage::description ( CLASS, "Merge/DoubleImage",
                                                     AddInputsCreate );


void DoubleImage::_validate(bool for_real)
{
	copy_info();
	Op* inputZero = input(0);
	int width = inputZero->input_format().width();
	int new_width = _param_wider*width;
	dx = ceil((float)new_width / 2);

	info_.pad(dx, 0);
  
}

void DoubleImage::_request(int x, int y, int r, int t, ChannelMask channels, int count)
{
	input(0)->request(x-dx, y, y+dx, t, channels, count);
}


/*! For each line in the area passed to request(), this will be called. It must
   calculate the image data for a region at vertical position y, and between
   horizontal positions x and r, and write it to the passed row
   structure. Usually this works by asking the input for data, and modifying
   it.

 */
void DoubleImage::engine ( int y, int x, int r,
                              ChannelMask channels, Row& row )
{
	Row inputRow(x, r);
	inputRow.get(input0(), y, x, r, channels);
	foreach ( z, channels ) {
		float* inptr = inputRow.writable(z) + x;
		const float* inend = inptr + (r - x);

		float* outptr = row.writable(z) + x;
		
		while (inptr < inend)
		{
			auto ptr_val = *inptr;
			inptr++;
			auto ptr_val2 = *inptr;

			float _gain = (ptr_val2 - ptr_val) / _param_wider;
			for (int j=0; j<_param_wider; ++j)
			{
				*outptr = ptr_val + j*_gain;
				outptr++;
			}
		}
	}
}

void DoubleImage::knobs(Knob_Callback f)
{
	Int_knob(f, &_param_wider, "width", "wide parameter");
	SetRange(f, 1, 5);
}
