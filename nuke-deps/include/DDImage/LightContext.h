// LightContext.h
// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_LightContext_H
#define DDImage_LightContext_H

#include "DDImage_API.h"
#include "LightOp.h"
#include "MatrixArray.h"

#include <vector>

namespace DD
{
  namespace Image
  {
    class Render;
    class Scene;

    /*!
        A context structure to reference environmental and transform
        information for the Light during rendering.
        Contains a MatrixArray for the light's transforms and an Axis object
        which contain transformed Eigen vectors for fast vector comparison.
        The MatrixArray's camera and projection matrices can be used for
        depth-buffer projections.
     */
    class DDImage_API LightContext
    {
      friend class Scene;
      friend class Render;

    private:
      /*! To get at the camera or scene transforms. */
      Scene* scene_;
      /*! Pointer to the light itself (it's matrix is invalid, use transform_!) */
      LightOp* light_;
      /*! Light's depth renderer (if it has one). */
      Render* depthmap_;
      /*! All the matrices one could ever hope for. */
      MatrixArray* transforms_, * mb_transforms_;
      /*! Light PXYZ vectors in world-space. */
      Axis vectors_;

    public:
      LightContext() : scene_(nullptr), light_(nullptr), depthmap_(nullptr),
        transforms_(nullptr), mb_transforms_(nullptr) {}

      /* Copy constructor */
      LightContext( const LightContext* lcontext) : 
        scene_(lcontext->scene_), 
        light_(lcontext->light_), 
        depthmap_(lcontext->depthmap_),
        transforms_(lcontext->transforms_), 
        mb_transforms_(lcontext->mb_transforms_), 
        vectors_(lcontext->vectors_) 
      {}

      virtual ~LightContext(){}

      /*! It's required that any class inheriting from LightContext * MUST * override the clone method */
      virtual LightContext* clone(){
        return new LightContext( this );
      }

      Scene* scene() const { return scene_; }
      void set_scene(Scene* s) { scene_ = s; }
      LightOp* light() const { return light_; }
      void set_light(LightOp* l) { light_ = l; }
      Render* depthmap() const { return depthmap_; }
      void set_depthmap(Render* r) { depthmap_ = r; }

      /* Transforms */
      void set_transforms(MatrixArray* m, MatrixArray* mb = nullptr)
      { transforms_ = m;
        mb_transforms_ = mb; }
      MatrixArray* transforms() const { return transforms_; }
      const Matrix4& matrix(int n) const { return transforms_->matrix(n); }
      const Matrix4& inverse_matrix(int n) const { return transforms_->inverse_matrix(n); }
      /* Motionblur Transforms */
      MatrixArray* mb_transforms() const { return mb_transforms_; }
      const Matrix4& mb_matrix(int n) const { return mb_transforms_->matrix(n); }
      const Matrix4& mb_inverse_matrix(int n) const { return mb_transforms_->inverse_matrix(n); }

      /* Set Light position and axis */
      void setWorldMatrix(const Matrix4& m)
      {
        vectors_.p.set(m.a03, m.a13, m.a23); // set the origin
        vectors_.x.set(m.a00, m.a10, m.a20); // X axis
        vectors_.y.set(m.a01, m.a11, m.a21); // Y axis
        vectors_.z.set(m.a02, m.a12, m.a22); // Z axis
        vectors_.x.normalize();
        vectors_.y.normalize();
        vectors_.z.normalize();
      }

      const Axis& vectors() const { return vectors_; }
      const Vector3& x() const { return vectors_.x; }
      const Vector3& y() const { return vectors_.y; }
      const Vector3& z() const { return vectors_.z; }
      const Vector3& p() const { return vectors_.p; }

    private:
      LightContext( const LightContext& ); //non-copyable
      LightContext& operator=( const LightContext& ); //non-assignable

    };
  }
}

#endif

// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.
