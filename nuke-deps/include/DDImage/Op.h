// Op.h
// Copyright (c) 2013 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_Op_h
#define DDImage_Op_h

#include <vector>
#include <cstdarg>
#include <memory>

#include "DDImage/Channel.h"
#include "DDImage/ddImageVersion.h"
#include "DDImage/Description.h"
#include "DDImage/Hash.h"
#include "DDImage/OutputContext.h"
#include "DDImage/Store.h"
#include "DDImage/Vector3.h"
#include "DDImage/Vector2.h"
#include "DDImage/ViewerContext.h"
#include "DDImage/ViewSet.h"
#include "DDImage/DopeItemFlags.h"
#include "DDImage/OpTimer.h"
#include "DDImage/Deprecation.h"

// Disable Windows DLL interface warning
#ifdef FN_OS_WINDOWS
  #pragma warning(disable:4251)
#endif

class Node;

namespace DD
{

  /*! \namespace DD::Image
     All functions and classes that are used for image processing by Nuke
     Plugins are in this namespace.
     Some utility and cross-platform compatibility functions are not in
     this namespace.
   */
  namespace Image
  {
    namespace MetaData
    {
      class Bundle;
    }

    class Lock;
    class Op;
    class Iop;
    class GeoOp;
    class ParticleOp;
    class DeepOnlyOp;
    class Knob;
    class Knob_Closure;
    typedef Knob_Closure& Knob_Callback;
    class ViewerContext;
    class Box;
    class Executable;
    class OpTree;
    class OpMessage;
    class OpMessageHandler;
    class OpTreeHandler;
    class EngineContext;

    typedef void (*TimeoutHandler)(void*);
    typedef unsigned long long Flags;

    class OpImpl;
    class NodeI;

    class PreValidateContextImpl;

    /**
     * Context for preValidate() to stop duplicate work being done
     * between calls
     */
    class DDImage_API PreValidateContext
    {
      PreValidateContextImpl* _context;
    public:
      PreValidateContext();
      PreValidateContextImpl* operator->() {
        return _context;
      }
      void clear();
      ~PreValidateContext();
    };

    //------------------------------------------------------------------------------
    
    /*! \class DD::Image::Op

        \brief Base class of all the things that can be created by
        Nuke nodes.

        Nuke will create these things, hook their inputs to other ones,
        manage animation and store animated values into these by calling
        the knobs() function, and delete these when the node is destroyed.

        Nuke will also invisibly manage multiple copies hidden inside a
        single node, where each copy is at a different time. This allows
        a later operator to ask for several time samples and merge them
        together in order to get motion blur.  */
    class DDImage_API Op
    {
      friend class EngineContext;

      std::unique_ptr<OpImpl> _pImpl;

      // Be sure to change this symbol if the ABI changes.
#define DDImage_Op_ABI kDDImage_ABI_version
      virtual int DDImage_Op_ABI();

    public:
      //! Combination of input number and pointer to Op.
      typedef std::pair<int, DD::Image::Op*> Output;

      //! Set of all outputs from this Op
      typedef std::set<Output> OutputSet;

      //! Enumeration for the use of doAnyHandles().   the values are defined non-consectively so that
      //! that eHandles | eHandlesCooked == eHandlesCooked
      enum HandlesMode {
        eNoHandles        = 0,              //! no handles are needed
        eHandlesUncooked  = 1,              //! handles are needed, but generate_tree does not necessarily need calling
        eHandlesCooked    = 3,              //! handles are needed, and generate_tree needs to be called, to cook out the 
        eHandlesMax       = eHandlesCooked
      };

#if FN_TRACK_MEMORY_ALLOCATIONS 
      //! Overloads to use the DDImage heap allocator
      void* operator new(size_t size);
      void  operator delete(void* mem);
      void* operator new[](size_t size);
      void  operator delete[](void* mem);
#endif // FN_TRACK_MEMORY_ALLOCATIONS 

    private:
      std::vector<Op*> _allInputs;    //!< input(0)..input(n)
      std::vector<Op*> _children;     //!< Stores ops that set their parent to this op
      OutputSet _outputs;             //!< Stores ops that use this as an input, and the input numbers they use.

      //! Whether the ops pointed to by _allInputs are actually for the right contexts
      bool _inputsValid;

      Node* node_;      //!< Node associated with this op (previously also stored parent ops, but not any more)
      Op*   _pParent;   //!< Parent of this op (e. g. points to the Read or such for a PostageStamp)

      enum State { eInvalid = 0, eInvalidWithError, eValid, eRealValid, eOpened, eError, eStateCount } _state;
      void setState(State);
      bool running_; //!< For Nuke's highlighting display
      bool opened_; //!< True if open() called at all
      bool cached_; //!< True for user- or Nuke-requested caching
      int  slowness_; //!< Slowness factor

      const char* error_message_;    //!< Maintained for binary compatibility only.  Do not use

      Hash hash_;   //!< Last hash sent to invalidate()

      unsigned request_pass; //!< Set by setRequested()
      Op* _firstOp;

      Op& operator=(const Op&); //!< Disabled assignment operator
      Op(const Op &); //!< Disabled copy constructor

      OutputContext outputContext_;

    protected:
      //! whether the _hash actually has been set by a call to invalidate(Hash) ever.
      bool _haveHash;  

      //! The constructor is protected so only subclasses can create the base class.
      Op(Node * node);

      /*! Operator-specific part of invalidate(). It will be called only
          if valid() was on.
          Recommended that this not be used for anything, it is provided for
          back-compatibility only. Instead you should implement close() and
          have that delete cached data if valid() is false.
          The default version does nothing.
       */
      virtual void _invalidate();

      /*! The subclass-dependent portion of validate(). This is called by
          validate() after checking to see if it is needed.

          The default version calls validate(for_real) on all the inputs
          and does nothing else.
       */
      virtual void _validate(bool for_real);

      /*! The Op-dependent portion of open(). This is called if it has
          not been called since the last invalidate(). For iops this is
          called just before the first call to engine(). It's purpose is
          to delay expensive setup until the very last moment. Usually it
          will open data files or calculate lookup tables. It can call
          error() to abort all the processing, if for instance your data
          files are missing.

          All _open() calls are done inside a single locked mutex, so there are
          no multithreading issues even if several instances of a plugin, or
          several different plugins, try to update a shared data structure.

          The default does nothing.
       */
      virtual void _open();

      /*!
         The subclass-dependent portion of close(). This will be called by
         close(). It will be called irregardless of valid state or whether
         you flagged an error during _open() or _validate().

         For back-compatibility, Nuke calls this after 5.0 seconds of idle
         time. Future versions may not call this at all unless
         callCloseAfter() is done, so you should add callCloseAfter(5.0)
         to your plugins to retain compatibility.

         Typically _close() will close files, destroy cached data, and call
         close() on internal operators. It should not destroy any data that
         it might have returned pointers to (i.e. metadata, cached images,
         etc) unless valid() is false. To make sure internal caches are
         destroyed, it should call both invalidate() and close() on any
         internal Op instances.

         Despite the name, this is not the opposite of open(). open() is
         undone by invalidate(). This may be called multiple times after
         open() and may be called before open() is ever called.

         The default implementation does nothing.
       */
      virtual void _close();

      /*! Create a new instance of some Op by name. This is assumed to be
          an internal part of this Op and the new Op's parent() is set to
          this. This works by calling find_description() and if that
          succeeds it calles the constructor() function in the description.

          An Op can use this to create any other operator, even one in a
          plugin, as part of itself.  You should do this inside _validate()
          and not inside the constructor so that any errors can be reported
          correctly.  If there is a problem finding a plugin or loading it
          this will call error() with an appropriate error message and
          return zero.  You should quit in this case, perhaps setting some
          flags so that further calls to your object will not crash.

          You will need to cast the result if you want to use it. For
          instance if it is an Iop you should use dynamic_cast<Iop*> on
          it. If this does not work (returning a zero) you should call
          error() with an appropriate message and give up.

          The name passed in is not the necessarily the name that that the node
          has in the UI: instead it is the name returned by the Class() function
          in C++ and Python for the node. (For example, the regular Merge node's
          internal name is "Merge2", and "Merge" refers to the original Merge 
          node, left in for compatibility.

          If you have a header file or other description of the object you
          can store settings into it using methods from the header file.  If
          you do not have the header file you can still use the
          field() method to put values into it.
       */
      Op* create(Node* node, const char* name);
      Op* create(const char* name);

      //! return if any of the inputs (recursively) wish to draw handles

      /*! Convenience function for doAnyHandles() to call.
       */
      HandlesMode anyInputHandles(ViewerContext*);

      //! return if any of the knobs on this node wish to draw handles

      /*! Convenience function for doAnyHandles() to call. This will check
          if the user interface window is open. If so it will go through
          all the knobs, and if Knob::build_handle() returns true it
          returns true
       */
      HandlesMode anyKnobHandles(ViewerContext*);

      /*! Convenience function for build_handles() to call. This will
          call add_input_handle() on each input to the operator.
       */
      void build_input_handles(ViewerContext*);

      /*! Convenience function for build_handles() to call. This will check
          if the user interface window is open. If so it will go through
          all the knobs, and if Knob::build_handle() returns true it
          calls Knob::add_draw_handle().
       */
      void build_knob_handles(ViewerContext*);
      
      //! Call this to tell the op that it should always have op trees for normal behaviour (for internal use)
      void disallowNoTrees();

      //! Function to be implemented by subclasses who implement build_handles. 
      //! Should return eHandlesCooked or eHandlesUncooked if a call to build_handles
      //! might add handles to be drawn, or eNoHandles, if no handles need drawing. 
      //! The default implementation of this returns the maximum of the values that
      //! anyInputHandles() or anyKnobHandles() return
      //!
      //! Subclasses that call build_input_handles from build_handles() should make
      //! anyInputHandles contribute towards the result with operator| - similarly,
      //! subclasses which call build_knob_handles() should return a value from this
      //! if anyKnobHandles() returns a value.
      //!
      //! Note that at the time that doAnyHandles has been called, the Op tree and inputs
      //! has not been generated, and knob values on this Op have not been stored.  If
      //! the result of this needs to depend upon knob values, fetch the values out with
      //! knob("knobName")->get_value().
      //!
      //! If this function returns cooked then it will (possibly expensively) generate an op
      //! tree, including storing the knob values, and then call build_handles upon that.
      //! If it returns uncooked then it can skip the op generation step, and will just
      //! call build_handles on an op, assuming that the op can use any arbitrary outputContext
      //! and does not need to have had its knobs cooked. (The uiContext() should be correct)
      //!
      //! If in previous versions you returned 'true' from this function, you should
      //! return eHandlesCooked.
      virtual HandlesMode doAnyHandles(ViewerContext* ctx);

    private:
      Lock* _metaLock;

      /** set the thread context to /context/ */
      static void setThreadContext(EngineContext* context);
      
      /** get the thread context */
      static EngineContext* getThreadContext();

    public:

      //! This is a tribool return value. Do not extend.
      enum PrevalidateResult {
        eFinished,
        eCancelled, // because the user actually clicked 'cancel'
        eAbandoned, // because the script changed
      };

      //! preValidate step to perform any potentially expensive steps before
      //! validate().  This is cancellable, and will return eCancalled/eAbandoned
      //! if the user cancelled or abandoned it.  (In the event of a cancellation
      //! a subsequent validate will still work, but will do the additional work).
      PrevalidateResult preValidate(ViewerContext* ctx = nullptr, PreValidateContext * = nullptr);

      //! get the current tree version
      static int treeVersion();

      //! is nuke processing something on a parallel thread?
      static bool nukeBusy();

      //!
      //! return the root for this Op.
      //! IMPORTANT: this may return NULL depending of the state of this Op
      //!            i.e. inside a set_field() call
      Op* rootOp() const;

      //! returns true if user has aborted or cancelled. leaves result in parameter,
      //! otherwise returns false.
      bool preValidateAborted(int treeStartingVersion, PrevalidateResult& result) const;

    public:
      //! The following are utility structures for use in the handle callbacks
      //! ItemSelectionList is returned to the callback for eMouseDrag events. For each GeoInfo object
      //! which is part of the current selection, this maps the GeoSelection::geoID of the GeoInfo to a 
      //! vector of indices of selected vertices for that object.
      typedef std::map<Hash, std::vector<unsigned int> > ItemSelectionList;

      enum HandleType {
        eNone = 0,                  // Invalid state
        eMouseLeftClick,            // The left mouse button has been pressed
        eMouseRightClick,           // The right mouse button has been pressed
        eMouseMove,                 // The mouse pointer has moved
        eMouseDrag,                 // Mouse move + button press
        eMouseRelease,              // A mouse button has been released
        eSelectionChanged,          // The current selection is changing
        eSelectionChangeFinished,   // The current selection change has finished
        eHandleDragTranslate,       // A 3D handle has been drag translated
        eHandleDragRotate,          // A 3D handle has been drag rotated
        eHandleDragScale,           // A 3D handle has been drag scaled
        eHandleDragPivot,           // The pivot of a 3D handle has been drag  translated
        eAllHandles                 // All of the above
      };

      struct EventContext
      {
        EventContext(ViewerContext* ctx)
        {
          _viewerContext = ctx;
          _eventType = eNone;
        }

        EventContext(ViewerContext* ctx, HandleType eventType)
        {
          _viewerContext = ctx;
          _eventType = eventType;
        }        

        HandleType                  _eventType;
        Matrix4                     _oldTransform;          // transform for axis knob before this event
        Matrix4                     _deltaTransform;        // Delta transform for axis knob since last event
        Vector3                     _worldSpaceDeltaMove;   // Mouse delta since last move in world space
        Vector2                     _screenSpaceDeltaMove;  // Mouse delta since last move in screen space
        Vector3                     _hitPoint;              // 3D world intersection position
        ViewerContext*              _viewerContext;
        const ItemSelectionList*    _selectionList;         // Maps object index to a vector of selected point indices
        const ItemSelectionList*    _weightedList;          // Maps object index to a vector of weighted point indices
      };

      typedef bool (EventCallback)(Op* op, EventContext eventCtx);

      //! return all the outputs of this node
      const OutputSet& getOutputs() const
      {
        return _outputs;
      }

      //! set whether the input ops are valid (are for the right context)
      virtual void setInputsValid(bool nowValid);

      //! return whether the input ops are valid (are for the right context)
      bool inputsValid() const
      {
        return _inputsValid;
      }

      //! returns whether or not an op can be cloned in the dag
      virtual bool can_clone() const
      {
        return true;
      }

      //! return true if the hash() value is meaningful and globally unique
      bool haveHash() const
      {
        return _haveHash;
      }

      const MetaData::Bundle* metaCache;
      Hash metaCacheHash;

      virtual std::vector<DD::Image::OutputContext> getPrevVersionsWanted() const
      {
        return std::vector<DD::Image::OutputContext>();
      }

      /// Implement this to supply metadata.  by default it will pass through to fetchMetaData
      /// on input(0).  The reference should be to an internal object that should remain valid
      /// until the Op is invalidated: returning a reference to the return value of another Op's
      /// fetchMetaData is acceptable.
      ///
      /// the key parameter will in future be used to allow only one piece of metadata to be
      /// calculated: presently nuke ignores it, and it is safe for plugins to ignore.
      virtual const MetaData::Bundle& _fetchMetaData(const char* keyname);

      /// Call this to fetch a metadata bundle containing the particular keyname, or NULL
      /// to fetch all available metadata.  You may get back all available data even if
      /// you had specified a particular item.
      const MetaData::Bundle& fetchMetaData(const char* keyname);

      static Op* create (Node* node, const char* name, Op* p_op); // can throw std::runtime_error
      static Op* create (const char* name, Op* p_op);             // can throw std::runtime_error

      /*!
          Returns the Nuke user interface node this operator belongs to.
          If you use DDImage in another program you could use this to
          store an arbitrary piece of user data.
       */
      Node* node() const;
      DD::Image::NodeI* getNode() const;

      /*! \fn void Op::node(Node *)
          Sets the Nuke user interface node this operator belongs to.
          Nuke uses this to report errors.
          Warning: do not attempt to set both parent() and node(), they
          use the same memory location!
       */
      void node(Node* v) { node_ = v; }

      /*! \fn Op* Op::firstOp()
         A single Node can create many instances of the Op. However the \e
         first one it creates is special: it is never deleted until the Node
         is, and it is also the one that methods such as knob_changed() and
         the initial knobs() call to create knobs is done on. This will
         return a pointer to that first instance. This may be this op or
         it may be one that was constructed earlier. The returned op will
         always be the same class as this, and have the same value for node().
       */
      Op* firstOp() const { return _firstOp; }
      void firstOp(Op* v) { _firstOp = v; }

      /*! \fn const Op* Op::parent() const
          Returns the operator this is inside of, if the constructor set it.
       */
      Op* parent() const { return _pParent; }

      /*! \fn void Op::parent(const Op* i)
          If an Operator implements itself by creating "child" operators,
          it can set the parent() of them to point at itself. Nuke uses
          this to produce operator names for error messages, for instance
          it would produce "Blur inside Foo" if your class Foo made a
          child Blur.

          Warning: do not attempt to set both parent() and node(), they
          use the same memory location!
       */
      void parent(Op* lpOp);

      /// node has been deleted in DAG
      virtual void detach() { }

      /// node has been created in DAG
      virtual void attach() { }

      virtual void pre_write_knobs() { }

      /*! Define the "knobs" that will be presented in the control panel
          for the operator.

          Normally you call \a callback once for each Knob you want on the
          panel.  The file Knobs.h describes a number of type-safe wrapper
          functions that you should use instead of calling \a callback
          directly.  This file also documents all the existing built-in
          types.

          When knobs are being created \a callback takes an enumeration value that
          defines the type of knob being created, and the name, label, and
          arbitrary extra data to pass to the constructor, and a pointer
          to data that may (depending on the knob type) be used to get a
          default value.

          When data is being stored \a callback ignores all the arguments
          except the pointer, this is then passed to the store() method of
          the knob that was created last time.

          Use of only the enumerated types avoids the need to link with
          Nuke, thus allowing your image operator to be used with other
          programs that only use the DDImage libarary.

          If desired a custom Knob may be created. In this case your
          operator will only work as a plugin to Nuke. This function must call
          the makeKnobs() function on the callback object and if true it should
          call the constructor for the custom knob. If false it should call
          \a callback, you can pass 0 for all the arguments except the pointer,
          which will be passed to the store() method of your Knob.
       */
      virtual void knobs(Knob_Callback);

      /*!
         Change the set of knobs this node has, by deleting \a n knobs after
         the \a afterthis knob, then inserting new knobs produced by calling
         the function \a f. The return value is the number of knobs created
         by \a f, which you probably want to save and pass to the next
         call.

         If \a n is zero or negative then this only creates knobs. If \a f is
         null then this only destroys knobs.

         Currently the old knobs are completly destroyed. Future versions may
         try to match up the new knobs with the old ones and preserve the
         values and widgets.

         Your knobs() function should also call \a f by using add_knobs().
         You also need to pick a knob that controls the rest of the knobs
         and set the Knob::KNOB_CHANGED_ALWAYS flag on it so that you
         can change the set of knobs on any changes.
       */
      int replace_knobs(Knob * afterthis, int n, void (* f)(void*, Knob_Callback), void*, const char* fileExt = nullptr);

      /*!
         Call f() and return how many knobs it created. This
         should be called inside a knobs() function to create the original
         set of knobs that will be changed by replace_knobs(), and to store
         the values into the current set of knobs. This will return zero
         if the callback is not creating knobs.
       */
      int add_knobs(void (* f)(void*, Knob_Callback), void*, Knob_Callback);

      /*!
       * Disable all knobs on the node that this op belongs to, other than the
       * ones on the Node tab. This can be called at the end of your knobs() method
       * to ensure that *all* knobs are disabled (for example, if a license check is
       * failed).
       */
      void set_unlicensed();

      /*! Find and return a Knob on the control panel for the node
          that controls this Op. Returns null if there is no knob with
          the given name.

          If knobs() has not been called, you may not get knobs you
          think should exist. This will happen if this Op was created
          directly by another Op. If such a controlling op wants to
          change the stored data, it should use the field() method
          to get the pointer to where to store it, rather than trying
          to change the value of a knob.

          If DDImage is not being used by Nuke this will return null.
          However field() will still work.
       */
      Knob* knob(const char* name) const;

      /*! Find a knob by index number. If the index number is larger than
          the number of knobs then null is returned. By starting at zero
          and incrementing until this returns null you will get every Knob
          known about.
       */
      Knob* knob(int) const;

      /*! Returns true if any knob is Knob::pushed(), meaning the user is holding
          it down with the mouse. Currently only implemented for the handles
          in the viewer, but the intention is to make it work for widgets
          in the control panels, too. This is used by Transform to enable
          the texture map preview.
       */
      bool pushed() const;

      /*! Returns true if the control panel for the node is open. This can
          be used to turn the visiblility of handles on/off.
       */
      bool panel_visible() const;

      /*! Returns true if the Nuke Node that created this Op is disabled. */
      bool node_disabled() const;

      /*! Returns true if the node is selected by the user. This can be used
         to turn a wireframe preview on/off.
       */
      bool node_selected() const;


      /*! Returns the color selected by the user to draw things in the viewer.
         The return value is of the form 0xrrggbb00.
       */
      unsigned node_gl_color() const;

      /*! Locate the memory that will be stored into by the knob with the
          given name and return a pointer to it. Returns null if not found.

          This works by calling knobs() with a special callback function
          that examines each callback for the correct name. This is not
          particularily fast so you may want to store the retuned pointer
          somewhere.

          This will work even if knob() does not work. It will work for
          internal Op instances made with create() or by direct construction
          that are not visible to the user and have no knobs.
       */
      void* field(const char* name);

      /*! Same as the other field(), but if the field is found then \a type
          is set to an enumeration (defined in Knobs.h) for the type of knob.
          This can be used to do type-safety tests of the storage location.
       */
      void* field(const char* name, int& type);

      /*! Copy data to a field in an extremely type unsafe manner. Basically
          this does memcpy(field(name),data,size). Make sure you know what
          you are doing. Returns the field or null if the field was not found.
       */
      void* set_field(const char* name, const void* source, int size);

      /*! Copy data from a field in an extremely type unsafe manner. Basically
          this does memcpy(data,field(name),size). Make sure you know what
          you are doing. Returns the field or null if the field was not found.
       */
      void* get_field(const char* name, void* dest, int size);

      //! Callback handling. Used to register callbacks to receive Viewer events listed in the HandleType callback.
      //! The op must implement an add_draw_handle call from it's build_handles to ensure that draw_handle gets called.
      //! The op can then register callbacks from within it's draw_handle to receive these events.
      void beginHandle(HandleType command, ViewerContext* ctx,
                        EventCallback* cb, int index, float x = 0, float y = 0, float z = 0, ViewerContext::Cursor cursor = ViewerContext::kNoCursor );

      void endHandle(ViewerContext* ctx);

      // a zero return code from this method indicates that it should not be called again for the passed in knob

      /*! Whenever the user moves a Knob this is called with a pointer to
          the Knob, on one of the Ops controlled by that Knob. The purpose
          is to automatically enable/disable or set the values of other
          knobs.

          Do not assume this is called on all instances of your Op! It will
          not be in cases of clones or multiple frame numbers in Nuke.  So
          storing results in this call is wrong!  To get values out of knobs
          you must either rely on them being stored and validate() being
          called, or you must ask for them using knob(name)->value().

          You must return non-zero if you do anything. Returning zero
          indicates that there is no need to call this on the same knob again.
          Base class returns zero always.
       */
      virtual int knob_changed(Knob*);

      // currently only triggered by some knobs but more will be supported in future.
      // Derived classes must return non-zero from this method if they want it to be called on that knob again.
      virtual int knob_change_finished(Knob* knob, bool changedByUser = true){return 0;}

      //! add the amount of time /microseconds/ to the debug info.  Avoid doing this
      //! manually, try to use the OpTimer class instead.
      void addTime(OpTimer::Category category, long long microsecondsCPU, long long microsecondsWall, bool isTopLevel = true);
      
      //! return the amount of CPU time taken by this op
      void getPerformanceInfo(OpTimer::Category category, OpTimer::PerformanceInfo& info ) const;
      
      //! reset all the performance timers
      static void resetTimers();

      //! turn performance timing on or off
      static void setTimingEnabled(bool enabled);

      //! return whether performance timing is on or not
      static bool isTimingEnabled();
      
      //! whether unconnected inputs on the DAG should be arranged clockwise or anticlockwise (default)
      virtual bool inputs_clockwise() const;

      virtual std::vector<int> input_order() const;

      //! overriden by Ops to specify the number of input arrows to display on the DAG
      //! the op has not been stored/generated.  nodeInputs is how many inputs are
      //! connected to the node right now (or rather, the index of the first unconnected
      //! input).  Returning -1 (which the default implementation always does) indicates
      //! that the DAG should use its default behaviour.
      virtual int inputArrows(int nodeInputs) const;

      /*! Execute the string in Nuke's scripting language.  Returns true on
          success, false if there is an error. The resulting output (either
          text or an error message) is in script_result().  You \e must call
          script_unlock() after you are done looking at the result, if you
          want to keep it around you should copy it to your own memory.
       */
      bool script_command(const char* command, bool py = true, bool eval = true) const;

      /*! "expand" the string in Nuke's scripting language.  Returns true on
          success, false if there is an error. The resulting output (either
          the expanded text or an error message) is in script_result().  You
          must call script_unlock() after you are done looking at the
          result, if you want to keep it around you should copy it to your
          own memory.

          The current version replaces backslash sequences, executes
          "[command]" and inserts the result, and replaces $variable with
          global tcl variables. It also will produce strange results if the
          DDString.has mismatched quotes or curly braces in it.

          If the string does not contain any special characters then
          \a result is set directly to it. Be aware of this if you attempt
          to delete \a string.

          This is very similar to script_command("return string") except
          it avoids some quoting problems and is much faster in the common
          case where there are no commands.
       */
      bool script_expand(const char* str) const;

      /*! The result of the most recent script_command() or script_expand().
          For multithreading this buffer is protected by a recursive lock
          that is set by calling script_command() or script_expand(). You
          \a must call script_unlock() to release the lock.
       */
      static const char* script_result(bool py = false);

      /*! Allow the memory for script_result() to the most recent
          script_command() or script_expand() to be reused. You \e must
          call this exactly once for every call to script_command() or
          script_expand()!
       */
      static void script_unlock();

      virtual ~Op();

      HandlesMode anyHandles(ViewerContext* ctx);

      /*! Construct a list of callbacks to draw handles for this Op.
          Because handles are drawn much more often than they are
          changed a great deal of time is saved by searching and building a
          list of the functions to call.

          The default implementation recursively calls all the input
          ops (with build_input_handles()) and then calls all the
          knobs (with build_knob_handles()).

          If the op wants to draw a custom GL interface such as outlines
          or even texture maps, it must call ViewerContext::add_draw_handle()
          with pointers to routines to be executed to draw the OpenGL.
          The convenience function Op::add_draw_handle() will cause the
          virtual function draw_handle() to be called for this.

          The op can also mess with the transformation in the context,
          switch the viewer between 2D and 3D, and do other modifications to
          the OpenGl state. Look at Transform, AxisOp and DrawIop for
          different ways of implementing this.
       */
      virtual void build_handles(ViewerContext*);

      /*! Convenience function to call ViewerContext::add_draw_handle() with
          a function that will call this->draw_handle().
       */
      void add_draw_handle(ViewerContext*);

      /*! Default single method for drawing handles. Simple operators can
          override this and override build_handles() to call add_op_handle()
          and then the base class version of build_handles().
       */
      virtual void draw_handle(ViewerContext*);

      /*! Call build_handles() on an input to this op. The input numbers are
          the user-visible numbers (not the result of split_inputs().

          This is better than calling build_handles() on the input
          directly, as it checks to see if the operator was visited
          already (not doing this can make it exponentially slow when
          there are many splits and joins in a script), and it saves
          and restores parts of the ctx like the transform so they can
          be changed by the operator, and it skips added operators like
          PostageStamp that should not be visible to the user.

          Special values for input: -1 means do *this* op, -2 means
          do each inputs where uses_input() is non-zero.
       */
      virtual void add_input_handle(int input, ViewerContext*);
      
      // these are for back-compatibility only:
      void add_op_handle(ViewerContext* ctx) { add_draw_handle(ctx); }

      /*! Same as knob->add_draw_handle(ctx). This is for back-compatibility only. */
      void add_knob_handle(Knob*, ViewerContext*);

      enum ViewableModes {
       eViewableMode2D = 1, //!< This op can be viewed in 2D
       eViewableMode3D = 2,  //!< This op can be viewed in 3D
       eViewableMode3D_2DReference = 4 //!< This op should be viewed in 3D with a 2D wipe overlay
      };

      /*! Return a bitmask of the suggested viewable modes supported for this op.

          The return value should a bitmask of or'ed together values from the ViewableModes enum.

          The Nuke viewer will automatically switch to 3D or 2D modes when connected to an op
           that is only vieweable in a single mode.

          The default implementation does the following:

            returns eViewableMode3D for ops that are not Iops or convertable to Iops.
            returns eViewableMode2D for Iops or ops that can be convertible to Iops, and if all their inputs are the same type as the op.
            returns ( eViewableMode2D | eViewableMode3D ) for Iops or ops that can be convertible to Iops if they have mixed inputs

      */
      virtual int getViewableModes() const;

      /*!
         Index of the first "optional" input. The inputs between this
         number and minimum_inputs() are displayed differently in Nuke
         to indicate that it is ok for them to not be connected. Used
         by the mask inputs. The default version returns minimum_inputs(),
         which means there are no optional inputs.
       */
      virtual int optional_input() const;

      /*! The minimum number of input connections the operator can have.
          Default is inputs(). */
      virtual int minimum_inputs() const;

      /*! The maximum number of input connections the operator can have.
          There may be more inputs() if split_input() is used.  The
          default return value is inputs().
       */
      virtual int maximum_inputs() const;

      /*! This virtual function lets an operator decide what kinds of things
          to connect to the input. It can use any test wanted, but usually
          uses RTTI dynamic_cast<class*> to see if the Op is the correct subclass.
          If this returns false, then Nuke will use default_input() to generate
          the op that will be connected to this input.

          If you implement split_input() \a n is the user-visible input
          number, but set_input() will be called with the split input
          number.

          The default version returns false, so you can't hook anything up.
       */
      virtual bool test_input(int, Op*) const;

      /*! \fn int Op::inputs() const
         Current number of inputs.
         Nuke will set this to inputNumber(node_inputs()).
       */
      int inputs() const { return static_cast<int>(_allInputs.size()); }

      /*!
         Current number of inputs that the user sees in the Nuke DAG.
         This may be different than inputs() if you implement split_input().
       */
      int node_inputs() const;
      enum GenerateType {
        OUTPUT_OP = 0,    //!< Return op with no knobs stored or inputs built
        ANY_CONTEXT,      //!< Return tree but maybe at a different OutputContext
        INPUT_OP,         //!< Same as ANY_CONTEXT but skip disabled ops
        INPUT_OP_PEEK,    //!< Same as INPUT_OP, but avoid setting uiContext
        EXECUTABLE,       //!< Returns op tree for right context. Skips disabled ops on inputs (but not main op)
        EXECUTABLE_SKIP,  //!< Returns op tree for right context. Skips disabled ops.
        EXECUTABLE_INPUT, //!< Returns op tree for right context, with skipping, and replaces PostageStamp/TextureMap ops
        UICONTEXT_ONLY,   //!< Don't try to make a tree, but try to set uiContext on the nodes as if we had
      };

      /*!
         Return an op tree that \e could be connected to input \a n. This allows
         you to peek at your input ops before validate() is called, such
         as in knobs() or in the constructor. Or you can use this to
         peek at an input after uses_input() returns zero or split_input()
         returns zero or negative.

         \a n is the user-visible input number. If you implement
         split_input() this may be different than input(n), but it is
         the same as input(n,0).

         \a type determines what exactly is in the returned op.
         - Op::OUTPUT_OP returns an Op that you can call Class() on but
         not much else. Knob values have not been stored in it and no
         inputs are hooked up.
         - Op::ANY_CONTEXT will return an op created for this version of the tree
         but it may return it set to a different OutputContext. This lets
         you call almost anything but you cannot depend on any animated
         values being correct.
         - Op::INPUT_OP is like ANY_CONTEXT but skips disabled nodes so it
         returns the actual op that will be used to create data. Used by code
         that wants to peek at the output data, such as the format or frame
         range, but does not require the exact correct frame number or view.
         - Op::EXECUTABLE forces the OutputContext to be a certain value.
         Don't use this as I have not decided what this certain value is yet
         (may be uiContext(), outputContext(), or inputContext()...)
         - Op::EXECUTABLE_INPUT both forces the OutputContext and skips
         disabled nodes, and also inserts hidden ops used to generate
         Postage stamps and texture maps. This is as close as possible to
         the actual Op that will be connected to this input.

         If the input is not connected, OUTPUT_OP will return NULL, since
         this is commonly used to check if an input is connected. Note that
         if default_input(n) returns NULL, you cannot distinguish a disconnected
         input from an input connected to a Switch or other non-op node that
         is itself disconnected.

         All other values for \a type return default_input(n) if the input
         is not connected.

         Except for \a type of OUTPUT_OP, this is slow, and if the context it
         uses is wrong, it will cause extra instances of the entire Op tree
         to be generated, which can be very wasteful depending on the Op.

         \see node_inputs(), DD::Image::Execute
       */
      Op* node_input(int, GenerateType = OUTPUT_OP) const;
      Op* node_input(int, GenerateType, const DD::Image::OutputContext* outputContext) const;

      /*! \fn Op* Op::input_op(int n) const
         Same as node_input(n, INPUT_OP), this is provided for back-compatibility */
      Op* input_op(int n = 0) const { return node_input(n, INPUT_OP); }

      /*!
        Change the number returned by inputs().  Actually resizes array.  New
        entries are NULL; old ones have removeOutput called on them if set.
      */
      virtual void inputs(int n);

      /*!
         Return input \a n. This is the same as input(n, 0) if you do not
         implement split_input(), but is much faster.

         This is null if set_input() has not been called.

         Nuke only calls set_input() just before validate() is called, and
         only if uses_input() returns non-zero. If you want to look at the
         input op before this (such as in inputContext() methods) you must
         use node_input().
       */
      Op* input(int) const;

      /*!
         Calculate the "flat" input number from the user-visible input number
         and an index into the values returned by split_input(). This is the
         same as n+offset of you do not implement split_input().
         \a offset is assumed to be less than abs(split_input(n)).
       */
      int inputNumber(int input, int offset) const;

      /*!
         Same as input(inputNumber(n, offset));
       */
      Op* input(int input, int offset) const;

      /// return the primary input

      /*! \fn Op* Op::input0() const
          Fast \e inline method of getting input(0).
       */
      Op* input0() const { return input(0); }

      /// return the second input, if it exists

      /*! \fn Op* Op::input1() const
          Fast \e inline method of getting input(1) if it is not null.
          Warning: this may crash if input(1) is null.
       */
      Op* input1() const { return input(1); }
      
      //! Returns a reference to a vector of all inputs, including input0 and other_inputs.
      //! Note that this may contain null pointers if some in between inputs are disconnected.
      const std::vector<Op*>& getInputs() const { return _allInputs; }
      
      //! Returns a reference to a vector of all child ops.
      //! Avoid using this; it is only really public for op tree traversal.
      const std::vector<Op*>& getChildren() const { return _children; }

      /*!
         Set input(n). This does not change inputs(), even if \a n is
         greater.

         This is a virtual function so that subclasses can set the input
         pointers for child operators.  The arguments to inputNumber() are
         also provided to assist with this.  Make sure that any derived
         classes still call the parent class version though.
       */
      virtual void set_input(int i, Op* op, int input, int offset);

      /*! \fn void Op::set_input0(Op* i)
          Fast \e inline method of setting input(0). */
      void set_input0(Op* op) { set_input(0, op, 0, 0); }
      void set_input(int i, Op* op) { set_input(i, op, i, 0); }
      void set_input(int i, Op& op) { set_input(i, &op, i, 0); }

      /*!
         Same as set_input(inputNumber(n, offset), op, n, offset);
       */
      void set_input(int input, int offset, Op* op);

      /*! \fn const OutputContext& Op::outputContext() const;
         The current context that this Op is supposed to produce a picture
         for. This includes the frame number, the view, etc.
       */
      const OutputContext& outputContext() const { return outputContext_; }

      /*!
         Change what is in outputContext(). Nuke calls this for you.
         Subclasses can override this, but they must call the base class with
         exactly the same context. This method is a convenient place to do
         calculations that are needed before any of the following methods work:
         - int split_input(int) const;
         - float uses_input(int) const;
         - const OutputContext& inputContext(int n, int m, OutputContext&) const;
         - Op* defaultInput(int n, const OutputContext&) const;
         The knob values have been stored at this point, but no inputs
         have been created.
       */
      virtual void setOutputContext(const OutputContext& c);

      /*!
         Calculate a value for the hash(). The incoming variable is a
         suggested value for the hash, calculated by Nuke. This
         is a hash of all the uses_input() Op hash values and all the knobs()
         that don't have the NO_RERENDER flag set.  A subclass can override
         this to modify the hash, such as to append the frame number in cases
         where the output image changes depending on the frame even if no
         controls are animated:
         \code
         void MyOp::append(Hash& hash) {
         this->actual_frame = clamp(int(outputContext.frame()), first, last);
         hash.append(actual_frame);
         }
         \endcode
         The default version does nothing.
       */
      virtual void append(Hash&);

      /*!    Return non-zero to indicate that this operator actually uses the data
          on the given input arrow. If this returns false then Nuke will not
          highlight the input arrow, and may not even construct the operator
          on this input (so the pointer may be null or garbage or an out-of-date
          version of the operator.

          The value may be used to control how bright the arrow is, 1.0 indicates
          full use, 0.0 indicates none. This allows a cross-dissolve operator
          to fade the arrows on/off. In addition, values <= .01 will appear
          off to the user but will still construct inputs, this is useful
          for operators that want to peek at the frame ranges or formats but
          appear to the user as though the input is unused.

          The default version returns 1.0.
       */
      virtual float uses_input(int) const;
      virtual void build_splits();

      /*!
         If this returns a number \a m greater or equal to zero, then the
         user-visible input \a n is split into \a m separate inputs (i.e. the
         number of inputs will be increased by m-1, and the next user-visible
         input will be at input n+m).

         This is used to look at the input at several different frames.

         If you override this you must also override inputContext() to
         return different values for each of these or they will all end up
         hooked to the same Op.

         Returning -1 acts like 1 but leaves the input connected to an
         arbitrary frame (same as what node_input(n, ANY_CONTEXT) returns).
         You are expected to use an Execute object to switch this
         input to specific frames.

         The default version returns 1.
       */
      virtual int split_input(int) const;

      /*!
         Return the context to use for the input connected to input(n, offset). The
         most common thing to do is to change the frame number.

         The default version returns outputContext().

         You can use \a scratch as a space to construct the context and
         return it.

         This cannot look at input \a n or above, as they have not been
         created yet.  Often though it is useful to look at these inputs, for
         instance to get the frame range to make a time-reversing operator. If
         you want to do this you use node_input() to generate a "likely"
         op. You can examine any data in it that you know will not depend on
         the frame number.
       */
      virtual const OutputContext& inputContext(int n, int offset, OutputContext&) const;

      /*!
         Return the context to show the user for the input connected to user
         input \a n. This is used if the frame number, view, etc that you
         want the user to see is different than the first one you actually
         want. For instance a time-blending operation may want the times
         frame-.5 and frame+.5, but wants the user to see the frame number
         unchanged.

         Typical implementation is to not mess with the frame number:
         \code
         const OutputContext* MyOp::inputUIContext(int n, OutputContext&) const {
         return &outputContext();
         }
         \endcode

         The default version returns null, which is for back compatibility.
         If null is returned, inputContext(n,0) is used.
       */
      virtual const OutputContext* inputUIContext(int n, OutputContext&) const;

      /*!
         Return the Op to connect to this input if the arrow is disconnected
         in Nuke, or if the Op Nuke tries fails the test_input() test.
         Making this method return a default instance of the desired Op
         type will avoid having to put many checks for null inputs in
         your code. This can assume outputContext() is already set.

         The default version returns NULL, so disconnected arrows get NULL
         in their inputs.

         If you implement split_input() \a n is the user-visible input
         number, but set_input() will be called with the split input
         number.
       */
      virtual Op* default_input(int) const;

      /*! \fn const Hash& Op::hash() const
         A value which will change as the output of the operator changes.
         Nuke calculates this as a side-effect of constructing the tree.
         See append(Hash&) for how this is calculated.
       */
      const Hash& hash() const { return hash_; }

      /*! Override frameTransformImplemented() to return true if the Op implements frameTransformDownstream and
       * frameTransformUpstream. These are used to transform the times on keyframes used for such panels as the
       * Dope Sheet and the Curve Editor, as well as the keyframes on the Viewer timeline.
       *
       * For example, if this node is a simple retime with a constant factor, then frameTransformDownstream should
       * be overridden to set outFrame to the frame /produced/ given an input frame; and frameTransformUpstream
       * should be overriden to set inFrame to the frame /required/ given an output frame.
       *
       * For time operators that use multiple frames the 'centre' one should be preferred: this is the same
       * one which will be return by inputUIContext (or if that isn't implemented by the Op, then inputContext.
       * The OutputContext is passed in so that the view and other things on it can be honoured if necessary;
       * ignore that frame on it.
       *
       * The downstream and upstream methods should match, so taking the result of frameTransformDownstream and
       * feeding it into frameTransformUpstream should produce the same result.  However, there are lots of exceptions
       * to this, in particular when there is no downstream mapping the outFrame is allowed to be NAN.
       *
       * Note that it may be more helpful to provide an extension of the mapping that is is use for the linear
       * part than actually reflecting clipping behaviour at the end.  These functions are used for display of
       * keyframes in the user interface and not for actual processing.
       *
       * Both transform methods also take an input index, which identifies which input is upstream along the
       * path being taken through the DAG.  This allows overrides to query upstream nodes for data required
       * for the time transform, e.g. input clip start/length.
       *
       * frameTransformDownstrean and frameTransformUpstream should return true if the mapping was successful,
       * or false if no mapping was available.
       */
      virtual bool frameTransformImplemented() const { return false; }
      virtual bool frameTransformDownstream(const OutputContext& context, unsigned int input, float inFrame, float& outFrame) const;
      virtual bool frameTransformUpstream(const OutputContext& context, unsigned int input, float outFrame, float& inFrame) const;

      /*! Flags for influencing how the Op should be represented in the dopesheet.
          Override as appropriate.
       */ 
      virtual DopeItemFlags::Mask getDopeItemFlags() const;

      /*! Overrides should return true if the specified knob should be omitted in the Dope Sheet.
         This can be used to hide knobs such as the Read node's 'file' knob.
       */
      virtual bool shouldHideInDopeSheet(Knob* knob) const { return false; }

      /*! Override for an Op to specify a specific knob's keys to appear in its
       *  dope item in the DopeSheet.
       *  This is intended for use by time nodes, where the bar representing a time
       *  clip is driven by animation keys on a knob.
       */
      virtual DD::Image::Knob* getDopeItemKnob() const { return nullptr; }

      virtual ViewSet viewsProduced() const { return ViewSet(); }
      virtual ViewSet viewsWantOn(int i) const { return ViewSet(true); }

      virtual ViewSet splitForViews() const { return ViewSet(); }

      static unsigned current_request_pass; // public for debugging messages

      /*! \fn bool Op::requested() const;
         Returns true if setRequested() has been called since the last time
         forget_request() was called.  This is used by Iop::request() style
         calls to see if this is the first time they were visited.
       */
      bool requested() const { return this->request_pass == current_request_pass; }

      /*! \fn void Op::setRequested()
         Make requested() return true. This is intended to be called by
         subclass methods such as Iop::request().
       */
      void setRequested() { this->request_pass = current_request_pass; }

      /*! Turn off requested() in this Op only
       *
       * Without this Iop::request() calls will accumulate their results
       * and the Iop::_request() virtual functions will not be called
       * unless the request is for more channels or pixels.
       */
      void forgetRequestShallow();

      void forget_request();

      /*!
         Recursively turn off requested() in this Op and all the inputs.
         Without this Iop::request() calls will accumulate their results
         and the Iop::_request() virtual functions will not be called
         unless the request is for more channels or pixels.

         Also see all_forget_request() which is much faster.
       */
      void forget_request(std::set<Op*>& forgotten);

      /*! \fn void Op::all_forget_request()
         Acts like forget_request() was called on every Op in existence.
         This is done by incrementing a static variable and is thus a much
         faster method.
       */
      static void all_forget_request() { ++current_request_pass; }
      static void new_request_pass() { all_forget_request(); }

      /*! Return what shape Nuke should draw the box in the DAG window.
          This is a string where the characters approximately show what
          shapes the ends of the box are. The default return value is "[]"
          (or "\)" for DeepOps);

          Other possible characters are < > ( ) r c p b { } / \ and !
       */
      virtual const char* node_shape() const;

      DD::Image::Hash curveHash() const;
      bool opCurrent() const;

      /*! Return the default color for the Node in the Nuke DAG window.
          The value is 0xRRGGBBAA. The AA byte is ignored right now.
          Returning 0 indicates that you want the fall-back color set in
          the user's preferences to be used. If you really want to return
          black, return 0xff.
       */
      virtual unsigned node_color() const;

      /*! Return help information for this node. This information is in the
          pop-up window that the user gets when they hit the [?] button in
          the lower-left corner of the control panel.

          The first character should not be a punctuation mark, they are all
          reserved for future use.
       */
      virtual const char* node_help() const = 0;

      /*! Return the text Nuke should draw on the arrow head for input \a i
          in the DAG window.  This should be a very short string, one letter
          ideally.  If you want you can print into the passed buffer and
          return a pointer to that, the buffer is very short (20
          characters). Return null or an empty string to not label the arrow.

          The default version checks maximum_inputs(). It leaves 1-input nodes
          blank. 2-input nodes are labeled B and A. Higher numbers of inputs
          get their index number + 1 as a label.
       */
      virtual const char* input_label(int, char*) const;

      /*!
         Return a longer string describing an input. This is used when there
         is more room to format the string into, such as a tooltip. If an
         empty string is returned then input_label() is used.
       */
      virtual std::string input_longlabel(int) const;

      /*! Programs (e.g. Nuke) can set this callback to change what code is
          called when node_redraw() is called. The default version does
          nothing. */
      static void (* node_redraw_cb)(const Op*);

      /*! \fn void Op::node_redraw()
          An operator should call this if it does anything to change it's
          color(), shape(), or label(), this tells Nuke to update the
          DAG display. If this operator has a parent() this does nothing.
       */
      void node_redraw() { node_redraw_cb(this); }

      /*! Programs can set this callback so they can use the node() to
          print the name of the Op. By default this just prints the Class()
          of this and all parent() operators.
       */
      static void (* print_name_cb)(std::ostream&, const Op*);

      /*! \fn void Op::print_name(std::ostream& o) const;
          Prints the name of the Op, for use in error or warning messages.
       */
      void print_name(std::ostream& o) const { print_name_cb(o, this); }

      enum NodeContext {
        eNodeGraph, /// part of the node graph ( Nuke )
        eTimeline, /// part of a timeline effect ( Hiero )
        eOpGraph /// part of an op graph (NukeEngine)
      };

      /*! return the node context for this op */
      NodeContext nodeContext() const;

      /*! Returns the same string as printed by print_name(ostream). 
      */
      std::string node_name() const;

      /*!
         Make sure the next call to validate() calls _validate(), by
         turning off the valid() flag, and calling _invalidate().

         This is a very fast function if valid() is false so you can call it
         many times.
       */
      void invalidateSameHash();

      /*!
         If valid(), change the hash() to a random number and call
         invalidateSameHash().  Since the hash is different, output operators
         that check it to see if the image has changed will think it has
         changed. Returns true if it was valid().

         Don't use this on Op instances produced by Nuke as it will mess up
         the hash() value which Nuke relies on not changing. Plugins can use
         this on internal Op instances, and other programs using DDImage can
         use this.

         If you just want to force _validate() to get called, use
         invalidateSameHash().
       */
      bool invalidate();

      /*!
         Checks if the hash is different, in which case it changes hash() to
         this new value, and calls invalidateSameHash(). Returns true if the
         hash is different.
       */
      bool invalidate(const Hash& hash);

      /*! Obsolete function, calls asapUpdate(*box) or asapUpdate() if
         box is NULL, returns true. */
      bool update(const Box* = nullptr);

      /*!
         Indicate that the output of this Op has changed. This may be called
         by a parallel thread, such as a socket listener, or other connection
         to an outside image source. The Nuke viewer will redraw with the new
         image.

         Currently you \e must also implement append(Hash&) and make it
         modify the hash.  The easiest way is to increment a counter when you
         call this and add call hash.append(counter) in that function. The
         viewer will not update if the hash does not change.

         This disables disk caching by the viewer, on the assumption that the
         resulting image will only be seen once.

         Return value from Nuke is currently always true.
       */
      void asapUpdate();

      /*!
         Same as asapUpdate() but the box indicates an area that you want the
         viewer to draw first. This area must be in viewer coordinates (including
         the current proxy settings). This is used by paint strokes.

         The optional \a direction indicates which direction to draw the box.
         Positive draws from bottom to top, negative from top to bottom. Zero
         indicates you don't care.
       */
      void asapUpdate(const Box& box, int direction = 0);

      /*! Turns on valid(), and if \a for_real is true, turns on real_valid().
          This returns immediately if these flags are already on.

          When this returns, info() has been filled in with information about
          the output image. If \a for_real is false this is a quick "simulation"
          used by Nuke. If \a for_real is true this is a more accurate version
          that incorporates the actual information that is read from input
          files, and can take considerably longer.

          If this has ever called error() since the last time invalidate()
          was called, this will immediately report the same error message,
          turn on abort(), and return. You must test for this after
          calling open() (this need for a test will go away when we
          fix it to use exceptions).  */
      void validate(bool for_real = true);

      /*! Acts as though valid() is off and does validate(). Thus you can be
         certain that _validate() is called. */
      virtual void force_validate(bool for_real = true);

      void update_handles() const;

      /*! \fn bool Op::valid() const
          Returns true after validate(false) or validate(true) has been called.
          You should not use the data in info() for anything if this is false.
          Instead call valid() first.
       */
      bool valid() const          { return _state >= eValid; }          //!< Returns true if it has been validated - including if there was an error

      /*! \fn bool Op::real_valid() const
          Returns true after validate(true) has been called. This means the
          data in info() is truly correct and tested against slow things like
          opening files. This is used by Nuke, normal programs that never call
          validate(false) will not need to use this and can use valid().
       */
      bool real_valid() const     { return _state >= eRealValid; }      //!< Returns true if it has been real-validated - including if there was an error
      bool inErrorState() const   { return _state == eError || _state == eInvalidWithError; }
      bool inInvalidState() const { return _state == eInvalid || _state == eInvalidWithError; }

      /*! \fn bool Op::opened() const
          Returns true after open() has been called.
          Returns false after invalidate() or close() has been called.
          This indicates that the operator is ready to have data extracted
          from it (by the engine() call in the case of an Iop). Normally
          this is turned on by operators calling open() on themselves on
          the first call to get data.
       */
      bool opened() const         { return _state >= eOpened; }

      /*! Gets the operator ready for producing data. Normally this is
          called by operator implementations on the first call to get some
          data from it. The main thing this does is call _open() if
          it has not been called since the last invalidate().

          If this has ever called error() since the last time invalidate()
          was called, this will immediately report the same error message,
          turn on abort(), and return. You must test for this after
          calling open() (this need for a test will go away when we
          fix it to use exceptions).
       */
      void open();
      void unopen();

      /*! \fn bool Op::running() const
          Returns true if engine() is currently executing. This is used
          to highlight the busy operators in Nuke. It is not very accurate
          (because multiple threads may turn it off) so don't use this for
          anything other than user feedback.
       */
      bool running() const { return running_; }

      /*! \fn void Op::running(bool)
          Turn the running highlight in the Nuke display on/off. You should
          do this around any code that extracts data. This is very fast
          (much faster than calling node_redraw()), it just sets/clears a local
          flag. Nuke works by running a parallel thread that wakes several
          times a second to see if the highlight changed and redraw the DAG
          display with the current highlighting.
       */
      void running(bool v) { running_ = v; }

      /*! \fn bool Op::not_closed() const
          Returns true after open() has been called.
          Returns false after close() has been called.
          invalidate() does not turn this off, this is an indicator that the
          Op may have resources allocated that could be freed.
       */
      bool not_closed() const { return opened_; }

      /*!
         Tell the Op to free all recreatable data, on the assumption that this
         operator will not be used again soon. This calls _close() and also
         turns off flags so it knows that _open() must be called again.
       */
      void close();

      /*!
         Indicate that close() (and thus _close()) should be called after the
         given number of seconds of idle. Passing zero is useful to get close()
         called as soon as the viewer stops updating.

         This is a one-shot timeout, the entry is removed after close() is
         called. Call this again if you want to be called again.  Only the
         last call is used, it replaces any previous timeout whether larger
         or smaller, so calling this with INFINITY will disable the callback.
       */
      void callCloseAfter(double seconds);

      /*!
         Call close() on some (currently only zero or one) Ops that asked
         for it to be called with a time less or equal to \a seconds. Return
         with the smallest time of any remaining callCloseAfter calls, or
         INFINITY if none. You should loop until this returns INFINITY (or
         perhaps some very large number).
       */
      static double callPendingClose(double seconds);

      /*! For use by Nuke, do not use */
      static void clearPendingClose();

      /*!
         This is called when Nuke inserts no-op operators such as postagestamps.
         Change the op's valid and request state to match exactly the input, and
         possibly move the cache from the input to here. The hash is copied because
         this must be a no-op.
       */
      void copyState(Op* input);
      bool cached() const { return cached_; }

      /*! \fn void Op::cached(bool)
          Indicates that caching of the output data is requested, so that asking
          for it again will be faster. Whether or not this does anything depends
          on the subclass of Op, but Iop uses this.
       */
      void cached(bool b) { cached_ = b; }

      virtual bool inUse() const { return false; }

      /*! Return pointer to this object as an Executable if you want Nuke to
          call execute() on this node when the user hits render.

          <i>Don't implement this if you just want to have a button the user
          presses to execute the node.</i> The tcl "execute" command will
          still work and you can make a script_knob that calls it.

          The default version returns nil.
       */
      virtual Executable* executable() { return nullptr; }

      /*!
         Change the knobs to reflect the values at a different frame (or
         other piece of context information), and call invalidate if they are
         different. This can be used to determine the Op's animation. Make
         sure the knobs are set back by calling this with outputContext()
         when done.
       */
      void setKnobsToContext(const OutputContext&);

      /*!
         Change Nuke to reflect a different frame number (and perhaps other
         aspects of the context), and change this op and its entire input
         tree to reflect this context. If \a update is true then it will also
         pause until the entire user interface is updated with the new frame.

         You will have to call validate() on the inputs before using them.

         This should only be called due to UI actions such as knob_changed().
       */
      void gotoContext(const OutputContext&, bool update);

      /*! Flags to alter the behaviour of status_callback
       */
      enum StatusFlags {
          StatusNone        = 0x00000000,  // No special behaviour
          StatusModal       = 0x00000001,  // Use a modal progress notification (ignored by progressFraction)
          StatusUseCallback = 0x00000002   // Make progressFraction attempt to call any progressUpdate callbacks
      };

      /*!
         Set the location of the progress bar. \a fraction should be between 0 and 1.
         If this is not called the fraction is zero.
         flags can control the behaviour of progress updates, see StatusFlags enum.
         Only StatusUseCallback affects progressFraction().
       */
      void progressFraction(double fraction, StatusFlags flags = StatusNone);

      /*! \fn void Op::progressFraction(int a, int b);
         Same as progressFraction(double(a) / b);
       */
      void progressFraction(int a, int b, StatusFlags flags = StatusNone) { progressFraction(double(a) / b, flags); }

      /*!
         Set a message to display on the progress meter. If this is not called, or
         if \a fmt is zero, then the message is the name of the operator.
         If you already have the message figured out, use a \a fmt of "%s",
         this is detected and the next argument is passed directly.
       */
      void progressMessage(const char* fmt, ...);
      void progressModalMessage(const char* fmt, ...);

      /*!
         Make the progress meter disappear (if it was displaying this op) and
         reset all the progress stuff to it's default values.
       */
      void progressDismiss();

      /*! Non-Nuke programs may set this pointer to get called when operators
          call progressFraction() and progressMessage(). This is mostly useful
          to get Write operators to print a display.

          The Op argument is set to the op that status() was called on.

          The fraction is passed unchanged.

          Total is unused and should be ignored.

          The message is the printed message from progressMessage().
          This buffered result will be freed when this returns, so you must
          copy it if you want to keep it.

          Flags indicate whether operating in modal mode, and whether doProgressUpdate should be called.

          An implementation should show the message (if any) to the user,
          and use the fraction to update a percent-done indicator.  You
          should avoid expensive operations if the indicator has not
          changed.  Here is a sample:

          \code
          void status_callback(const Op*, float fraction, float total, const char* t, StatusFlags flags)
          {
            static int previous = 0;
            int percent = int(fraction*100+.5);
            if (t)
              printf("\r%s:\033[K", t);
            else if (percent == previous)
              return;
            previous = percent;
            printf("%3d%%\b\b\b\b", percent);
            fflush(stdout);
          }
          // ...
          Op::status_callback = status_callback;
          \endcode
       */
      static void (* status_callback)(const Op*, float, float, const char*, StatusFlags);

      /*! \fn int Op::slowness() const
          0 means no cache is necessary.  This is for operators that
          directly calculate their output (and thus read no inputs), and for
          operators that internally buffer their output (so any additional
          caching is useless). Any operator that returns zero will disable
          any attempt to cache the output.

          Non-zero numbers are supposed to indicate the slowness of the
          calculation. In theory this will be used to intelligently
          place caches where they do the most good, but the values
          are currently ignored. A sequence of operators would have the
          request() counts multiplied by their slowness and these added
          together to get an idea how expensive a branch is. So this value
          should be linear with the speed of the operator.

          The default value is 10. This value is used by color correctors
          and similar pixel operators. Almost any other operator will be
          slower, you should judge how long it takes relative to a color
          corrector and multiply accordingly.

          1 means really fast and should be returned by an operator that
          does not look at pixels, but simply copies blocks of them from
          the input to the output. Shuffle and Crop are examples of this.
       */
      int slowness() const { return slowness_; }
      void slowness(int newval) { slowness_ = newval; }

      /*! \fn bool Op::firstEngineRendersWholeRequest() const

        Override this method and return true if the first engine call renders 
        the entire request area and locks all the other render threads.
         
        This may be used to reuse the spare render threads in other parts of the tree
         and only call this op's engine call from one thread.

        It is also useful in Nuke as it will hide the 'white-lines' if this method
        returns true and the viewer is connected to this op.

        Note it is valid to return true or false dynamically from this method between
        validate passes.
      */
      virtual bool firstEngineRendersWholeRequest() const;

      //! can be overriden by ops to update their UI when their control panel is open.
      //! if you override it, return true, otherwise it will not be called again.
      virtual bool updateUI(const OutputContext& context) { return false; }

    public:

      /*!
         Returns the full_size_format() of node_input(0). This is needed
         to convert uv values to pixels or to look at the pixel aspect ratio.
         It can also be used to initialize default values of fields in the
         constructor. If the input is not connected, or not connected to a node
         producing an Iop, the format of Iop::default_input() is returned.

         Note that until validate() is called, this returns only a "guess".
         Be prepared for the format being different in validate().
       */
      const Format& input_format() const;

      /*!
         Return the "user interface Context" from the node that generated
         this Op. See Knob::outputContext() for more information. This is
         most useful for finding out the frame number or view the user is
         interested in right now.  This is not necessarily the same as the
         Context passed to setOutputContext()!
       */
      const OutputContext& uiContext() const;

      /*! An operator can call this to report an error. This will abort all trees
          that the op is in - i. e. calling \a aborted() on this op or any op in
          one of the trees it's in will subsequently return true.

          The fact that this was called is recorded and open()
          will immediately cause the error again and return unless
          you call invalidate() on this.

          The current version truncates the error message at 2048 characters
          unless the format is "%s" in which case it knows to use the
           first argument directly.
       */
      void error(const char* fmt, ...);

      /*! Print a warning message.  On Unix these go to stderr and are
          prefixed with print_name().
       */
      void warning(const char* fmt, ...);

      /*! An operator can call this to report an error. This will abort all trees
          that the op is in - i. e. calling \a aborted() on this op or any op in
          one of the trees it's in will subsequently return true.

          The fact that this was called is recorded and open()
          will immediately cause the error again and return unless
          you call invalidate() on this.

          The current version truncates the error message at 2048 characters
          unless the format is "%s" in which case it knows to use the
          first argument directly.
       */
      void critical(const char* fmt, ...);

      /*! Print a debug message.  These will only go to stdout/stderr, unless the user has turned on verbose mode
       */
      void debug(const char* fmt, ...);

      // NOTE: these functions are no longer static, as they need to apply to the op's tree.
      // When called from op code this won't be a problem.  Other code will need to access the
      // OpTreeManager.
      
      void abort() const;                   //!< Abort all trees the op is in
      bool aborted() const;                 //!< True if one of the trees the op is in was aborted; ops should check this while processing and return quickly when true
      
      void cancel() const;                  //!< Cancel all trees the op is in.  Make sure you also call \a abort() to stop processing
      bool cancelled() const;               //!< True if one of the trees the op is in was cancelled (by user interaction)
      
      bool addToTree(OpTree* lpTree);       //!< Returns true if the op was added to the tree, or false if its reference count increased
      bool removeFromTree(OpTree* lpTree);  //!< Returns false if the op was removed from the tree, or true if its reference count decreased
      bool isInTree(OpTree* lpTree) const;  //!< Checks if an op is in a particular tree, for debugging purposes
      bool isInAnyTree() const;             //!< Checks if an op is in any tree at all

      //! Returns the treeHandler for a given Op
      OpTreeHandler* getTreeHandler() const;

      //! Returns an op with an active error (the first one found if any), or NULL if none.
      //! Deprecated.  Use other error functions such as \a Op::hasError() or \a OpTree::getFirstOpWithError() instead.
      static Op* error_op();
      //! Returns the error string from an op with an active error (the return from \a error_op()), or NULL if none.
      //! Deprecated.  Use other error functions such as \a OpMessageHandler::getFirstError() instead.
      static const char* error_message();
      
      static void clearAbortAndError() {}   //!< To be removed

      bool hasError() const;

      //!< Return true if this op, or any of it's children, is in an error state
      //!< Not thread safe; only call from the main thread
      bool opOrChildHasError() const;
      
      //!< The same as opOrChildHasError, but will return the first Op which is in an error state.
      //!< Not thread safe; only call from the main thread
      const Op* getErroredOp() const;

      //! Pop-up schemes determine when pop-up messages are displayed for op errors
      enum PopUpScheme {
        ePU_Never,        //!< Don't ever display pop-ups for this op
        ePU_Done,         //!< Previously only one pop-up was to display (ePU_Once), and it's now been shown
        ePU_Once,         //!< Display only the next pop-up for this op
        ePU_Default,      //!< Display pop-ups according to the Nuke default: while loading scripts or changing knobs
        ePU_Always        //!< Display every pop-up for this op
      };
      void setPopUpScheme(PopUpScheme lScheme)        { _popUpScheme = lScheme; }
      PopUpScheme getPopUpScheme() const              { return _popUpScheme; }
      
      //! Returns whether a pop-up should be enabled for the next message, modifying internal data when necessary
      bool arePopUpsEnabled();
      
      //! This is used by Nuke to indicate when ops using the default pop-up scheme should show pop-ups.  Not recommended for customisation.
      static void SetDefaultPopUpEnable(bool lEnable);
      
      OpMessageHandler& getMsgHandler()               { return *_pMsgHandler; }
      const OpMessageHandler& getMsgHandler() const   { return *_pMsgHandler; }
      
      //! Returns a string of debug information about the op.  For internal use.
      std::string getDebugInfo() const;
      std::string getDetailedDebugInfo() const;

      /*!
         Call validate() and return true if it did not cause error() to be called.
         During this call calls to error() do not cause abort(), and thus will not
         kill running parallel threads.

         All new code should call this instead of validate() to be compatible with
         future versions.

         Returns false if error was called. More detail can be retrieved from the
         message in the op's message handler - see \a getMsgHandler().
         Returns true if error() was not called.
       */
      bool tryValidate(bool for_real = true);


      //! Cast to an Iop. This is much cheaper and safer than using dynamic_cast.
      virtual Iop* iop();
      virtual const Iop* iop() const;

      //! Cast to a GeoOp. This is much cheaper and safer than using dynamic_cast.
      virtual GeoOp* geoOp();
      virtual const GeoOp* geoOp() const;

      //! Cast to a ParticleOp. This is much cheaper and safer than using dynamic_cast.
      virtual ParticleOp* particleOp();
      virtual const ParticleOp* particleOp() const;

      //! Cast to a DeepOnlyOp. This is much cheaper and safer than using dynamic_cast.
      virtual DeepOnlyOp* deepOnlyOp();
      virtual const DeepOnlyOp* deepOnlyOp() const;

      /*! \class DD::Image::Op::Description

          A subclass of Op that wants to appear on Nuke's menu or be
          created by a Nuke script command must create one or more static
          instances of this structure.  The constructor adds itself to a
          list which Nuke scans to find all the possible operators.

          The code that goes into a plugin to create the entry looks like this:

          \code
         static const char* const MYCLASS = "MyOp";
         class MyOp : public Op {
         static const Description description;
         public:
         const char* Class() const {return MYCLASS;}
         ...
         };

         static Op* build(Node* node) {return new MyOp(node);}
         const Op::Description MyOp::description(MYCLASS, build);
          \endcode

          It is required that the string inside the description and returned
          by Op::Class() be the exact same pointer. This is checked by Nuke.
          It is also required that this string is the same as base name of
          the plugin file, or the name of a .tcl file that says "load foo"
          where foo is the name of this plugin file. Nuke only stores the
          name in the saved script, so this is necessary for it to find the
          plugin again.

       */
      class DDImage_API Description : public DD::Image::Description
      {
      protected:

        /*! Internals of the constructor, adds the description to the internal
            array and calls add_callback.
         */
        static void add(DD::Image::Description*);
        static const Description* current_;
        typedef Op* (*OpConstructor)(Node*);
        typedef Iop* (*IopConstructor)(Node*);
        OpConstructor constructor_;
      public:
        const char* name; //!< Nuke command
        Op* constructor(Node* node) const //!< Nuke calls this to create an instance.
        {
          current_ = this;
          return constructor_(node);
        }

        /*! The constructor adds this Op to the internal table of available
           script commands.

           If the License is not null then it is tested and
           if the test fails the description is not added to the table, making it
           impossible to call the constructor. You should check the license
           pointer when your constructor is called to make sure some cracker
           has not changed it or cleared it. */
        Description(const char* n, OpConstructor c, License * l = nullptr) :
          constructor_(c), name(n)
        {
          license = l;
          ctor(add);
        }

        Description(const char* n, OpConstructor c, NodeBuilder nodeBuilder) :
          constructor_(c), name(n)
        {
          license = nullptr;
          ctor(add, nodeBuilder);
        }

        /*! For back compatibility with Nuke4.0 plugins, this takes an
           extra "menu" argument which is ignored (menu entries are all
           set by tcl commands now). */
        Description(const char* n, const char* /*menu*/, OpConstructor c) :
          constructor_(c), name(n)
        {
          license = nullptr;
          ctor(add);
        }

        /*! For back compatibility with Nuke4.0 plugins, this takes an
           extra "menu" argument which is ignored (menu entries are all
           set by tcl commands now). */
        Description(const char* n, const char* /*menu*/, IopConstructor c) :
          constructor_((OpConstructor)c), name(n)
        {
          license = nullptr;
          ctor(add);
        }

        /*! Return the i'th Op::Description known about, or return a null pointer
            for the last one. This does not load any plugins, it only returns
            the ones that have been loaded so far. */
        static const Description* find(int i);
      };

      /*! \fn const char* Op::Class() const;

         Return the command name that will be stored in Nuke scripts. This
         must be unique for each different function and must be the same
         string (ie the same pointer) as is in the Op::Description object
         that constructed this instance.

         If you don't plan to make a Nuke operator you can have this return
         null.
       */
      virtual const char* Class() const = 0;

      /*!
         Return a name for this class that will be shown to the user. The
         default implementation returns Class(). You can return a different
         (ie more user-friendly) name instead here, and there is no need
         for this to be unique.

         Nuke currently will remove any trailing digits and underscores from
         this and add a new number to make a unique name for the new node.
       */
      virtual const char* displayName() const;

      /*!
         Return the default path used to search for online help.
         Usually the Class() but can be overridden.
       */
      virtual std::string docsPath() const;
	  
      //! A function _designed_ to send keyboard (and other) events to the selected node.
      //! Could be used for a variety of event & data.
      virtual bool onAction(const ViewerContext* c, const DD::Image::Flags f, void* d)
      {
        return false;
      }

      /*!
         This is used to insert operators that produce data needed by the
         user interface, such as TextureMap, Histograms, and
         PostageStamp. Usually this is called by one of the outputs of this
         op to insert something that operator needs for the user interface.

         Nuke will create the described operator and connect it to the output
         of this->firstOp(), inserting it between it and whatever was
         connected to there. The new operator is returned. You can then
         validate() it and execute() it to get information from it. Or you
         can rely on side-effects of lines being pulled through the operator,
         or on execute() being called during idle by Nuke, which will avoid
         slowing down the main calculation.

         If this is called again with the same description the previous
         created one is returned again. So at most one operator of each
         type is created for any Node.
       */
      Op* get(const Description* description);

      /*! Search for an Op::Description that has the given name.
          Returns the description if found.  If it fails, it throws a
          std::runtime_error which contains the message from plugin_error().

          First this searches all the ones that are known about.
          Then it tries to load a plugin using plugin_load() and
          sees if that defines the operator.
       */
      static const Description* find_description(const char* name, const Op* op_); // can throw std::runtime_error

      /*! This function prints a formated message to stdout. If DDImage runs
          in Nuke in interactive mode, this function will open a modal dialog
          box instead.

          \arg t defines the type of dialog that will appear:
          - 'i' will show a message dialog
          - '!' will show an alert dialog
          - '?' will show a question and return 1 if the user click 'yes', and 0 otherwise
       */
      static int message_f(char t, const char*, ...);
      /*! \see Op::message_f */
      static int message_vf(char t, const char*, va_list);

      /*! Add a one-shot timeout callback.

          The function will be called by Nuke at \a t seconds after this function
          is called. The optional void* argument is passed to the callback (you
          may want to pass \a this).

          The timeout will be called even if the Op is destroyed. If you don't
          want this to happen, make sure you call remove_timeout in the
          destructor.
       */
      static void add_timeout(float t, TimeoutHandler, void* v = nullptr);

      /*! Inside a timeout callback you can call this to add another timeout.
          Rather than the time being measured from "now", it is measured from when
          the system call elapsed that caused this timeout to be called. This will
          result in far more accurate spacing of the timeout callbacks, it also has
          slightly less system call overhead.

          It is undefined what this does if called from outside a timeout callback.
       */
      static void repeat_timeout(float t, TimeoutHandler, void* = nullptr);
      /*! Returns true if the timeout exists and has not been called yet. */
      static bool has_timeout(TimeoutHandler, void* = nullptr);

      /*! Removes a timeout callback.
          It is harmless to remove a timeout callback that no longer exists.
       */
      static void remove_timeout(TimeoutHandler, void* = nullptr);
      
    protected:
      //! virtual function that is called by preValidate to do actual work.  The base
      //! class version calls preValidate() recursively on all inputs.  An override of this
      //! should either call that and return its return value if non-eFinished, eCancelled
      //! if the user actually clicked 'cancel' in a progress bar, or eAbandoned if the tree
      //! became obsolete.
      virtual PrevalidateResult doPreValidate(ViewerContext* ctx = nullptr, PreValidateContext* = nullptr);
      
    private:
      
      //! Resets any active errors and warnings on this op by clearing its message list.
      //! It ensures any messages from knobs are maintained, though, so calling this function does not guarantee
      //! that there will be no messages on the op afterwards.
      void clearMessagesFromOp();
      
      //! Add the given op/number to the list of outputs for this op.  (For internal use by set_input)
      void addOutput(Output output);

      //! Remove the given op/number from the list of outputs for this op.  (For internal use by set_input)
      void removeOutput(Output output);

      void addChild(Op* lpOp);      //!< Not thread safe; only call from the main thread
      void removeChild(Op* lpOp);   //!< Not thread safe; only call from the main thread
      
      //! Tree handler function wrapping to deal with ops without valid trees
      template <typename TreeHandlerFn, typename ManagerFn>
      bool CallCheckedTreeHandlerFn(TreeHandlerFn lFn, ManagerFn lFallbackFn) const;
      
      //! This function applies any required changes for the current state, such as clearing errors when appropriate
      void commitState();
      
      //! Returns whether pop-ups should be enabled for the next message, updating internal data as necessary.
      //! NOTE: not thread-safe.
      bool arePopUpsEnabled_Unsafe();
      
      // These should really be scoped_ or shared_ptrs, but we need to avoid a dependency on Boost
      OpTreeHandler*      _pTreeHandler;    //!< Handles op tree manipulation for this op
      OpMessageHandler*   _pMsgHandler;     //!< Handles message reporting for this op
      
      PopUpScheme         _popUpScheme;     //!< Determines when pop-up messages are displayed for op errors
      static int          _sPopUpEnableSem; //!< Semaphore; pop-ups enabled for ops using the default scheme when >0
      
    };

    /*!
     * Return the current global setting for frames per second.
     */
    extern DDImage_API float (* root_real_fps)();

    inline static void operator|= (Op::HandlesMode& lhs, Op::HandlesMode rhs)
    {
      lhs = Op::HandlesMode(int(lhs) | int(rhs));
    }

    inline static Op::HandlesMode operator| (Op::HandlesMode& lhs, Op::HandlesMode rhs)
    {
      return Op::HandlesMode(int(lhs) | int(rhs));
    }

    //! Cast an op to a known subclass. This is cheaper than dynamic_cast, as well as being safer over module boundaries.
    template<class T> inline T op_cast( Op* op ) { return dynamic_cast<T>(op); }
    template<> inline Iop* op_cast<Iop*>( Op* op ) { return op ? op->iop() : nullptr; }
    template<> inline GeoOp* op_cast<GeoOp*>( Op* op ) { return op ? op->geoOp() : nullptr; }
    template<> inline ParticleOp* op_cast<ParticleOp*>( Op* op ) { return op ? op->particleOp() : nullptr; }
    template<> inline DeepOnlyOp* op_cast<DeepOnlyOp*>( Op* op ) { return op ? op->deepOnlyOp() : nullptr; }

  }
}

#endif // DDImage_Op_h

// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.
