#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <string.h>

// Размер заголовка BMP-файла
#define HEADER_SIZE 49

// Функция для получения размера файла
int getFileSize(const char* fileName) {
    int fileSize = 0;
    FILE* fileDescriptor = fopen(fileName, "rb");

    // Проверяем, удалось ли открыть файл
    if (fileDescriptor == NULL) {
        fileSize = -1; // Если файл не удалось открыть, возвращаем -1
    }
    else {
        fseek(fileDescriptor, 0, SEEK_END); // Переходим в конец файла
        fileSize = ftell(fileDescriptor); // Получаем размер файла
        fclose(fileDescriptor); // Закрываем файл
    }

    return fileSize;
}

// Функция для встраивания заголовка с степенью и размером в контейнер
void encodeHeaderDegreeAndSize(FILE* startBmp, FILE* encodeBmp, int textLength, unsigned short int degree) {
    for (unsigned short int j = 0; j < HEADER_SIZE; j++) {
        char tempSymbol = 0;
        fread(&tempSymbol, 1, 1, startBmp);
        fwrite(&tempSymbol, 1, 1, encodeBmp);
    }

    fwrite(&degree, 1, 1, encodeBmp); // Записываем степень в контейнер

    int size = 0;
    for (short int i = 3; i >= 0; i--) {
        size = textLength;
        size >>= (8 * i);
        size &= 255;
        fwrite(&size, 1, 1, encodeBmp); // Записываем размер текста в контейнер
    }
    fseek(startBmp, 5, SEEK_CUR); // Пропускаем несколько байт в исходном BMP-файле
}

// Функция для выбора степени кодирования
unsigned short int chooseDegree() {
    unsigned short int degree;
    printf("Choose the value of encoding depth 1-2-4-8\n");
    char temp = scanf("%hu", &degree);
    while ((temp = getchar()) != '\n'); // Очищаем буфер ввода

    if (degree != 1 && degree != 2 && degree != 4 && degree != 8) {
        printf("Choose valid degree");
        return chooseDegree(); // Рекурсивный вызов, если выбрана недопустимая степень
    }

    return degree;
}

// Функция для получения количества символов в файле
int getSymbolAmount(FILE* encodeBmp) {
    int amount = 0, size = 0;
    unsigned char symbol = 0;

    for (int i = 3; i >= 0; i--) {
        fread(&symbol, 1, 1, encodeBmp);
        size = (unsigned int)symbol;
        size <<= (8 * i);
        amount |= size;
    }

    return amount;
}

// Функция для выполнения кодирования
void performEncoding() {
    unsigned short int degree = chooseDegree();

    char fileName[100];
    fileName[99] = 0;
    printf("Enter file message name with extension. Example: text.txt ");
    int temp = scanf("%s", fileName);
    if (temp == 0) {
        printf("Error occured while searching\n");
        return;
    }

    FILE* textFile = fopen(fileName, "rb");
    if (textFile == NULL) {
        printf("Error occured while opening file\n");
        return;
    }

    FILE* startBmp = fopen("picture.bmp", "rb");
    FILE* encodeBmp = fopen("encoded.bmp", "wb");

    int textLength = getFileSize(fileName), imgLength = getFileSize("picture.bmp");
    char textMask = 255 >> (8 - degree), imgMask = 255 << degree;
    char textSymbol, imgSymbol, sym;

    if (textLength >= (imgLength * degree / 8) - 54) {
        printf("Message is too long.\n");
        fclose(textFile);
        fclose(startBmp);
        fclose(encodeBmp);
        return;
    }

    encodeHeaderDegreeAndSize(startBmp, encodeBmp, textLength, degree);

    while (!feof(textFile)) {
        fread(&textSymbol, 1, 1, textFile);
        for (short int temp = 8 / degree; temp > 0; temp--) {
            fread(&imgSymbol, 1, 1, startBmp);
            sym = textSymbol >> degree * (temp - 1);
            sym &= textMask;
            imgSymbol &= imgMask;
            imgSymbol |= sym;
            fwrite(&imgSymbol, 1, 1, encodeBmp);
        }
    }

    while (!feof(startBmp)) {
        char tempSymbol;
        fread(&tempSymbol, 1, 1, startBmp);
        fwrite(&tempSymbol, 1, 1, encodeBmp);
    }

    fclose(textFile);
    fclose(startBmp);
    fclose(encodeBmp);
}

// Функция для выполнения декодирования
void performDecoding() {
    FILE* encodeBmp = fopen("encoded.bmp", "rb");
    FILE* textFile = fopen("decoded.txt", "wb");
    fseek(encodeBmp, HEADER_SIZE, SEEK_SET);
    int degree = fgetc(encodeBmp);
    unsigned int amount = getSymbolAmount(encodeBmp);

    char symbol, imgSymbol = 0;
    char imgMask = ~(255 << degree);
    for (unsigned int counter = 0; counter < amount; counter++) {
        symbol = 0;
        for (int i = 8 / degree; i > 0; i--) {
            if (!feof(encodeBmp)) {
                fread(&imgSymbol, 1, 1, encodeBmp);
                imgSymbol &= imgMask;
                imgSymbol <<= degree * (i - 1);
                symbol |= imgSymbol;
            }
        }
        fwrite(&symbol, 1, 1, textFile);
    }

    fclose(encodeBmp);
    fclose(textFile);
    printf("Decoding has been done. Search decoded.txt\n");
}

int main() {
    while (1) {
        printf("Choose the option:\n1. encode your image [picture.bmp]\n2. decode your image\n3. quit\n");
        unsigned short int choice = 0;

        int tmp = scanf("%hu", &choice);
        if (tmp == 0) {
            printf("Error occured while scanning the choice\n");
            return 1; // Возвращаем 1, чтобы указать на ошибку
        }

        while ((tmp = getchar()) != '\n');

        if (choice == 1) {
            performEncoding();
        }
        else if (choice == 2) {
            performDecoding();
        }
        else if (choice == 3) {
            return 0; // Выход из программы
        }
        else {
            printf("Unknown option\n");
        }
    }

    return 0;
}