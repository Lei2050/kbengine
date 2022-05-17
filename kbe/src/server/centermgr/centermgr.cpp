
#include "centermgr/centermgr.h"
#include "centermgr/centermgr_interface.h"
#include "centermgr/centerdata_server.h"
#include "network/message_handler.h"
#include "dbmgr/dbmgr_interface.h"

#include "helper/debug_helper.h"

namespace KBEngine 
{

	ServerConfig g_serverConfig;
	KBE_SINGLETON_INIT(Centermgr);

	Centermgr::Centermgr(Network::EventDispatcher& dispatcher,
		Network::NetworkInterface& ninterface,
		COMPONENT_TYPE componentType,
		COMPONENT_ID componentID):
		ServerApp(dispatcher, ninterface, componentType, componentID),
		apps_(),
		appStartOrder_(0),
		tickTimer_(),
		centerData_(NULL)
	{
		KBEngine::Network::MessageHandlers::pMainMessageHandlers = &CentermgrInterface::messageHandlers;
	}

	Centermgr::~Centermgr()
	{
		for (APP_INFOS::iterator iter = apps_.begin(); iter != apps_.end(); ++iter)
		{
			SAFE_RELEASE(iter->second);
		}
		apps_.clear();

		tickTimer_.cancel();
	}

	bool Centermgr::run()
	{
		return ServerApp::run();
	}

	void Centermgr::mainProcess()
	{
		networkInterface_.processChannels(KBEngine::Network::MessageHandlers::pMainMessageHandlers);
	}

	bool Centermgr::initializeEnd()
	{
		centerData_ = new CenterDataServer(GlobalDataServer::CENTER_DATA);
		centerData_->addConcernComponentType(DBMGR_TYPE);

		tickTimer_ = this->dispatcher().addTimer(1000000 / g_kbeSrvConfig.gameUpdateHertz(), this,
			reinterpret_cast<void *>(CENTERMGR_TIMEOUT_TICK));

		return true;
	}

	void Centermgr::finalise()
	{
		SAFE_RELEASE(centerData_);
	}

	void Centermgr::handleTimeout(TimerHandle handle, void * arg)
	{
		switch (reinterpret_cast<uintptr>(arg))
		{
		case CENTERMGR_TIMEOUT_TICK:
			mainProcess();
			return;
		default:
			break;
		}

		ServerApp::handleTimeout(handle, arg);
	}

	void Centermgr::onComponentActiveTickTimeout()
	{
		for (APP_INFOS::iterator iter = apps_.begin(); iter != apps_.end(); iter++)
		{
			if (iter->second->pChannel == NULL)
				continue;

			Network::Bundle* pBundle = Network::Bundle::createPoolObject(OBJECTPOOL_POINT);
			pBundle->newMessage(DbmgrInterface::onAppActiveTick);
			(*pBundle) << g_componentType;
			(*pBundle) << g_componentID;
			iter->second->pChannel->send(pBundle);
			INFO_MSG(fmt::format("Centermgr::onComponentActiveTickTimeout: app tick at time {}\n", timestamp()));
		}
	}

	void Centermgr::onChannelDeregister(Network::Channel * pChannel)
	{
		for (APP_INFOS::iterator iter = apps_.begin(); iter != apps_.end(); iter++)
		{
			if (iter->second->pChannel == pChannel)
			{
				SAFE_RELEASE(iter->second);
				apps_.erase(iter->first);
				return;
			}
		}

		ServerApp::onChannelDeregister(pChannel);
	}

	void Centermgr::onAppRegister(Network::Channel* pChannel, COMPONENT_TYPE componentType, COMPONENT_ID componentID, 
		uint32 intaddr, uint16 intport, uint32 extaddr, uint16 extport, std::string& extaddrEx)
	{
		// TODO: 检查连接进来的ip是否合法(如在配置表中）

		INFO_MSG(fmt::format("Centermgr::onAppRegister: componentType:{0}, "
			"componentID:{1}, intaddr:{2}, intport:{3}, extaddr:{4}, extport:{5},  from {6}.\n",
			COMPONENT_NAME_EX((COMPONENT_TYPE)componentType),
			componentID,
			inet_ntoa((struct in_addr&)intaddr),
			ntohs(intport),
			(extaddr != 0 ? inet_ntoa((struct in_addr&)extaddr) : "nonsupport"),
			ntohs(extport),
			pChannel->c_str()));

		APP_INFO *appInfo = new APP_INFO;
		appInfo->componentType = componentType;

		appInfo->pChannel = pChannel;
		// shufeng：注意这里 channel 为 EXTERNAL ，但调整连接检测间隔为 INTERNAL，以便和所连接的 app 保持一致
		appInfo->pChannel->startInactivityDetection(Network::g_channelInternalTimeout, Network::g_channelInternalTimeout  / 2.f);
		
		appInfo->cid = componentID;
		appInfo->pIntAddr.reset(new Network::Address(intaddr, intport));	// pIntAddr 的类型是 KBEShared_ptr
		appInfo->pExtAddr.reset(new Network::Address(extaddr, extport));
		if (extaddrEx.size() > 0)
			strncpy(appInfo->externalAddressEx, extaddrEx.c_str(), MAX_NAME);

		appStartOrder_ += 1;
		appInfo->globalOrderid = appStartOrder_;

		apps_.insert(std::pair<COMPONENT_ID, APP_INFO*>(componentID, appInfo));

		Network::Bundle* pBundle = Network::Bundle::createPoolObject(OBJECTPOOL_POINT);
		pBundle->newMessage(DbmgrInterface::onRegisterCentermgr);
		(*pBundle) << appStartOrder_;
		pChannel->send(pBundle);

		centerData_->onGlobalDataClientLogon(pChannel, CENTERMGR_TYPE);
	}

	void Centermgr::onAppActiveTick(Network::Channel* pChannel, COMPONENT_TYPE componentType, COMPONENT_ID componentID)
	{
		INFO_MSG(fmt::format("Centermgr::onAppActiveTick: app({}:{}) tick at time {}\n", COMPONENT_NAME_EX(componentType), pChannel->c_str(), timestamp()));
		APP_INFOS::iterator iter = apps_.find(componentID);
		if (iter != apps_.end())
		{
			if(iter->second->pChannel != NULL)
				iter->second->pChannel->updateLastReceivedTime();
		}
		else
		{
			ERROR_MSG(fmt::format("Centermgr::onAppActiveTick: cant find app by componentID({})\n", componentID));
		}
	}

	void Centermgr::onBroadcastCenterDataChanged(Network::Channel* pChannel, KBEngine::MemoryStream& s)
	{
		DEBUG_MSG(fmt::format("Centermgr::onBroadcastCenterDataChanged from {}\n", pChannel->c_str()));

		uint8 dataType;
		std::string key, value;
		bool isDelete;
		COMPONENT_TYPE componentType;

		s >> dataType;
		s >> isDelete;
		s.readBlob(key);
		if (!isDelete)
		{
			s.readBlob(value);
		}
		s >> componentType;

		if (isDelete)
		{
			centerData_->del(pChannel, componentType, key);
		}
		else
		{
			centerData_->write(pChannel, componentType, key, value);
		}
	}

	void Centermgr::onEntityCallCrossServer(Network::Channel * pChannel, KBEngine::MemoryStream & s)
	{
		// 如果做如下判断，需要明确CenterMgr也是内部组件
		//if (pChannel->isExternal())
		//	return;

		COMPONENT_ORDER centerID;
		s >> centerID;

		const APP_INFO *appinfo = getAppInfo(centerID);
		if (appinfo != NULL)
		{
			Network::Bundle* bundle = Network::Bundle::createPoolObject(OBJECTPOOL_POINT);
			bundle->newMessage(DbmgrInterface::onEntityCallCrossServer);
			bundle->append(s);
			appinfo->pChannel->send(bundle);
		}
		else
		{
			ERROR_MSG(fmt::format("Centermgr::onEntityCallCrossServer: cannot find server:centerID = {}\n", centerID));
		}

		s.done();
	}

	void Centermgr::requestAcrossServer(Network::Channel * pChannel, KBEngine::MemoryStream & s)
	{
		DEBUG_MSG("Centermgr::requestAcrossServer->>>");
		// 如果做如下判断，需要明确CenterMgr也是内部组件
		//if (pChannel->isExternal())
		//	return;

		COMPONENT_ORDER dstCenterID;
		s >> dstCenterID;

		const Centermgr::APP_INFO *dstApp = getAppInfo(dstCenterID);
		if (dstApp == NULL)
		{
			// acrossEntityCall->centerID() << acrossEntityCall->componentID() << g_centerID << g_componentID << id() << accountName << dbip << dbName;
			COMPONENT_ID cid;
			COMPONENT_ORDER srcCenterID;
			std::string accountName;
			s >> cid >> srcCenterID >> cid >> accountName;
			ERROR_MSG(fmt::format("Centermgr::onEntityCallCrossServer: cannot find server:centerID = {} for account({})\n", dstCenterID, accountName));

			// TODO: 跨服失败回调，使用者发起请需求需要得知结果以便下一步的行为，如果没有就不知道明确的失败时间点。可以不用，没有结果返回就是失败？
		}
		else
		{
			Network::Bundle *bundle = Network::Bundle::createPoolObject(OBJECTPOOL_POINT);
			bundle->newMessage(DbmgrInterface::receiveAcrossServerRequest);
			bundle->append(s);
			dstApp->pChannel->send(bundle);
		}

		s.done();
	}

	void Centermgr::requestAcrossServerSuccess(Network::Channel * pChannel, KBEngine::MemoryStream & s)
	{
		DEBUG_MSG("Centermgr::requestAcrossServerSuccess->>>");
		COMPONENT_ORDER centerID;
		s >> centerID;

		const APP_INFO *appinfo = getAppInfo(centerID);
		if (appinfo != NULL)
		{
			Network::Bundle* bundle = Network::Bundle::createPoolObject(OBJECTPOOL_POINT);
			bundle->newMessage(DbmgrInterface::receiveAcrossServerSuccess);
			bundle->append(s);
			appinfo->pChannel->send(bundle);
		}
		else
		{
			ERROR_MSG(fmt::format("Centermgr::requestAcrossServerSuccess: cannot find server:centerID = {}\n", centerID));
		}

		s.done();		
	}

	Centermgr::APP_INFOS const &Centermgr::getConnectedAppInfos()
	{
		return apps_;
	}

	Centermgr::APP_INFO const *Centermgr::getAppInfo(COMPONENT_ORDER centerID)
	{
		APP_INFOS::iterator iter = apps_.begin();
		for (; iter != apps_.end(); iter++)
		{
			if (iter->second->globalOrderid == centerID)
				return iter->second;
		}

		return NULL;
	}
}

