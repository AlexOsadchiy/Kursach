#include <stdio.h>
#include <Windows.h>
#include <iostream>
#include <conio.h>
#include <locale.h>

using namespace std;

void DeletPartition(HANDLE, byte);

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
		puts("1 - Вывод информации о разделах");
		puts("2 - Создать раздел");
		puts("3 - Удалить раздел");
		puts("4 - Увеличить раздел");
		puts("5 - Уменьшить раздел");
		puts("0 - Выход");
		switch (getch())
		{
		case '1':
			printf("\nРаздел 1:\nCвободно %d", lpdlDriveLayoutInfo->PartitionEntry[0].PartitionLength);
			printf("\nРаздел 2:\nCвободно %d", lpdlDriveLayoutInfo->PartitionEntry[1].PartitionLength);
			printf("\nРаздел 3:\nCвободно %d", lpdlDriveLayoutInfo->PartitionEntry[2].PartitionLength);
			printf("\nРаздел 4:\nCвободно %d", lpdlDriveLayoutInfo->PartitionEntry[3].PartitionLength);
			break;
		case '2':
			break;
		case '3':
			puts("Введите номер раздела: ");
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
					puts("Раздел успешно удален");
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
					puts("Раздел успешно удален");
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
					puts("Раздел успешно удален");
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
					puts("Раздел успешно удален");
					flEnd = 0;
					break;
				default:
					puts("Неправильно!!!Попробуй еще раз");
					break;
				}
			}
			break;
		case '4':
			int sizeHigh;
			puts("Введите номер раздела: ");
			scanf("%d", &sizeHigh);
			dgp.PartitionNumber = sizeHigh;
			puts("Увеличить раздел на (Мегабайт): ");
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
			puts("Введите номер раздела: ");
			scanf("%d", &sizeLow);
			dgp.PartitionNumber = sizeLow;
			puts("Уменьшить раздел на (Мегабайт): ");
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