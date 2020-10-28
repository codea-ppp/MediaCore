#ifndef PUSH_LOADBALANCE_MESSAGE_H_
#define PUSH_LOADBALANCE_MESSAGE_H_

#include <vector>
#include <zlog.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "RespondMediaMenuPullingMessage.h"

namespace MediaCoreMessageFormat
{
	// 推送负载均衡的信令和响应拉取负载均衡的信令只有 type 是不同的
	// 但是 PushMediaMenuMessage 是用于推送增量的，RespondMediaMenuPullMessage 是相应拉取信令的全量推送
	typedef RespondMediaMenuPullMessage PushMediaMenuMessage;

	// 所以只有初始化函数重新定义了
	void InitPushLoadBalanceMessage(PushMediaMenuMessage* message)
	{
		message->type	= 0x22;
		message->length = 0x00;
		message->tid	= 0x00;
		message->ip		= nullptr;
		message->port	= nullptr;
	}

	// 这里是在创建函数的别名, 相当于类型的 typedef
	// SetMediaMetaData 不需要创建别名
	const auto& SendPushMediaMenuMessage	= SendRespondMediaMenuPullMessage;
	const auto& ClearPushMediaMenuMessage	= ClearRespondMediaMenuPullMessage;
}

#endif 
