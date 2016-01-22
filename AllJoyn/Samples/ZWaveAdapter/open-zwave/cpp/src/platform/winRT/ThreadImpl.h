//-----------------------------------------------------------------------------
//
//	ThreadImpl.h
//
//	WinRT implementation of a cross-platform thread
//
//	Copyright (c) 2015 Microsoft Corporation
//	All rights reserved.
//
//	SOFTWARE NOTICE AND LICENSE
//
//	This file is part of OpenZWave.
//
//	OpenZWave is free software: you can redistribute it and/or modify
//	it under the terms of the GNU Lesser General Public License as published
//	by the Free Software Foundation, either version 3 of the License,
//	or (at your option) any later version.
//
//	OpenZWave is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU Lesser General Public License for more details.
//
//	You should have received a copy of the GNU Lesser General Public License
//	along with OpenZWave.  If not, see <http://www.gnu.org/licenses/>.
//
//-----------------------------------------------------------------------------
#ifndef _ThreadImpl_H
#define _ThreadImpl_H

#include <string>
#include <windows.h>
#include "platform/Thread.h"
#include <ppltasks.h>

namespace OpenZWave
{
	class Thread;
	class Event;

	/** \brief Windows-specific implementation of the Thread class.
	 */
	class ThreadImpl
	{
	private:
		friend class Thread;

		ThreadImpl( Thread* _owner, string const& _name );
		~ThreadImpl();

		bool Start( Thread::pfnThreadProc_t _pfnThreadProc, Event* _exitEvent, void* _context );
		void Sleep( uint32 _milliseconds );
		bool Terminate();

		bool IsSignalled();

		void Run();
		static DWORD WINAPI ThreadProc( void* _pArg );

		Thread*					m_owner;
		Event*					m_exitEvent;
		Thread::pfnThreadProc_t	m_pfnThreadProc;
		void*					m_context;
		bool					m_bIsRunning;
		string					m_name;

		static int32			s_threadTerminateTimeout;
	};

} // namespace OpenZWave

#endif //_ThreadImpl_H

