
#if defined(DEFINE_IN_INTERFACE)
#undef CENTERMGR_INTERFACE_MACRO_H
#endif	// DEFINE_IN_INTERFACE

#ifndef CENTERMGR_INTERFACE_MACRO_H
#define CENTERMGR_INTERFACE_MACRO_H
#endif // CENTERMGR_INTERFACE_MACRO_H

#include "network/interface_defs.h"

namespace KBEngine {

#if defined(NETWORK_INTERFACE_DECLARE_BEGIN)
#undef CENTERMGR_MESSAGE_HANDLER_STREAM
#endif

#if defined(DEFINE_IN_INTERFACE)
#if defined(CENTERMGR)
#define CENTERMGR_MESSAGE_HANDLER_STREAM(NAME)										\
	void NAME##CentermgrMessagehandler_stream::handle(Network::Channel* pChannel,	\
												KBEngine::MemoryStream& s)		\
	{																			\
			KBEngine::Centermgr::getSingleton().NAME(pChannel, s);					\
	}																			\

#else
#define CENTERMGR_MESSAGE_HANDLER_STREAM(NAME)										\
	void NAME##CentermgrMessagehandler_stream::handle(Network::Channel* pChannel,	\
												KBEngine::MemoryStream& s)		\
	{																			\
	}																			\

#endif
#else
#define CENTERMGR_MESSAGE_HANDLER_STREAM(NAME)										\
	class NAME##CentermgrMessagehandler_stream : public Network::MessageHandler		\
	{																			\
	public:																		\
		virtual void handle(Network::Channel* pChannel,							\
												KBEngine::MemoryStream& s);		\
	};																			\

#endif

#define CENTERMGR_MESSAGE_DECLARE_STREAM(NAME, MSG_LENGTH)							\
	CENTERMGR_MESSAGE_HANDLER_STREAM(NAME)											\
	NETWORK_MESSAGE_DECLARE_STREAM(Centermgr, NAME,									\
				NAME##CentermgrMessagehandler_stream, MSG_LENGTH)					\
																				\


	/**
	Centermgr消息宏，  只有 1 个参数的消息
	*/
#if defined(NETWORK_INTERFACE_DECLARE_BEGIN)
#undef CENTERMGR_MESSAGE_HANDLER_ARGS1
#endif

#if defined(DEFINE_IN_INTERFACE)
#if defined(CENTERMGR)
#define CENTERMGR_MESSAGE_HANDLER_ARGS1(NAME, ARG_TYPE1, ARG_NAME1)					\
	void NAME##CentermgrMessagehandler1::handle(Network::Channel* pChannel,			\
												KBEngine::MemoryStream& s)		\
	{																			\
			ARG_TYPE1 ARG_NAME1;												\
			s >> ARG_NAME1;														\
			KBEngine::Centermgr::getSingleton().NAME(pChannel, ARG_NAME1);		\
	}																			\

#else
#define CENTERMGR_MESSAGE_HANDLER_ARGS1(NAME, ARG_TYPE1, ARG_NAME1)				\
	void NAME##CentermgrMessagehandler1::handle(Network::Channel* pChannel,			\
												KBEngine::MemoryStream& s)		\
	{																			\
	}																			\

#endif
#else
#define CENTERMGR_MESSAGE_HANDLER_ARGS1(NAME, ARG_TYPE1, ARG_NAME1)				\
	class NAME##CentermgrMessagehandler1 : public Network::MessageHandler		\
	{																			\
	public:																		\
		virtual void handle(Network::Channel* pChannel,							\
							KBEngine::MemoryStream& s);							\
	};																			\

#endif

#define CENTERMGR_MESSAGE_DECLARE_ARGS1(NAME, MSG_LENGTH, ARG_TYPE1, ARG_NAME1)		\
	CENTERMGR_MESSAGE_HANDLER_ARGS1(NAME, ARG_TYPE1, ARG_NAME1) 					\
	NETWORK_MESSAGE_DECLARE_ARGS1(Centermgr, NAME,									\
				NAME##CentermgrMessagehandler2, MSG_LENGTH, ARG_TYPE1, ARG_NAME1)	\

/**
	Centermgr消息宏，  只有 2 个参数的消息
*/
#if defined(NETWORK_INTERFACE_DECLARE_BEGIN)
	#undef CENTERMGR_MESSAGE_HANDLER_ARGS2
#endif

#if defined(DEFINE_IN_INTERFACE)
#if defined(CENTERMGR)
#define CENTERMGR_MESSAGE_HANDLER_ARGS2(NAME, ARG_TYPE1, ARG_NAME1,					\
											ARG_TYPE2, ARG_NAME2)				\
	void NAME##CentermgrMessagehandler2::handle(Network::Channel* pChannel,			\
												KBEngine::MemoryStream& s)		\
	{																			\
			ARG_TYPE1 ARG_NAME1;												\
			s >> ARG_NAME1;														\
			ARG_TYPE2 ARG_NAME2;												\
			s >> ARG_NAME2;														\
			KBEngine::Centermgr::getSingleton().NAME(pChannel,						\
													ARG_NAME1, ARG_NAME2);		\
	}																			\

#else
#define CENTERMGR_MESSAGE_HANDLER_ARGS2(NAME, ARG_TYPE1, ARG_NAME1,					\
											ARG_TYPE2, ARG_NAME2)				\
	void NAME##CentermgrMessagehandler2::handle(Network::Channel* pChannel,			\
												KBEngine::MemoryStream& s)		\
	{																			\
	}																			\

#endif
#else
#define CENTERMGR_MESSAGE_HANDLER_ARGS2(NAME, ARG_TYPE1, ARG_NAME1,					\
											ARG_TYPE2, ARG_NAME2)				\
	class NAME##CentermgrMessagehandler2 : public Network::MessageHandler			\
	{																			\
	public:																		\
		virtual void handle(Network::Channel* pChannel,							\
							KBEngine::MemoryStream& s);							\
	};																			\

#endif

#define CENTERMGR_MESSAGE_DECLARE_ARGS2(NAME, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,		\
											ARG_TYPE2, ARG_NAME2)				\
	CENTERMGR_MESSAGE_HANDLER_ARGS2(NAME, ARG_TYPE1, ARG_NAME1, 					\
											ARG_TYPE2, ARG_NAME2)				\
	NETWORK_MESSAGE_DECLARE_ARGS2(Centermgr, NAME,									\
				NAME##CentermgrMessagehandler2, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,	\
											ARG_TYPE2, ARG_NAME2)				\


/**
Centermgr消息宏， 只有 7 个参数的消息
*/
#if defined(NETWORK_INTERFACE_DECLARE_BEGIN)
#undef CENTERMGR_MESSAGE_HANDLER_ARGS7
#endif

#if defined(DEFINE_IN_INTERFACE)
#if defined(CENTERMGR)
#define CENTERMGR_MESSAGE_HANDLER_ARGS7(NAME, ARG_TYPE1, ARG_NAME1,				\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7)				\
	void NAME##CentermgrMessagehandler7::handle(Network::Channel* pChannel,		\
												KBEngine::MemoryStream& s)		\
		{																		\
			ARG_TYPE1 ARG_NAME1;												\
			s >> ARG_NAME1;														\
			ARG_TYPE2 ARG_NAME2;												\
			s >> ARG_NAME2;														\
			ARG_TYPE3 ARG_NAME3;												\
			s >> ARG_NAME3;														\
			ARG_TYPE4 ARG_NAME4;												\
			s >> ARG_NAME4;														\
			ARG_TYPE5 ARG_NAME5;												\
			s >> ARG_NAME5;														\
			ARG_TYPE6 ARG_NAME6;												\
			s >> ARG_NAME6;														\
			ARG_TYPE7 ARG_NAME7;												\
			s >> ARG_NAME7;														\
			KBEngine::Centermgr::getSingleton().NAME(pChannel,					\
										ARG_NAME1, ARG_NAME2, ARG_NAME3, 		\
										ARG_NAME4, ARG_NAME5, ARG_NAME6,		\
										ARG_NAME7);								\
		}																		\

#else
#define CENTERMGR_MESSAGE_HANDLER_ARGS7(NAME, ARG_TYPE1, ARG_NAME1,				\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7)				\
	void NAME##CentermgrMessagehandler7::handle(Network::Channel* pChannel,		\
												KBEngine::MemoryStream& s)		\
		{																		\
		}																		\

#endif
#else
#define CENTERMGR_MESSAGE_HANDLER_ARGS7(NAME, ARG_TYPE1, ARG_NAME1,				\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7)				\
	class NAME##CentermgrMessagehandler7 : public Network::MessageHandler		\
	{																			\
	public:																		\
		virtual void handle(Network::Channel* pChannel,							\
							KBEngine::MemoryStream& s);							\
	};																			\

#endif

#define CENTERMGR_MESSAGE_DECLARE_ARGS7(NAME, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,	\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7)				\
	CENTERMGR_MESSAGE_HANDLER_ARGS7(NAME, ARG_TYPE1, ARG_NAME1, 				\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7)				\
	NETWORK_MESSAGE_DECLARE_ARGS7(Centermgr, NAME,								\
				NAME##CentermgrMessagehandler7, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7)				\

}	// end namespace KBEngine