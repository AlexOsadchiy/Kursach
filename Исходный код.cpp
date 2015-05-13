#include <stdio.h>
#include <Windows.h>
#include <iostream>
#include <conio.h>
#include <locale.h>

using namespace std;

void GetDiskGeometry(HANDLE);
void GrowPartition(HANDLE, int);


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

	
	DRIVE_LAYOUT_INFORMATION *lpdlDriveLayoutInfo = (DRIVE_LAYOUT_INFORMATION *)malloc(1024 * 1024);
	if (!lpdlDriveLayoutInfo)
	{
		int error = GetLastError();
		printf("\nError %d", error);
	} 
	DWORD dwRead;

	int result;
	result = DeviceIoControl(hFile,
		IOCTL_DISK_GET_DRIVE_LAYOUT,
		NULL,
		0,
		lpdlDriveLayoutInfo,
		1024 * 1024,
		&dwRead,
		NULL);
	if (!result)
	{
		int error = GetLastError();
		printf("\nError %d", error);
	}

	DISK_GROW_PARTITION dgp;
	DWORD dwBytesReturned = 0;
	
	setlocale(LC_ALL, "Russian");

	int flEnd = 1;

	while (1)
	{
		cout << "1 - ����� ���������� � ��������" << endl;
		cout << "2 - ������� ������" << endl;
		cout << "3 - ������� ������" << endl;
		cout << "4 - ��������� ������" << endl;
		cout << "5 - ��������� ������" << endl;
		cout << "0 - �����" << endl;
		switch (getch())
		{
		case '1':
			cout << "������ 1:\nC������� %d" << lpdlDriveLayoutInfo->PartitionEntry[0].PartitionLength.QuadPart << endl;
			cout << "������ 2:\nC������� %d" << lpdlDriveLayoutInfo->PartitionEntry[1].PartitionLength.QuadPart << endl;
			cout << "������ 3:\nC������� %d" << lpdlDriveLayoutInfo->PartitionEntry[2].PartitionLength.QuadPart << endl;
			cout << "������ 4:\nC������� %d" << lpdlDriveLayoutInfo->PartitionEntry[3].PartitionLength.QuadPart << endl;
			GetDiskGeometry(hFile);
			break;
		case '2':
			
			break;
		case '3':
			cout << "������� ����� �������: " << endl;
			while (flEnd)
			{
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
					cout << "������ ������� ������" << endl;
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
					cout << "������ ������� ������" << endl;
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
					cout << "������ ������� ������" << endl;
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
					cout << "������ ������� ������" << endl;
					flEnd = 0;
					break;
				default:
					cout << "�����������!!!�������� ��� ���" << endl;
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

	cout << "���������� ���������:  " << diskGeometry.Geometry.Cylinders.QuadPart << endl;
	cout << "������� � �������:     " << diskGeometry.Geometry.TracksPerCylinder << endl;
	cout << "�������� �� �������:   " << diskGeometry.Geometry.SectorsPerTrack << endl;
	cout << "���� � �������:        " << diskGeometry.Geometry.BytesPerSector << endl;
	int col = diskGeometry.Geometry.Cylinders.QuadPart*diskGeometry.Geometry.TracksPerCylinder*diskGeometry.Geometry.SectorsPerTrack;
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
	scanf("%d", &numPart);
	cout << endl;
	growPartition.PartitionNumber = numPart;

	cout << "��������� ������ �� (��������): ";
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

