#include "PublishTopics.hpp"


namespace Umati
{
	namespace Util
	{
		const std::string PublishTopics::Prefix_UmatiEmo = std::string("/umati/emo");
		const std::string PublishTopics::Postfix_DashboardClientOnline = std::string("/Dashboard_Client_online");
		const std::string PublishTopics::Postfix_Information = std::string("/Information");
		const std::string PublishTopics::Postfix_Stacklight = std::string("/Monitoring/Stacklight");
		const std::string PublishTopics::Postfix_Tools = std::string("/ToolManagement/Tools");
		const std::string PublishTopics::Postfix_ProductionPlan = std::string("/ProductionPlan");
		const std::string PublishTopics::Postfix_StateMode = std::string("/StateMode");
		const std::string PublishTopics::Postfix_JobCurrentStateNumber = std::string("/ProductionPlan/JobCurrentStateNumber");
		const std::string PublishTopics::Postfix_Online = std::string("/Online");


		PublishTopics::PublishTopics(std::string topicPrefix)
			: TopicPrefix(topicPrefix), 
			ClientOnline(Prefix_UmatiEmo + topicPrefix + Postfix_DashboardClientOnline),
			Information(Prefix_UmatiEmo + topicPrefix + Postfix_Information),
			Stacklight(Prefix_UmatiEmo + topicPrefix + Postfix_Stacklight),
			Tools(Prefix_UmatiEmo + topicPrefix + Postfix_Tools),
			ProductionPlan(Prefix_UmatiEmo + topicPrefix + Postfix_ProductionPlan),
			StateMode(Prefix_UmatiEmo + topicPrefix + Postfix_StateMode),
			Online(Prefix_UmatiEmo + topicPrefix + Postfix_Online),
			JobCurrentStateNumber(Prefix_UmatiEmo + topicPrefix + Postfix_JobCurrentStateNumber)
		{
		}

		bool PublishTopics::isValid()
		{
			return TopicPrefix.find('<') == std::string::npos;
		}
	}
}
