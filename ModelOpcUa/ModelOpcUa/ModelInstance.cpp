
#include "ModelInstance.hpp"

namespace ModelOpcUa {

	void PlaceholderNode::addInstance(PlaceholderElement instance)
	{
		this->Instances.push_back(instance);
	}

	std::list<PlaceholderElement> PlaceholderNode::getInstances()
	{
		return this->Instances;
	}
}
