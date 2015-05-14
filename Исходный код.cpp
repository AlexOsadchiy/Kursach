#include <stdio.h>
#include <Windows.h>
#include <iostream>
#include <conio.h>
#include <locale.h>

using namespace std;

void GetDiskGeometry(HANDLE);
void GrowPartition(HANDLE, int);
void AddPartition(HANDLE);
DRIVE_LAYOUT_INFORMATION* GetDriveLayoutInformation(HANDLE);

/* структура элемента раздела */
struct Part {
	BYTE ActFlag;			/* признак активного раздела */
							/* физический адрес начала раздела */
	BYTE Begin_Hd;			/* # головки */
	WORD Begin_SecTrk;		/* # сектора и дорожки */
	BYTE SysCode;			/* код системы */
							/* физический адрес конца раздела */
	BYTE End_Hd;			/* # головки */
	WORD End_SecTrk;		/* # сектора и дорожки */
	DWORD RelSec;			/* # сектора початку */
	DWORD Size;				/* количество секторов */
};

int main()
{
	byte buf[512];
	Part partOne;
	DWORD numberOfBytesRead;

	HANDLE hFile = CreateFile("\\\\.\\PhysicalDrive1", 
						GENERIC_READ | GENERIC_WRITE, 
						FILE_SHARE_READ | FILE_SHARE_WRITE, 
						NULL, 
						OPEN_EXISTING,
						NULL, 
						NULL);

	if (hFile == INVALID_HANDLE_VALUE)
	{
		int error = GetLastError();
		printf("\nError %d", error);
	}
	
	SetFilePointer(hFile, 0, 0, FILE_BEGIN);
	if (!ReadFile(hFile, &buf, 512, &numberOfBytesRead, NULL))
	{
		int error = GetLastError();
		printf("\nError %d", error);
	}

	partOne.ActFlag = *(buf + 446);
	partOne.Begin_Hd = *(buf + 447);
	partOne.Begin_SecTrk = *(buf + 448);
	partOne.SysCode = *(buf + 450);
	partOne.End_Hd = *(buf + 451);
	partOne.End_SecTrk = *(buf + 452);
	partOne.RelSec = *(buf + 453);
	partOne.Size = *(buf + 458);

	setlocale(LC_ALL, "Russian");

	int flEnd = 1;

	while (1)
	{
		cout << "1 - Вывод информации о разделах" << endl;
		cout << "2 - Создать раздел" << endl;
		cout << "3 - Удалить раздел" << endl;
		cout << "4 - Увеличить раздел" << endl;
		cout << "5 - Уменьшить раздел" << endl;
		cout << "0 - Выход" << endl;
		switch (getch())
		{
		case '1':
			DRIVE_LAYOUT_INFORMATION * lpdlDriveLayoutInfo;
			int size;
			size = sizeof(DRIVE_LAYOUT_INFORMATION)+4 * sizeof(PARTITION_INFORMATION);
			lpdlDriveLayoutInfo = (DRIVE_LAYOUT_INFORMATION*)malloc(size);
			lpdlDriveLayoutInfo = GetDriveLayoutInformation(hFile);

			for (int i = 0; i < 4; i++)
			{
				cout << "Индакатор загрузки: " << lpdlDriveLayoutInfo->PartitionEntry[i].BootIndicator << endl;
				cout << "Номер раздела: " << lpdlDriveLayoutInfo->PartitionEntry[i].PartitionNumber<< endl;
				cout << "Тип раздела: " << lpdlDriveLayoutInfo->PartitionEntry[i].PartitionType << endl;
				cout << "Стартовое смещение: " << lpdlDriveLayoutInfo->PartitionEntry[i].StartingOffset.QuadPart << endl;
				cout << "Раздел " << i << ":\nCвободно " << lpdlDriveLayoutInfo->PartitionEntry[i].PartitionLength.QuadPart / 1024 / 1024 << " Mb" << endl;
			}
			
			GetDiskGeometry(hFile);
			break;
		case '2':
			AddPartition(hFile);
			break;
		case '3':
			cout << "Введите номер раздела: " << endl;
			while (flEnd)
			{
				fflush(stdin);
				switch (getch())
				{
				case '1':
					for (int i = 446; i < 462; i++)
						buf[i] = 0;
					SetFilePointer(hFile, 0, 0, FILE_BEGIN);
					if (!WriteFile(hFile, &buf, 512, &numberOfBytesRead, NULL))
					{
						int error = GetLastError();
						cout << "\nError" << error << endl;
					}
					cout << "Раздел успешно удален" << endl;
					flEnd = 0;
					break;
				case '2':
					for (int i = 462; i < 478; i++)
						buf[i] = 0;
					SetFilePointer(hFile, 0, 0, FILE_BEGIN);
					if (!WriteFile(hFile, &buf, 512, &numberOfBytesRead, NULL))
					{
						int error = GetLastError();
						cout << "\nError" << error << endl;
					}
					cout << "Раздел успешно удален" << endl;
					flEnd = 0;
					break;
				case '3':
					for (int i = 478; i < 494; i++)
						buf[i] = 0;
					SetFilePointer(hFile, 0, 0, FILE_BEGIN);
					if (!WriteFile(hFile, &buf, 512, &numberOfBytesRead, NULL))
					{
						int error = GetLastError();
						printf("\nError %d", error);
					}
					cout << "Раздел успешно удален" << endl;
					flEnd = 0;
					break;
				case '4':
					for (int i = 494; i < 510; i++)
						buf[i] = 0;
					SetFilePointer(hFile, 0, 0, FILE_BEGIN);
					if (!WriteFile(hFile, &buf, 512, &numberOfBytesRead, NULL))
					{
						int error = GetLastError();
						printf("\nError %d", error);
					}
					cout << "Раздел успешно удален" << endl;
					flEnd = 0;
					break;
				default:
					cout << "Неправильно!!!Попробуй еще раз" << endl;
					break;
				}
			}
			break;
		case '4':
			GrowPartition(hFile, 1);
			break;
		case '5':
			GrowPartition(hFile, -1);
			break;
		case '0': 
			return 0;
		default: 
			break;
		}
	}

	return 0;
}

void GetDiskGeometry(HANDLE hDevice)
{
	int result = 0;

	DISK_GEOMETRY_EX diskGeometry;
	DWORD BytesReturned = 0;

	result = DeviceIoControl(
		hDevice,					
		IOCTL_DISK_GET_DRIVE_GEOMETRY_EX,	 
		NULL,								 
		0,									 
		&diskGeometry,				
		sizeof(diskGeometry),				
		&BytesReturned,			  
		NULL								  
		);

	if (!result)
	{
		int error = GetLastError();
		cout << "\nError " << error;
		return;
	}

	cout << "Количество цилиндров:  " << diskGeometry.Geometry.Cylinders.QuadPart << endl;
	cout << "Дорожек в цилинде:     " << diskGeometry.Geometry.TracksPerCylinder << endl;
	cout << "Секторов на дорожке:   " << diskGeometry.Geometry.SectorsPerTrack << endl;
	cout << "Байт в секторе:        " << diskGeometry.Geometry.BytesPerSector << endl;
	int col = diskGeometry.Geometry.Cylinders.QuadPart*diskGeometry.Geometry.TracksPerCylinder*diskGeometry.Geometry.SectorsPerTrack;
	cout << "Количество сектров:    " << col << endl;
	cout << "Size in byte           " << col*diskGeometry.Geometry.BytesPerSector << endl;
	cout << "Размер диска в байтах: " << diskGeometry.DiskSize.QuadPart << endl;
}

void GrowPartition(HANDLE hDevice, int operation)
{
	DISK_GROW_PARTITION growPartition;
	DWORD BytesReturned = 0;

	int size;
	int numPart;
	int result = 0;

	cout << "Введите номер раздела: ";
	scanf("%d", &numPart);
	cout << endl;
	growPartition.PartitionNumber = numPart;

	cout << "Уменьшить раздел на (Мегабайт): ";
	cin >> size;
	growPartition.BytesToGrow.QuadPart = operation * size * 1024 * 1024;

	result = DeviceIoControl(
		hDevice,
		IOCTL_DISK_GROW_PARTITION,
		&growPartition,
		sizeof(growPartition),
		NULL,
		0,
		&BytesReturned,
		NULL);

	if (!result)
	{
		int error = GetLastError();
		cout << "\nError " << error << endl;
		return;
	}

	cout << "Раздел изменен" << endl;
}

void AddPartition(HANDLE hDevice)
{
	DRIVE_LAYOUT_INFORMATION *setDriveLayoutInformation;
	DRIVE_LAYOUT_INFORMATION *getDriveLayoutInformation;

	DWORD dwBytesReturned = 0;
	DWORD size = 0;

	int result = 0;
	int lengthPartition = 0;
	int newOffset = 0;
	int partitionType = 0;

	cout << "Вас приветствует мастер создания разделов" << endl;
	cout << "Введите размер раздела в мегабайтах: ";
	cin >> lengthPartition;
	cout << endl;

	cout << "Выберите тип раздела:" << endl;
	cout << "1. NTFS" << endl;
	cout << "2. FAT" << endl;
	cout << "3. EXTENDED" << endl;
	cin >> partitionType;
		switch (partitionType)
	{
		case '1':
			fflush(stdin);
			partitionType = 0x07;
			break;
		case '2':
			fflush(stdin);
			partitionType = 0x06;
			break;
		case '3':
			fflush(stdin);
			partitionType = 0x05;
			break;
	}

	size = sizeof(DRIVE_LAYOUT_INFORMATION)+4 * sizeof(PARTITION_INFORMATION);
	setDriveLayoutInformation = (DRIVE_LAYOUT_INFORMATION*)malloc(size);
	if (!setDriveLayoutInformation)
	{
		int error = GetLastError();
		cout << "\nError " << error << endl;
		return;
	}

	getDriveLayoutInformation = (DRIVE_LAYOUT_INFORMATION*)malloc(size);
	if (!getDriveLayoutInformation)
	{
		int error = GetLastError();
		cout << "\nError " << error << endl;
		return;
	}

	getDriveLayoutInformation = GetDriveLayoutInformation(hDevice);

	setDriveLayoutInformation->PartitionCount = 4;
	setDriveLayoutInformation->Signature = 0xA4B57300;

	for (int i = 0; i < 4; i++)
	{
		newOffset += getDriveLayoutInformation->PartitionEntry[i].PartitionLength.QuadPart;
		if (getDriveLayoutInformation->PartitionEntry[i].PartitionLength.QuadPart == 0)
		{	
			getDriveLayoutInformation->PartitionEntry[i].StartingOffset.QuadPart = 512 + newOffset;
			getDriveLayoutInformation->PartitionEntry[i].PartitionLength.QuadPart = lengthPartition * 1024 * 1024;
			getDriveLayoutInformation->PartitionEntry[i].HiddenSectors = 0;
			getDriveLayoutInformation->PartitionEntry[i].PartitionNumber = i;
			getDriveLayoutInformation->PartitionEntry[i].PartitionType = 0x05;
			getDriveLayoutInformation->PartitionEntry[i].BootIndicator = 0;
			getDriveLayoutInformation->PartitionEntry[i].RecognizedPartition = 1;
			getDriveLayoutInformation->PartitionEntry[i].RewritePartition = 1;
			break;
		}
	}

	result = DeviceIoControl(
		hDevice,         
		IOCTL_DISK_SET_DRIVE_LAYOUT,
		getDriveLayoutInformation,         
		size,      
		NULL,                       
		0,                           
		&dwBytesReturned,  
		NULL  
		);

	if (!result)
	{
		int error = GetLastError();
		cout << "\nError " << error << endl;
		return;
	}

	cout << "Раздел успешно создан" << endl;
}

DRIVE_LAYOUT_INFORMATION* GetDriveLayoutInformation(HANDLE hDevice)
{
	DRIVE_LAYOUT_INFORMATION *driveLayoutInformation;
	DWORD dwBytesReturned = 0;

	int result;
	int size;

	size = sizeof(DRIVE_LAYOUT_INFORMATION) + 4 * sizeof(PARTITION_INFORMATION);
	driveLayoutInformation = (DRIVE_LAYOUT_INFORMATION*)malloc(size);
	if (!driveLayoutInformation)
	{
		int error = GetLastError();
		cout << "\nError " << error << endl;
		return 0;
	}

	result = DeviceIoControl(hDevice,
		IOCTL_DISK_GET_DRIVE_LAYOUT,
		NULL,
		0,
		driveLayoutInformation,
		size,
		&dwBytesReturned,
		NULL);

	if (!result)
	{
		int error = GetLastError();
		cout << "\Error " << error << endl;
		return 0;
	}

	return driveLayoutInformation;
}