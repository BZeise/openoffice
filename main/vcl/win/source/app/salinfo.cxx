/**************************************************************
 * 
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 * 
 *   http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 * 
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#include "svsys.h"
#include "rtl/ustrbuf.hxx"

#include "tools/debug.hxx"
#include "tools/string.hxx"

#include "vcl/window.hxx"

#include "win/salsys.h"
#include "win/salframe.h"
#include "win/salinst.h"
#include "win/saldata.hxx"

#include "svdata.hxx"

#include <hash_map>

SalSystem* WinSalInstance::CreateSalSystem()
{
    return new WinSalSystem();
}

WinSalSystem::~WinSalSystem()
{
}

// -----------------------------------------------------------------------

static BOOL CALLBACK ImplEnumMonitorProc( HMONITOR hMonitor,
                                          HDC hDC,
                                          LPRECT lpRect,
                                          LPARAM dwData )
{
	WinSalSystem* pSys = reinterpret_cast<WinSalSystem*>(dwData);
	return pSys->handleMonitorCallback( reinterpret_cast<sal_IntPtr>(hMonitor),
									    reinterpret_cast<sal_IntPtr>(hDC),
										reinterpret_cast<sal_IntPtr>(lpRect) );
}

sal_Bool WinSalSystem::handleMonitorCallback( sal_IntPtr hMonitor, sal_IntPtr, sal_IntPtr )
{
    MONITORINFOEXW aInfo;
    aInfo.cbSize = sizeof( aInfo );
    if( GetMonitorInfoW( reinterpret_cast<HMONITOR>(hMonitor), &aInfo ) )
    {
        aInfo.szDevice[CCHDEVICENAME-1] = 0;
        rtl::OUString aDeviceName( reinterpret_cast<const sal_Unicode *>(aInfo.szDevice) );
        std::map< rtl::OUString, unsigned int >::const_iterator it =
            m_aDeviceNameToMonitor.find( aDeviceName );
        if( it != m_aDeviceNameToMonitor.end() )
        {
            DisplayMonitor& rMon( m_aMonitors[ it->second ] );
            rMon.m_aArea = Rectangle( Point( aInfo.rcMonitor.left,
                                             aInfo.rcMonitor.top ),
                                      Size( aInfo.rcMonitor.right - aInfo.rcMonitor.left,
                                            aInfo.rcMonitor.bottom - aInfo.rcMonitor.top ) );
            rMon.m_aWorkArea = Rectangle( Point( aInfo.rcWork.left,
                                                 aInfo.rcWork.top ),
                                          Size( aInfo.rcWork.right - aInfo.rcWork.left,
                                                aInfo.rcWork.bottom - aInfo.rcWork.top ) );
            if( (aInfo.dwFlags & MONITORINFOF_PRIMARY) != 0 )
				m_nPrimary = it->second;
        }
    }
    return sal_True;
}

void WinSalSystem::clearMonitors()
{
	m_aMonitors.clear();
	m_nPrimary = 0;
}

bool WinSalSystem::initMonitors()
{
    if( m_aMonitors.size() > 0 )
        return true;
    
    bool winVerOk = true;

    // multi monitor calls not available on Win95/NT
    if ( aSalShlData.maVersionInfo.dwPlatformId == VER_PLATFORM_WIN32_NT )
    {
        if ( aSalShlData.maVersionInfo.dwMajorVersion <= 4 )
            winVerOk = false;	// NT
    }
    else if( aSalShlData.maVersionInfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS )
    {
        if ( aSalShlData.maVersionInfo.dwMajorVersion == 4 && aSalShlData.maVersionInfo.dwMinorVersion == 0 )
            winVerOk = false;	// Win95
    }
    if( winVerOk )
    {
        int nMonitors = GetSystemMetrics( SM_CMONITORS );
        if( nMonitors == 1 )
        {
            int w = GetSystemMetrics( SM_CXSCREEN );
            int h = GetSystemMetrics( SM_CYSCREEN );
            m_aMonitors.push_back( DisplayMonitor( rtl::OUString(),
                                                   rtl::OUString(),
                                                   Rectangle( Point(), Size( w, h ) ),
                                                   Rectangle( Point(), Size( w, h ) ),
                                                   0 ) );
            m_aDeviceNameToMonitor[ rtl::OUString() ] = 0;
            m_nPrimary = 0;
            RECT aWorkRect;
            if( SystemParametersInfo( SPI_GETWORKAREA, 0, &aWorkRect, 0 ) )
                m_aMonitors.back().m_aWorkArea =  Rectangle( aWorkRect.left, aWorkRect.top,
                                                             aWorkRect.right, aWorkRect.bottom );
        }
        else
        {
            DISPLAY_DEVICEW aDev;
            aDev.cb = sizeof( aDev );
            DWORD nDevice = 0;
			std::hash_map< rtl::OUString, int, rtl::OUStringHash > aDeviceStringCount;
            while( EnumDisplayDevicesW( NULL, nDevice++, &aDev, 0 ) )
            {
                if( (aDev.StateFlags & DISPLAY_DEVICE_ACTIVE)
                    && !(aDev.StateFlags & DISPLAY_DEVICE_MIRRORING_DRIVER) ) // sort out non/disabled monitors
                {
                    aDev.DeviceName[31] = 0;
                    aDev.DeviceString[127] = 0;
                    rtl::OUString aDeviceName( reinterpret_cast<const sal_Unicode *>(aDev.DeviceName) );
                    rtl::OUString aDeviceString( reinterpret_cast<const sal_Unicode *>(aDev.DeviceString) );
					if( aDeviceStringCount.find( aDeviceString ) == aDeviceStringCount.end() )
						aDeviceStringCount[ aDeviceString ] = 1;
					else
						aDeviceStringCount[ aDeviceString ]++;
                    m_aDeviceNameToMonitor[ aDeviceName ] = m_aMonitors.size();
                    m_aMonitors.push_back( DisplayMonitor( aDeviceString,
                                                           aDeviceName,
                                                           Rectangle(),
                                                           Rectangle(),
                                                           aDev.StateFlags ) );
                }
            }
            HDC aDesktopRC = GetDC( NULL );
            EnumDisplayMonitors( aDesktopRC, NULL, ImplEnumMonitorProc, reinterpret_cast<LPARAM>(this) );

			// append monitor numbers to name strings
			std::hash_map< rtl::OUString, int, rtl::OUStringHash > aDevCount( aDeviceStringCount );
			unsigned int nMonitors = m_aMonitors.size();
			for( unsigned int i = 0; i < nMonitors; i++ )
			{
				const rtl::OUString& rDev( m_aMonitors[i].m_aName );
				if( aDeviceStringCount[ rDev ] > 1 )
				{
					int nInstance = aDeviceStringCount[ rDev ] - (-- aDevCount[ rDev ] );
					rtl::OUStringBuffer aBuf( rDev.getLength() + 8 );
					aBuf.append( rDev );
					aBuf.appendAscii( " (" );
					aBuf.append( sal_Int32( nInstance ) );
					aBuf.append( sal_Unicode(')') );
					m_aMonitors[ i ].m_aName = aBuf.makeStringAndClear();
				}
			}
        }
    }
    else
    {
        int w = GetSystemMetrics( SM_CXSCREEN );
        int h = GetSystemMetrics( SM_CYSCREEN );
        m_aMonitors.push_back( DisplayMonitor( rtl::OUString(),
                                               rtl::OUString(),
                                               Rectangle( Point(), Size( w, h ) ),
                                               Rectangle( Point(), Size( w, h ) ),
                                               0 ) );
        m_aDeviceNameToMonitor[ rtl::OUString() ] = 0;
        m_nPrimary = 0;
        RECT aWorkRect;
        if( SystemParametersInfo( SPI_GETWORKAREA, 0, &aWorkRect, 0 ) )
            m_aMonitors.back().m_aWorkArea =  Rectangle( aWorkRect.left, aWorkRect.top,
                                                         aWorkRect.right, aWorkRect.bottom );
    }
    
    return m_aMonitors.size() > 0;
}

unsigned int WinSalSystem::GetDisplayScreenCount()
{
    initMonitors();
    return m_aMonitors.size();
}

bool WinSalSystem::IsMultiDisplay()
{
    return false;
}

unsigned int WinSalSystem::GetDefaultDisplayNumber()
{
    initMonitors();
    return m_nPrimary;
}

Rectangle WinSalSystem::GetDisplayScreenPosSizePixel( unsigned int nScreen )
{
    initMonitors();
    return (nScreen < m_aMonitors.size()) ? m_aMonitors[nScreen].m_aArea : Rectangle();
}

Rectangle WinSalSystem::GetDisplayWorkAreaPosSizePixel( unsigned int nScreen )
{
    initMonitors();
    return (nScreen < m_aMonitors.size()) ? m_aMonitors[nScreen].m_aWorkArea : Rectangle();
}

rtl::OUString WinSalSystem::GetScreenName( unsigned int nScreen )
{
	initMonitors();
	return (nScreen < m_aMonitors.size()) ? m_aMonitors[nScreen].m_aName : rtl::OUString();
}

// -----------------------------------------------------------------------
/* We have to map the button identifier to the identifier used by the Win32
   Platform SDK to specify the default button for the MessageBox API.
   The first dimension is the button combination, the second dimension
   is the button identifier.
*/
static int DEFAULT_BTN_MAPPING_TABLE[][8] =
{
    //  Undefined        OK             CANCEL         ABORT          RETRY          IGNORE         YES             NO
    { MB_DEFBUTTON1, MB_DEFBUTTON1, MB_DEFBUTTON1, MB_DEFBUTTON1, MB_DEFBUTTON1, MB_DEFBUTTON1, MB_DEFBUTTON1, MB_DEFBUTTON1 }, //OK
    { MB_DEFBUTTON1, MB_DEFBUTTON1, MB_DEFBUTTON2, MB_DEFBUTTON1, MB_DEFBUTTON1, MB_DEFBUTTON1, MB_DEFBUTTON1, MB_DEFBUTTON1 }, //OK_CANCEL
    { MB_DEFBUTTON1, MB_DEFBUTTON1, MB_DEFBUTTON1, MB_DEFBUTTON1, MB_DEFBUTTON2, MB_DEFBUTTON3, MB_DEFBUTTON1, MB_DEFBUTTON1 }, //ABORT_RETRY_IGNO
    { MB_DEFBUTTON1, MB_DEFBUTTON1, MB_DEFBUTTON3, MB_DEFBUTTON1, MB_DEFBUTTON1, MB_DEFBUTTON1, MB_DEFBUTTON1, MB_DEFBUTTON2 }, //YES_NO_CANCEL
    { MB_DEFBUTTON1, MB_DEFBUTTON1, MB_DEFBUTTON1, MB_DEFBUTTON1, MB_DEFBUTTON1, MB_DEFBUTTON1, MB_DEFBUTTON1, MB_DEFBUTTON2 }, //YES_NO
    { MB_DEFBUTTON1, MB_DEFBUTTON1, MB_DEFBUTTON2, MB_DEFBUTTON1, MB_DEFBUTTON1, MB_DEFBUTTON1, MB_DEFBUTTON1, MB_DEFBUTTON1 }  //RETRY_CANCEL
};

int WinSalSystem::ShowNativeMessageBox(const String& rTitle, const String& rMessage, int nButtonCombination, int nDefaultButton)
{
    DBG_ASSERT( nButtonCombination >= SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_OK &&
                nButtonCombination <= SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_RETRY_CANCEL &&
                nDefaultButton >= SALSYSTEM_SHOWNATIVEMSGBOX_BTN_OK &&
                nDefaultButton <= SALSYSTEM_SHOWNATIVEMSGBOX_BTN_NO, "Invalid arguments!" );

    int nFlags = MB_TASKMODAL | MB_SETFOREGROUND | MB_ICONWARNING | nButtonCombination;

    if (nButtonCombination >= SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_OK &&
        nButtonCombination <= SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_RETRY_CANCEL &&
        nDefaultButton >= SALSYSTEM_SHOWNATIVEMSGBOX_BTN_OK &&
        nDefaultButton <= SALSYSTEM_SHOWNATIVEMSGBOX_BTN_NO)
        nFlags |= DEFAULT_BTN_MAPPING_TABLE[nButtonCombination][nDefaultButton];

    //#107209 hide the splash screen if active
    ImplSVData* pSVData = ImplGetSVData();
    if (pSVData->mpIntroWindow)
        pSVData->mpIntroWindow->Hide();

    return MessageBoxW(
        0,
        reinterpret_cast<LPCWSTR>(rMessage.GetBuffer()),
        reinterpret_cast<LPCWSTR>(rTitle.GetBuffer()),
        nFlags);
}
