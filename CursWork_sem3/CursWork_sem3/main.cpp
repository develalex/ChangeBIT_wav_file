#include <tchar.h>
#include <conio.h>
#include <Windows.h>
#include <iostream>
#include <string>
using namespace std;
// ���������, ����������� ��������� WAV �����.
struct WAVEHEADER
{
	char chunkId[4];
	unsigned long chunkSize;
	char format[4];
	char subchunk1Id[4];
	unsigned long subchunk1Size;
	unsigned short wFormatTag;
	unsigned short nChannels;
	unsigned long SamplesPerSec;
	unsigned long AvgBytesPerSec;
	unsigned short blockAlign;
	unsigned short BitsPerSample;
	char subchunk2Id[4];
	unsigned long subchunk2Size;
};
bool UP =  false;// true - ����������� ��� false - ��������� �������� 
WAVEHEADER header, newheader;


void PrintInfo(WAVEHEADER &head){
	// ����� ������� ���������� � ����� �� ���������
	cout << endl << " *** ���������� � ����� ***" << endl;
	cout << "������: " << head.wFormatTag << endl;
	cout << "��� �� ������: " << head.BitsPerSample << endl;
	cout << "���-�� �������: " << head.subchunk2Size * 8 / head.BitsPerSample  << endl;
	cout << "���-�� �������: " << head.nChannels << endl;
	cout << "������ ����� ������: " << head.subchunk2Size << endl;
	cout << "������� �������������: " << head.SamplesPerSec <<" ��"<< endl;
	cout << "������ ������ � ������ (��� ������): " << head.blockAlign << endl;
	cout << " **************************" << endl;
}
int main()
{


	setlocale(LC_ALL, "rus");
	FILE *file;
	errno_t err;
	string filename;
	cout << "������� ��� wav ����� ��� ����������  � ������� ��������: ";
	cin >> filename;
	filename.append(".wav");// ��������� ���������� �����
	//������� ������� ����
	err = fopen_s(&file, filename.c_str(), "rb");
	if (err)
	{
		printf_s("������ %d ��� �������� ����� %c %d",filename, err);
		system("pause");
		//delete filename;
		return 0;
	}
	// ������ ���������
	fread_s(&header, sizeof(WAVEHEADER), sizeof(WAVEHEADER), 1, file);

	// ���� ������ WAV ����� �� ���, �� ������ ��������� �� ������ � ������� �� �����
	if (header.wFormatTag != 1){
		PrintInfo(header);
		cout <<endl<< "������ ����� �� 1 PCM. ������ �� ������ �������." << endl << endl;
		fclose(file);
		system("pause");
		return 0;
	}
	cout << "��� ����� ������?" << endl;
	cout << "0-������� ���������� � �����" << endl;
	cout << "1-��������� ���-�� ��� �� ������ � 2 ����" << endl;
	cout << "2-��������� ���-�� ��� �� ������ � 2 ����" << endl;
	int res;
	do{
		cin >> res;
	} while (!((res >= 0) && (res <= 2)));
	if (res == 0){
		system("cls");
		PrintInfo(header);
		fclose(file);
		system("pause");
		return 0;
	}
	if (res == 1)UP = true;
	if (res == 2)UP = false;
	system("cls");
	cout << "�������� ���� " << filename << endl;
	PrintInfo(header);
	
		// ������ ���������� ����� � ������
	BYTE* arr = new BYTE[header.chunkSize];
	fread_s(arr, header.chunkSize, header.chunkSize, 1, file);
	fclose(file);
	
	// *************������������� ��������� ****************
	// ���������� ����� �� ������ ���������� ���������� 8-32
	// ���������� ������� 
	const int SampleCount = (header.subchunk2Size * 8 / header.BitsPerSample) / header.nChannels;
	newheader = header;

	// ���� UP true  ����������� ��� � ��� ���� �� ������, ���� false ��������� � 2 ����
	newheader.BitsPerSample = UP ? (header.BitsPerSample * 2) : (header.BitsPerSample / 2);
	if (newheader.BitsPerSample < 8){
		newheader.BitsPerSample = 8;
		cout << "MIN BitPerSample is 8" << endl;
	}
	if (newheader.BitsPerSample > 32){
		newheader.BitsPerSample = 32;
		cout << "Max BitPerSample is 32" << endl;
	}
	//ByteAlign ����� ��������� �������� 1,2,4  ***** 
	//����������� ������ ���-�� ����
	newheader.blockAlign = (newheader.BitsPerSample / 8)*header.nChannels;

	// ���� ���-�� ��� �� ������� � ���������� �� ����������� ���-�� ���� �� 1
	if (newheader.blockAlign<(newheader.BitsPerSample / 8))  newheader.blockAlign++;

	// ��������� �������� ByteAlign ������ ���� 1,2 ��� 4 ��� 8
	if ((newheader.blockAlign != 1) && (newheader.blockAlign != 2) &&
		(newheader.blockAlign != 4) && (newheader.blockAlign != 8))
		cout << "������ ByteAlign : " << newheader.blockAlign << endl;

	newheader.AvgBytesPerSec = newheader.SamplesPerSec*newheader.blockAlign;
	newheader.subchunk2Size = newheader.blockAlign*SampleCount;// !!!!!!!!
	newheader.chunkSize = newheader.subchunk2Size + sizeof(header) - 8;
	// ***********����� ��������� ���������**************

		// ������� ����� ��� �����. ��������� (converted) ����� ����������� �����
	string newfilename = filename;
	newfilename.resize(filename.size() - 4); // �������� ���������� �����
	newfilename.append("(").append(std::to_string(newheader.BitsPerSample)).append("bit)").append(".wav");
	
	// ������� ������� ����
	err = fopen_s(&file,newfilename.c_str(), "w+b");
	if (err)
	{
		printf_s("���������� ������� ���� %�, %d",newfilename, err);
		system("pause");
		return 0;
	}
	
	cout << endl << "������� �������������� c " << header.BitsPerSample << " ��� � "
		<< newheader.BitsPerSample << " ���." << endl;
	//���������� � ����� ���� ����� header
	fwrite(&newheader,1, sizeof(newheader), file);

	int block = (newheader.blockAlign - header.blockAlign)/header.nChannels;
	if (block == 0){
		cout << "���� ������ �� ������� ���������" << endl;
		fwrite(arr, 1, header.chunkSize, file);
		fclose(file);
	}
	else{
		int newarrCount = 0;// �������� ��� ������ ������� ������
		int arrCount = 0; // �������� ��������� ������� ������
		int needCopyByte = (abs(block) );;// ���-�� ���� ����������� ��� ����������� 
		// ������ ������ ������ ����� ������
		BYTE* newarr = new BYTE[newheader.subchunk2Size];

		// �������� �� ��������� ����� ������
		for (unsigned int itSample = 0; itSample < (header.subchunk2Size / header.blockAlign); itSample++){
			for (int itChannel = 0; itChannel < header.nChannels; itChannel++){

				// ���� ����� ������ ����� ������ ���������, �� ��������� ������� �����
				if (block > 0){
					for (int i = 0; i < block; i++){
						newarr[newarrCount] = 0;
						newarrCount++;
					}
				}
				// ���� ����� ������ ������ ������ �� ���������� ������� �����
				if (block < 0){
					for (int i = 0; i > block; i--){
						arrCount++;
					}
				}

				// **************  ����������� � ����� ������  **************
				// ������������� �� ������ ������ ������ � �������� �� � ����� ������
				for (int itByte = 0; itByte < needCopyByte; itByte++){
					newarr[newarrCount] = arr[arrCount];
					
					//*************************************************
					//���� �������� ���� ����� 8 ���, �� ��� �������� � 8 ����� � �����
					//���������� ��������� 128. �.�. � �������� � 16 ����� 
					//�������� ���� �� ������ (�� -32768 �� +32767). � � 8 �����
					// ��� ����� (�� 0 �� 255 )
					if ((header.BitsPerSample>8) && (newheader.BitsPerSample <= 8)){
						newarr[newarrCount] = (BYTE)(newarr[newarrCount] + 128);
					}
					//���� ������������ � 8 ��� �� �������, �� ���� ������ 128
					if ((header.BitsPerSample <= 8) && (newheader.BitsPerSample > 8)){
						newarr[newarrCount] = (BYTE)(newarr[newarrCount] - 128);
					}
					//*************************************************
					newarrCount++;
					arrCount++;
				}// ����� itByte
			}// ����� itChannel
		}// �����  itSample

		fwrite(newarr,1, newheader.chunkSize, file);
		fclose(file);// ��������� ����
		delete[]newarr;// ����������� ������
	} //����� if (block == 0) else

	delete[]arr;// ����������� ������
	cout << endl << "���� " << filename << " �������������. ������ ����� ���� " << newfilename << endl;
	cout << " ***** ��������� ������ ����� *****" << endl;
	PrintInfo(newheader);
	
	system("pause");
	return 0;
}