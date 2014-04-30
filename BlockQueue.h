/*
 * Blocking queue based on boost thread library, Version 1.0.0,
 * Copyright (C) 2014, Ren Bin (ayrb13@gmail.com)
 * 
 * This library is free software. Permission to use, copy, modify,
 * and/or distribute this software for any purpose with or without fee
 * is hereby granted, provided that the above copyright notice and
 * this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * a.k.a. as Open BSD license
 * (http://www.openbsd.org/cgi-bin/cvsweb/~checkout~/src/share/misc/license.template)
 *
 * You can get latest version of this library from github
 * (https://github.com/ayrb13/BlockQueue)
 */

#ifndef _BLOCKQUEUE_H_
#define _BLOCKQUEUE_H_

#include <deque>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>
#include <boost/noncopyable.hpp>

template<typename T>
class BlockQueue : boost::noncopyable
{
public:
	typedef T element_type;
	typedef std::deque<element_type> container;
	typedef typename container::size_type size_type;
public:
	//return current size of queue
	size_type size()
	{
		return m_queue.size();
	}
	//pop a element and return it
	element_type pop()
	{
		boost::mutex::scoped_lock sl(m_mx);
		while(size() == 0)
		{
			m_conditionNotEmpty.wait(sl);
		}
		element_type ele = m_queue.front();
		m_queue.pop_front();
		return ele;
	}
	//pop a element and assign it ele
	void pop(element_type& ele)
	{
		boost::mutex::scoped_lock sl(m_mx);
		while(size() == 0)
		{
			m_conditionNotEmpty.wait(sl);
		}
		ele = m_queue.front();
		m_queue.pop_front();
	}
	//try pop a element in "millisecond" time
	//if invoked success, this function will assign it to ele and return true
	//else return false
	bool timed_pop(element_type& ele, long millisecond)
	{
		boost::mutex::scoped_lock sl(m_mx);
		if(size() == 0)
		{
			if(m_conditionNotEmpty.timed_wait(sl,
				boost::posix_time::milliseconds(millisecond)))
			{
					if(size() > 0)
					{
						ele = m_queue.front();
						m_queue.pop_front();
						return true;
					}
					else
					{
						return false;
					}
			}
			else
			{
				return false;
			}
		}
		else
		{
			ele = m_queue.front();
			m_queue.pop_front();
			return true;
		}
	}
	//push a element in queue
	void push(const element_type& ele)
	{
		boost::mutex::scoped_lock sl(m_mx);
		m_queue.push_back(ele);
		m_conditionNotEmpty.notify_one();
	}
	//clear the queue
	void clear()
	{
		boost::mutex::scoped_lock sl(m_mx);
		m_queue.clear();
		m_conditionNotEmpty.notify_one();
	}
private:
	container m_queue;
	boost::condition m_conditionNotEmpty;
	boost::mutex m_mx;
};

#endif//_BLOCKQUEUE_H_
