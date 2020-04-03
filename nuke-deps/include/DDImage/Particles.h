// Particles.h
// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_Particles_H
#define DDImage_Particles_H

#include "Primitive.h"
#include "Point.h"

namespace DD
{
  namespace Image
  {

    /*! Particles primitive.
     */
    class DDImage_API Particles : public Primitive
    {
    protected:
      Point::RenderMode render_mode_;

      /*! Destructive copy. */
      void copy(const Particles*);

    public:
      /* virtual */ const char* Class() const;
      Particles(Point::RenderMode render_mode, unsigned points = 0, unsigned start = 0);
      Particles(int render_mode, unsigned points = 0, unsigned start = 0);
      Particles(const Particles &);

      Point::RenderMode render_mode() const { return render_mode_; }
      void render_mode(Point::RenderMode v) { render_mode_ = v; }
      void render_mode(int v) { render_mode_ = (Point::RenderMode) v; }

      Particles& operator = (const Particles& b) { copy(&b);
                                                   return *this; }

      /*! Copy this Particles into a new one, and return a pointer to it.
          Vertex data is duplicated, point indices are unchanged.
       */
      /*virtual*/ Primitive* duplicate() const;
      /*virtual*/ unsigned faces() const { return 0; }

      /*! Push a single rParticles primitive into the rendering stream.
       */
      /*virtual*/ void tessellate(Scene*, PrimitiveContext*) const;

      /*virtual*/ void draw_wireframe(ViewerContext*, PrimitiveContext*,
                                      Primitive* prev_prim = nullptr) const;
      /*virtual*/ void draw_solid(ViewerContext*, PrimitiveContext*,
                                  Primitive* prev_prim = nullptr) const;
      /*virtual*/ void draw_primitive_num(ViewerContext*, PrimitiveContext*) const;

      /*virtual*/ PrimitiveType getPrimitiveType()  const { return eParticles;  }

    protected:
      virtual void tesselateImpl(Scene* scene, PrimitiveContext* ptx, float defaultRadius) const;
    };

  }
}

#endif

// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.
