#pragma once
#include <string>

namespace Umati
{
	namespace Util
	{
		class PublishTopics
		{
		protected:
			static const std::string Prefix_UmatiEmo;
			static const std::string Postfix_DashboardClientOnline;
			static const std::string Postfix_Information;
			static const std::string Postfix_Stacklight;
			static const std::string Postfix_Tools;
			static const std::string Postfix_ProductionPlan;
			static const std::string Postfix_StateMode;
			static const std::string Postfix_Online;
			static const std::string Postfix_JobCurrentStateNumber;

		public:
			PublishTopics(std::string topicPrefix);
			bool isValid();

			const std::string TopicPrefix; // "/ISW/ExampleMachine"
			const std::string ClientOnline; // "/umati/emo/ISW/ExampleMachine/Dashboard_Client_online",
			const std::string Information; //"/umati/emo/ISW/ExampleMachine/Information",
			const std::string Stacklight; //"/umati/emo/ISW/ExampleMachine/Monitoring/Stacklight",
			const std::string Tools; //	"/umati/emo/ISW/ExampleMachine/ToolManagement/Tools",
			const std::string ProductionPlan; // "/umati/emo/ISW/ExampleMachine/ProductionPlan",
			const std::string StateMode; // "/umati/emo/ISW/ExampleMachine/StateMode",
			const std::string Online; // "/umati/emo/ISW/ExampleMachine/Online",
			const std::string JobCurrentStateNumber; // "/umati/emo/ISW/ExampleMachine/ProductionPlan/JobCurrentStateNumber",
		};
	}
}
