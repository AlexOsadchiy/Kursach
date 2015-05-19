#include <stdio.h>
#include <Windows.h>
#include <iostream>
#include <conio.h>
#include <locale.h>
#include <string.h> 
#include <stdlib.h>

using namespace std;

void GetDiskGeometry(HANDLE);
void GrowPartition(HANDLE, int);
void PartitionManager(HANDLE);
void AddPartition(HANDLE, DWORD, BYTE, BYTE, BOOLEAN);
void DeletePartition(HANDLE);
void AddPartitionGPT(HANDLE, DWORD);
PDRIVE_LAYOUT_INFORMATION_EX GetDriveLayoutGPT(HANDLE);
DRIVE_LAYOUT_INFORMATION* GetDriveLayoutInformation(HANDLE);
HANDLE SelectionDisk();

/* ��������� �������� ������� */
struct Part {
	BYTE ActFlag;			/* ������� ��������� ������� */
							/* ���������� ����� ������ ������� */
	BYTE Begin_Hd;			/* # ������� */
	WORD Begin_SecTrk;		/* # ������� � ������� */
	BYTE SysCode;			/* ��� ������� */
							/* ���������� ����� ����� ������� */
	BYTE End_Hd;			/* # ������� */
	WORD End_SecTrk;		/* # ������� � ������� */
	DWORD RelSec;			/* # ������� ������� */
	DWORD Size;				/* ���������� �������� */
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
		cout << "1 - ����� ���������� � ��������" << endl;
		cout << "2 - ������� ������" << endl;
		cout << "3 - ������� ������" << endl;
		cout << "4 - ��������� ������" << endl;
		cout << "5 - ��������� ������" << endl;
		cout << "6 - ������� ����" << endl;
		cout << "0 - �����" << endl;
		switch (_getch())
		{
		case '1':
			PDRIVE_LAYOUT_INFORMATION_EX getDriveLayoutInformation;
			int size;
			size = sizeof(PDRIVE_LAYOUT_INFORMATION_EX)+64 * sizeof(PARTITION_INFORMATION_EX);
			getDriveLayoutInformation = (PDRIVE_LAYOUT_INFORMATION_EX)calloc(1, size);
			getDriveLayoutInformation = GetDriveLayoutGPT(hFile);
			
			/*for (int i = 0; i < 16; i++)
			{
				cout << "������: " << i << endl << endl;
				cout << "StartingOffset          " << getDriveLayoutInformation->PartitionEntry[i].StartingOffset.QuadPart << endl;
				cout << "PartitionLength         " << getDriveLayoutInformation->PartitionEntry[i].PartitionLength.QuadPart << endl;
				cout << "HiddenSectors           " << getDriveLayoutInformation->PartitionEntry[i].HiddenSectors << endl;
				cout << "PartitionNumber         " << getDriveLayoutInformation->PartitionEntry[i].PartitionNumber << endl;
				cout << "PartitionType           " << getDriveLayoutInformation->PartitionEntry[i].PartitionType << endl;
				cout << "BootIndicator           " << (short)getDriveLayoutInformation->PartitionEntry[i].BootIndicator << endl;
				cout << "RecognizedPartition     " << (short)getDriveLayoutInformation->PartitionEntry[i].RecognizedPartition << endl;
				cout << "RewritePartition        " << (int)getDriveLayoutInformation->PartitionEntry[i].RewritePartition << endl;
			}*/
			
			for (int i = 0; i < 16; i++)
			{
				cout << "������: " << i << endl << endl;
				cout << "StartingOffset          " << getDriveLayoutInformation->PartitionEntry[i].StartingOffset.QuadPart << endl;
				cout << "PartitionLength         " << getDriveLayoutInformation->PartitionEntry[i].PartitionLength.QuadPart << endl;
				cout << "PartitionNumber         " << getDriveLayoutInformation->PartitionEntry[i].PartitionNumber << endl;
				//cout << "PartitionType           " << getDriveLayoutInformation->PartitionEntry[i].Gpt.PartitionType. << endl;
				cout << "RewritePartition        " << (int)getDriveLayoutInformation->PartitionEntry[i].RewritePartition << endl;
			}

			getDriveLayoutInformation->PartitionEntry[2] = getDriveLayoutInformation->PartitionEntry[1];
			getDriveLayoutInformation->PartitionEntry[2].PartitionNumber = 3;
			getDriveLayoutInformation->PartitionEntry[2].RewritePartition = 1;
			getDriveLayoutInformation->PartitionEntry[2].StartingOffset.QuadPart += 104857600;
			
			getDriveLayoutInformation->PartitionCount = 3;

			
			DWORD dwBytesReturned;
			int result;
			result = DeviceIoControl(hFile,
				IOCTL_DISK_SET_DRIVE_LAYOUT_EX,
				getDriveLayoutInformation,
				size,
				NULL,
				0,
				&dwBytesReturned,
				NULL);

			if (!result)
			{
				int error;
				error = GetLastError();
				cout << "\nError " << error << endl;
			}
			//GetDiskGeometry(hFile);
			break;
		case '2':
			PartitionManager(hFile);
			break;
		case '3':
			DeletePartition(hFile);
			break;
		case '4':
			GrowPartition(hFile, 1);
			break;
		case '5':
			GrowPartition(hFile, -1);
			break;
		case '6':
			hFile = SelectionDisk();
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
	WORD result = 0;
	LONGLONG col = 0;
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

	cout << "���������� ���������:  " << diskGeometry.Geometry.Cylinders.QuadPart << endl;
	cout << "������� � �������:     " << diskGeometry.Geometry.TracksPerCylinder << endl;
	cout << "�������� �� �������:   " << diskGeometry.Geometry.SectorsPerTrack << endl;
	cout << "���� � �������:        " << diskGeometry.Geometry.BytesPerSector << endl;
	col = diskGeometry.Geometry.Cylinders.QuadPart*diskGeometry.Geometry.TracksPerCylinder*diskGeometry.Geometry.SectorsPerTrack;
	cout << "���������� �������:    " << col << endl;
	cout << "Size in byte           " << col*diskGeometry.Geometry.BytesPerSector << endl;
	cout << "������ ����� � ������: " << diskGeometry.DiskSize.QuadPart << endl;
}

void GrowPartition(HANDLE hDevice, int operation)
{
	DISK_GROW_PARTITION growPartition;
	DWORD BytesReturned = 0;

	int size;
	int numPart;
	int result = 0;

	cout << "������� ����� �������: ";
	scanf_s("%d", &numPart);
	cout << endl;
	growPartition.PartitionNumber = numPart;

	cout << "�������� ������ �� (��������): ";
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

	cout << "������ �������" << endl;
}

void PartitionManager(HANDLE hDevice)
{
	DWORD lengthPartition = 0;
	BOOLEAN recognizedPartition = 0;
	BOOLEAN logicalDisk = 0;
	BYTE partitionType = 0;

	system("CLS");

	cout << "��� ������������ ������ �������� ��������" << endl;

	cout << "������� ������ ������� � ����������: ";
	cin >> lengthPartition;
	cout << endl;

	cout << "�������� ��� �������:" << endl;
	cout << "1. ��������" << endl;
	cout << "2. �����������" << endl;
	cout << "3. ���������� ����" << endl;
	cout << "4. ������ GPT" << endl;
	cin>> partitionType;

		switch (partitionType)
	{
		case '1':
			partitionType = 0x06;
			recognizedPartition = 1;
			AddPartition(hDevice, lengthPartition, recognizedPartition, partitionType, logicalDisk);
			break;
		case '2':
			partitionType = 0x05;
			recognizedPartition = 0;
			AddPartition(hDevice, lengthPartition, recognizedPartition, partitionType, logicalDisk);
			break;
		case '3':
			partitionType = 0x06;
			recognizedPartition = 1;
			logicalDisk = 1;
			AddPartition(hDevice, lengthPartition, recognizedPartition, partitionType, logicalDisk);
			break;
		case '4':
			AddPartitionGPT(hDevice, lengthPartition);
			break;
	}
	cout << "������ ������� ������" << endl;
}

void AddPartition(HANDLE hDevice, DWORD lengthPartition, BYTE recognizedPartition, BYTE partitionType, BOOLEAN logicalDisk)
{
	DRIVE_LAYOUT_INFORMATION *getDriveLayoutInformation;
	DWORD dwBytesReturned = 0;
	DWORD size = 0;
	BYTE result = 0;
	LONGLONG newOffset = 0;
	BYTE partitionNumber = 0;

	size = sizeof(DRIVE_LAYOUT_INFORMATION)+16 * sizeof(PARTITION_INFORMATION);

	getDriveLayoutInformation = (DRIVE_LAYOUT_INFORMATION*)malloc(size);
	if (!getDriveLayoutInformation)
	{
		int error = GetLastError();
		cout << "\nError " << error << endl;
		return;
	}

	getDriveLayoutInformation = GetDriveLayoutInformation(hDevice);

	for (int i = 0; i < 16; i++)
	{	
		if (getDriveLayoutInformation->PartitionEntry[i].PartitionLength.QuadPart < 0)
			getDriveLayoutInformation->PartitionEntry[i].PartitionLength.QuadPart = 0;
	}

	BYTE count = 0;

	if (logicalDisk)
	{
		BYTE i = 0;
		DWORD newHidden = 0;
		
		int fl = 0;
		for (i = 0; i < 16 && fl == 0; i++)
		{
			
			if (getDriveLayoutInformation->PartitionEntry[i].RecognizedPartition != 0)
			{
				newOffset += getDriveLayoutInformation->PartitionEntry[i].PartitionLength.QuadPart;
			}
			if (getDriveLayoutInformation->PartitionEntry[i].RecognizedPartition == 0)
			for (int j = i + 1; j <= i + 4; j++)
			{
				newOffset += getDriveLayoutInformation->PartitionEntry[j].PartitionLength.QuadPart;
				newHidden += getDriveLayoutInformation->PartitionEntry[j].PartitionLength.QuadPart / 512;
				if (getDriveLayoutInformation->PartitionEntry[j].PartitionLength.QuadPart <= 0)
				{
					getDriveLayoutInformation->PartitionEntry[j].StartingOffset.QuadPart = 2 * 65536 + newOffset;
					getDriveLayoutInformation->PartitionEntry[j].PartitionLength.QuadPart = lengthPartition * 1024 * 1024;
					getDriveLayoutInformation->PartitionEntry[j].HiddenSectors = newHidden + 128;
					getDriveLayoutInformation->PartitionEntry[j].PartitionNumber = j;
					getDriveLayoutInformation->PartitionEntry[j].PartitionType = partitionType;
					getDriveLayoutInformation->PartitionEntry[j].BootIndicator = 0;
					getDriveLayoutInformation->PartitionEntry[j].RecognizedPartition = recognizedPartition;
					getDriveLayoutInformation->PartitionEntry[j].RewritePartition = 1;
					getDriveLayoutInformation->PartitionEntry[i].RewritePartition = 1;
					fl = 1;
					break;
				}
				if (j == i + 4)
					i = j;
			}
		}
	}
	else
	{
		for (int i = 0; i < 16; i++)
		{	//���� ��������� ����������� ������, �� ���������� ��� ��� ���������� ��� ���� ����� ��������� ������ - ��� ����� ��� ���� ����� ���� ������� ����� ������� ��������� � ������������
			if (getDriveLayoutInformation->PartitionEntry[i].RecognizedPartition == 0 && getDriveLayoutInformation->PartitionEntry[i].PartitionLength.QuadPart > 0)
			{
				i = i + 4;
				continue;
			}
			getDriveLayoutInformation->PartitionEntry[i].RecognizedPartition = 2;
		}

		for (int i = 0; i < 16; i++)
		{
			if (getDriveLayoutInformation->PartitionEntry[i].RecognizedPartition == 0)					//���� ��������� ����������� ������, �� ���������� ��� ��� ����������
			{
				newOffset += getDriveLayoutInformation->PartitionEntry[i].PartitionLength.QuadPart;
				i = i + 4;
				continue;
			}
			newOffset += getDriveLayoutInformation->PartitionEntry[i].PartitionLength.QuadPart;
			if (recognizedPartition == 0)
				partitionNumber = 0;
			else partitionNumber = i + 1;
			if (getDriveLayoutInformation->PartitionEntry[i].PartitionLength.QuadPart <= 0)
			{
				getDriveLayoutInformation->PartitionEntry[i].StartingOffset.QuadPart = 65536 + newOffset;
				getDriveLayoutInformation->PartitionEntry[i].PartitionLength.QuadPart = lengthPartition * 1024 * 1024;
				getDriveLayoutInformation->PartitionEntry[i].HiddenSectors = newOffset / 512 + 128;
				getDriveLayoutInformation->PartitionEntry[i].PartitionNumber = partitionNumber;
				getDriveLayoutInformation->PartitionEntry[i].PartitionType = partitionType;
				getDriveLayoutInformation->PartitionEntry[i].BootIndicator = 0;
				getDriveLayoutInformation->PartitionEntry[i].RecognizedPartition = recognizedPartition;
				getDriveLayoutInformation->PartitionEntry[i].RewritePartition = 1;
				break;
			}
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
}

void DeletePartition(HANDLE hDevice)
{
	DRIVE_LAYOUT_INFORMATION *setDriveLayoutInformation;
	DWORD dwBytesReturned = 0;
	DWORD size = 0;
	BYTE result = 0;
	DWORD partitionNumber = 0;

	system("CLS");

	cout << "������� ����� �������: ";
	fflush(stdin);
	scanf_s("%d", &partitionNumber);
	cout << endl;

	size = sizeof(DRIVE_LAYOUT_INFORMATION)+16 * sizeof(PARTITION_INFORMATION);

	setDriveLayoutInformation = (DRIVE_LAYOUT_INFORMATION*)malloc(size);
	if (!setDriveLayoutInformation)
	{
		int error = GetLastError();
		cout << "\nError " << error << endl;
		return;
	}

	setDriveLayoutInformation = GetDriveLayoutInformation(hDevice);

	for (int j = 0; j < 16; j++)
	{
		if (setDriveLayoutInformation->PartitionEntry[j].PartitionNumber == partitionNumber)
		{
			setDriveLayoutInformation->PartitionEntry[j].StartingOffset.QuadPart = 0;
			setDriveLayoutInformation->PartitionEntry[j].PartitionLength.QuadPart = 0;
			setDriveLayoutInformation->PartitionEntry[j].HiddenSectors = 0;
			setDriveLayoutInformation->PartitionEntry[j].PartitionNumber = 0;			//������� ����� �������� ��� ���� ����� ����� ���� ������� ����������� ������, ��� ��� � ���� ����� 0, ��� ��� ��������� �������� �������� �� ����
			setDriveLayoutInformation->PartitionEntry[j].PartitionType = 0;
			setDriveLayoutInformation->PartitionEntry[j].BootIndicator = 0;
			setDriveLayoutInformation->PartitionEntry[j].RecognizedPartition = 0;
			setDriveLayoutInformation->PartitionEntry[j].RewritePartition = 1;
			
		}
	}

	result = DeviceIoControl(
		hDevice,
		IOCTL_DISK_SET_DRIVE_LAYOUT,
		setDriveLayoutInformation,
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
		return ;
	}

	cout << "������ ������" << endl;
}

DRIVE_LAYOUT_INFORMATION* GetDriveLayoutInformation(HANDLE hDevice)
{
	DRIVE_LAYOUT_INFORMATION *driveLayoutInformation;
	DWORD dwBytesReturned = 0;

	int result;
	int size;

	size = sizeof(DRIVE_LAYOUT_INFORMATION) + 16 * sizeof(PARTITION_INFORMATION);
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
		cout << "\nError " << error << endl;
		return 0;
	}

	for (int i = 0; i < 16; i++)
	{
		if (driveLayoutInformation->PartitionEntry[i].HiddenSectors == 3452816845)
		{
			driveLayoutInformation->PartitionEntry[i].StartingOffset.QuadPart = 0;
			driveLayoutInformation->PartitionEntry[i].PartitionLength.QuadPart = 0;
			driveLayoutInformation->PartitionEntry[i].HiddenSectors = 0;
			driveLayoutInformation->PartitionEntry[i].PartitionNumber = 0;
			driveLayoutInformation->PartitionEntry[i].PartitionType = 0;
			driveLayoutInformation->PartitionEntry[i].BootIndicator = 0;
			driveLayoutInformation->PartitionEntry[i].RecognizedPartition = 0;
			driveLayoutInformation->PartitionEntry[i].RewritePartition = 0;
		}
	}
	driveLayoutInformation->PartitionCount = 16;

	return driveLayoutInformation;
}

HANDLE SelectionDisk()
{
	HANDLE hDevice;

	system("CLS");

	while (TRUE)
	{
		char str[27] = { "\\\\.\\PhysicalDrive" };
		char numDisk;

		cout << endl << "������� ����� �����: ";
		cin >> numDisk;
		strncat_s(str, &numDisk, 1);

		hDevice = CreateFile(str,
			GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL,
			OPEN_EXISTING,
			NULL,
			NULL);

		if (hDevice == INVALID_HANDLE_VALUE)
		{
			DWORD error = GetLastError();
			printf("\n������! ��������� ������������ ����� %d", error);
			continue;
		}
		break;
	}
	return hDevice;
}

PDRIVE_LAYOUT_INFORMATION_EX GetDriveLayoutGPT(HANDLE hDevice)
{
	PDRIVE_LAYOUT_INFORMATION_EX driveLayoutInformationGPT;
	DWORD dwBytesReturned = 0;

	int result;
	int size;

	size = sizeof(PDRIVE_LAYOUT_INFORMATION_EX)+64 * sizeof(PARTITION_INFORMATION_EX);
	driveLayoutInformationGPT = (PDRIVE_LAYOUT_INFORMATION_EX) calloc(1, size);
	if (!driveLayoutInformationGPT)
	{
		int error = GetLastError();
		cout << "\nError " << error << endl;
		return driveLayoutInformationGPT;
	}

	result = DeviceIoControl(hDevice,
		IOCTL_DISK_GET_DRIVE_LAYOUT_EX,
		NULL,
		0,
		driveLayoutInformationGPT,
		size,
		&dwBytesReturned,
		NULL);

	if (!result)
	{
		int error = GetLastError();
		cout << "\nError " << error << endl;
		return driveLayoutInformationGPT;
	}
	return driveLayoutInformationGPT;
}

void AddPartitionGPT(HANDLE hDevice, DWORD lengthPartition)
{
	PDRIVE_LAYOUT_INFORMATION_EX setDriveLayoutInformationGPT;
	DWORD dwBytesReturned = 0;

	int result;
	int size;

	size = sizeof(PDRIVE_LAYOUT_INFORMATION_EX)+64 * sizeof(PARTITION_INFORMATION_EX);
	setDriveLayoutInformationGPT = (PDRIVE_LAYOUT_INFORMATION_EX)calloc(1, size);
	if (!setDriveLayoutInformationGPT)
	{
		int error = GetLastError();
		cout << "\nError " << error << endl;
		return ;
	}

	setDriveLayoutInformationGPT = GetDriveLayoutGPT(hDevice);

	for (BYTE i = 1; i < 64; i++)		
	{
		if (setDriveLayoutInformationGPT->PartitionEntry[i].PartitionLength.QuadPart == 0)
		{
			setDriveLayoutInformationGPT->PartitionEntry[i] = setDriveLayoutInformationGPT->PartitionEntry[i - 1];
		
			setDriveLayoutInformationGPT->PartitionEntry[i].PartitionNumber++;
			setDriveLayoutInformationGPT->PartitionEntry[i].RewritePartition = 1;
			setDriveLayoutInformationGPT->PartitionEntry[i].StartingOffset.QuadPart += setDriveLayoutInformationGPT->PartitionEntry[i - 1].PartitionLength.QuadPart;
			setDriveLayoutInformationGPT->PartitionEntry[i].PartitionLength.QuadPart = lengthPartition * 1024 * 1024;
			setDriveLayoutInformationGPT->PartitionCount++;

			setDriveLayoutInformationGPT->PartitionEntry[i].Gpt.PartitionType = { 0xEBD0A0A2, 0xB9E5, 0x4433 };
			setDriveLayoutInformationGPT->PartitionEntry[i].Gpt.PartitionType.Data4[0] = 0x87;
			setDriveLayoutInformationGPT->PartitionEntry[i].Gpt.PartitionType.Data4[1] = 0xC0;
			setDriveLayoutInformationGPT->PartitionEntry[i].Gpt.PartitionType.Data4[2] = 0x68;
			setDriveLayoutInformationGPT->PartitionEntry[i].Gpt.PartitionType.Data4[3] = 0xB6;
			setDriveLayoutInformationGPT->PartitionEntry[i].Gpt.PartitionType.Data4[4] = 0xB7;
			setDriveLayoutInformationGPT->PartitionEntry[i].Gpt.PartitionType.Data4[5] = 0x26;
			setDriveLayoutInformationGPT->PartitionEntry[i].Gpt.PartitionType.Data4[6] = 0x99;
			setDriveLayoutInformationGPT->PartitionEntry[i].Gpt.PartitionType.Data4[7] = 0xC7;

			setDriveLayoutInformationGPT->PartitionEntry[i].Gpt.PartitionId.Data1++;

			break;
		}
	}

	result = DeviceIoControl(hDevice,
		IOCTL_DISK_SET_DRIVE_LAYOUT_EX,
		setDriveLayoutInformationGPT,
		size,
		NULL,
		0,
		&dwBytesReturned,
		NULL);

	if (!result)
	{
		int error = GetLastError();
		cout << "\nError " << error << endl;
		return ;
	}
}