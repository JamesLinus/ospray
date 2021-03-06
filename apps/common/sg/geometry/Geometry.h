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

#pragma once

#include "sg/common/Material.h"

namespace ospray {
  namespace sg {

    struct Geometry : public sg::Renderable
    {
      Geometry(const std::string &type);

      /*! \brief returns a std::string with the c++ name of this class */
      virtual std::string toString() const override;

      /*! geometry type, i.e., 'spheres', 'cylinders', 'trianglemesh', ... */
      const std::string type; 
      
      /*! material for this geometry */
      // std::shared_ptr<Material> material;
    };

    // Inlined member definitions /////////////////////////////////////////////

    inline Geometry::Geometry(const std::string &type)
      : type(type)
    {
      createChild("material", "Material");
      createChild("type", "string");
    }

    inline std::string Geometry::toString() const
    {
      return "ospray::sg::Geometry";
    }
    
  } // ::ospray::sg
} // ::ospray


