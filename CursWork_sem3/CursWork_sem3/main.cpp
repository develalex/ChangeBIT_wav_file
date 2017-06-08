#include <tchar.h>
#include <conio.h>
#include <Windows.h>
#include <iostream>
#include <string>
using namespace std;
// Структура, описывающая заголовок WAV файла.
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
bool UP =  false;// true - Увеличиваем или false - уменьшаем битность 
WAVEHEADER header, newheader;


void PrintInfo(WAVEHEADER &head){
	// Метод выводит информацию о файле из заголовка
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
	filename.append(".wav");// Добавляем расширение файла
	//Пробуем открыть файл
	err = fopen_s(&file, filename.c_str(), "rb");
	if (err)
	{
		printf_s("Ошибка %d при открытии файла %c %d",filename, err);
		system("pause");
		//delete filename;
		return 0;
	}
	// Читаем заголовок
	fread_s(&header, sizeof(WAVEHEADER), sizeof(WAVEHEADER), 1, file);

	// Если формат WAV файла не РСМ, то выдаем сообщение об ошибке и выходим из проги
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
	
		// Читаем оставшуюся часть в массив
	BYTE* arr = new BYTE[header.chunkSize];
	fread_s(arr, header.chunkSize, header.chunkSize, 1, file);
	fclose(file);
	
	// *************Пересчитываем заголовок ****************
	// Количество битов на отсчет ограничено значениями 8-32
	// Количество сэмплов 
	const int SampleCount = (header.subchunk2Size * 8 / header.BitsPerSample) / header.nChannels;
	newheader = header;

	// Если UP true  увеличиваем бит в два раза на отсчет, если false уменьшаем в 2 раза
	newheader.BitsPerSample = UP ? (header.BitsPerSample * 2) : (header.BitsPerSample / 2);
	if (newheader.BitsPerSample < 8){
		newheader.BitsPerSample = 8;
		cout << "MIN BitPerSample is 8" << endl;
	}
	if (newheader.BitsPerSample > 32){
		newheader.BitsPerSample = 32;
		cout << "Max BitPerSample is 32" << endl;
	}
	//ByteAlign может принимать значения 1,2,4  ***** 
	//Высчитываем нужное кол-во байт
	newheader.blockAlign = (newheader.BitsPerSample / 8)*header.nChannels;

	// Если кол-во бит не влезает в переменную то увеличиваем кол-во байт на 1
	if (newheader.blockAlign<(newheader.BitsPerSample / 8))  newheader.blockAlign++;

	// Проверяем значение ByteAlign должно быть 1,2 или 4 или 8
	if ((newheader.blockAlign != 1) && (newheader.blockAlign != 2) &&
		(newheader.blockAlign != 4) && (newheader.blockAlign != 8))
		cout << "Ошибка ByteAlign : " << newheader.blockAlign << endl;

	newheader.AvgBytesPerSec = newheader.SamplesPerSec*newheader.blockAlign;
	newheader.subchunk2Size = newheader.blockAlign*SampleCount;// !!!!!!!!
	newheader.chunkSize = newheader.subchunk2Size + sizeof(header) - 8;
	// ***********Конец пересчета заголовка**************

		// Создаем новое имя файла. добавляем (converted) перед расширением файла
	string newfilename = filename;
	newfilename.resize(filename.size() - 4); // Отрубаем расширение файла
	newfilename.append("(").append(std::to_string(newheader.BitsPerSample)).append("bit)").append(".wav");
	
	// Пробуем создать файл
	err = fopen_s(&file,newfilename.c_str(), "w+b");
	if (err)
	{
		printf_s("Невозможно открыть файл %с, %d",newfilename, err);
		system("pause");
		return 0;
	}
	
	cout << endl << "Пробуем конвертировать c " << header.BitsPerSample << " бит в "
		<< newheader.BitsPerSample << " бит." << endl;
	//записываем в новый файл новый header
	fwrite(&newheader,1, sizeof(newheader), file);

	int block = (newheader.blockAlign - header.blockAlign)/header.nChannels;
	if (block == 0){
		cout << "Блок данных не требует изменения" << endl;
		fwrite(arr, 1, header.chunkSize, file);
		fclose(file);
	}
	else{
		int newarrCount = 0;// Итератор для нового массива данных
		int arrCount = 0; // Итератор исходного массива данных
		int needCopyByte = (abs(block) );;// Кол-во байт необходимых для копирования 
		// Задаем размер нового блока данных
		BYTE* newarr = new BYTE[newheader.subchunk2Size];

		// Проходим по исходному блоку данных
		for (unsigned int itSample = 0; itSample < (header.subchunk2Size / header.blockAlign); itSample++){
			for (int itChannel = 0; itChannel < header.nChannels; itChannel++){

				// Если новый размер блока больше исходного, то добавляем нулевые байты
				if (block > 0){
					for (int i = 0; i < block; i++){
						newarr[newarrCount] = 0;
						newarrCount++;
					}
				}
				// Если новый размер данных меньше то пропускаем старшие байты
				if (block < 0){
					for (int i = 0; i > block; i--){
						arrCount++;
					}
				}

				// **************  Копирование в новый массив  **************
				// Прогуливаемся по байтам одного канала и копируем их в новый массив
				for (int itByte = 0; itByte < needCopyByte; itByte++){
					newarr[newarrCount] = arr[arrCount];
					
					//*************************************************
					//Если исходная инфа более 8 бит, то при переходе к 8 битам и менее
					//Необходимо прибавить 128. Т.к. в например в 16 битах 
					//значения идет со знаком (от -32768 до +32767). а в 8 битах
					// без знака (от 0 до 255 )
					if ((header.BitsPerSample>8) && (newheader.BitsPerSample <= 8)){
						newarr[newarrCount] = (BYTE)(newarr[newarrCount] + 128);
					}
					//Если конвертируем с 8 бит на большее, то надо отнять 128
					if ((header.BitsPerSample <= 8) && (newheader.BitsPerSample > 8)){
						newarr[newarrCount] = (BYTE)(newarr[newarrCount] - 128);
					}
					//*************************************************
					newarrCount++;
					arrCount++;
				}// Конец itByte
			}// Конец itChannel
		}// Конец  itSample

		fwrite(newarr,1, newheader.chunkSize, file);
		fclose(file);// Закрываем файл
		delete[]newarr;// Освобождаем память
	} //Конец if (block == 0) else

	delete[]arr;// Освобождаем память
	cout << endl << "Файл " << filename << " конвертирован. Создан новый файл " << newfilename << endl;
	cout << " ***** Параметры нового файла *****" << endl;
	PrintInfo(newheader);
	
	system("pause");
	return 0;
}