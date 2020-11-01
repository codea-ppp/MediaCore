#ifndef MEDIACORE_MESSAGE_FORMAT_H_
#define MEDIACORE_MESSAGE_FORMAT_H_

#include "DetailMessageFormat/MessageType.h"
#include "DetailMessageFormat/StreamMessage.h"
#include "DetailMessageFormat/KeepAliveMessage.h"
#include "DetailMessageFormat/StopStreamMessage.h"
#include "DetailMessageFormat/ResourceServerReport.h"
#include "DetailMessageFormat/PullMediaMenuMessage.h"
#include "DetailMessageFormat/PushMediaMenuMessage.h"
#include "DetailMessageFormat/PushLoadBalanceMessage.h"
#include "DetailMessageFormat/ClientStreamTriggerMessage.h"
#include "DetailMessageFormat/PullOtherLoadBalanceMessage.h"
#include "DetailMessageFormat/ClientPullMediaStreamMessage.h"
#include "DetailMessageFormat/RespondMediaMenuPullingMessage.h"
#include "DetailMessageFormat/RespondLoadBalancePullingMessage.h"
#include "DetailMessageFormat/LoadBalancePullMediaStreamMessage.h"
#include "DetailMessageFormat/LoadBalanceRespondMediaPullingMessage.h"
#include "DetailMessageFormat/ResourceServerRespondMediaPullingMessage.h"

namespace MediaCoreMessageFormat
{
	template <typename Message>
	int SetHead(Message* m, uint32_t type, uint32_t length, uint32_t tid)
	{
		if (m == nullptr)
		{
			dzlog_error("message == nullptr");
			return -1;
		}

		m->type		= type;
		m->length	= length;
		m->tid		= tid;

		return 0;
	}

	template <typename Message> 
	int SetTransactionId(Message* m, uint32_t tid) 
	{
		if (m == nullptr)
		{
			dzlog_error("message == nullptr");
			return -1;
		}

		m->tid = tid;
		return 0;
	}
}

#endif 
