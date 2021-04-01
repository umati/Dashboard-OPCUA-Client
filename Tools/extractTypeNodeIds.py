import re
import xml.etree.ElementTree as ET
from string import Template

root = ET.parse('umati.xml').getroot()
namespaces = {'default': 'http://opcfoundation.org/UA/2011/03/UANodeSet.xsd'}

objectTypeList = []

patternNodeId = re.compile("^(?:ns=(\d)+;)?([igs]=.*)$")

lines = []

for objectType in root.findall('default:UAObjectType', namespaces):
    NodeId = objectType.get('NodeId')
    # print("'{}': {}".format(NodeId, objectType.get('BrowseName')))

    DisplayName = objectType.find('default:DisplayName', namespaces).text;
    match = patternNodeId.match(NodeId)
    if not match:
        print("Illegal NodeId: {}".format(NodeId))

    NodeIdIdentifier = match.group(2)

    line = '\t\t\t\tconst ModelOpcUa::NodeId_t {} {{UmatiNamespaceUri, "{}" }};'.format(DisplayName, NodeIdIdentifier)
    lines.append(line)
    print(line)

outputTemplate = Template(open("UmatiTypeNodeIds.hpp.template", "r").read())
d = {'nodeIdDefinitions': "\n".join(lines)}
out = open("UmatiTypeNodeIds.hpp", "w")
out.write(outputTemplate.substitute(d))
