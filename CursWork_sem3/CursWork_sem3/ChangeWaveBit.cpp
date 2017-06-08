#include <tchar.h>
#include <conio.h>
#include <Windows.h>
#include <iostream>
#include <string>
using namespace std;
// Strukture WAVE file
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
bool UP =  false;// true - Increase , false - Decrease bit 
WAVEHEADER header, newheader;


void PrintInfo(WAVEHEADER &head){
	// Print info from fileheader
	cout << endl << " *** Информация о файле ***" << endl;
	cout << "Формат: " << head.wFormatTag << endl;
	cout << "Бит на отсчет: " << head.BitsPerSample << endl;
	cout << "Кол-во сэмплов: " << head.subchunk2Size * 8 / head.BitsPerSample  << endl;
	cout << "Кол-во каналов: " << head.nChannels << endl;
	cout << "Размер блока данных: " << head.subchunk2Size << endl;
	cout << "Частота дискритизации: " << head.SamplesPerSec <<" Гц"<< endl;
	cout << "Размер сэмпла в байтах (все каналы): " << head.blockAlign << endl;
	cout << " **************************" << endl;
}
int main()
{


	setlocale(LC_ALL, "rus");
	FILE *file;
	errno_t err;
	string filename;
	cout << "Введите имя wav файла без расширения  в текущем каталоге: ";
	cin >> filename;
	filename.append(".wav");// Add the file extension
	//Trying to open the file
	err = fopen_s(&file, filename.c_str(), "rb");
	if (err)
	{
		printf_s("Ошибка %d при открытии файла %c %d",filename, err);
		system("pause");
		//delete filename;
		return 0;
	}
	// Read header
	fread_s(&header, sizeof(WAVEHEADER), sizeof(WAVEHEADER), 1, file);

	// If format WAV file not PCM, then print errormassage and exit
	if (header.wFormatTag != 1){
		PrintInfo(header);
		cout <<endl<< "Формат файла не 1 PCM. Ничего не сможем сделать." << endl << endl;
		fclose(file);
		system("pause");
		return 0;
	}
	cout << "Что будем делать?" << endl;
	cout << "0-Вывести информацию о файле" << endl;
	cout << "1-Увеличить кол-во бит на отсчет в 2 раза" << endl;
	cout << "2-Уменьшить кол-во бит на отсчет в 2 раза" << endl;
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
	cout << "Исходный файл " << filename << endl;
	PrintInfo(header);
	
		// Read the rest of the array
	BYTE* arr = new BYTE[header.chunkSize];
	fread_s(arr, header.chunkSize, header.chunkSize, 1, file);
	fclose(file);
	
	// ************* We recalculate the title ****************
	// The number of bits per count is limited to 8-32
	// Number of samples
	const int SampleCount = (header.subchunk2Size * 8 / header.BitsPerSample) / header.nChannels;
	newheader = header;

	// If UP true, the bit is doubled by counting, if false is reduced by 2 times
	newheader.BitsPerSample = UP ? (header.BitsPerSample * 2) : (header.BitsPerSample / 2);
	if (newheader.BitsPerSample < 8){
		newheader.BitsPerSample = 8;
		cout << "MIN BitPerSample is 8" << endl;
	}
	if (newheader.BitsPerSample > 32){
		newheader.BitsPerSample = 32;
		cout << "Max BitPerSample is 32" << endl;
	}
	// ByteAlign can take the values 1,2,4 *****
	// Calculate the required number of bytes
	newheader.blockAlign = (newheader.BitsPerSample / 8)*header.nChannels;

	// If the number of bits does not fit into the variable, then increase the number of bytes by 1
	if (newheader.blockAlign<(newheader.BitsPerSample / 8))  newheader.blockAlign++;

	// Check the value of ByteAlign should be 1,2 or 4 or 8
	if ((newheader.blockAlign != 1) && (newheader.blockAlign != 2) &&
		(newheader.blockAlign != 4) && (newheader.blockAlign != 8))
		cout << "Ошибка ByteAlign : " << newheader.blockAlign << endl;

	newheader.AvgBytesPerSec = newheader.SamplesPerSec*newheader.blockAlign;
	newheader.subchunk2Size = newheader.blockAlign*SampleCount;// !!!!!!!!
	newheader.chunkSize = newheader.subchunk2Size + sizeof(header) - 8;
	// *********** End of header recalculation **************

	// Create a new file name. add (converted) before the file extension
	string newfilename = filename;
	newfilename.resize(filename.size() - 4); 
	newfilename.append("(").append(std::to_string(newheader.BitsPerSample)).append("bit)").append(".wav");
	
	// Try to file create
	err = fopen_s(&file,newfilename.c_str(), "w+b");
	if (err)
	{
		printf_s("Невозможно открыть файл %с, %d",newfilename, err);
		system("pause");
		return 0;
	}
	
	cout << endl << "Пробуем конвертировать c " << header.BitsPerSample << " бит в "
		<< newheader.BitsPerSample << " бит." << endl;
	//write new header to file
	fwrite(&newheader,1, sizeof(newheader), file);

	int block = (newheader.blockAlign - header.blockAlign)/header.nChannels;
	if (block == 0){
		cout << "Блок данных не требует изменения" << endl;
		fwrite(arr, 1, header.chunkSize, file);
		fclose(file);
	}
	else{
		int newarrCount = 0;// Iterator of new array
		int arrCount = 0; // Iterator of source data array
		int needCopyByte = (abs(block) );;// Number of bytes required for copying
// Set the size of the new data block
		BYTE* newarr = new BYTE[newheader.subchunk2Size];

		
		for (unsigned int itSample = 0; itSample < (header.subchunk2Size / header.blockAlign); itSample++){
			for (int itChannel = 0; itChannel < header.nChannels; itChannel++){

				// If the new block size is larger, then add zero bytes
				if (block > 0){
					for (int i = 0; i < block; i++){
						newarr[newarrCount] = 0;
						newarrCount++;
					}
				}
				// If the new data size is less then skip the high-order bytes
				if (block < 0){
					for (int i = 0; i > block; i--){
						arrCount++;
					}
				}

				// ************** Copying to a new array **************
				// Walk through the bytes of one channel and copy them into a new array
				for (int itByte = 0; itByte < needCopyByte; itByte++){
					newarr[newarrCount] = arr[arrCount];
					
					// ************************************************ *
					// If the source info is more than 8 bits, then when changing to 8 bits or less
					// You need to add 128. In for example 16 bits
					// value goes with a sign (-32768 to +32767). But in 8 bits
					// without sign (from 0 to 255)
					if ((header.BitsPerSample>8) && (newheader.BitsPerSample <= 8)){
						newarr[newarrCount] = (BYTE)(newarr[newarrCount] + 128);
					}
					// If you convert from 8 bits to more, you need to take 128
					if ((header.BitsPerSample <= 8) && (newheader.BitsPerSample > 8)){
						newarr[newarrCount] = (BYTE)(newarr[newarrCount] - 128);
					}
					//*************************************************
					newarrCount++;
					arrCount++;
				}// End of itByte
			}// End of itChannel
		}// End of itSample

		fwrite(newarr,1, newheader.chunkSize, file);
		fclose(file);// Close file
		delete[]newarr;// memory free
	} 

	delete[]arr;// memory free
	cout << endl << "Файл " << filename << " конвертирован. Создан новый файл " << newfilename << endl;
	cout << " ***** Параметры нового файла *****" << endl;
	PrintInfo(newheader);
	
	system("pause");
	return 0;
}