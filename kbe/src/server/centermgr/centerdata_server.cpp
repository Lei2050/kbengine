/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2020 KBEngine.

KBEngine is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

KBEngine is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with KBEngine.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "centermgr/centerdata_server.h"

#include "network/channel.h"
#include "helper/debug_helper.h"
#include "common/format.h"
#include "centermgr/centermgr.h"
#include "dbmgr/dbmgr_interface.h"

namespace KBEngine {

CenterDataServer::CenterDataServer(DATA_TYPE dataType) :
	GlobalDataServer(dataType)
{
}

void CenterDataServer::broadcastDataChanged(Network::Channel* pChannel, COMPONENT_TYPE componentType, const std::string& key,
	const std::string& value, bool isDelete)
{
	INFO_MSG(fmt::format("CenterDataServer::broadcastDataChanged: writer({0}, addr={4}), keySize={1}, valSize={2}, isDelete={3}\n",
		COMPONENT_NAME_EX(componentType), key.size(), value.size(), (int)isDelete, pChannel->c_str()));

	const Centermgr::APP_INFOS &apps = Centermgr::getSingleton().getConnectedAppInfos();
	Centermgr::APP_INFOS::const_iterator iter = apps.begin();
	for (; iter != apps.end(); iter++)
	{
		Network::Channel * appChannel = iter->second->pChannel;
		if (appChannel == pChannel)
			continue;

		Network::Bundle *pBundle = Network::Bundle::createPoolObject(OBJECTPOOL_POINT);
		(*pBundle).newMessage(DbmgrInterface::onBroadcastCenterDataChanged);
		
		(*pBundle) << isDelete;
		ArraySize len = key.length();
		(*pBundle) << len;
		(*pBundle).assign(key.data(), len);
		if (!isDelete)
		{
			len = value.length();
			(*pBundle) << len;
			(*pBundle).assign(value.data(), len);
		}
		(*pBundle) << componentType;

		appChannel->send(pBundle);
	}
}

void CenterDataServer::onGlobalDataClientLogon(Network::Channel * client, COMPONENT_TYPE componentType)
{
	bool isDelete = false;
	
	DATA_MAP_KEY iter = dict_.begin();
	for (; iter != dict_.end(); iter++)
	{
		Network::Bundle* pBundle = Network::Bundle::createPoolObject(OBJECTPOOL_POINT);
		(*pBundle).newMessage(DbmgrInterface::onBroadcastCenterDataChanged);

		(*pBundle) << isDelete;
		ArraySize len = iter->first.length();
		(*pBundle) << len;
		(*pBundle).assign(iter->first.data(), len);

		len = iter->second.length();
		(*pBundle) << len;
		(*pBundle).assign(iter->second.data(), len);

		(*pBundle) << componentType;

		client->send(pBundle);
	}
}

}	// end namespace KBEngine