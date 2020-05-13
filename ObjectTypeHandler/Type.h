//
// Created by Dominik on 13.05.2020.
//

#ifndef DASHBOARD_OPCUACLIENT_TYPE_H
#define DASHBOARD_OPCUACLIENT_TYPE_H
#include "ModelOpcUa/ModelInstance.hpp"
#include "ModelOpcUa/ModelDefinition.hpp"

namespace Umati
{
    namespace Dashboard
    {
        namespace TypeDefinition {
            class Type {
            public:
                ModelOpcUa::PlaceholderElement typeInformation;
                std::map<ModelOpcUa::PlaceholderElement, ModelOpcUa::StructureNode> typeMemberVariables;
            };
        }
    }
}


#endif //DASHBOARD_OPCUACLIENT_TYPE_H
