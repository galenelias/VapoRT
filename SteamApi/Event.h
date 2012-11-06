#pragma once

#include <map>
//#include <memory>
#include <cassert>
#include <functional>

// These event classes are not thread-safe

typedef unsigned long EventListenerID_t;
const EventListenerID_t c_InvalidListenerID = 0;

template<typename TSender, typename TArg = void>
class IEvent
{
public:
	virtual EventListenerID_t AddListener(std::function<void(TSender*, TArg)> listener) = 0;
	virtual void RemoveListener(EventListenerID_t listenerId) = 0;
};

template<typename TSender>
class IEvent<TSender, void>
{
public:
	virtual EventListenerID_t AddListener(std::function<void(TSender*)> listener) = 0;
	virtual void RemoveListener(EventListenerID_t listenerId) = 0;
};

template<typename TSender, typename TArg = void>
class Event : public IEvent<TSender, TArg>
{
public:
	Event() : m_listenerCounter(1), m_listeners()
	{
	}

	~Event()
	{
		assert(m_listeners.empty());
	}

	EventListenerID_t AddListener(std::function<void(TSender*, TArg)> listener)
	{
		EventListenerID_t listenerId = m_listenerCounter++;
		m_listeners[listenerId] = listener;
		return listenerId;
	}

	void RemoveListener(EventListenerID_t listenerId)
	{
		m_listeners.erase(listenerId);
	}

	void operator()(TSender* pSender, TArg argument)
	{

		std::map<EventListenerID_t, std::function<void(TSender*, TArg)>> listeners(m_listeners);
		for (auto i = listeners.begin(); i != listeners.end(); ++i)
		{
			i->second(pSender, argument);
		}
	}

private:
	EventListenerID_t m_listenerCounter;
	std::map<EventListenerID_t, std::function<void(TSender*, TArg)>> m_listeners;
};

template<typename TSender>
class Event<TSender, void> : public IEvent<TSender, void>
{
public:
	Event() : m_listenerCounter(1), m_listeners()  //Start at 1, so we can use zero as a sentinel event ID
	{
	}

	~Event()
	{
		assert(m_listeners.empty());
	}

	EventListenerID_t AddListener(std::function<void(TSender*)> listener)
	{
		EventListenerID_t listenerId = m_listenerCounter++;
		m_listeners[listenerId] = listener;
		return listenerId;
	}

	void RemoveListener(EventListenerID_t listenerId)
	{
		m_listeners.erase(listenerId);
	}

	void operator()(TSender* pSender)
	{
		std::map<EventListenerID_t, std::function<void(TSender*)>> listeners(m_listeners);
		for (auto i = listeners.begin(); i != listeners.end(); ++i)
		{
			i->second(pSender);
		}
	}

private:
	EventListenerID_t m_listenerCounter;
	std::map<EventListenerID_t, std::function<void(TSender*)>> m_listeners;
};

template<typename TSender, typename TArg = void>
class AutoEventListener
{
public:
	AutoEventListener(IEvent<TSender, TArg> *pEvent, std::function<void(TSender*, TArg)> && listener)
		: m_pEvent(pEvent), m_ListenerID()
	{
		if (m_pEvent)
			m_ListenerID = m_pEvent->AddListener(listener);
	}

	AutoEventListener()
		: m_pEvent(nullptr), m_ListenerID(c_InvalidListenerID)
	{	}

	AutoEventListener & operator=(AutoEventListener && rhs)
	{
		std::swap(m_pEvent, rhs.m_pEvent);
		std::swap(m_ListenerID, rhs.m_ListenerID);
	}

	void AddListener(IEvent<TSender, TArg> *pEvent, std::function<void(TSender*, TArg)> && listener)
	{
		m_pEvent = pEvent;
		if (m_pEvent)
			m_ListenerID = m_pEvent->AddListener(listener);
	}

	void RemoveListener()
	{
		if (m_pEvent && m_ListenerID)
		{
			m_pEvent->RemoveListener(m_ListenerID);
		}
		m_pEvent = nullptr;
		m_ListenerID = c_InvalidListenerID;
	}

	~AutoEventListener()
	{
		if (m_pEvent && m_ListenerID)
			m_pEvent->RemoveListener(m_ListenerID);
	}

private:
	IEvent<TSender, TArg> *m_pEvent;
	EventListenerID_t      m_ListenerID;
};