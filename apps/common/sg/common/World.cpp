// ======================================================================== //
// Copyright 2009-2017 Intel Corporation                                    //
//                                                                          //
// Licensed under the Apache License, Version 2.0 (the "License");          //
// you may not use this file except in compliance with the License.         //
// You may obtain a copy of the License at                                  //
//                                                                          //
//     http://www.apache.org/licenses/LICENSE-2.0                           //
//                                                                          //
// Unless required by applicable law or agreed to in writing, software      //
// distributed under the License is distributed on an "AS IS" BASIS,        //
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. //
// See the License for the specific language governing permissions and      //
// limitations under the License.                                           //
// ======================================================================== //

#include "sg/common/World.h"

namespace ospray {
  namespace sg {

    World::World()
    {
      createChild("bounds", "box3f");
    }

    box3f World::bounds() const
    {
      box3f bounds = empty;
      for (const auto &child : properties.children)
        bounds.extend(child.second->bounds());
      return bounds;
    }

    std::string World::toString() const
    {
      return "ospray::viewer::sg::World";
    }

    //! serialize into given serialization state
    void sg::World::serialize(sg::Serialization::State &state)
    {
      sg::Serialization::State savedState = state;
      for (auto node: nodes)
        node->serialize(state);
      state = savedState;
    }

    void World::preCommit(RenderContext &ctx)
    {
      numGeometry=0;
      oldWorld = ctx.world;
      ctx.world = std::static_pointer_cast<sg::World>(shared_from_this());
      if (ospModel)
        ospRelease(ospModel);
      ospModel = ospNewModel();
      ospCommit(ospModel);
      setValue((OSPObject)ospModel);
      oldModel = ctx.currentOSPModel;
      ctx.currentOSPModel = ospModel;
    }

    void World::postCommit(RenderContext &ctx)
    {
      ospCommit(ospModel);
      ctx.world = oldWorld;
      ctx.currentOSPModel = oldModel;
    }

    void World::preRender(RenderContext &ctx)
    {
      preCommit(ctx);
      // oldWorld = ctx.world;
      // ctx.world = std::static_pointer_cast<sg::World>(shared_from_this());
    }

    void World::postRender(RenderContext &ctx)
    {
       // ospCommit(ospModel);
       // ctx.world = oldWorld;
       // ctx.currentOSPModel = oldModel;
      postCommit(ctx);
    }

    InstanceGroup::InstanceGroup()
      : baseTransform(ospcommon::one), worldTransform(ospcommon::one),
        cachedTransform(ospcommon::one)
    {
      createChild("bounds", "box3f");
      createChild("visible", "bool", true);
      createChild("position", "vec3f");
      createChild("rotation", "vec3f", vec3f(0),
                     NodeFlags::required      |
                     NodeFlags::valid_min_max |
                     NodeFlags::gui_slider).setMinMax(-vec3f(2*3.15f),
                                                      vec3f(2*3.15f));
      createChild("scale", "vec3f", vec3f(1.f));
    }

        /*! \brief return bounding box in world coordinates.
      
      This function can be used by the viewer(s) for calibrating
      camera motion, setting default camera position, etc. Nodes
      for which that does not apply can simpy return
      box3f(empty) */
    box3f InstanceGroup::computeBounds() const
    {
      box3f cbounds = empty;
      for (const auto &child : properties.children)
        cbounds.extend(child.second->bounds());
      if (cbounds.empty())
        return cbounds;
      const vec3f lo = cbounds.lower;
      const vec3f hi = cbounds.upper;
      box3f bounds = ospcommon::empty;
      bounds.extend(xfmPoint(worldTransform,vec3f(lo.x,lo.y,lo.z)));
      bounds.extend(xfmPoint(worldTransform,vec3f(hi.x,lo.y,lo.z)));
      bounds.extend(xfmPoint(worldTransform,vec3f(lo.x,hi.y,lo.z)));
      bounds.extend(xfmPoint(worldTransform,vec3f(hi.x,hi.y,lo.z)));
      bounds.extend(xfmPoint(worldTransform,vec3f(lo.x,lo.y,hi.z)));
      bounds.extend(xfmPoint(worldTransform,vec3f(hi.x,lo.y,hi.z)));
      bounds.extend(xfmPoint(worldTransform,vec3f(lo.x,hi.y,hi.z)));
      bounds.extend(xfmPoint(worldTransform,vec3f(hi.x,hi.y,hi.z)));
      return bounds;
    }

    box3f InstanceGroup::bounds() const
    {
      return child("bounds").valueAs<box3f>();
    }

    void InstanceGroup::traverse(RenderContext &ctx, const std::string& operation)
    {
      if (instanced && operation == "render")
      {
        preRender(ctx);
        postRender(ctx);
      }
      else 
        Node::traverse(ctx,operation);
    }

    void InstanceGroup::preCommit(RenderContext &ctx)
    {
      std::cout << __PRETTY_FUNCTION__ << " \"" << name() << "\"\n";
      numGeometry=0;
      if (instanced) {
        // oldWorld = ctx.world;
        // ctx.world = std::static_pointer_cast<sg::World>(shared_from_this());
        instanceDirty=true;

        oldModel = ctx.currentOSPModel;
        oldTransform = ctx.currentTransform;

        updateTransform(ctx);
        cachedTransform=ctx.currentTransform;
        ctx.currentTransform = worldTransform;

        if (ospModel)
          ospRelease(ospModel);
        ospModel = ospNewModel();
        // ospCommit(ospModel);
        setValue((OSPObject)ospModel);
        ctx.currentOSPModel = ospModel;
      }
    }

    void InstanceGroup::postCommit(RenderContext &ctx)
    {
      if (instanced)
      {
        ctx.currentOSPModel = ospModel;

        //instancegroup caches render calls in commit.  
        for (auto child : properties.children)
          child.second->traverse(ctx, "render");

        ospCommit(ospModel);

        ctx.currentOSPModel = oldModel;
        ctx.currentTransform = oldTransform;
      }
      child("bounds").setValue(computeBounds());
    }

    void InstanceGroup::preRender(RenderContext &ctx)
    {
      // std::cout << __PRETTY_FUNCTION__ << " \"" << name() << "\"\n";
      if (instanced) {
      // oldWorld = ctx.world;
        // ctx.world = std::static_pointer_cast<sg::World>(shared_from_this());
        oldModel = ctx.currentOSPModel;
        oldTransform = ctx.currentTransform;
        // ospModel = ospNewModel();
        // ospCommit(ospModel);
        // setValue((OSPObject)ospModel);
        if (cachedTransform != ctx.currentTransform)
          instanceDirty=true;
        if (instanceDirty)
          updateInstance(ctx);
        ctx.currentOSPModel = ospModel;
        ctx.currentTransform = worldTransform;
      }
    }

    void InstanceGroup::postRender(RenderContext &ctx)
    {
      if (instanced)
      {
        if (child("visible").value() == true)
        {
          if (ctx.world->ospModel)
            ospAddGeometry(ctx.world->ospModel,ospInstance);
        }
        ctx.currentOSPModel = oldModel;
        ctx.currentTransform = oldTransform;
      }
    }

    void InstanceGroup::updateTransform(RenderContext &ctx)
    {
        vec3f scale = child("scale").valueAs<vec3f>();
        vec3f rotation = child("rotation").valueAs<vec3f>();
        vec3f translation = child("position").valueAs<vec3f>();
        worldTransform = ctx.currentTransform*baseTransform*ospcommon::affine3f::translate(translation)*
        ospcommon::affine3f::rotate(vec3f(1,0,0),rotation.x)*
        ospcommon::affine3f::rotate(vec3f(0,1,0),rotation.y)*
        ospcommon::affine3f::rotate(vec3f(0,0,1),rotation.z)*
        ospcommon::affine3f::scale(scale);
    }

    void InstanceGroup::updateInstance(RenderContext &ctx)
    {
        std::cout << __PRETTY_FUNCTION__ << std::endl;
        updateTransform(ctx);
        cachedTransform=ctx.currentTransform;

        // ctx.world = oldWorld;
        if (ospInstance)
          ospRelease(ospInstance);

        ospcommon::affine3f test = ospcommon::one;
        ospInstance = ospNewInstance(ospModel,(osp::affine3f&)worldTransform);
        // ospInstance = ospNewInstance(ospModel,(osp::affine3f&)one);
        ospCommit(ospInstance);
        instanceDirty=false;
    }

    OSP_REGISTER_SG_NODE(World);
    OSP_REGISTER_SG_NODE(InstanceGroup);
  }
}
