/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 * 
 * Copyright 2019-2022 (c) Moritz Walker, ISW University of Stuttgart (for umati and VDW e.V.)
 */

#include "TypeDictionary.hpp"

std::ostream& operator<<(std::ostream& out, const UA_DataTypeArray* &m_dataTypeArray) {
    out << "\n\n---------------------\n\n";
    for (size_t i = 0; i < m_dataTypeArray->typesSize; i++) {
        auto &type = m_dataTypeArray->types[i];
        out << "{\n";
        out << "   UA_TYPENAME(" << type.typeName << "), /* .typeName */\n";
        out << "   {" << type.typeId.namespaceIndex << ", " << type.typeId.identifierType << "{" << type.typeId.identifier.numeric << "}}, /* .typeId */\n";
        out << "   {" << type.binaryEncodingId.namespaceIndex << ", " << type.binaryEncodingId.identifierType << "{" << type.binaryEncodingId.identifier.numeric << "}}, /* .binaryEncodingId */\n";
        out << "   " << type.memSize << ", /* .memSize */\n";
        out << "   " << type.typeKind << ", /* .typeKind */\n";
        out << "   " << type.pointerFree << ", /* .pointerFree */\n";
        out << "   " << type.overlayable << ", /* .overlayable */\n";
        out << "   " << type.membersSize << ", /* .membersSize */\n";
        out << "   " << type.typeName << "_members /* .members */\n";
        out << "}\n\n";
    }

    for (size_t i = 0; i < m_dataTypeArray->typesSize; i++) {
        auto &type = m_dataTypeArray->types[i];
        out << "/* " << type.typeName << "*/\n";
        out << "static UA_DataTypeMember " << type.typeName << "_members[" << type.membersSize << "] = {\n";

        for (size_t j = 0; j < type.membersSize; j++) {
            auto &member = type.members[j];
            out << "{\n";
            out << "  "
                        << "UA_TYPENAME(" << member.memberName << "), /* .memberName */\n";
            out << "  "
                        << "&" << member.memberType->typeName << ", /* .memberType */\n";
            out << "  " << unsigned(member.padding) << ", /* .padding */\n";
            out << "  " << bool(member.isArray) << ", /* .isArray */\n";
            out << "  " << bool(member.isOptional) << ", /* .isOptional */\n";
            out << "},\n";
        }
        out << "};\n\n";
    }
    return out;
}