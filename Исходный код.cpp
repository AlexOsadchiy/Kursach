#include <stdio.h>
#include <Windows.h>
#include <iostream>
#include <conio.h>
#include <locale.h>

using namespace std;

void DeletPartition(HANDLE, byte);

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
		puts("1 - ����� ���������� � ��������");
		puts("2 - ������� ������");
		puts("3 - ������� ������");
		puts("4 - ��������� ������");
		puts("5 - ��������� ������");
		puts("0 - �����");
		switch (getch())
		{
		case '1':
			printf("\n������ 1:\nC������� %d", lpdlDriveLayoutInfo->PartitionEntry[0].PartitionLength);
			printf("\n������ 2:\nC������� %d", lpdlDriveLayoutInfo->PartitionEntry[1].PartitionLength);
			printf("\n������ 3:\nC������� %d", lpdlDriveLayoutInfo->PartitionEntry[2].PartitionLength);
			printf("\n������ 4:\nC������� %d", lpdlDriveLayoutInfo->PartitionEntry[3].PartitionLength);
			break;
		case '2':
			break;
		case '3':
			puts("������� ����� �������: ");
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
						printf("\nError %d", error);
					}
					puts("������ ������� ������");
					flEnd = 0;
					break;
				case '2':
					for (int i = 462; i < 478; i++)
						buf[i] = 0;
					SetFilePointer(hFile, 0, 0, FILE_BEGIN);
					if (!WriteFile(hFile, &buf, 512, &numberOfBytesRead, NULL))
					{
						int error = GetLastError();
						printf("\nError %d", error);
					}
					puts("������ ������� ������");
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
					puts("������ ������� ������");
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
					puts("������ ������� ������");
					flEnd = 0;
					break;
				default:
					puts("�����������!!!�������� ��� ���");
					break;
				}
			}
			break;
		case '4':
			int sizeHigh;
			puts("������� ����� �������: ");
			scanf("%d", &sizeHigh);
			dgp.PartitionNumber = sizeHigh;
			puts("��������� ������ �� (��������): ");
			scanf("%d", &sizeHigh );
			dgp.BytesToGrow.QuadPart = sizeHigh  * 1024 * 1024;

			result = DeviceIoControl(
				hFile,
				IOCTL_DISK_GROW_PARTITION,
				&dgp,
				sizeof dgp,
				NULL,
				0,
				&dwBytesReturned,
				NULL);
			if (!result)
			{
				int error = GetLastError();
				printf("\nError %d", error);
			}
			break;
		case '5':
			int sizeLow;
			puts("������� ����� �������: ");
			scanf("%d", &sizeLow);
			dgp.PartitionNumber = sizeLow;
			puts("��������� ������ �� (��������): ");
			scanf("%d", &sizeLow);
			dgp.BytesToGrow.QuadPart = -sizeLow * 1024 * 1024;

			result = DeviceIoControl(
				hFile,
				IOCTL_DISK_GROW_PARTITION,
				&dgp,
				sizeof dgp,
				NULL,
				0,
				&dwBytesReturned,
				NULL);
			if (!result)
			{
				int error = GetLastError();
				printf("\nError %d", error);
			}
			break;
		case '0': 
			return 0;
		default: 
			break;
		}
	}

	return 0;
}

void DeletPartition(HANDLE hFile, byte buf)
{
	DWORD numberOfBytesRead;

	if (!WriteFile(hFile, &buf, 512, &numberOfBytesRead, NULL))
	{
		int error = GetLastError();
		printf("\nError %d", error);
	}
}