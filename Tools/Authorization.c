/*
 *  R.app : a Cocoa front end to: "R A Computer Language for Statistical Data Analysis"
 *  
 *  R.app Copyright notes:
 *                     Copyright (C) 2004  The R Foundation
 *                     written by Stefano M. Iacus and Simon Urbanek
 *
 *                  
 *  R Copyright notes:
 *                     Copyright (C) 1995-1996   Robert Gentleman and Ross Ihaka
 *                     Copyright (C) 1998-2001   The R Development Core Team
 *                     Copyright (C) 2002-2004   The R Foundation
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  A copy of the GNU General Public License is available via WWW at
 *  http://www.gnu.org/copyleft/gpl.html.  You can also obtain it by
 *  writing to the Free Software Foundation, Inc., 59 Temple Place,
 *  Suite 330, Boston, MA  02111-1307  USA.
 */


#include "Authorization.h"

#include <Security/Authorization.h>
#include <Security/AuthorizationTags.h>

AuthorizationRef rootAuthorizationRef=0;

int removeRootAuthorization()
{
	if (rootAuthorizationRef) {
		AuthorizationFree (rootAuthorizationRef, kAuthorizationFlagDefaults);
		rootAuthorizationRef=0;
	}
	return 0;
}

int requestRootAuthorization(int forceFresh)
{
    OSStatus myStatus;
    AuthorizationFlags myFlags = kAuthorizationFlagDefaults;	
	
	if (rootAuthorizationRef) {
		if (!forceFresh)
			return 0;
		removeRootAuthorization();
	}
	
    myStatus = AuthorizationCreate(NULL, kAuthorizationEmptyEnvironment,
                                   myFlags, &rootAuthorizationRef);
    if (myStatus != errAuthorizationSuccess)
        return -1;
    do {
        AuthorizationItem myItems = {kAuthorizationRightExecute, 0, NULL, 0};
        AuthorizationRights myRights = {1, &myItems};
        myFlags = kAuthorizationFlagDefaults | kAuthorizationFlagInteractionAllowed |
            kAuthorizationFlagPreAuthorize | kAuthorizationFlagExtendRights;
        myStatus = AuthorizationCopyRights (rootAuthorizationRef, &myRights, NULL, myFlags, NULL );
		
        if (myStatus != errAuthorizationSuccess) break;
        return 0;
	} while (0);
	AuthorizationFree (rootAuthorizationRef, kAuthorizationFlagDefaults);
	rootAuthorizationRef=0;
	return -1;
}

int runRootScript(const char* script, char** args, FILE **fptr, int keepAuthorized) {
    OSStatus myStatus;
	AuthorizationFlags myFlags = kAuthorizationFlagDefaults;
	
	if (!rootAuthorizationRef && requestRootAuthorization(0)) return -1;
	
	myStatus = AuthorizationExecuteWithPrivileges
		(rootAuthorizationRef, script, myFlags, args, fptr);
	
	if (!keepAuthorized) removeRootAuthorization();
	
	return (myStatus == errAuthorizationSuccess)?0:-1;
}

