#pragma once

#include <map>

#include "server/serverapp.h"
#include "common/singleton.h"
#include "common/timer.h"

namespace KBEngine
{

class CenterDataServer;

class Centermgr: public ServerApp,
				public Singleton<Centermgr>
{
public:
	typedef Components::ComponentInfos APP_INFO;
	// key 不能是 ip，同一 ip 有可能有多个同类 app，可以是 componentID，要求跨服的所有服务器cid 必须唯一
	typedef std::map<COMPONENT_ID, APP_INFO*> APP_INFOS;

	enum TimeOutType
	{
		CENTERMGR_TIMEOUT_TICK = TIMEOUT_SERVERAPP_MAX + 1,	// +1 不会与父类冲突
		CENTERMGR_TIMEOUT_MAX
	};

	Centermgr(Network::EventDispatcher& dispatcher,
		Network::NetworkInterface& ninterface,
		COMPONENT_TYPE componentType,
		COMPONENT_ID componentID);

	~Centermgr();

	virtual bool run();

	void mainProcess();

	virtual bool initializeEnd();
	virtual void finalise();

	virtual void onComponentActiveTickTimeout();

	virtual void onChannelDeregister(Network::Channel * pChannel);

	virtual void handleTimeout(TimerHandle handle, void * arg);

	APP_INFOS const &getConnectedAppInfos();

	APP_INFO const *getAppInfo(COMPONENT_ORDER centerID);

	/* 网络接口
	 * 某个 app 请求注册
	*/
	void onAppRegister(Network::Channel* pChannel, COMPONENT_TYPE componentType, COMPONENT_ID componentID,
		uint32 intaddr, uint16 intport, uint32 extaddr, uint16 extport, std::string& extaddrEx);

	/* 网络接口
	 * 连接的 app 报告活动状态
	 */
	virtual void onAppActiveTick(Network::Channel* pChannel, COMPONENT_TYPE componentType, COMPONENT_ID componentID);

	/* 网络接口
	* KBEngine.centerData 数据改变
	*/
	void onBroadcastCenterDataChanged(Network::Channel* pChannel, KBEngine::MemoryStream& s);

	/* 网络接口
	* 收到跨服call请求, 由某个app上的entityCallCrossServer发起
	*/
	void onEntityCallCrossServer(Network::Channel* pChannel, KBEngine::MemoryStream& s);

	/* 网络接口
	* 跨服登录请求
	*/
	void requestAcrossServer(Network::Channel *pChannel, KBEngine::MemoryStream& s);

	/* 网络接口
	* 请求跨服登录成功
	*/
	void requestAcrossServerSuccess(Network::Channel *pChannel, KBEngine::MemoryStream& s);

private:
	APP_INFOS apps_;

	COMPONENT_ORDER appStartOrder_;	// 组件启动顺序

	TimerHandle	tickTimer_;

	CenterDataServer* centerData_;
};

}	// end namespace KBEngine