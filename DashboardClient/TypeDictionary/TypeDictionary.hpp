/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 * 
 * Copyright 2019-2022 (c) Moritz Walker, ISW University of Stuttgart (for umati and VDW e.V.)
 */

#pragma once

#include <stack>
#include <list>
#include <ModelOpcUa/ModelDefinition.hpp>
#include <open62541/client_highlevel.h>


namespace Umati {
    namespace TypeDictionary {

        typedef struct {
            std::string TypeName;
            std::string Name;
            uint64_t Length;
            std::string LengthField;
            bool IsLengthInBytes;
            std::string SwitchField; 
            uint64_t SwitchValue;
            std::string SwitchOperand;
            std::string Terminator;
            bool redundant;
        } Field;

        typedef struct {
            std::string BaseType;
            std::string Name;
            std::vector<Field> Fields;
            std::string Documentation;
            ModelOpcUa::NodeId_t NodeId;
            ModelOpcUa::NodeId_t BinaryNodeId;
            uint64_t Kind;
        } StructuredType;

        typedef struct {
            std::string Name;
            int64_t Value;
        } EnumeratedValue;
        typedef struct {
            std::string Name;
            uint64_t LengthInBits;
            std::vector<EnumeratedValue> EnumeratedValues;
            ModelOpcUa::NodeId_t NodeId;
        } EnumeratedType;

        typedef struct {
            std::vector<StructuredType> StructuredTypes;
            std::vector<EnumeratedType> EnumeratedTypes;
            std::string TargetNamespace;
        } TypeDictionary;

        template <class T>
        class DependecyGraph {

            std::map<T*, std::list<T*>> adj;
            std::map<T*, bool> visited;
            std::stack<T*> resultStack;

            public:
                DependecyGraph() {
                }
                
                void addEdge(T* v, T* w) {
                    adj[v].push_back(w);
                    visited[v] = false;
                    visited[w] = false;
                }
                
                void topologicalSortUtil(T* v, std::stack<T*>& Stack) {
                    visited[v] = true;
                    typename std::list<T*>::iterator i;
                    for (i = adj[v].begin(); i != adj[v].end(); ++i)
                        if (!visited[*i])
                            topologicalSortUtil(*i, Stack);
                    Stack.push(v);
                }
                
                void topologicalSort() {          
                    for (auto const& el: visited) {
                        if (visited[el.first] == false) {
                            topologicalSortUtil(el.first, resultStack);
                        }
                    }
                }

                std::stack<T*>& getResult() {
                    return resultStack;
                }
        };

        std::ostream& operator<<(std::ostream& out, const UA_DataTypeArray* &m_dataTypeArray);
    }
}