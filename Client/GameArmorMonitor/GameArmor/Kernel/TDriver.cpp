#include "TDriver.h"

#include <stdio.h>

TDriver::TDriver(void)
{
	driverHandle = NULL;
	   
	removable = TRUE;

	driverName = NULL;
	driverPath = NULL;
	driverDosName = NULL;

	initialized = FALSE;
	loaded = FALSE;
	started = FALSE;
}

TDriver::~TDriver(void)
{
	if(driverHandle != NULL)
	{
		CloseHandle(driverHandle); 
		driverHandle = NULL; 
	}
   
    UnloadDriver();
}

void TDriver::SetRemovable(BOOL value)
{
	removable = value;
}

BOOL TDriver::IsInitialized(void)
{
	return initialized;
}

BOOL TDriver::IsLoaded(void)
{
	return loaded;
}

BOOL TDriver::IsStarted(void)
{
	return started;
}

DWORD TDriver::InitDriver(LPCTSTR path)
{
	if(initialized)
	{
		if(UnloadDriver() != DRV_SUCCESS)
			return DRV_ERROR_ALREADY_INITIALIZED;
	}

	driverPath = (LPTSTR)malloc(strlen(path) + 1);

	if(driverPath == NULL)
		return DRV_ERROR_MEMORY;

	strcpy(driverPath, path);

	LPTSTR sPos1 = strrchr(driverPath, (int)'\\');

	if (sPos1 == NULL)
		sPos1 = driverPath;

	LPTSTR sPos2 = strrchr(sPos1, (int)'.');

	if (sPos2 == NULL || sPos1 > sPos2)
	{
		free(driverPath);
		driverPath = NULL;

		return DRV_ERROR_INVALID_PATH_OR_FILE;
	}
	
	driverName = (LPTSTR) malloc (sPos2 - sPos1);
	
	if(driverName == NULL)
	{
		free(driverPath);
		driverPath = NULL;

		return DRV_ERROR_MEMORY;
	}

	memcpy(driverName, sPos1 + 1, sPos2 - sPos1 - 1);
	
	driverName[sPos2 - sPos1 - 1] = 0;

	driverDosName = (LPTSTR) malloc (strlen(driverName) + 5);

	if(driverDosName == NULL)
	{
		free(driverPath);
		driverPath = NULL;

		free(driverName);
		driverName = NULL;

		return DRV_ERROR_MEMORY;
	}

	sprintf(driverDosName, "\\\\.\\%s", driverName);

	initialized = TRUE;
	return DRV_SUCCESS;
}

DWORD TDriver::InitDriver(LPCTSTR name, LPCTSTR path, LPCTSTR dosName)
{
	if(initialized)
	{
		if(UnloadDriver() != DRV_SUCCESS)
			return DRV_ERROR_ALREADY_INITIALIZED;
	}

	LPTSTR dirBuffer;
	if (path != NULL) 
	{
		DWORD len = (DWORD)(strlen(name) + strlen(path) + 1);
		dirBuffer = (LPTSTR) malloc (len);

		if(dirBuffer == NULL)
			return DRV_ERROR_MEMORY;

		strcpy(dirBuffer, path);

	}
	else 
	{
		LPTSTR pathBuffer;
        DWORD len = GetCurrentDirectory(0, NULL);
      
		pathBuffer = (LPTSTR) malloc (len);

		if(pathBuffer == NULL)
			return DRV_ERROR_MEMORY;

        if (GetCurrentDirectory(len, pathBuffer) != 0) 
		{
			len = (DWORD)(strlen(pathBuffer) + strlen(name) + 6);
			dirBuffer = (LPTSTR) malloc (len);

			if(dirBuffer == NULL)
			{
				free(pathBuffer);

				return DRV_ERROR_MEMORY;
			}

			sprintf(dirBuffer, "%s\\%s.sys", pathBuffer, name);

			if(GetFileAttributes(dirBuffer) == 0xFFFFFFFF)
			{
				free(pathBuffer);
				free(dirBuffer);

				LPCTSTR sysDriver = "\\system32\\Drivers\\";
				LPTSTR sysPath;
	    	    
				DWORD len = GetWindowsDirectory(NULL, 0);
     			sysPath = (LPTSTR) malloc (len + strlen(sysDriver));

				if(sysPath == NULL)
					return DRV_ERROR_MEMORY;

				if (GetWindowsDirectory(sysPath, len) == 0) 
				{
					free(sysPath);
					
					return DRV_ERROR_UNKNOWN;
				}
	
				strcat(sysPath, sysDriver);
				len = (DWORD)(strlen(sysPath) + strlen(name) + 5);

				dirBuffer = (LPTSTR) malloc (len);

				if(dirBuffer == NULL)
					return DRV_ERROR_MEMORY;

				sprintf(dirBuffer, "%s%s.sys", sysPath, name);

				free(sysPath);

				if(GetFileAttributes(dirBuffer) == 0xFFFFFFFF)
				{
					free(dirBuffer);

					return DRV_ERROR_INVALID_PATH_OR_FILE;
				}
			}
        }

		else
		{
			free(pathBuffer);

			return DRV_ERROR_UNKNOWN;
		}
	}
	
	driverPath = dirBuffer;

	driverName = (LPTSTR)malloc(strlen(name) + 1);

	if(driverName == NULL)
	{
		free(driverPath);
		driverPath = NULL;
		
		return DRV_ERROR_MEMORY;
	}

	strcpy(driverName, name);
	
	LPCTSTR auxBuffer;
	if(dosName != NULL)
        auxBuffer = dosName;
	
	else
		auxBuffer = name;

	if(auxBuffer[0] != '\\' && auxBuffer[1] != '\\')
	{
		driverDosName = (LPTSTR) malloc (strlen(auxBuffer) + 5);

		if(driverDosName == NULL)
		{
			free(driverPath);
			driverPath = NULL;

			free(driverName);
			driverName = NULL;

			return DRV_ERROR_MEMORY;
		}

		sprintf(driverDosName, "\\\\.\\%s", auxBuffer);
	}
	else
	{
		driverDosName = (LPTSTR) malloc (strlen(auxBuffer));

		if(driverDosName == NULL)
		{
			free(driverPath);
			driverPath = NULL;

			free(driverName);
			driverName = NULL;

			return DRV_ERROR_MEMORY;
		}

		strcpy(driverDosName, auxBuffer);
	}

	initialized = TRUE;

	return DRV_SUCCESS;
}


DWORD TDriver::LoadDriver(LPCTSTR name, LPCTSTR path, LPCTSTR dosName, BOOL start)
{
	DWORD retCode = InitDriver(name, path, dosName);

	if(retCode == DRV_SUCCESS)
		retCode = LoadDriver(start);

	return retCode;
}

DWORD TDriver::LoadDriver(LPCTSTR path, BOOL start)
{
	DWORD retCode = InitDriver(path);

	if(retCode == DRV_SUCCESS)
		retCode = LoadDriver(start);

	return retCode;
}

DWORD TDriver::LoadDriver(BOOL start)
{
	bool thenFail = false;

	if(loaded)
		return DRV_SUCCESS;

	if(!initialized)
		return DRV_ERROR_NO_INITIALIZED;

	SC_HANDLE SCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	DWORD retCode = DRV_SUCCESS;
	
	if (SCManager == NULL) 
		return DRV_ERROR_SCM;

createAgain:
    SC_HANDLE  SCService = CreateService(SCManager,			  // SCManager database
									     driverName,            // nombre del servicio
							    		 driverName,            // nombre a mostrar
										 SERVICE_ALL_ACCESS,    // acceso total
										 SERVICE_KERNEL_DRIVER, // driver del kernel
										 SERVICE_DEMAND_START,  // comienzo bajo demanda
										 SERVICE_ERROR_NORMAL,  // control de errores normal
										 driverPath,	          // path del driver
										 NULL,                  // no pertenece a un grupo
										 NULL,                  // sin tag
										 NULL,                  // sin dependencias
										 NULL,                  // cuenta local del sistema
										 NULL                   // sin password
										 );

    
	int err = GetLastError();

	if (SCService == NULL && !thenFail) 
	{
		SCService = OpenService(SCManager, driverName, SERVICE_ALL_ACCESS);
		
		if (SCService == NULL) 
			retCode = DRV_ERROR_SERVICE;
		else
		{
			thenFail = true;
			
			SERVICE_STATUS status;
			ControlService(SCService, SERVICE_CONTROL_STOP, &status);
			DeleteService(SCService);

			goto createAgain;
		}
	}

    CloseServiceHandle(SCService);
	SCService=NULL;

	CloseServiceHandle(SCManager);
	SCManager = NULL;

	if(retCode == DRV_SUCCESS)
	{
		loaded = TRUE;

		if(start)
			retCode = StartDriver();
	}

	return retCode;
}

DWORD TDriver::UnloadDriver(BOOL forceClearData)
{
	DWORD retCode = DRV_SUCCESS;

	if (started)
	{
		if ((retCode = StopDriver()) == DRV_SUCCESS) 
		{
			if(removable)
			{
				SC_HANDLE SCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
				
				if (SCManager == NULL) 
					return DRV_ERROR_SCM;

				SC_HANDLE SCService = OpenService(SCManager, driverName, SERVICE_ALL_ACCESS);
				
				if (SCService != NULL)
				{
					if(!DeleteService(SCService))
						retCode = DRV_ERROR_REMOVING;
					else
						retCode = DRV_SUCCESS;
				}

				else
					retCode = DRV_ERROR_SERVICE;

				CloseServiceHandle(SCService);
				SCService = NULL;

				CloseServiceHandle(SCManager);
				SCManager = NULL;

				if(retCode == DRV_SUCCESS)
					loaded = FALSE;
			}
		}
	}

	if(initialized) 
	{
		if(retCode != DRV_SUCCESS && forceClearData == FALSE)
			return retCode;
		
		initialized = FALSE;
	}

	return retCode;
}

DWORD TDriver::StartDriver(void)
{
	if(started)
		return DRV_SUCCESS;

	SC_HANDLE SCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	DWORD retCode;
	
	if (SCManager == NULL) 
		return DRV_ERROR_SCM;

    SC_HANDLE SCService = OpenService(SCManager,
		                              driverName,
				                      SERVICE_ALL_ACCESS);
    
	if (SCService == NULL) 
        return DRV_ERROR_SERVICE;

    
    if (!StartService( SCService, 0, NULL)) 
	{
        if (GetLastError() == ERROR_SERVICE_ALREADY_RUNNING) 
		{
			removable = FALSE;

			retCode = DRV_SUCCESS;
		}
		else
			retCode = DRV_ERROR_STARTING;
    }
	else
		retCode = DRV_SUCCESS;

  
    CloseServiceHandle(SCService);
	SCService = NULL;

	CloseServiceHandle(SCManager);
	SCManager = NULL;

	if(retCode == DRV_SUCCESS)
	{
		started = TRUE;

		retCode = OpenDevice();
	}

    return retCode;
}

DWORD TDriver::StopDriver(void)
{
	if(!started)
		return DRV_SUCCESS;

	SC_HANDLE SCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	DWORD retCode;
	
	if (SCManager == NULL) 
		return DRV_ERROR_SCM;

   
    SERVICE_STATUS  status;

    SC_HANDLE SCService = OpenService(SCManager, driverName, SERVICE_ALL_ACCESS);
    
	if (SCService != NULL)
	{
		CloseHandle(driverHandle); 
		driverHandle = NULL; 

		if(!ControlService(SCService, SERVICE_CONTROL_STOP, &status))
			retCode = DRV_ERROR_STOPPING;

		else
			retCode = DRV_SUCCESS;
	}
	else
		retCode = DRV_ERROR_SERVICE;

    CloseServiceHandle(SCService);
	SCService = NULL;

	CloseServiceHandle(SCManager);
	SCManager = NULL;

	if(retCode == DRV_SUCCESS)
		started = FALSE;

    return retCode;
}

DWORD TDriver::OpenDevice(void)
{
	if (driverHandle != NULL) 
		CloseHandle(driverHandle);

    driverHandle = CreateFile(driverDosName,
							  GENERIC_READ | GENERIC_WRITE,
							  0,
                              NULL,
                              OPEN_EXISTING,
                              FILE_ATTRIBUTE_NORMAL,
                              NULL);


	int err = GetLastError();

    if(driverHandle == INVALID_HANDLE_VALUE)
		return DRV_ERROR_INVALID_HANDLE;
	
	return DRV_SUCCESS;
}

HANDLE TDriver::GetDriverHandle(void)
{
	return driverHandle;
}

DWORD TDriver::WriteIo(DWORD code, PVOID buffer, DWORD count)
{
	if(driverHandle == NULL)
		return DRV_ERROR_INVALID_HANDLE;

	DWORD bytesReturned;

	BOOL returnCode = DeviceIoControl(driverHandle,
								      code,
								      buffer,
								      count,
								      NULL,
								      0,
								      &bytesReturned,
								      NULL);

	if(!returnCode)
		return DRV_ERROR_IO;

	return DRV_SUCCESS;
}

DWORD TDriver::ReadIo(DWORD code, PVOID buffer, DWORD count)
{
	if(driverHandle == NULL)
		return DRV_ERROR_INVALID_HANDLE;

	DWORD bytesReturned;
	BOOL retCode = DeviceIoControl(driverHandle,
								   code,
								   NULL,
								   0,
								   buffer,
								   count,
								   &bytesReturned,
								   NULL);

	if(!retCode)
		return DRV_ERROR_IO;

	return bytesReturned;
}

DWORD TDriver::RawIo(DWORD code, PVOID inBuffer, DWORD inCount, PVOID outBuffer, DWORD outCount)
{
	if(driverHandle == NULL)
		return DRV_ERROR_INVALID_HANDLE;

	DWORD bytesReturned;
	BOOL retCode = DeviceIoControl(driverHandle,
								   code,
								   inBuffer,
								   inCount,
								   outBuffer,
								   outCount,
								   &bytesReturned,
								   NULL);

	if(!retCode)
		return DRV_ERROR_IO;

	return bytesReturned;
}