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

#include "entitydef/entitycall_cross_server.h"
#include "entitydef/entitycallabstract.h"
#include "entitydef/entitydef.h"
#include "entitydef/remote_entity_method.h"
#include "entitydef/scriptdef_module.h"
#include "network/channel.h"
#include "pyscript/py_macros.h"
#include "server/components.h"

#include "../../server/dbmgr/dbmgr_interface.h"


namespace KBEngine
{
	SCRIPT_METHOD_DECLARE_BEGIN(EntityCallCrossServer)
	SCRIPT_METHOD_DECLARE("__reduce_ex__", reduce_ex__, METH_VARARGS, 0)
	SCRIPT_METHOD_DECLARE_END()

	SCRIPT_MEMBER_DECLARE_BEGIN(EntityCallCrossServer)
	SCRIPT_MEMBER_DECLARE_END()

	SCRIPT_GETSET_DECLARE_BEGIN(EntityCallCrossServer)
	SCRIPT_GETSET_DECLARE_END()
	SCRIPT_INIT(EntityCallCrossServer, 0, 0, 0, 0, 0)

	EntityCallCrossServer::EntityCallCrossServer(COMPONENT_ORDER centerID, EntityCall *entitycall) :
		EntityCall(getScriptType(), entitycall->pScriptDefModule(), NULL, entitycall->componentID(), entitycall->id(), entitycall->type()),
		prototype_(entitycall->type()),
		centerID_(centerID)
	{
		switch (prototype_)
		{
		case ENTITYCALL_TYPE_CELL:
		case ENTITYCALL_TYPE_CELL_VIA_BASE:
			type_ = ENTITYCALL_TYPE_CROSS_SERVER_CELL;
			break;
		case ENTITYCALL_TYPE_BASE:
		case ENTITYCALL_TYPE_BASE_VIA_CELL:
			type_ = ENTITYCALL_TYPE_CROSS_SERVER_BASE;
			break;
		case ENTITYCALL_TYPE_CLIENT:
		case ENTITYCALL_TYPE_CLIENT_VIA_CELL:
		case ENTITYCALL_TYPE_CLIENT_VIA_BASE:
			type_ = ENTITYCALL_TYPE_CROSS_SERVER_CLIENT;
			break;
		default:
			break;
		}
	}

	EntityCallCrossServer::EntityCallCrossServer(ScriptDefModule * pScriptModule, const Network::Address * pAddr, 
		COMPONENT_ID componentID, ENTITY_ID eid, ENTITYCALL_TYPE type, ENTITYCALL_TYPE prototype, COMPONENT_ORDER centerID)
		:EntityCall(getScriptType(), pScriptModule, pAddr, componentID, eid, type),
		prototype_(prototype),
		centerID_(centerID)
	{
	}

	EntityCallCrossServer::~EntityCallCrossServer()
	{}

	PyObject* EntityCallCrossServer::onScriptGetAttribute(PyObject* attr)
	{
		wchar_t* PyUnicode_AsWideCharStringRet0 = PyUnicode_AsWideCharString(attr, NULL);
		char* ccattr = strutil::wchar2char(PyUnicode_AsWideCharStringRet0);
		PyMem_Free(PyUnicode_AsWideCharStringRet0);

		MethodDescription* pMethodDescription = NULL;

		switch (prototype_)
		{
		case ENTITYCALL_TYPE_CELL:
			pMethodDescription = pScriptModule_->findCellMethodDescription(ccattr);
			break;
		case ENTITYCALL_TYPE_BASE:
			pMethodDescription = pScriptModule_->findBaseMethodDescription(ccattr);
			break;
		case ENTITYCALL_TYPE_CLIENT:
			pMethodDescription = pScriptModule_->findClientMethodDescription(ccattr);
			break;
		case ENTITYCALL_TYPE_CELL_VIA_BASE:
			pMethodDescription = pScriptModule_->findCellMethodDescription(ccattr);
			break;
		case ENTITYCALL_TYPE_BASE_VIA_CELL:
			pMethodDescription = pScriptModule_->findBaseMethodDescription(ccattr);
			break;
		case ENTITYCALL_TYPE_CLIENT_VIA_CELL:
			pMethodDescription = pScriptModule_->findClientMethodDescription(ccattr);
			break;
		case ENTITYCALL_TYPE_CLIENT_VIA_BASE:
			pMethodDescription = pScriptModule_->findClientMethodDescription(ccattr);
			break;
		default:
			break;
		};

		if (pMethodDescription != NULL)
		{
			free(ccattr);

			return createRemoteMethod(pMethodDescription);
		}

		free(ccattr);
		return ScriptObject::onScriptGetAttribute(attr);
	}

	void EntityCallCrossServer::c_str(char * s, size_t size)
	{
		const char * entitycallName =
			(type_ == ENTITYCALL_TYPE_CROSS_SERVER_CELL) ? "CellCrossServer" :
			(type_ == ENTITYCALL_TYPE_CROSS_SERVER_BASE) ? "BaseCrossServer" :
			(type_ == ENTITYCALL_TYPE_CROSS_SERVER_CLIENT) ? "ClientCrossServer" : "???";

		kbe_snprintf(s, size, "%s centerid:%d, id:%d, utype:%u, component=%s[%" PRIu64 "].",
			entitycallName, centerID_, id_, utype_,
			COMPONENT_NAME_EX(ENTITYCALL_COMPONENT_TYPE_MAPPING[type_]),
			componentID_);
	}

	Network::Channel * EntityCallCrossServer::getChannel()
	{
		return Components::getSingleton().getDbmgrChannel();
	}

	void EntityCallCrossServer::newCall(Network::Bundle & bundle)
	{
		bundle.newMessage(DbmgrInterface::requestEntityCallCrossServer);
		bundle << centerID_;
		bundle << componentID_;
		bundle << id();
		bundle << prototype_;
	}

	PyObject * EntityCallCrossServer::__unpickle__(PyObject * self, PyObject * args)
	{
		Py_ssize_t size = PyTuple_Size(args);
		if (size != 6)
		{
			ERROR_MSG("EntityCallCrossServer::__unpickle__: args is error! size != 6.\n");
			S_Return;
		}

		ENTITY_ID eid = 0;
		COMPONENT_ID componentID = 0;
		ENTITY_SCRIPT_UID utype = 0;
		int16 type = 0;
		int16 prototype = 0;
		COMPONENT_ORDER centerID = 0;

		if (!PyArg_ParseTuple(args, "iKHhhi", &eid, &componentID, &utype, &type, &prototype, &centerID))
		{
			ERROR_MSG("EntityCallCrossServer::__unpickle__: args is error!\n");
			S_Return;
		}

		ScriptDefModule *sm = EntityDef::findScriptModule(utype);
		if (sm == NULL)
		{
			ERROR_MSG(fmt::format("EntityCallCrossServer::__unpickle__: not found utype {}!\n", utype));
			S_Return;
		}

		return new EntityCallCrossServer(sm, NULL, componentID, eid, (ENTITYCALL_TYPE)type, (ENTITYCALL_TYPE)prototype, centerID);
	}

	void EntityCallCrossServer::onInstallScript(PyObject * mod)
	{
		static PyMethodDef __unpickle__Method =
		{ "EntityCallCrossServer", (PyCFunction)&EntityCallCrossServer::__unpickle__, METH_VARARGS, 0 };

		PyObject* pyFunc = PyCFunction_New(&__unpickle__Method, NULL);
		script::Pickler::registerUnpickleFunc(pyFunc, "EntityCallCrossServer");

		Py_DECREF(pyFunc);
	}

	PyObject * EntityCallCrossServer::__py_reduce_ex__(PyObject * self, PyObject * protocol)
	{
		PyObject* unpickleMethod = script::Pickler::getUnpickleFunc("EntityCallCrossServer");
		if (unpickleMethod == NULL)
			return NULL;

		EntityCallCrossServer* entitycall = static_cast<EntityCallCrossServer*>(self);

		PyObject* args = PyTuple_New(2);
		PyTuple_SET_ITEM(args, 0, unpickleMethod);

		PyObject* args1 = PyTuple_New(6);
		PyTuple_SET_ITEM(args1, 0, PyLong_FromLong(entitycall->id()));
		PyTuple_SET_ITEM(args1, 1, PyLong_FromUnsignedLongLong(entitycall->componentID()));
		PyTuple_SET_ITEM(args1, 2, PyLong_FromUnsignedLong(entitycall->utype()));

		int16 mbType = static_cast<int16>(entitycall->type());
		PyTuple_SET_ITEM(args1, 3, PyLong_FromLong(mbType));

		mbType = static_cast<int16>(entitycall->prototype_);
		PyTuple_SET_ITEM(args1, 4, PyLong_FromLong(mbType));

		PyTuple_SET_ITEM(args1, 5, PyLong_FromLong(entitycall->centerID_));

		PyTuple_SET_ITEM(args, 1, args1);

		return args;
	}


}	// end namespace KBEngine